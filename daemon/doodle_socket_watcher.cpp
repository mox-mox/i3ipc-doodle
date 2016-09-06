#include "doodle.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>





//{{{
Doodle::Socket_watcher::Socket_watcher(ev::loop_ref loop, Doodle* doodle, std::string& socket_path) : ev::io(loop), doodle(doodle), head(nullptr), socket_path(socket_path)
{
	debug<<"void Doodle::Socket_watcher::init() at "<<this<<std::endl;
	debug<<"Socket path: "<<socket_path<<std::endl;

	int fd;
	if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
	{
		throw std::runtime_error("Could not create a Unix socket.");
	}
	std::cout<<"....................FD: "<<fd<<std::endl;

	set(fd, ev::READ);
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;


	if(socket_path.length() >= sizeof(addr.sun_path)-1)
	{
		throw std::runtime_error("Unix socket path \"" + socket_path + "\" is too long. "
		                         "Maximum allowed size is " + std::to_string(sizeof(addr.sun_path)) + "." );
	}
	//for(unsigned int i=0; i<sizeof(addr.sun_path); i++)
	//{
	//	addr.sun_path[i] = 0;
	//}

	socket_path.copy(addr.sun_path, socket_path.length()); addr.sun_path[socket_path.length()] = '\0';
	std::cout<<"addr.sun_path: |";
	for(unsigned int i=0; i<50; i++)
	{
		std::cout<<"  "<<addr.sun_path[i]<<'|';
	}
	std::cout<<"."<<std::endl;
	std::cout<<"addr.sun_path: |";
	for(unsigned int i=0; i<50; i++)
	{
		std::cout<<std::setw(3)<<static_cast<unsigned int>(addr.sun_path[i])<<'|';
	}
	std::cout<<"."<<std::endl;

  	// Unix sockets beginning with a null character map to the invisible unix socket space.
  	// Since Strings that begin with a null character a difficult to handle, use % instead
  	// and translate % to the null character here.
	if(addr.sun_path[0] == '%') addr.sun_path[0] = '\0';

	//unlink(&socket_path[0]);  !!! DO NOT ATTEMPT TO REMOVE OLD SOCKETS HERE or the detection of other daemons gets broken!!!

	while(bind(fd, static_cast<struct sockaddr*>(static_cast<void*>(&addr)), sizeof(addr.sun_path)) == -1 )
	{
		if(errno == EADDRINUSE)
		{
			if(args.restart)
			{
				kill_other_daemon();
				usleep(1000000);
				continue;
			}
			else
			{
				error<<"Doodle daemon seems to be running. If you are sure it is not runnung, remove the socket file: "<<socket_path<<'.'<<std::endl;
				exit(EXIT_FAILURE);
			}
		}
		error<<"Cannot create socket in \"" + socket_path + "\" because of a system error. Please report this as a bug."<<std::endl;
		throw std::runtime_error("Cannot create socket in \"" + socket_path + "\" because of a system error. Please report this as a bug.");
	}



	if( listen(fd, 5) == -1 )
	{
		throw std::runtime_error("Could not listen() to socket " + socket_path + ".");
	}
	set<Socket_watcher, reinterpret_cast<void (Socket_watcher::*)(ev::io& socket_watcher, int revents)>( &Socket_watcher::socket_watcher_cb) >(this);
}
//}}}

//{{{
void Doodle::Socket_watcher::kill_other_daemon(void)
{
	//{{{ Create a temporary socket to send the kill command to the other client

	int client_socket_fd;
	if((client_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
	{
		throw std::runtime_error("Could not create a Unix socket.");
	}

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;


	if(socket_path.length() >= sizeof(addr.sun_path)-1)
	{
		throw std::runtime_error("Unix socket path \"" + socket_path + "\" is too long. "
				"Maximum allowed size is " + std::to_string(sizeof(addr.sun_path)) + "." );
	}

	socket_path.copy(addr.sun_path, socket_path.length()); addr.sun_path[socket_path.length()] = '\0';
	std::cout<<"addr.sun_path: |";
	for(unsigned int i=0; i<50; i++)
	{
		std::cout<<"  "<<addr.sun_path[i]<<'|';
	}
	std::cout<<"."<<std::endl;
	std::cout<<"addr.sun_path: |";
	for(unsigned int i=0; i<50; i++)
	{
		std::cout<<std::setw(3)<<static_cast<unsigned int>(addr.sun_path[i])<<'|';
	}
	std::cout<<"."<<std::endl;

	// Unix sockets beginning with a null character map to the invisible unix socket space.
	// Since Strings that begin with a null character a difficult to handle, use % instead
	// and translate % to the null character here.
	if(addr.sun_path[0] == '%') addr.sun_path[0] = '\0';


	if( connect(client_socket_fd, static_cast<struct sockaddr*>(static_cast<void*>(&addr)), sizeof(addr.sun_path)) == -1 )
	{
		throw std::runtime_error("Could not connect to socket "+socket_path+". Error: "+std::strerror(errno));
	}
	//}}}


	std::string cmd = "{\"cmd\":\"kill\",\"args\":[]}";

	uint16_t length = cmd.length();
	std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
	credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);

	std::string killmsg = credential + cmd;

	int write_count = 0;
	while(write_count < static_cast<int>(killmsg.length()))
	{
		int n;
		switch((n=write(client_socket_fd, &killmsg[write_count], killmsg.length()-write_count)))
		{
			case -1:
				throw std::runtime_error("Write error on the connection using fd." + std::to_string(client_socket_fd) + ".");
			case  0:
				std::cout<<"Received EOF (Client has closed the connection)."<<std::endl;
				throw std::runtime_error("Write error on the connection using fd." + std::to_string(client_socket_fd) + ".");
				return;
			default:
				write_count+=n;
		}
	}

	//killmsg.resize(1024);
	//read(client_socket_fd, &killmsg[0], killmsg.length());

	close(client_socket_fd);
}
//}}}

//{{{
Doodle::Socket_watcher::~Socket_watcher(void)
{
	debug<<"Doodle::Socket_watcher::~Socket_watcher() at "<<this<<std::endl;

	if(head)
	{
		Client_watcher* w = head->next;

		while(w && w != head)
		{
			Client_watcher* current = w;
			w = w->next;
			delete current;
		}
		delete head;
	}

	std::cout<<"++++++++++++++++++++++++++++++++++++++++++++++ close fd "<<fd<<" and unlink "<<socket_path<<" +++++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
	close(fd);
	unlink(&socket_path[0]);
}
//}}}

//{{{
void Doodle::Socket_watcher::socket_watcher_cb(Socket_watcher& w, int)
{
	debug<<"void Doodle::Socket_watcher::socket_watcher_cb(Socket_watcher& w at "<<&w<<") at "<<this<<std::endl;
	new Client_watcher(fd, &head, doodle, loop);
}
//}}}
