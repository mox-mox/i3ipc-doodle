#include <client.hpp>
#include "main.hpp"
#include "sockets.hpp"
#include <stropts.h>
#include <sstream>
#include <json/json.h>
#include <fstream>



//{{{
fs::path check_histfile(fs::path dir = "default")
{
	//{{{ Check empty (disable history)

	if(dir == "")
	{
		logger<<"History disabled"<<std::endl;
		return dir;
	}
	//}}}

	//{{{ Check default value

	if(dir == "default")
	{
		debug<<"Checking default history path = "<<dir<<'.'<<std::endl;
		if(dir=data_dir/"history"; fs::exists(dir) && fs::is_regular_file(dir))
		{
			debug<<"History found in "<<dir<<std::endl;
			return dir;
		}
		else if(!fs::exists(dir))
		{
			logger<<"No history file found in default history file path "<<dir<<" => Create it and set it as history file"<<std::endl;
			std::ofstream histfile(dir);
			if(histfile.is_open())
			{
				debug<<"Created default history file = "<<dir<<'.'<<std::endl;
				return dir;
			}
			else
			{
				error<<"Could not create default history file = "<<dir<<'.'<<std::endl;
				return "";
			}
		}
		else
		{
			error<<"Cannot set default history file "<<dir<<": Path exists but is not a regular file. Continue without history."<<std::endl;
			return "";
		}
	}
	//}}}

	//{{{ Check user provided value

	debug<<"Checking user provided history path = "<<dir<<'.'<<std::endl;
	if(fs::exists(dir) && fs::is_regular_file(dir))
	{
		debug<<"History found in "<<dir<<std::endl;
		return dir;
	}
	else if(!fs::exists(dir))
	{
		logger<<"No history file found in user provided history path "<<dir<<" => Create it and set it as history file"<<std::endl;
		std::fstream histfile(dir);
		if(histfile.is_open())
		{
			debug<<"Created user provided history file = "<<dir<<'.'<<std::endl;
			return dir;
		}
		else
		{
			error<<"Could not create user provided history file = "<<dir<<'.'<<std::endl;
			return "";
		}
	}
	else
	{
		error<<"Cannot set user provided history file "<<dir<<": Path exists but is not a regular file. Continue without history."<<std::endl;
		return "";
	}
	//}}}

	return "";
}
//}}}



//{{{
Client::Client(INIReader& config_reader) :
	loop(uvw::Loop::getDefault()),
	sigint(loop->resource<uvw::SignalHandle>()),
	daemon_pipe(loop->resource<uvw::PipeHandle>()),
	console(loop->resource<uvw::TTYHandle>(uvw::StdIN, true)),
	repl(check_histfile(config_reader.Get("client", "histfile", "default")), "doodle client > ")

{
	(void) config_reader;
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
			//std::cout<<"Server close"<<std::endl;
	});
	daemon_pipe->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) {
			sock.close();
			std::cout<<"Socket close"<<std::endl;
	});
	
    daemon_pipe->on<uvw::DataEvent>([this](const uvw::DataEvent& evt, auto&){
			std::cout<<'\n'<<std::string(&evt.data[0], evt.length)<<std::endl;
			repl.draw();
    });

	daemon_pipe->open(open_socket(user_socket_path));

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
	console->on<uvw::CloseEvent>([](const uvw::CloseEvent&, uvw::TTYHandle& console) { console.reset(); /*std::cout<<"TTY close"<<std::endl;*/ });
	//console->mode(uvw::details::UVTTYModeT::IO);
	console->mode(uvw::details::UVTTYModeT::RAW);
	console->read();
	//}}}



	//}}}
}
//}}}

//{{{
Client::~Client(void)
{
	console->reset();
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

	logger<<"Returning from event loop"<<std::endl;
	return retval;
}
//}}}
