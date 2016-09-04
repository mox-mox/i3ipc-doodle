#include "doodle.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>




////{{{
//Doodle::Socket_watcher::Socket_watcher(ev::loop_ref loop, Doodle* doodle) : ev::io(loop), doodle(doodle), head(nullptr)
//{
//	debug<<"Doodle::Socket_watcher::Socket_watcher() at "<<this<<std::endl;
//}
////}}}

//{{{
//void Doodle::Socket_watcher::init(std::string& sock_path)
Doodle::Socket_watcher::Socket_watcher(ev::loop_ref loop, Doodle* doodle, std::string& socket_path) : ev::io(loop), doodle(doodle), head(nullptr), socket_path(socket_path)
{
	//socket_path=sock_path;
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

	socket_path.copy(addr.sun_path, socket_path.length());

	unlink(&socket_path[0]);



	////////////////////
	while(bind(fd, static_cast<struct sockaddr*>(static_cast<void*>(&addr)), socket_path.length()+1) == -1 )
	{
		std::cout<<"TRYING TO BIND fd "<<fd<<"to "<<socket_path<<std::endl;
		switch(errno)
		{
			case EINVAL: //The socket is already bound to an address. || addrlen is wrong, or addr is not a valid address for this socket's domain.
				error<<"The socket is already bound to an address, or address is invalid: "<<socket_path<<'.'<<std::endl;
				throw std::runtime_error("The socket is already bound to an address, or address is invalid: " + socket_path + '.');
				break;
			case EBADF:         //sockfd is not a valid file descriptor.
			case ENOTSOCK:      //The file descriptor sockfd does not refer to a socket.
			case EFAULT:        //addr points outside the user's accessible address space.
			case ENOMEM:        //Insufficient kernel memory was available.
				error<<"The socket is invalid or kernel error: "<<socket_path<<'.'<<std::endl;
				throw std::runtime_error("The socket is invalid or kernel error: " + socket_path + '.');
				break;

			case ELOOP:         //Too many symbolic links were encountered in resolving addr.
			case EROFS:         //The socket inode would reside on a read-only filesystem.
			case ENOENT:        // component in the directory prefix of the socket pathname does not exist.
			//case EACCES:        //the address is protected, and the user is not the superuser.
			case EACCES:        //each permission is denied on a component of the path prefix.  (See also path_resolution(7).)
			case ENOTDIR:       // component of the path prefix is not a directory.
			case ENAMETOOLONG:  //ddr is too long.
			case EADDRNOTAVAIL: // nonexistent interface was requested or the requested address was not local.
				error<<"The socket path is invalid file sytem is not writable: "<<socket_path<<'.'<<std::endl;
				throw std::runtime_error("The socket path is invalid file sytem is not writable: " + socket_path + '.');
				break;
			case EADDRINUSE: //The given address is already in use.
				if(args.restart)
				{
					kill();
//
//					//{{{ Kill the old process
//
//					int client_socket_fd;
//					if((client_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
//					{
//						throw std::runtime_error("Could not create a Unix socket.");
//					}
//
//					struct sockaddr_un addr;
//					addr.sun_family = AF_UNIX;
//
//					// Unix sockets beginning with a null character map to the invisible unix socket space.
//					// Since Strings that begin with a null character a difficult to handle, use @ instead
//					// and translate @ to the null character here.
//					if(socket_path[0] == '@') socket_path[0] = '\0';
//
//
//
//					if(socket_path.length() >= sizeof(addr.sun_path)-1)
//					{
//						throw std::runtime_error("Unix socket path \"" + socket_path + "\" is too long. "
//						                         "Maximum allowed size is " + std::to_string(sizeof(addr.sun_path)) + "." );
//					}
//
//					socket_path.copy(addr.sun_path, socket_path.length());
//
//
//					if( connect(client_socket_fd, static_cast<struct sockaddr*>(static_cast<void*>(&addr)), socket_path.length()+1) == -1 )
//					{
//						throw std::runtime_error("Could not connect to socket "+socket_path+".");
//					}
//
//					//const char* killmsg = "{\"cmd\":\"kill\",\"args\":[]}";
//
//
//					std::string cmd = "{\"cmd\":\"kill\",\"args\":[]}";
//
//					uint16_t length = cmd.length();
//					std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
//					credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);
//
//					std::string killmsg = credential + cmd;
//
//					int write_count = 0;
//					//while(write_count < static_cast<int>(sizeof(killmsg)))
//					while(write_count < static_cast<int>(killmsg.length()))
//					{
//						int n;
//						switch((n=write(client_socket_fd, &killmsg[write_count], killmsg.length()-write_count)))
//						{
//							case -1:
//								throw std::runtime_error("Write error on the connection using fd." + std::to_string(client_socket_fd) + ".");
//							case  0:
//								std::cout<<"Received EOF (Client has closed the connection)."<<std::endl;
//								throw std::runtime_error("Write error on the connection using fd." + std::to_string(client_socket_fd) + ".");
//								return;
//							default:
//								write_count+=n;
//						}
//					}
//
//					//killmsg.resize(1024);
//					//read(client_socket_fd, &killmsg[0], killmsg.length());
//
//					close(client_socket_fd);
//					//}}}
//
					usleep(1000000);
					continue;
				}
				else
				{
					error<<"Doodle daemon seems to be running. If you are sure it is not runnung, remove the socket file: "<<socket_path<<'.'<<std::endl;
					exit(EXIT_FAILURE);
				}
				break;
		}
	}
	////////////////////



	if( listen(fd, 5) == -1 )
	{
		throw std::runtime_error("Could not listen() to socket " + socket_path + ".");
	}
	set<Socket_watcher, reinterpret_cast<void (Socket_watcher::*)(ev::io& socket_watcher, int revents)>( &Socket_watcher::socket_watcher_cb) >(this);
	//start();
}
//}}}

//{{{
void Doodle::Socket_watcher::kill(void)
{
	//{{{ Kill the old process

	int client_socket_fd;
	if((client_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
	{
		throw std::runtime_error("Could not create a Unix socket.");
	}

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


	if( connect(client_socket_fd, static_cast<struct sockaddr*>(static_cast<void*>(&addr)), socket_path.length()+1) == -1 )
	{
		throw std::runtime_error("Could not connect to socket "+socket_path+".");
	}

	//const char* killmsg = "{\"cmd\":\"kill\",\"args\":[]}";


	std::string cmd = "{\"cmd\":\"kill\",\"args\":[]}";

	uint16_t length = cmd.length();
	std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
	credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);

	std::string killmsg = credential + cmd;

	int write_count = 0;
	//while(write_count < static_cast<int>(sizeof(killmsg)))
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
	//}}}
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

	std::cout<<"++++++++++++++++++++++++++++++++++++++++++++++ close fd "<<fd<<" and unlink +++++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
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
