#include "doodle.hpp"
//#include <uvw.hpp>
//#include "sockets.hpp"
//#include <cstring>




//void Doodle::on_window_change(const i3ipc::window_event_t&)
//{
//	std::cout<<"on window change"<<std::endl;
//}
//void Doodle::on_workspace_change(const i3ipc::workspace_event_t&)
//{
//	std::cout<<"on workspce change"<<std::endl;
//}


//{{{
int Doodle::operator()(void)
{
	int retval = 0;

	return retval;
}
//}}}



//
////{{{
//int Doodle::operator()(void)
//{
//	int retval = 0;
//	//i3ipc::connection i3_conn;
//	std::shared_ptr<uvw::Loop> loop = uvw::Loop::getDefault();
//
//	//{{{ i3 event subscriptions and callbacks
//
//	//i3_conn.subscribe(i3ipc::ET_WORKSPACE | i3ipc::ET_WINDOW);
//	//i3_conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
//	//i3_conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));
//	//i3_conn.connect_event_socket();
//
//
//	// Have libuv watch for i3 events
//    //std::shared_ptr<uvw::PipeHandle> i3_watcher = loop->resource<uvw::PipeHandle>();
//    //i3_watcher->on<uvw::DataEvent>([this](const uvw::DataEvent& evt, auto&){
//	//		i3_conn.handle_event(reinterpret_cast<uint8_t*>(&evt.data[0]), evt.length);
//    //});
//
//	//i3_watcher->on<uvw::CloseEvent>([&loop](const uvw::CloseEvent &, uvw::PipeHandle &) {
//	//		loop->walk([](uvw::BaseHandle &h){ h.close(); });
//	//		std::cout<<"Server close"<<std::endl;
//	//});
//	//i3_watcher->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) {
//	//		sock.close();
//	//		std::cout<<"Socket close"<<std::endl;
//	//});
//
//
//	//i3_watcher->open(i3_conn.get_event_socket_fd());
//	//i3_watcher->read();
//	//}}}
//
//	//{{{ Watch for signals
//	
//
//    std::shared_ptr<uvw::SignalHandle> SIGINT_watcher = loop->resource<uvw::SignalHandle>();
//    SIGINT_watcher->on<uvw::SignalEvent>([&loop](const uvw::SignalEvent& evt, auto&){
//			std::cout<<"got signal SIGINT ("<<evt.signum<<")"<<std::endl;
//			loop->walk([](uvw::BaseHandle &h){ h.close(); });
//    });
//    SIGINT_watcher->start(SIGINT);
//
//    //std::shared_ptr<uvw::SignalHandle> SIGTERM_watcher = loop->resource<uvw::SignalHandle>();
//    //SIGTERM_watcher->on<uvw::SignalEvent>([&loop](const uvw::SignalEvent& evt, auto&){
//	//		std::cout<<"got signal SIGTERM ("<<evt.signum<<")"<<std::endl;
//	//		loop->walk([](uvw::BaseHandle &h){ h.close(); });
//    //});
//    //SIGTERM_watcher->start(SIGTERM);
//
//	//std::cout<< "SIGABRT   : "<<SIGABRT   <<std::endl;
//	//std::cout<< "SIGALRM   : "<<SIGALRM   <<std::endl;
//	//std::cout<< "SIGBUS    : "<<SIGBUS    <<std::endl;
//	//std::cout<< "SIGCHLD   : "<<SIGCHLD   <<std::endl;
//	//std::cout<< "SIGCONT   : "<<SIGCONT   <<std::endl;
//	//std::cout<< "SIGFPE    : "<<SIGFPE    <<std::endl;
//	//std::cout<< "SIGHUP    : "<<SIGHUP    <<std::endl;
//	//std::cout<< "SIGILL    : "<<SIGILL    <<std::endl;
//	//std::cout<< "SIGINT    : "<<SIGINT    <<std::endl;
//	//std::cout<< "SIGKILL   : "<<SIGKILL   <<std::endl;
//	//std::cout<< "SIGPIPE   : "<<SIGPIPE   <<std::endl;
//	//std::cout<< "SIGQUIT   : "<<SIGQUIT   <<std::endl;
//	//std::cout<< "SIGSEGV   : "<<SIGSEGV   <<std::endl;
//	//std::cout<< "SIGSTOP   : "<<SIGSTOP   <<std::endl;
//	//std::cout<< "SIGTERM   : "<<SIGTERM   <<std::endl;
//	//std::cout<< "SIGTSTP   : "<<SIGTSTP   <<std::endl;
//	//std::cout<< "SIGTTIN   : "<<SIGTTIN   <<std::endl;
//	//std::cout<< "SIGTTOU   : "<<SIGTTOU   <<std::endl;
//	//std::cout<< "SIGUSR1   : "<<SIGUSR1   <<std::endl;
//	//std::cout<< "SIGUSR2   : "<<SIGUSR2   <<std::endl;
//	//std::cout<< "SIGPOLL   : "<<SIGPOLL   <<std::endl;
//	//std::cout<< "SIGPROF   : "<<SIGPROF   <<std::endl;
//	//std::cout<< "SIGSYS    : "<<SIGSYS    <<std::endl;
//	//std::cout<< "SIGTRAP   : "<<SIGTRAP   <<std::endl;
//	//std::cout<< "SIGURG    : "<<SIGURG    <<std::endl;
//	//std::cout<< "SIGVTALRM : "<<SIGVTALRM <<std::endl;
//	//std::cout<< "SIGXCPU   : "<<SIGXCPU   <<std::endl;
//	//std::cout<< "SIGXFSZ   : "<<SIGXFSZ   <<std::endl;
//
//	//}}}
//
//
//	loop->run();
//
//	return retval;
//}
////}}}
//



//
////{{{
//int Doodle::operator()(void)
//{
//	int retval = 0;
//
//	//i3_conn.connect_event_socket();
//
//	//logger<<"---------------Starting the event loop---------------"<<std::endl;
//
//	////{{{ Watcher for i3 events
//
//	////ev::io i3_watcher; TODO
//	////i3_watcher.set < i3ipc::connection, &i3ipc::connection::handle_event > (&i3_conn);
//	////i3_watcher.set(i3_conn.get_event_socket_fd(), ev::READ);
//
//	////i3_watcher.start();
//	////}}}
//
//	////{{{ Watchers for POSIX signals
//
//	////{{{ Watcher for the SIGUSR1 POSIX signal
//
//	//ev::sig SIGUSR1_watcher;
//	//SIGUSR1_watcher.set < Doodle, &Doodle::SIGUSR1_cb > (this);
//	//SIGUSR1_watcher.set(SIGUSR1);
//	//SIGUSR1_watcher.start();
//	////}}}
//
//	////{{{ Watcher for the SIGTERM POSIX signal
//
//	//ev::sig SIGTERM_watcher;
//	//SIGTERM_watcher.set < Doodle, &Doodle::SIGTERM_cb > (this);
//	//SIGTERM_watcher.set(SIGTERM);
//	//SIGTERM_watcher.start();
//	////}}}
//
//	////{{{ Watcher for the SIGINT POSIX signal
//
//	//ev::sig SIGINT_watcher;
//	//SIGINT_watcher.set < Doodle, &Doodle::SIGTERM_cb > (this);
//	//SIGINT_watcher.set(SIGINT);
//	//SIGINT_watcher.start();
//	////}}}
//	////}}}
//
//	////socket_watcher.start(); TODO
//
//
//	//loop->run();
//
//	////logger<<"Returning from event loop"<<std::endl;
//
//	return retval;
//}
////}}}
//
