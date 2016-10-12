#include "cli.hpp"
#include <sstream>
#include <unistd.h>
#include "getopt_pp.h"
#include <iomanip>
#include "io_watcher.hpp"


Args args;
Settings settings;


//
////{{{ Extend an IO watcher with a wrtie-queue
//
//namespace ev
//{
//	struct socket : ev::io
//	{
//		using ev::io::io;
//		socket(int fd, int events, ev::loop_ref loop) : ev::io(loop)
//		{
//			if( -1 == fd ) throw std::runtime_error("Received invalid Unix socket");
//			ev::io::set(fd, events);
//		}
//		socket(int fd, int events)
//		{
//			if( -1 == fd ) throw std::runtime_error("Received invalid Unix socket");
//			ev::io::set(fd, events);
//		}
//
//		std::deque<std::string> write_data;
//
//		//{{{ Template Magic to allow using callbacks with "void socket_watcher_cb(ev::socket& socket_watcher, int revents);"
//
//		//{{{
//		template < class K, void (K::* method)(ev::socket& w, int) >
//		static void method_thunk(struct ev_loop* loop, ev_watcher* w, int revents)
//		{
//			(void) loop;
//			(static_cast < K*> (w->data)->*method)(*reinterpret_cast<ev::socket*>(w), revents);
//		}
//
//		template < class K, void (K::* method)(ev::socket& w, int) >
//		void set(K* object) throw()
//		{
//			this->data = (void*) object;
//			this->cb = reinterpret_cast<void (*)(struct ev_loop*, struct ev_io*, int)>(&method_thunk < K, method >);
//		}
//		//}}}
//
//		//{{{
//		template < void (* function)(ev::socket& w, int) >
//		static void function_thunk(struct ev_loop* loop, ev_watcher* w, int revents)
//		{
//			(void) loop;
//			function(*reinterpret_cast<ev::socket*>(w), revents);
//		}
//
//		template < void (* function)(ev::socket& w, int) >
//		void set(void* data = 0) throw()
//		{
//			this->data = data;
//			this->cb = reinterpret_cast<void (*)(struct ev_loop*, struct ev_io*, int)>(&function_thunk < function >);
//		}
//		//}}}
//
//		//}}}
//
//
//	friend socket& operator<<(socket& lhs, std::string& data)
//	{
//		std::stringstream ss(data); std::string token;
//
//		ss >> token;
//		std::string cmd = "{\"cmd\":\"" + token + "\",\"args\":[";
//
//		//for(bool first=true, token = ""; ss>>token; first=false)
//		bool first = true;
//		token = "";
//		while(ss >> token)
//		{
//			if(!first)
//			{
//				cmd += ',';
//			}
//			first = false;
//			cmd += "\"";
//			cmd += token;
//			cmd += "\"";
//		token = "";
//			usleep(100000);
//		}
//		cmd += "]}";
//
//		uint16_t length = cmd.length();
//		std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
//		credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);
//
//		std::cout<<"Writing credential: "<<credential<<std::endl;
//		lhs.write_data.push_back(credential);
//		std::cout<<"Writing cmd: "<<cmd<<std::endl;
//		lhs.write_data.push_back(cmd);
//		if(!lhs.is_active()) lhs.start();
//		std::cout<<"...done"<<std::endl;
//
//		return lhs;
//	}
//
//	};
//
//}
////}}}
//
////{{{
//bool read_n(int fd, char buffer[], int size, ev::socket& watcher)	// Read exactly size bytes
//{
//	int read_count = 0;
//	while(read_count < size)
//	{
//		int n;
//		switch((n=read(fd, &buffer[read_count], size-read_count)))
//		{
//			case -1:
//				throw std::runtime_error("Read error on the connection using fd." + std::to_string(fd) + ".");
//			case  0:
//				std::cout<<"Received EOF (Client has closed the connection)."<<std::endl;
//				watcher.stop();
//				watcher.loop.break_loop(ev::ALL);
//				return true;
//			default:
//				read_count+=n;
//		}
//	}
//	return false;
//}
////}}}
//
////{{{
//void write_n(int fd, char buffer[], int size)	// Write exactly size bytes
//{
//	int write_count = 0;
//	while(write_count < size)
//	{
//		int n;
//		switch((n=write(fd, &buffer[write_count], size-write_count)))
//		{
//			case -1:
//				throw std::runtime_error("Write error on the connection using fd." + std::to_string(fd) + ".");
//			case  0:
//				std::cout<<"Received EOF (Client has closed the connection)."<<std::endl;
//				throw std::runtime_error("Write error on the connection using fd." + std::to_string(fd) + ".");
//				return;
//			default:
//				write_count+=n;
//		}
//	}
//}
////}}}
//


//{{{
void stdin_cb(ev::io& w, int revent)
{
	(void) revent;

	std::string buf;
	std::getline(std::cin, buf);
	//std::cin>>buf;

	std::cout<<"|"<<buf<<"|"<<std::endl;

	(*static_cast<ev::Socket*>(w.data))<<buf;
}
//}}}


//{{{ Help and version messages

std::string help_message(std::string progname)
{
	std::string message;
	message += "Usage: "+progname+" [commands]\nOptions:\n";
	//message += "	-h|--help           : Show this help and exit.\n";
	message += "	-v|--version        : Show version information and exit.\n";
	message += "	-s|--socket  <path> : Where to store the socket for user communication. Default: \"" + DOODLE_SOCKET_PATH + "\".\n";
	message += "Commands:\n";

	return message;
}

void version_message()
{
	std::cout<<DOODLE_PROGRAM_NAME<<":"<<std::endl;
	std::cout<<"Git branch: "<<GIT_BRANCH<<", git commit hash: "<<GIT_COMMIT_HASH<<", Version: "<<DOODLE_VERSION_MAJOR<<":"<<DOODLE_VERSION_MINOR<<"."<<std::endl<<std::endl;
}
//}}}

int main(int argc, char* argv[])
{
	//{{{ Argument handling
	GetOpt::GetOpt_pp ops(argc, argv);

	ops.exceptions(std::ios::failbit|std::ios::eofbit);
	try
	{
		//ops>>GetOpt::OptionPresent('h', "help",       args.show_help);
		//ops>>GetOpt::OptionPresent('v', "version",    args.show_version);

		//ops>>GetOpt::OptionPresent('c', "config",     args.config_set);
		//ops>>GetOpt::OptionPresent('d', "data",       args.data_set);
		ops>>GetOpt::OptionPresent('s', "socket",     args.socket_set);
		//ops>>GetOpt::Option('c',        "config",     settings.config_path, DOODLE_CONFIG_PATH);
		//ops>>GetOpt::Option('d',        "data",       settings.data_path, DOODLE_CONFIG_PATH);
		ops>>GetOpt::Option('s',        "socket",     settings.socket_path, DOODLE_SOCKET_PATH);
	}
	catch(GetOpt::GetOptEx ex)
	{
		std::cerr<<"Error in arguments"<<std::endl;

		std::cerr<<help_message(argv[0])<<std::endl;
		return -1;
	}
	if( args.show_help )
	{
		std::cout<<help_message(argv[0])<<std::endl;
		return 0;
	}
	if( args.show_version )
	{
		version_message();
		return 0;
	}
	//}}}


	settings.socket_path.append(1, '\0');

	ev::default_loop loop;
	ev::Socket socket_watcher(settings.socket_path, loop);


	//{{{ Create a libev io watcher to respond to terminal input

	ev::io stdin_watcher(loop);
	stdin_watcher.set<stdin_cb>(static_cast<void*>(&socket_watcher));
	stdin_watcher.set(STDIN_FILENO, ev::READ);
	stdin_watcher.start();
	//}}}

	loop.run();

	return 0;
}

