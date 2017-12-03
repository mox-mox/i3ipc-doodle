#include <doodle.hpp>
#include "main.hpp"

#include <uvw.hpp>


//{{{
Doodle::Doodle(void) : i3_conn()
{
	//{{{ i3 event subscriptions

	std::cout<<"1"<<std::endl;
	i3_conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	std::cout<<"2"<<std::endl;
	i3_conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));
	std::cout<<"3"<<std::endl;

	if( !i3_conn.subscribe(i3ipc::ET_WORKSPACE|i3ipc::ET_WINDOW))
	{
		error<<"could not connect"<<std::endl;
		throw "Could not subscribe to the workspace- and window change events.";
	}
	std::cout<<"4"<<std::endl;
	//}}}
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
	std::cout<<"5"<<std::endl;

	i3_conn.connect_event_socket();
	std::cout<<"6"<<std::endl;

	logger<<"---------------Starting the event loop---------------"<<std::endl;

	auto loop = uvw::Loop::getDefault();

	// Create a resource that will listen to STDIN
    auto console = loop->resource<uvw::TTYHandle>(uvw::StdIN, true);
    console->on<uvw::DataEvent>([](auto& evt, auto& hndl){
			(void) hndl;
			std::cout<<"Got something from STDIN: "<<std::endl;
			std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
    });
	console->on<uvw::CloseEvent>([](const uvw::CloseEvent &, uvw::TTYHandle &) { std::cout<<"TTY close"<<std::endl; });
	console->read();










	//{{{ Listen to the i3 pipe

    auto pipe = loop->resource<uvw::PipeHandle>();
	pipe->init();
    //pipe->on<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::PipeHandle &) {
	//		//handle.close();
	//		std::cout<<"Wrote to the pipe"<<std::endl;
    //});

    //pipe->on<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
    //    auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{ 'a' });
    //    handle.tryWrite(std::move(dataTryWrite), 1);
    //    auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
    //    handle.write(std::move(dataWrite), 2);
    //});
	//pipe->on<uvw::CloseEvent>([&loop](const uvw::CloseEvent &, uvw::PipeHandle &) {
	//		loop->walk([](uvw::BaseHandle &h){ h.close(); });
	//		std::cout<<"Server close"<<std::endl;
	//});
	//pipe->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) {
	//		sock.close();
	//		std::cout<<"Socket close"<<std::endl;
	//});
	
    pipe->on<uvw::DataEvent>([](const uvw::DataEvent& evt, auto&){
			std::cout<<"Got something from the pipe: "<<std::endl;
			std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
    });

	//pipe->connect(socket_path);
	pipe->open(i3_conn.get_event_socket_fd());

	pipe->read();



	//}}}

    loop->run();
	loop->walk([](uvw::BaseHandle &h){ h.close(); });




	//while( true )
	//{
	//	std::cout<<"7"<<std::endl;
	//	i3_conn.handle_event();
	//	std::cout<<"8"<<std::endl;
	//}

	logger<<"Returning from event loop"<<std::endl;

	return retval;
}
//}}}

