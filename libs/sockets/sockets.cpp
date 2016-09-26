#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>



int open_server_socket(std::string& socket_path)
{
	int fd;
	if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
	{
		throw std::runtime_error("Could not create a Unix socket.");
	}
	std::cout<<"....................FD: "<<fd<<std::endl;

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;


	if(socket_path.length() >= sizeof(addr.sun_path)-1)
	{
		throw std::runtime_error("Unix socket path \"" + socket_path + "\" is too long. "
		                         "Maximum allowed size is " + std::to_string(sizeof(addr.sun_path)) + "." );
	}

	socket_path.copy(addr.sun_path, socket_path.length());

  	// Unix sockets beginning with a null character map to the invisible unix socket space.
  	// Since Strings that begin with a null character a difficult to handle, use % instead
  	// and translate % to the null character here.
	if(addr.sun_path[0] == '%') addr.sun_path[0] = '\0';


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






	//unlink(&socket_path[0]);  !!! DO NOT ATTEMPT TO REMOVE OLD SOCKETS HERE or the detection of other daemons gets broken!!!

	while(bind(fd, static_cast<struct sockaddr*>(static_cast<void*>(&addr)), sizeof(addr.sun_family)+socket_path.length()) == -1 )
	{
		if(errno == EADDRINUSE)
		{
			if(args.replace)
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
}
