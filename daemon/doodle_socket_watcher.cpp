#include "doodle.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "console_stream.hpp"




	//{{{
Doodle::Socket_watcher::Socket_watcher(ev::loop_ref loop, Doodle* doodle) : ev::io(loop), doodle(doodle), head(nullptr)
{
	debug<<"Doodle::Socket_watcher::Socket_watcher() at "<<this<<std::endl;
}
//}}}

	//{{{
void Doodle::Socket_watcher::init(std::string& sock_path)
{
	socket_path=sock_path;
	debug<<"void Doodle::Socket_watcher::init() at "<<this<<std::endl;
	debug<<"Socket path: "<<socket_path<<std::endl;

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
	set<Socket_watcher, reinterpret_cast<void (Socket_watcher::*)(ev::io& socket_watcher, int revents)>( &Socket_watcher::socket_watcher_cb) >(this);
	//start();
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
	close(fd);
	unlink(&socket_path[0]);
}
//}}}

void Doodle::Socket_watcher::socket_watcher_cb(Socket_watcher& w, int)
{
	debug<<"void Doodle::Socket_watcher::socket_watcher_cb(Socket_watcher& w at "<<&w<<") at "<<this<<std::endl;
	new Client_watcher(fd, &head, doodle, loop);
}
