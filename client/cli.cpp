#include "cli.hpp"
#include <sstream>
#include <unistd.h>

std::string socket_path = DOODLE_SOCKET_PATH_DEFAULT;

//{{{ Extend an IO watcher with a wrtie-queue

namespace ev
{
	struct socket : ev::io
	{
		using ev::io::io;
		socket(int fd, int events, ev::loop_ref loop) : ev::io(loop)
		{
			if( -1 == fd ) throw std::runtime_error("Received invalid Unix socket");
			ev::io::set(fd, events);
		}
		socket(int fd, int events)
		{
			if( -1 == fd ) throw std::runtime_error("Received invalid Unix socket");
			ev::io::set(fd, events);
		}

		std::deque<std::string> write_data;

		//{{{ Template Magic to allow using callbacks with "void socket_watcher_cb(ev::socket& socket_watcher, int revents);"

		//{{{
		template < class K, void (K::* method)(ev::socket& w, int) >
		static void method_thunk(struct ev_loop* loop, ev_watcher* w, int revents)
		{
			(void) loop;
			(static_cast < K*> (w->data)->*method)(*reinterpret_cast<ev::socket*>(w), revents);
		}

		template < class K, void (K::* method)(ev::socket& w, int) >
		void set(K* object) throw()
		{
			this->data = (void*) object;
			this->cb = reinterpret_cast<void (*)(struct ev_loop*, struct ev_io*, int)>(&method_thunk < K, method >);
		}
		//}}}

		//{{{
		template < void (* function)(ev::socket& w, int) >
		static void function_thunk(struct ev_loop* loop, ev_watcher* w, int revents)
		{
			(void) loop;
			function(*reinterpret_cast<ev::socket*>(w), revents);
		}

		template < void (* function)(ev::socket& w, int) >
		void set(void* data = 0) throw()
		{
			this->data = data;
			this->cb = reinterpret_cast<void (*)(struct ev_loop*, struct ev_io*, int)>(&function_thunk < function >);
		}
		//}}}

		//}}}


	friend socket& operator<<(socket& lhs, std::string& data)
	{

		std::stringstream ss(data); std::string token;

		ss >> token;
		std::string cmd = "{\"cmd\":\"" + token + "\",\"args\":[";

		//for(bool first=true, token = ""; ss>>token; first=false)
		bool first = true;
		token = "";
		while(ss >> token)
		{
			if(!first)
			{
				cmd += ',';
			}
			first = false;
			cmd += "\"";
			cmd += token;
			cmd += "\"";
		token = "";
			usleep(100000);
		}
		cmd += "]}";

		uint16_t length = cmd.length();
		std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
		credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);

		std::cout<<"Writing credential: "<<credential<<std::endl;
		lhs.write_data.push_back(credential);
		std::cout<<"Writing cmd: "<<cmd<<std::endl;
		lhs.write_data.push_back(cmd);
		if(!lhs.is_active()) lhs.start();
		std::cout<<"...done"<<std::endl;

		return lhs;
	}

	};

}
//}}}

//{{{
void stdin_cb(ev::io& w, int revent)
{
	(void) revent;

	std::string buf;
	std::getline(std::cin, buf);
	//std::cin>>buf;

	std::cout<<"|"<<buf<<"|"<<std::endl;

	(*static_cast<ev::socket*>(w.data))<<buf;
}
//}}}

//{{{
void write_n(int fd, char buffer[], int size)	// Write exactly size bytes
{
	int write_count = 0;
	while(write_count < size)
	{
		int n;
		switch((n=write(fd, &buffer[write_count], size-write_count)))
		{
			case -1:
				throw std::runtime_error("Write error on the connection using fd." + std::to_string(fd) + ".");
			case  0:
				std::cout<<"Received EOF (Client has closed the connection)."<<std::endl;
				throw std::runtime_error("Write error on the connection using fd." + std::to_string(fd) + ".");
				return;
			default:
				write_count+=n;
		}
	}
}
//}}}

//{{{
void socket_write_cb(ev::socket& w, int revent)
{
	(void) revent;
	if(w.write_data.empty())
	{
		w.stop();
	}
	else
	{
		write_n(w.fd, &w.write_data.front()[0], w.write_data.front().length());
		w.write_data.pop_front();
	}
}
//}}}

//{{{
bool read_n(int fd, char buffer[], int size, ev::socket& watcher)	// Read exactly size bytes
{
	int read_count = 0;
	while(read_count < size)
	{
		int n;
		switch((n=read(fd, &buffer[read_count], size-read_count)))
		{
			case -1:
				throw std::runtime_error("Read error on the connection using fd." + std::to_string(fd) + ".");
			case  0:
				std::cout<<"Received EOF (Client has closed the connection)."<<std::endl;
				watcher.stop();
				watcher.loop.break_loop(ev::ALL);
				return true;
			default:
				read_count+=n;
		}
	}
	return false;
}
//}}}

//{{{
void socket_read_cb(ev::socket& watcher, int revents)
{
	(void) revents;
	struct
	{
		char doodleversion[sizeof(DOODLE_PROTOCOL_VERSION)-1];
		uint16_t length;
	}  __attribute__ ((packed)) header;

	if(read_n(watcher.fd, static_cast<char*>(static_cast<void*>(&header)), sizeof(header), watcher))
	{
		return;
	}

	std::string buffer(header.length, '\0');
	if(read_n(watcher.fd, &buffer[0], header.length, watcher)) { return; }

	////////////////////////////////////////
	std::cout<<"	Received: |"<<buffer<<"|"<<std::endl;	// Do something with the received data
	////////////////////////////////////////
}
//}}}

//{{{ Help and version messages

std::string help_message(std::string progname)
{
	std::string message;
	message += "Usage: "+progname+" [commands]\nOptions:\n";
	//message += "	-h|--help           : Show this help and exit.\n";
	message += "	-v|--version        : Show version information and exit.\n";
	message += "	-s|--socket  <path> : Where to store the socket for user communication. Default: \"" + DOODLE_SOCKET_PATH_DEFAULT + "\".\n";
	message += "Commands:\n";
	
	return message;
}

void version_message()
{
	std::cout<<DOODLE_PROGRAM_NAME<<":"<<std::endl;
	std::cout<<"Git branch: "<<GIT_BRANCH<<", git commit hash: "<<GIT_COMMIT_HASH<<", Version: "<<DOODLE_VERSION_MAJOR<<":"<<DOODLE_VERSION_MINOR<<"."<<std::endl<<std::endl;
}
//}}}

int main(void)
{

	ev::default_loop loop;

	//{{{ Standard Unix socket creation

	ev::socket socket_watcher_write(socket(AF_UNIX, SOCK_STREAM, 0), ev::WRITE, loop);
	ev::socket socket_watcher_read(socket_watcher_write.fd, ev::READ, loop);

	socket_watcher_write.set<socket_write_cb>(nullptr);
	socket_watcher_read.set<socket_read_cb>(nullptr);

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;

	// Unix sockets beginning with a null character map to the invisible unix socket space.
	// Since Strings that begin with a null character a difficult to handle, use @ instead
	// and translate @ to the null character here.
	if(socket_path[0] == '@') socket_path[0] = '\0';



	if(socket_path.length() >= sizeof(addr.sun_path)-1)
	{
		throw std::runtime_error("Unix socket path \"" + socket_path + "\" is too long. "
		                         "Maximum allowed size is " + std::to_string(sizeof(addr.sun_path)) + "." );
	}

	socket_path.copy(addr.sun_path, socket_path.length());


	if( connect(socket_watcher_write.fd, static_cast<struct sockaddr*>(static_cast<void*>(&addr)), socket_path.length()+1) == -1 )
	{
		throw std::runtime_error("Could not connect to socket "+socket_path+".");
	}

	socket_watcher_write.start();
	socket_watcher_read.start();

	//}}}

	//{{{ Create a libev io watcher to respond to terminal input

	ev::io stdin_watcher(loop);
	stdin_watcher.set<stdin_cb>(static_cast<void*>(&socket_watcher_write));
	stdin_watcher.set(STDIN_FILENO, ev::READ);
	stdin_watcher.start();
	//}}}

	loop.run();

	return 0;
}

