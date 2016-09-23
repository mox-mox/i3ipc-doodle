#include "doodle.hpp"
#include <fstream>
#include <functional>
#include <json/json.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <map>
#include <memory>






//{{{
Doodle::Doodle(void) :
	i3_conn(),
	current_workspace(""),
	nojob(),
	current_job(&nojob),
	loop(),
	idle(true),
	xcb_conn(xcb_connect(NULL, NULL)),
	screen(xcb_setup_roots_iterator(xcb_get_setup(xcb_conn)).data),
	idle_watcher_timer(loop),
	socket_watcher(loop, this, settings.socket_path),
	terminal(this)
{
	//{{{ Create the individual jobs

	for( auto&f: fs::directory_iterator(settings.config_dir/"jobs"))
	{
		std::cout<<"f.path(): "<<f.path()<<", extension: "<<f.path().extension()<<std::endl;
		if(f.path().extension() == ".job")
		{
			try
			{
				std::cout<<"try to push job "<<f.path().stem()<<std::endl;
				jobs.push_back({ f.path(), loop });
			}
			catch(std::runtime_error&e)
			{
				error<<"Caught exception \""<<e.what()<<"\" while constructing job "<<f.path().filename()<<". ... removing that job from the job list."<<std::endl;
			}
		}
	}

	nojob.start(std::chrono::steady_clock::now());										// Account for time spent on untracked jobs
	//}}}

	//{{{ Idle time detection

	if( settings.max_idle_time )
	{
		idle_watcher_timer.set<Doodle, &Doodle::idle_time_watcher_cb>(this);
		idle_watcher_timer.set(settings.max_idle_time, settings.max_idle_time);
		idle_watcher_timer.start();
	}
	else
	{
		idle = false;
	}
	//}}}

	//{{{ i3 event subscriptions

	simulate_workspace_change(i3_conn.get_workspaces());	// Inject a fake workspace change event to start tracking the first workspace.
	//simulate_window_change(i3_conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
	//
	i3_conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	i3_conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));

	if( !i3_conn.subscribe(i3ipc::ET_WORKSPACE|i3ipc::ET_WINDOW))
	{
		error<<"could not connect"<<std::endl;
		throw "Could not subscribe to the workspace- and window change events.";
	}
	//}}}
}
//}}}

//{{{
Doodle::~Doodle(void)
{
	xcb_disconnect(xcb_conn);
}
//}}}

//{{{
inline Doodle::win_id_lookup_entry Doodle::find_job(const std::string& window_name)
{
	win_id_lookup_entry retval {
		&nojob, ""
	};

	for( Job& j : jobs )					// Search all the jobs to see, if one matches the newly focused window
	{
		if((retval.matching_name = j.match(current_workspace, window_name)) != "" )
		{
			if( !settings.detect_ambiguity )// For normal operation, just report the first match.
			{
				retval.job = &j;
				return retval;
			}
			else							// To detect ambiguity, continue searching to see if there are other matches
			{
				if( retval.job != &nojob )
				{
					error<<"Ambiguity: Window name \""<<window_name<<"\" matched "<<retval.job->get_jobname()<<" and "<<j.get_jobname()<<"."<<std::endl;
					// TODO: Show an errow window that asks to which job the window belongs to.
				}
				else
				{
					retval.job = &j;
				}
			}
		}
	}
	return retval;
}
//}}}

//{{{
void Doodle::on_window_change(const i3ipc::window_event_t& evt)
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	//{{{ Print some information about the event

	//#ifdef DEBUG
	//	logger<<"on_window_change() called "<<++win_evt_count<<"th time. Type: "<<static_cast < char > (evt.type)<<std::endl;
	//	if( evt.container != nullptr )
	//	{
	//		//logger<<"	id = "<<evt.container->id<<std::endl;
	//		logger<<"	name = "<<evt.container->name<<std::endl;
	//		//logger<<"	type = "<<evt.container->type<<std::endl;
	//		//logger<<"	urgent = "<<evt.container->urgent<<std::endl;
	//		//logger<<"	focused = "<<evt.container->focused<<std::endl;
	//	}
	//#endif
	//}}}
	if(((evt.type == i3ipc::WindowEventType::FOCUS) || (evt.type == i3ipc::WindowEventType::TITLE)) && (evt.container != nullptr))
	{
		Job* old_job = current_job;
		win_id_lookup_entry& entry = win_id_lookup[evt.container->id];

		if( !entry.job || (entry.matching_name == "") || !std::regex_search(evt.container->name, std::regex(entry.matching_name)))
		{
			entry = find_job(evt.container->name);
		}
		current_job = entry.job;
		if( old_job != current_job )
		{
			if( old_job )
			{
				old_job->stop(now);
			}
			if( current_job )
			{
				// A window change may imply user activity, so if the user is considered idle, update that status
				if( idle ) idle_time_watcher_cb(idle_watcher_timer, 2);

				if( !idle )
				{
					current_job->start(now);
				}
			}
		}
		logger<<"New current_job: "<<current_job->get_jobname()<<std::endl;
	}
	else if( evt.type == i3ipc::WindowEventType::CLOSE )
	{
		win_id_lookup.erase(evt.container->id);
		simulate_window_change(i3_conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
	}
}
//}}}

//{{{
void Doodle::on_workspace_change(const i3ipc::workspace_event_t& evt)
{
	if( evt.type == i3ipc::WorkspaceEventType::FOCUS )
	{
		logger<<"New current_workspace: "<<evt.current->name<<std::endl;
		current_workspace = evt.current->name;
		simulate_window_change(i3_conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
	}
#ifdef DEBUG
	else
	{
		//logger<<"Ignoring Workspace event"<<std::endl;
	}
#endif
}
//}}}

//{{{
std::ostream& operator<<(std::ostream&stream, Doodle const&doodle)
{
	stream<<"Doodle class:"<<std::endl;
	stream<<"	Current job: "<<doodle.current_job->get_jobname()<<std::endl;
	stream<<"	Current workspace: "<<doodle.current_workspace<<std::endl;
	stream<<"	Jobs:"<<std::endl;
	for( const Job& job : doodle.jobs )
	{
		stream<<"		"<<job<<std::endl;
	}
	stream<<"	Known windows:"<<std::endl<<"		win_id		jobname		matching_name"<<std::endl;
	for( auto it : doodle.win_id_lookup )
	{
		stream<<"		"<<it.first<<"	"<<it.second.job<<"	"<<it.second.matching_name<<std::endl;
	}
	return stream;
}
//}}}

//{{{
bool Doodle::simulate_workspace_change(std::vector < std::shared_ptr < i3ipc::workspace_t>>workspaces)
{	// Iterate through all workspaces and call on_workspace_change() for the focussed one.
	for( std::shared_ptr < i3ipc::workspace_t > &workspace : workspaces )
	{
		if( workspace->focused )
		{
			on_workspace_change({ i3ipc::WorkspaceEventType::FOCUS,  workspace, nullptr });
			return true;
		}
	}
	error<<"No workspace is focused."<<std::endl;
	return false;	// Should never be reached
}
//}}}

//{{{
bool Doodle::simulate_window_change(std::list < std::shared_ptr < i3ipc::container_t>>nodes)
{	// Iterate through all containers and call on_window_change() for the focussed one.
	for( std::shared_ptr < i3ipc::container_t > &container : nodes )
	{
		if( container->focused )
		{
			on_window_change({ i3ipc::WindowEventType::FOCUS,  container });
			return true;
		}
		else
		{
			if( simulate_window_change(container->nodes)) return true;
		}
	}
	return false;	// Should never be reached
}
//}}}

//{{{Callbacks

//{{{
void Doodle::SIGUSR1_cb(void)
{
	debug<<"Received SIGUSR1!"<<std::endl;
	//debug<<*this<<std::endl;
	debug<<"Pending count: "<<ev_pending_count(loop)<<"."<<std::endl;
}
//}}}

//{{{
void Doodle::SIGTERM_cb(void)
{
	logger<<"Shutting down doodle"<<std::endl;
	loop.break_loop(ev::ALL);
}
//}}}

//{{{
void Doodle::idle_time_watcher_cb(ev::timer& timer, int revents)
{
	(void) revents;
	xcb_screensaver_query_info_cookie_t cookie = xcb_screensaver_query_info(xcb_conn, screen->root);
	xcb_screensaver_query_info_reply_t* info = xcb_screensaver_query_info_reply(xcb_conn, cookie, NULL);

	uint32_t idle_time = info->ms_since_user_input/1000;// use seconds

	debug<<"Checking idle time("<<revents<<"): "<<idle_time<<"."<<std::endl;
	free(info);

	uint32_t repeat_value = 1;
	if( !idle )
	{
		// Restart the watcher to trigger when idle_time might reach max_idle_time for the first time.
		// See solution 2 of http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Be_smart_about_timeouts
		uint32_t repeat_value = settings.max_idle_time-idle_time;
		// If the value was allowed to become zero (because of truncation), the watcher would never be started again
		repeat_value = repeat_value ? repeat_value : 1;
	}
	timer.repeat = repeat_value;
	timer.again();

	if((idle_time >= settings.max_idle_time) && !idle )
	{
		idle = true;
		logger<<"Going idle"<<std::endl;
		current_job->stop(std::chrono::steady_clock::now());
	}
	else if((idle_time < settings.max_idle_time) && idle )
	{
		idle = false;
		logger<<"Going busy again"<<std::endl;
		current_job->start(std::chrono::steady_clock::now());
	}
}
//}}}
//}}}

//{{{
int Doodle::operator()(void)
{
	int retval = 0;

	i3_conn.prepare_to_event_handling();

	logger<<"---------------Starting the event loop---------------"<<std::endl;

	//{{{ Watcher for i3 events

	ev::io i3_watcher;
	i3_watcher.set < i3ipc::connection, &i3ipc::connection::handle_event > (&i3_conn);
	i3_watcher.set(i3_conn.get_file_descriptor(), ev::READ);
	i3_watcher.start();
	//}}}

	//{{{ Watchers for POSIX signals

	//{{{ Watcher for the SIGUSR1 POSIX signal

	ev::sig SIGUSR1_watcher;
	SIGUSR1_watcher.set < Doodle, &Doodle::SIGUSR1_cb > (this);
	SIGUSR1_watcher.set(SIGUSR1);
	SIGUSR1_watcher.start();
	//}}}

	//{{{ Watcher for the SIGTERM POSIX signal

	ev::sig SIGTERM_watcher;
	SIGTERM_watcher.set < Doodle, &Doodle::SIGTERM_cb > (this);
	SIGTERM_watcher.set(SIGTERM);
	SIGTERM_watcher.start();
	//}}}

	//{{{ Watcher for the SIGINT POSIX signal

	ev::sig SIGINT_watcher;
	SIGINT_watcher.set < Doodle, &Doodle::SIGTERM_cb > (this);
	SIGINT_watcher.set(SIGINT);
	SIGINT_watcher.start();
	//}}}
	//}}}

	socket_watcher.start();


	loop.run();

	logger<<"Returning from event loop"<<std::endl;

	return retval;
}
//}}}
