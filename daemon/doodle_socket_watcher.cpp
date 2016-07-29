#include "doodle.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "logstream.hpp"




	//{{{
Doodle::Socket_watcher::Socket_watcher(ev::loop_ref loop, Doodle* doodle, std::string socket_path) : ev::io(loop), doodle(doodle), head(nullptr)
{
	std::cout<<"Doodle::Socket_watcher::Socket_watcher() at "<<this<<std::endl;
	std::cout<<"Socket path: "<<socket_path<<std::endl;

	int fd;
	if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
	{
		throw std::runtime_error("Could not create a Unix socket.");
	}

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

	if( bind(fd, static_cast<struct sockaddr*>(static_cast<void*>(&addr)), socket_path.length()+1) == -1 )
	{
		throw std::runtime_error("Could not bind to socket " + socket_path + ".");
	}

	if( listen(fd, 5) == -1 )
	{
		throw std::runtime_error("Could not listen() to socket " + socket_path + ".");
	}
	//set<Socket_watcher, &Socket_watcher::socket_watcher_cb>(this);
	set<Socket_watcher, reinterpret_cast<void (Socket_watcher::*)(ev::io& socket_watcher, int revents)>( &Socket_watcher::socket_watcher_cb) >(this);
	//start();
}
//}}}

	//{{{
Doodle::Socket_watcher::Socket_watcher() : doodle(nullptr)
{
	std::cout<<"Doodle::Socket_watcher::Socket_watcher() at "<<this<<std::endl;
}
//}}}

//{{{
Doodle::Socket_watcher::~Socket_watcher(void)
{
	std::cout<<"deleting the socket_watcher"<<std::endl;
	close(fd);
}
//}}}

void Doodle::Socket_watcher::socket_watcher_cb(Socket_watcher& socket_watcher, int revents)
{
	(void) revents;
	//new Client_watcher(fd, head, this, socket_watcher.loop);
	new Client_watcher(fd, head, doodle, loop);
}
















//void Doodle::Socket_watcher::operator()(int events)
//{
//	std::cout<<"Doodle::Socket_watcher::operator()() called."<<std::endl;
//}
