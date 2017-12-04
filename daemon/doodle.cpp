#include <doodle.hpp>
#include "main.hpp"



//{{{
Doodle::Doodle(void) :
	i3_conn(),
	loop(uvw::Loop::getDefault())
{

	//{{{ Create the individual jobs

	logger<<"Config dir: "<<config_dir<<std::endl;
	for( auto&f: fs::directory_iterator(fs::path(config_dir)/"jobs"))
	{
		if(f.path().extension() == ".job")
		{
			try
			{
				//jobs.push_back( Job::create_from_jobfile(f.path(), loop) );
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
		error<<"could not connect"<<std::endl;
		throw "Could not subscribe to the workspace- and window change events.";
	}
	i3_conn.connect_event_socket();
	//}}}
}
//}}}


//{{{
Doodle::~Doodle(void)
{
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
int Doodle::operator()(void)
{
	int retval = 0;
	logger<<"---------------Starting the event loop---------------"<<std::endl;

	//{{{ Create watchers
//
//	//{{{ Create a resource that will listen to STDIN
//
//    auto console = loop->resource<uvw::TTYHandle>(uvw::StdIN, true);
//    console->on<uvw::DataEvent>([](auto& evt, auto& hndl){
//			(void) hndl;
//			std::cout<<"Got something from STDIN: "<<std::endl;
//			std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
//    });
//	console->on<uvw::CloseEvent>([](const uvw::CloseEvent &, uvw::TTYHandle &) { std::cout<<"TTY close"<<std::endl; });
//	console->read();
//	//}}}
//
//
//	//{{{ Create a resource that will be a timer
//
//    auto timer = loop->resource<uvw::TimerHandle>();
//    timer->on<uvw::TimerEvent>([this](const auto &, auto &){
//			std::cout<<"TIMER: "<<loop->now().count()<<std::endl;
//    });
//	timer->start(uvw::TimerHandle::Time(2000),uvw::TimerHandle::Time(1000));
//	//}}}
//

	//{{{ Create a resource that will listen to <Ctrl+c>/SIGINT

    auto sigint = loop->resource<uvw::SignalHandle>();
    sigint->on<uvw::SignalEvent>([this](const auto &, auto &){
			std::cout<<"Got SIGINT"<<std::endl;
			loop->stop();
    });
	sigint->start(SIGINT);
	//}}}

	//{{{ Create a ressource that will listen to the i3 socket


	//while( true )
	//{
	//	i3_conn.handle_event();
	//}

    auto pipe = loop->resource<uvw::PipeHandle>();
	pipe->init();
    pipe->on<uvw::DataEvent>([this](const uvw::DataEvent& evt, auto&){
			// Pass the received data to i3ipcpp
			i3_conn.handle_event(reinterpret_cast<uint8_t*>(evt.data.get()), evt.length);
    });



    //pipe->on<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::PipeHandle &) {
	//		std::cout<<"Established connection to i3"<<std::endl;
    //});
	//pipe->on<uvw::CloseEvent>([&loop](const uvw::CloseEvent &, uvw::PipeHandle &) {
	//		loop->walk([](uvw::BaseHandle &h){ h.close(); });
	//		std::cout<<"Server close"<<std::endl;
	//});
	//pipe->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) {
	//		sock.close();
	//		std::cout<<"Socket close"<<std::endl;
	//});
	

	//pipe->connect(socket_path);
	pipe->open(i3_conn.get_event_socket_fd());
	pipe->read();
	//}}}

	//}}}

    loop->run();
	// Destroy the watchers after the loop stops
	loop->walk([](uvw::BaseHandle &h){ h.close(); });

	logger<<"Returning from event loop"<<std::endl;

	return retval;
}
//}}}

