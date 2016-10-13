#include "doodle.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include "sockets.hpp"





//{{{
Doodle::Socket_watcher::Socket_watcher(ev::loop_ref loop, Doodle* doodle, std::string& socket_path) : ev::io(loop), doodle(doodle), head(nullptr), socket_path(socket_path)
{
	debug<<"Doodle::Socket_watcher::Socket_watcher() at "<<this<<": Socket path = "<<socket_path<<'.'<<std::endl;

	//fd=open_server_socket(socket_path);

	while((fd=open_server_socket(socket_path))<=0)
	{
		if(errno == EADDRINUSE)
		{
			if(args.replace)
			{
				debug<<"Other doodle daemon detected -> Try to kill it."<<std::endl;
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

	set(fd, ev::READ);
	set<Socket_watcher, reinterpret_cast<void (Socket_watcher::*)(ev::io& socket_watcher, int revents)>( &Socket_watcher::socket_watcher_cb) >(this);
}
//}}}

//{{{
void Doodle::Socket_watcher::kill_other_daemon(void)
{
	int fd = open_client_socket(socket_path);

	std::string cmd = "{\"cmd\":\"kill\",\"args\":[]}";

	uint16_t length = cmd.length();
	std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
	credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);

	std::string killmsg = credential + cmd;

	write_n(fd, &killmsg[0], killmsg.length());

	//killmsg.resize(1024);
	//read(fd, &killmsg[0], killmsg.length());

	close(fd);
}
//}}}



//{{{
Doodle::Socket_watcher::~Socket_watcher(void)
{
	debug<<"Doodle::Socket_watcher::~Socket_watcher() at "<<this<<": closing fd = "<<fd<<'.'<<std::endl;

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

//{{{
void Doodle::Socket_watcher::socket_watcher_cb(Socket_watcher& w, int)
{
	debug<<"void Doodle::Socket_watcher::socket_watcher_cb(Socket_watcher& w at "<<&w<<") at "<<this<<std::endl;
	new Client_watcher(fd, &head, doodle, loop);
}
//}}}
