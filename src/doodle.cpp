#include "doodle.hpp"
#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include "logstream.hpp"
#include <functional>
#include <json/json.h>

//{{{
Doodle::Doodle(const std::string& config_path) : conn(), config_path(config_path), current_workspace(""), nojob(), current_job(&nojob), idle(false), connection(xcb_connect (NULL, NULL)), screen(xcb_setup_roots_iterator (xcb_get_setup (connection)).data)
{
	std::ifstream config_file(config_path+"/doodlerc");

	Json::Value configuration_root;
	Json::Reader reader;

	if( !reader.parse(config_file, configuration_root, false))
	{
		error<<reader.getFormattedErrorMessages()<<std::endl;
	}

	//{{{ Get the configuration options

	std::cout<<"retrieving config"<<std::endl;
	Json::Value config;
	if( configuration_root.isMember("config"))
	{
		config = configuration_root.get("config", "no config");
	}
	else
	{
		config = configuration_root;
	}
	settings.max_idle_time = config.get("max_idle_time", settings.MAX_IDLE_TIME_DEFAULT_VALUE).asUInt();
	//std::cout<<"	max_idle_time = "<<settings.max_idle_time<<std::endl;
	settings.detect_ambiguity = config.get("detect_ambiguity", settings.DETECT_AMBIGUITY_DEFAULT_VALUE).asBool();
	//std::cout<<"	detect_ambiguity = "<<settings.detect_ambiguity<<std::endl;
	//}}}

	//{{{ Create the individual jobs

	for( auto&f: std::experimental::filesystem::directory_iterator(config_path+"/jobs"))
	{
		if((f.path() != config_path+"/doodlerc") && std::string::npos == f.path().string().find("_backup") && std::experimental::filesystem::is_regular_file(f))
		{
			try
			{
				jobs.push_back({f.path(), loop});
			}
			catch(std::runtime_error& e)
			{
				error<<"Caught exception \""<<e.what()<<"\" while constructing job "<<f.path().filename()<<". ... removing that job from the job list."<<std::endl;
			}
		}
	}
	//}}}

	nojob.start(std::chrono::steady_clock::now());										// Account for time spent on untracked jobs

	simulate_workspace_change(conn.get_workspaces());	// Inject a fake workspace change event to start tracking the first workspace.
	//simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.

	conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));

	if( !conn.subscribe(i3ipc::ET_WORKSPACE|i3ipc::ET_WINDOW))
	{
		error<<"could not connect"<<std::endl;
		throw "Could not subscribe to the workspace- and window change events.";
	}
}
//}}}

//{{{
Doodle::~Doodle(void)
{
	xcb_disconnect(connection);
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
				current_job->start(now);
			}
		}
		logger<<"New current_job: "<<current_job->get_jobname()<<std::endl;
	}
	else if( evt.type == i3ipc::WindowEventType::CLOSE )
	{
		win_id_lookup.erase(evt.container->id);
		simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
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
		simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
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

//{{{
void Doodle::SIGUSR1_cb(void)
{
	std::cout<<"Received SIGUSR1!"<<std::endl;
	std::cout<<*this<<std::endl;
}
//}}}

//{{{
void Doodle::SIGTERM_cb(void)
{
	logger<<"Shutting down doodle"<<std::endl;
	std::cout<<"Shutting down doodle"<<std::endl;
	loop.break_loop(ev::ALL);
}
//}}}

//{{{
void Doodle::idle_watcher(ev::timer& timer, int revents)
{
	(void) revents;
	std::cout<<"Checking idle time"<<std::endl;
    xcb_screensaver_query_info_cookie_t cookie = xcb_screensaver_query_info (connection, screen->root);
    xcb_screensaver_query_info_reply_t *info = xcb_screensaver_query_info_reply (connection, cookie, NULL);

    uint32_t idle_time = info->ms_since_user_input;
    free (info);

	if((idle_time > settings.max_idle_time) && !idle)
	{
		idle = true;
		logger<<"Going idle"<<std::endl;
		// TODO: Stop currently active job
	}
	else if((idle_time < settings.max_idle_time) && idle)
	{
		idle = false;
		logger<<"Going busy again"<<std::endl;
		// TODO: (Re-)Start currently active job
	}
	timer.again();
}
//}}}

//{{{
int Doodle::operator()(void)
{
	int retval = 0;
	conn.prepare_to_event_handling();

	std::cout<<"---------------Starting the event loop---------------"<<std::endl;

	//{{{ Watcher for i3 events

	ev::io i3_watcher;
	i3_watcher.set < i3ipc::connection, &i3ipc::connection::handle_event > (&conn);
	i3_watcher.set(conn.get_file_descriptor(), ev::READ);
	i3_watcher.start();
	//}}}

	//{{{ Watcher for idle time

	ev::timer idle_watcher_timer;
	idle_watcher_timer.set < Doodle, &Doodle::idle_watcher > (this);
	idle_watcher_timer.set(settings.max_idle_time/1000, settings.max_idle_time/1000);
	idle_watcher_timer.start();
	//}}}

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

	loop.run();



	std::cout<<"Returning from event loop"<<std::endl;

	return retval;
}
//}}}
