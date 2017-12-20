#include <client.hpp>
#include "main.hpp"
#include "sockets.hpp"
#include <stropts.h>
#include <sstream>
#include <json/json.h>






//{{{
Client::Client(void) :
	loop(uvw::Loop::getDefault()),
	sigint(loop->resource<uvw::SignalHandle>()),
	daemon_pipe(loop->resource<uvw::PipeHandle>()),
	console(loop->resource<uvw::TTYHandle>(uvw::StdIN, true)),
	repl("doodle client > ")

{

	//{{{ Setup Repl
	
	repl.default_mappings();
	//}}}




	//{{{ Initialise all event watchers
	
	//{{{ <Ctrl+c>/SIGINT watcher

    sigint->on<uvw::SignalEvent>([this](const auto &, auto &){
			std::cout<<"Got SIGINT"<<std::endl;
			loop->stop();
    });
	sigint->start(SIGINT);
	//}}}

	//{{{ Daemon pipe

	daemon_pipe->init();
    daemon_pipe->on<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{ 'a' });
        handle.tryWrite(std::move(dataTryWrite), 1);
        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        handle.write(std::move(dataWrite), 2);
    });
	daemon_pipe->on<uvw::CloseEvent>([this](const uvw::CloseEvent &, uvw::PipeHandle &) {
			loop->walk([](uvw::BaseHandle &h){ h.close(); });
			std::cout<<"Server close"<<std::endl;
	});
	daemon_pipe->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) {
			sock.close();
			std::cout<<"Socket close"<<std::endl;
	});
	
    daemon_pipe->on<uvw::DataEvent>([this](const uvw::DataEvent& evt, auto&){
			std::cout<<'\n'<<std::string(&evt.data[0], evt.length)<<std::endl;
			repl.draw();
    });

	daemon_pipe->open(open_socket(doodle_socket_path));

	daemon_pipe->read();
	//}}}

	//{{{ Console watcher

    console->on<uvw::DataEvent>([this](auto& evt, uvw::TTYHandle&){
		bool draw = true;
		for(std::string& line : repl.insert(&evt.data[0], evt.length))
		{
			draw=false;
			if(line == KILL_PILL) loop->walk([](uvw::BaseHandle &h){ h.close(); });;
			daemon_pipe->write(line.data(), line.length());
		}
		if(draw) repl.draw();
    });
	console->on<uvw::CloseEvent>([](const uvw::CloseEvent&, uvw::TTYHandle& console) { console.reset(), std::cout<<"TTY close"<<std::endl; });
	//console->mode(uvw::details::UVTTYModeT::IO);
	console->mode(uvw::details::UVTTYModeT::RAW);
	console->read();
	//}}}


	//}}}
}
//}}}

//{{{
int Client::operator()(void)
{
	int retval = 0;
	logger<<"---------------Starting the event loop---------------"<<std::endl;

	repl.draw();

    loop->run();
	// Destroy the watchers after the loop stops
	loop->walk([](uvw::BaseHandle &h){ h.close(); });

	console->reset();
	logger<<"Returning from event loop"<<std::endl;
	return retval;
}
//}}}
