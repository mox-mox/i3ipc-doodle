#include <doodle.hpp>
#include "main.hpp"


//{{{
Doodle::Doodle(void) :
	i3_conn(),
	loop(uvw::Loop::getDefault()),
	current_window(),
	current_job(nullptr),
	idle(true),
	suspended(false),
	xcb_conn(xcb_connect(NULL, NULL)),
	screen(xcb_setup_roots_iterator(xcb_get_setup(xcb_conn)).data),
	idle_timer(loop->resource<uvw::TimerHandle>()),
	sigint(loop->resource<uvw::SignalHandle>()),
	i3_pipe(loop->resource<uvw::PipeHandle>())
	//client_pipe(loop->resource<uvw::PipeHandle>())
{

	//{{{ Create the individual jobs

	debug<<"Config dir: "<<config_dir<<std::endl;
	for( auto&f: fs::directory_iterator(fs::path(config_dir)/"jobs"))
	{
		if(f.path().extension() == ".job")
		{
			try
			{
				jobs.push_back({f.path(), loop});
			}
			catch(std::runtime_error&e)
			{
				error<<"Caught exception \""<<e.what()<<"\" while constructing job "<<f.path().filename()<<". ... removing that job from the job list."<<std::endl;
			}
		}
	}

	//}}}

	//{{{ i3 event subscriptions

	//simulate_workspace_change(i3_conn.get_workspaces());	// Inject a fake workspace change event to start tracking the first workspace.
	////simulate_window_change(i3_conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.

	i3_conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	i3_conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));

	if( !i3_conn.subscribe(i3ipc::ET_WORKSPACE|i3ipc::ET_WINDOW))
	{
		notify_critical<<"Doodle"<<"Could not connect to i3"<<std::endl;
		error<<"Could not subscribe to the workspace- and window change events."<<std::endl;
		throw "Could not subscribe to the workspace- and window change events.";
	}
	i3_conn.connect_event_socket();
	//}}}

	//{{{ Initialise all event watchers
	
	//{{{ idle time watcher

	if(max_idle_time_ms > milliseconds(0))
	{
		idle_timer->on<uvw::TimerEvent>(std::bind( &Doodle::on_idle_timer, this, std::placeholders::_1, std::placeholders::_2 ));
		idle_timer->start(milliseconds(20),milliseconds(0));
	}
	else
	{
		idle = false;
	}
	//}}}

	//{{{ <Ctrl+c>/SIGINT watcher

    sigint->on<uvw::SignalEvent>([this](const auto &, auto &){
			std::cout<<"Got SIGINT"<<std::endl;
			loop->stop();
    });
	sigint->start(SIGINT);
	//}}}

	//{{{ i3 socket watcher

	i3_pipe->init();
    i3_pipe->on<uvw::DataEvent>([this](const uvw::DataEvent& evt, auto&){
			// Pass the received data to i3ipcpp
			i3_conn.handle_event(reinterpret_cast<uint8_t*>(evt.data.get()), evt.length);
    });
	i3_pipe->open(i3_conn.get_event_socket_fd());
	i3_pipe->read();
	//}}}
	//}}}
}
//}}}

//{{{
Doodle::~Doodle(void)
{
	xcb_disconnect(xcb_conn);
}
//}}}

//
////{{{
//void Doodle::on_window_change(const i3ipc::window_event_t& evt)
//{
//	notify_normal<<sett(5000)<<"New current window: "<< evt.container->name <<std::endl;
//}
////}}}
//
////{{{
//void Doodle::on_workspace_change(const i3ipc::workspace_event_t& evt)
//{
//	if( evt.type == i3ipc::WorkspaceEventType::FOCUS )
//	{
//		notify_normal<<sett(5000)<<"New current_workspace: "<<evt.current->name<<std::endl;
//	}
//}
////}}}
//



//{{{
inline Job* Doodle::find_job(void)
{
	if(detect_ambiguity)// For normal operation, just report the first match.
	{
		std::vector<Job*> matches;
		for(Job& j : jobs)
		{
			if(j==current_window) matches.push_back(&j);
		}
		if(matches.size() > 1)
		{
			error<<"Ambiguity detected: Window name \""<<current_window.window_name<<"\" matched:\n";
			for(auto& j : matches)
			{
				error<<'\t'<<j->get_jobname();
			}
			error<<std::endl;
			// TODO: Show an errow window that asks to clarify which job the window belongs to.
		}
		return matches.size() ? matches[0] : nullptr;
	}
	else
	{
		auto it = std::find(jobs.begin(), jobs.end(), current_window);
			//std::find_if(jobs.begin(), jobs.end(), [&](Job& j) { return j.match(current_workspace, current_window_name); });
		return it != jobs.end() ? &*it : nullptr;
	}
}
//}}}

//{{{
void Doodle::on_window_change(const i3ipc::window_event_t& evt)
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	(void) now;
	// A window change may imply user activity, so if the user is considered idle, update that status
	//if( idle ) idle_time_watcher_cb(idle_watcher_timer, 2);
	if( idle ) on_idle_timer(uvw::TimerEvent(), *idle_timer);
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

	current_window.window_name = evt.container->name;

	if(((evt.type == i3ipc::WindowEventType::FOCUS) || (evt.type == i3ipc::WindowEventType::TITLE)) && (evt.container != nullptr))
	{
		Job* old_job = current_job;

		auto indexed_job = win_id_cache.find(evt.container->id);
		if(indexed_job == win_id_cache.end() || indexed_job->second == nullptr || *indexed_job->second!=current_window)
		{
			//win_id_cache[evt.container->id] = find_job(current_window_name);
			win_id_cache[evt.container->id] = find_job();
		}
		current_job = win_id_cache[evt.container->id];

		if( old_job != current_job )
		{
			if(old_job) old_job->stop(now);
			if(current_job && !idle) current_job->start(now);
			logger<<"Active job: "<<(old_job?old_job->get_jobname():"none")<<" --> "<<(current_job?current_job->get_jobname():"none")<<std::endl;
		}
	}
	else if( evt.type == i3ipc::WindowEventType::CLOSE )
	{
		win_id_cache.erase(evt.container->id);
		simulate_window_change(i3_conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
	}
}
//}}}

//{{{
void Doodle::on_workspace_change(const i3ipc::workspace_event_t& evt)
{
	if( evt.type == i3ipc::WorkspaceEventType::FOCUS )
	{
		debug<<"New current_workspace: "<<evt.current->name<<std::endl;
		notify_low<<sett(5000)<<"New current_workspace: "<<evt.current->name<<std::endl;
		current_window.workspace_name = evt.current->name;
		simulate_window_change(i3_conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
	}
	else
	{
		debug<<"Ignoring Workspace event"<<std::endl;
	}
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
void Doodle::on_idle_timer(const uvw::TimerEvent&, uvw::TimerHandle& timer)
{
	xcb_screensaver_query_info_cookie_t cookie = xcb_screensaver_query_info(xcb_conn, screen->root);
	xcb_screensaver_query_info_reply_t* info = xcb_screensaver_query_info_reply(xcb_conn, cookie, NULL);

	milliseconds idle_time_ms = milliseconds(info->ms_since_user_input);

	debug<<"Checking idle time: "<<idle_time_ms<<". (max_idle_time_ms = "<<max_idle_time_ms<<")"<<std::endl;
	free(info);

	milliseconds repeat_value_ms = milliseconds(1000);
	if( !idle )
	{
		// Restart the watcher to trigger when idle_time_ms might reach max_idle_time_ms for the first time.
		// See solution 2 of http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Be_smart_about_timeouts
		repeat_value_ms = max_idle_time_ms-idle_time_ms;
		// If the value was allowed to become zero, the watcher would never be started again
		//debug<<"repeat_value: "<<repeat_value_ms<<", milliseconds(0): "<<milliseconds(0)<<"."<<std::endl;
		repeat_value_ms = repeat_value_ms > milliseconds(0) ? repeat_value_ms : milliseconds(1000);
	}
	//debug<<"repeat_value: "<<repeat_value_ms<<"."<<std::endl;

	timer.start(repeat_value_ms,milliseconds(0));

	if((idle_time_ms >= max_idle_time_ms) && !idle )
	{
		idle = true;
		debug<<"Going idle"<<std::endl;
		if(current_job) current_job->stop(std::chrono::steady_clock::now());
	}
	else if((idle_time_ms < max_idle_time_ms) && idle )
	{
		idle = false;
		debug<<"Going busy again"<<std::endl;
		//if(current_job) current_job->start(std::chrono::steady_clock::now(), current_workspace, current_window_name);
		if(current_job) current_job->start(std::chrono::steady_clock::now());
	}
}
//}}}

//{{{
int Doodle::operator()(void)
{
	int retval = 0;
	logger<<"---------------Starting the event loop---------------"<<std::endl;

    loop->run();
	// Destroy the watchers after the loop stops
	loop->walk([](uvw::BaseHandle &h){ h.close(); });

	logger<<"Returning from event loop"<<std::endl;
	return retval;
}
//}}}


//{{{
std::ostream& operator<<(std::ostream&stream, Doodle const&doodle)
{
	stream<<"Doodle class:"<<std::endl;
	stream<<"	Current job: "<<(doodle.current_job?doodle.current_job->get_jobname():"none")<<std::endl;
	stream<<"	Current workspace: "<<doodle.current_window.workspace_name<<std::endl;
	stream<<"	Jobs:"<<std::endl;
	for( const Job& job : doodle.jobs )
	{
		stream<<"		"<<job<<std::endl;
	}
	stream<<"	Known windows:"<<std::endl<<"		win_id		jobname		matching_name"<<std::endl;
	for( auto it : doodle.win_id_cache )
	{
		stream<<"		"<<it.first<<"	"<<(it.second?it.second->get_jobname():"none")<<std::endl;
	}
	return stream;
}
//}}}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//
////{{{
//inline Job* Doodle::find_job(void)
//{
//	if( !settings.detect_ambiguity )// For normal operation, just report the first match.
//	{
//		std::vector<Job*> matches;
//		for(Job& j : jobs)
//		{
//			if(j.match(current_workspace, current_window_name)) matches.push_back(&j);
//		}
//		if(matches.size() > 1)
//		{
//			error<<"Ambiguity detected: Window name \""<<current_window_name<<"\" matched:\n";
//			for(auto j : matches)
//			{
//				error<<'\t'<<j->get_jobname();
//			}
//			error<<std::endl;
//			// TODO: Show an errow window that asks to clarify which job the window belongs to.
//		}
//		return matches.size() ? matches[0] : nullptr;
//	}
//	else
//	{
//		std::_Deque_iterator<Job, Job&, Job*> it =
//			std::find_if(jobs.begin(), jobs.end(), [&](Job& j) { return j.match(current_workspace, current_window_name); });
//		return it != jobs.end() ? &*it : nullptr;
//	}
//}
////}}}
//
////{{{
//void Doodle::on_window_change(const i3ipc::window_event_t& evt)
//{
//	std::cout<<*this<<std::endl;
//	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
//	// A window change may imply user activity, so if the user is considered idle, update that status
//	if( idle ) idle_time_watcher_cb(idle_watcher_timer, 2);
//	//{{{ Print some information about the event
//
//	//#ifdef DEBUG
//	//	logger<<"on_window_change() called "<<++win_evt_count<<"th time. Type: "<<static_cast < char > (evt.type)<<std::endl;
//	//	if( evt.container != nullptr )
//	//	{
//	//		//logger<<"	id = "<<evt.container->id<<std::endl;
//	//		logger<<"	name = "<<evt.container->name<<std::endl;
//	//		//logger<<"	type = "<<evt.container->type<<std::endl;
//	//		//logger<<"	urgent = "<<evt.container->urgent<<std::endl;
//	//		//logger<<"	focused = "<<evt.container->focused<<std::endl;
//	//	}
//	//#endif
//	//}}}
//
//	current_window_name = evt.container->name;
//
//	if(((evt.type == i3ipc::WindowEventType::FOCUS) || (evt.type == i3ipc::WindowEventType::TITLE)) && (evt.container != nullptr))
//	{
//		Job* old_job = current_job;
//		std::map<window_id, Job*>::iterator indexed_job = win_id_cache.find(evt.container->id);
//
//		if(indexed_job == win_id_cache.end() || indexed_job->second == nullptr || !indexed_job->second->match(current_workspace, current_window_name))
//		{
//			//win_id_cache[evt.container->id] = find_job(current_window_name);
//			win_id_cache[evt.container->id] = find_job();
//		}
//		current_job = win_id_cache[evt.container->id];
//		if( old_job != current_job )
//		{
//			if(old_job && current_job)
//			{
//				if(old_job != current_job)
//				{
//					old_job->stop(now);
//					current_job->start(now, current_workspace, current_window_name);
//				}
//			}
//			else
//			{
//				if(old_job) old_job->stop(now);
//				if(current_job && !idle) current_job->start(now, current_workspace, current_window_name);
//			}
//		}
//		logger<<"New current_job: "<<current_job<<(current_job?current_job->get_jobname():"none")<<std::endl;
//	}
//	else if( evt.type == i3ipc::WindowEventType::CLOSE )
//	{
//		win_id_cache.erase(evt.container->id);
//		simulate_window_change(i3_conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
//	}
//}
////}}}
//
////{{{
//void Doodle::on_workspace_change(const i3ipc::workspace_event_t& evt)
//{
//	if( evt.type == i3ipc::WorkspaceEventType::FOCUS )
//	{
//		logger<<"New current_workspace: "<<evt.current->name<<std::endl;
//		current_workspace = evt.current->name;
//		simulate_window_change(i3_conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
//	}
//#ifdef DEBUG
//	else
//	{
//		//logger<<"Ignoring Workspace event"<<std::endl;
//	}
//#endif
//}
////}}}
//
////{{{
//std::ostream& operator<<(std::ostream&stream, Doodle const&doodle)
//{
//	stream<<"Doodle class:"<<std::endl;
//	stream<<"	Current job: "<<(doodle.current_job?doodle.current_job->get_jobname():"none")<<std::endl;
//	stream<<"	Current workspace: "<<doodle.current_workspace<<std::endl;
//	stream<<"	Jobs:"<<std::endl;
//	for( const Job& job : doodle.jobs )
//	{
//		stream<<"		"<<job<<std::endl;
//	}
//	stream<<"	Known windows:"<<std::endl<<"		win_id		jobname		matching_name"<<std::endl;
//	for( auto it : doodle.win_id_cache )
//	{
//		stream<<"		"<<it.first<<"	"<<(it.second?it.second->get_jobname():"none")<<std::endl;
//	}
//	return stream;
//}
////}}}
//
////{{{
//bool Doodle::simulate_workspace_change(std::vector < std::shared_ptr < i3ipc::workspace_t>>workspaces)
//{	// Iterate through all workspaces and call on_workspace_change() for the focussed one.
//	for( std::shared_ptr < i3ipc::workspace_t > &workspace : workspaces )
//	{
//		if( workspace->focused )
//		{
//			on_workspace_change({ i3ipc::WorkspaceEventType::FOCUS,  workspace, nullptr });
//			return true;
//		}
//	}
//	error<<"No workspace is focused."<<std::endl;
//	return false;	// Should never be reached
//}
////}}}
//
////{{{
//bool Doodle::simulate_window_change(std::list < std::shared_ptr < i3ipc::container_t>>nodes)
//{	// Iterate through all containers and call on_window_change() for the focussed one.
//	for( std::shared_ptr < i3ipc::container_t > &container : nodes )
//	{
//		if( container->focused )
//		{
//			on_window_change({ i3ipc::WindowEventType::FOCUS,  container });
//			return true;
//		}
//		else
//		{
//			if( simulate_window_change(container->nodes)) return true;
//		}
//	}
//	return false;	// Should never be reached
//}
////}}}
//



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//{{{


//
//	//{{{ Create watchers
//
//	////{{{ Create a resource that will listen to STDIN
//
//	//std::shared_ptr<uvw::TTYHandle> console = loop->resource<uvw::TTYHandle>(uvw::StdIN, true);
//    //console->on<uvw::DataEvent>([](auto& evt, auto& hndl){
//	//		(void) hndl;
//	//		std::cout<<"Got something from STDIN: "<<std::endl;
//	//		std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
//    //});
//	//console->on<uvw::CloseEvent>([](const uvw::CloseEvent &, uvw::TTYHandle &) { std::cout<<"TTY close"<<std::endl; });
//	//console->read();
//	////}}}
//
//	////{{{ Generic timer
//
//	//std::shared_ptr<uvw::TimerHandle> timer = loop->resource<uvw::TimerHandle>();
//    //timer->on<uvw::TimerEvent>([this](const uvw::TimerEvent &, uvw::TimerHandle & ci){
//	//		std::cout<<"TIMER: "<<loop->now().count()<<std::endl;
//    //});
//	//timer->start(milliseconds(2000),milliseconds(1000));
//	////}}}
//
//	//{{{ Idle time watcher
//
//	std::shared_ptr<uvw::TimerHandle> idle_timer = loop->resource<uvw::TimerHandle>();
//    idle_timer->on<uvw::TimerEvent>(std::bind( &Doodle::on_idle_timer, this, std::placeholders::_1, std::placeholders::_2 ));
//	idle_timer->start(milliseconds(20),milliseconds(0));
//	//}}}
//
//	//{{{ Create a resource that will listen to <Ctrl+c>/SIGINT
//
//	std::shared_ptr<uvw::SignalHandle> sigint = loop->resource<uvw::SignalHandle>();
//    sigint->on<uvw::SignalEvent>([this](const auto &, auto &){
//			std::cout<<"Got SIGINT"<<std::endl;
//			loop->stop();
//    });
//	sigint->start(SIGINT);
//	//}}}
//
//	//{{{ Create a ressource that will listen to the i3 socket
//
//
//	//while( true )
//	//{
//	//	i3_conn.handle_event();
//	//}
//
//	std::shared_ptr<uvw::PipeHandle> i3_pipe = loop->resource<uvw::PipeHandle>();
//	i3_pipe->init();
//    i3_pipe->on<uvw::DataEvent>([this](const uvw::DataEvent& evt, auto&){
//			// Pass the received data to i3ipcpp
//			i3_conn.handle_event(reinterpret_cast<uint8_t*>(evt.data.get()), evt.length);
//    });
//
//
//
//    //i3_pipe->on<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::PipeHandle &) {
//	//		std::cout<<"Established connection to i3"<<std::endl;
//    //});
//	//i3_pipe->on<uvw::CloseEvent>([&loop](const uvw::CloseEvent &, uvw::PipeHandle &) {
//	//		loop->walk([](uvw::BaseHandle &h){ h.close(); });
//	//		std::cout<<"Server close"<<std::endl;
//	//});
//	//i3_pipe->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) {
//	//		sock.close();
//	//		std::cout<<"Socket close"<<std::endl;
//	//});
//	
//
//	//i3_pipe->connect(socket_path);
//	i3_pipe->open(i3_conn.get_event_socket_fd());
//	i3_pipe->read();
//	//}}}
//
//	//}}}
//

//}}}
