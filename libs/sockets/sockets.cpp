#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include "console_stream.hpp"

structset-prooset-prop Sock
{
	int fd;
	struct sockaddr_un addr;
};



static void open_socket(std::string& socket_path, Sock& sock)
{
	if((sock.fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
	{
		throw std::runtime_error("Could not create a Unix socket.");
	}
	//std::cout<<"....................FD: "<<sock.fd<<std::endl;

	sock.addr.sun_family = AF_UNIX;


	if(socket_path.length() >= sizeof(sock.addr.sun_path)-1)
	{
		throw std::runtime_error("Unix socket path \"" + socket_path + "\" is too long. "
		                         "Maximum allowed size is " + std::to_string(sizeof(sock.addr.sun_path)) + "." );
	}

	socket_path.copy(sock.addr.sun_path, socket_path.length());

  	// Unix sockets beginning with a null character map to the invisible unix socket space.
  	// Since Strings that begin with a null character a difficult to handle, use % instead
  	// and translate % to the null character here.
	if(sock.addr.sun_path[0] == '%') sock.addr.sun_path[0] = '\0';


	std::cout<<"sock.addr.sun_path: |";
	for(unsigned int i=0; i<50; i++)
	{
		std::cout<<"  "<<sock.addr.sun_path[i]<<'|';
	}
	std::cout<<"."<<std::endl;

	std::cout<<"sock.addr.sun_path: |";
	for(unsigned int i=0; i<50; i++)
	{
		std::cout<<std::setw(3)<<static_cast<unsigned int>(sock.addr.sun_path[i])<<'|';
	}
	std::cout<<"."<<std::endl;
	std::cout<<"sock.fd = "<<sock.fd<<std::endl;
}




//{{{
int open_server_socket(std::string& socket_path)
{
	Sock sock;
	open_socket(socket_path, sock);
	std::cout<<"sock.fd = "<<sock.fd<<std::endl;

	//unlink(&socket_path[0]);  !!! DO NOT ATTEMPT TO REMOVE OLD SOCKETS HERE or the detection of other daemons gets broken!!!

	while(bind(sock.fd, static_cast<struct sockaddr*>(static_cast<void*>(&sock.addr)), sizeof(sock.addr.sun_family)+socket_path.length()) == -1 )
	{
		if(errno == EADDRINUSE)
		{
			//if(args.replace)
			//{
			//	//kill_other_daemon();
			//	usleep(1000000);
			//	continue;
			//}
			//else
			{
				error<<"Doodle daemon seems to be running. If you are sure it is not runnung, remove the socket file: "<<socket_path<<'.'<<std::endl;
				exit(EXIT_FAILURE);
			}
		}
		error<<"Cannot create socket in \"" + socket_path + "\" because of a system error. Please report this as a bug."<<std::endl;
		throw std::runtime_error("Cannot create socket in \"" + socket_path + "\" because of a system error. Please report this as a bug.");
	}



	if( listen(sock.fd, 5) == -1 )
	{
		throw std::runtime_error("Could not listen() to socket " + socket_path + ".");
	}
	return sock.fd;
}
//}}}






//{{{
int open_client_socket(std::string& socket_path)
{
	Sock sock;
	open_socket(socket_path, sock);

	if( connect(sock.fd, static_cast<struct sockaddr*>(static_cast<void*>(&sock.addr)), sizeof(sock.addr.sun_family)+socket_path.length()) == -1 )
	{
		throw std::runtime_error("Could not connect to socket "+socket_path+".");
	}
	return sock.fd;
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
//bool read_n(int fd, char buffer[], int size, ev::socket& watcher)	// Read exactly size bytes
bool read_n(int fd, char buffer[], int size)	// Read exactly size bytes
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
				//watcher.stop();
				//watcher.loop.break_loop(ev::ALL);
				return true;
			default:
				read_count+=n;
		}
	}
	return false;
}
//}}}















