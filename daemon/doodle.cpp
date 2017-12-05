#include <doodle.hpp>
#include "main.hpp"



//{{{
Doodle::Doodle(void) :
	i3_conn(),
	loop(uvw::Loop::getDefault()),
	idle(true),
	suspended(false),
	xcb_conn(xcb_connect(NULL, NULL)),
	screen(xcb_setup_roots_iterator(xcb_get_setup(xcb_conn)).data),
	idle_timer(loop->resource<uvw::TimerHandle>()),
	sigint(loop->resource<uvw::SignalHandle>()),
	i3_pipe(loop->resource<uvw::PipeHandle>())
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

    idle_timer->on<uvw::TimerEvent>(std::bind( &Doodle::on_idle_timer, this, std::placeholders::_1, std::placeholders::_2 ));
	idle_timer->start(uvw::TimerHandle::Time(20),uvw::TimerHandle::Time(0));
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


//{{{
void Doodle::on_window_change(const i3ipc::window_event_t& evt)
{
	notify_normal<<sett(5000)<<"New current window: "<< evt.container->name <<std::endl;
}
//}}}

//{{{
void Doodle::on_workspace_change(const i3ipc::workspace_event_t& evt)
{
	if( evt.type == i3ipc::WorkspaceEventType::FOCUS )
	{
		notify_normal<<sett(5000)<<"New current_workspace: "<<evt.current->name<<std::endl;
	}
}
//}}}

//{{{
void Doodle::on_idle_timer(const uvw::TimerEvent&, uvw::TimerHandle& timer)
{
	xcb_screensaver_query_info_cookie_t cookie = xcb_screensaver_query_info(xcb_conn, screen->root);
	xcb_screensaver_query_info_reply_t* info = xcb_screensaver_query_info_reply(xcb_conn, cookie, NULL);

	uint32_t idle_time_ms = info->ms_since_user_input;

	debug<<"Checking idle time: "<<idle_time_ms<<"ms. (max_idle_time_ms = "<<max_idle_time_ms<<"ms)"<<std::endl;
	free(info);

	int32_t repeat_value_ms = 1000;
	if( !idle )
	{
		// Restart the watcher to trigger when idle_time_ms might reach max_idle_time_ms for the first time.
		// See solution 2 of http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Be_smart_about_timeouts
		repeat_value_ms = max_idle_time_ms-idle_time_ms;
		// If the value was allowed to become zero, the watcher would never be started again
		repeat_value_ms = repeat_value_ms > 0 ? repeat_value_ms : 1000;
	}
	debug<<"repeat_value: "<<repeat_value_ms<<"ms."<<std::endl;

	timer.start(uvw::TimerHandle::Time(repeat_value_ms),uvw::TimerHandle::Time(0));

	if((idle_time_ms >= max_idle_time_ms) && !idle )
	{
		idle = true;
		debug<<"Going idle"<<std::endl;
		//if(current_job) current_job->stop(std::chrono::steady_clock::now());
	}
	else if((idle_time_ms < max_idle_time_ms) && idle )
	{
		idle = false;
		debug<<"Going busy again"<<std::endl;
		//if(current_job) current_job->start(std::chrono::steady_clock::now(), current_workspace, current_window_name);
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
//	//timer->start(uvw::TimerHandle::Time(2000),uvw::TimerHandle::Time(1000));
//	////}}}
//
//	//{{{ Idle time watcher
//
//	std::shared_ptr<uvw::TimerHandle> idle_timer = loop->resource<uvw::TimerHandle>();
//    idle_timer->on<uvw::TimerEvent>(std::bind( &Doodle::on_idle_timer, this, std::placeholders::_1, std::placeholders::_2 ));
//	idle_timer->start(uvw::TimerHandle::Time(20),uvw::TimerHandle::Time(0));
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
