#include "doodle.hpp"
#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include "logstream.hpp"
#include <functional>
#include <json/json.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <ev.h>
#include <cstring>

#include "doodle.hpp"
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <functional>
#include <cstdio>







//{{{
struct Doodle::client_watcher : ev::io
{
	client_watcher* prev;
	client_watcher* next;

	ev::io write_watcher;
	std::deque<std::string> write_data;

	//{{{
	client_watcher(int main_fd, client_watcher* next, ev::loop_ref loop) : ev::io(loop), write_watcher(loop)
	{
		if( -1 == main_fd )   throw std::runtime_error("Passed invalid Unix socket");
		int client_fd = accept(main_fd, NULL, NULL);
		if( -1 == client_fd ) throw std::runtime_error("Received invalid Unix socket");
		ev::io::set(client_fd, ev::READ);
		if(!next)
		{
			prev = next = this;
		}
		else // if next is valid
		{
			this->next = next;
			this->prev = next->prev;
			next->prev = this;
			prev->next = this;
		}

		set<client_watcher, reinterpret_cast<void (client_watcher::*)(ev::io& socket_watcher, int revents)>( &client_watcher::client_watcher_cb) >(nullptr);

		start();

		write_watcher.set < client_watcher, &client_watcher::write_cb > (this);
		write_watcher.set(client_fd, ev::WRITE);
		write_watcher.start();
	}
	//}}}

	//{{{
	~client_watcher(void)
	{
		//std::cout<<"~client_watcher()"<<std::endl;
		close(fd);
		write_watcher.stop();
		stop();
		if(this != next && this != prev)
		{
			next->prev = this->prev;
			prev->next = this->next;
		}
	}
	//}}}


	//{{{
	void write_cb(ev::io& w, int revent)
	{
		(void) revent;
		std::deque<std::string>& write_data = static_cast<client_watcher*>(w.data)->write_data;
		if(write_data.empty())
		{
			w.stop();
		}
		else while(!write_data.empty())
		{
			int write_count = 0;
			int write_size = write_data.front().length();
			while(write_count < write_size)
			{
				int n;
				switch((n=write(w.fd, &write_data.front()[write_count], write_size-write_count)))
				{
					case -1:
						throw std::runtime_error("Write error on the connection using fd." + std::to_string(w.fd) + ".");
					case  0:
						std::cout<<"Received EOF (Client has closed the connection)."<<std::endl;
						delete &w;
						//throw std::runtime_error("Write error on the connection using fd." + std::to_string(w.fd) + ".");
						return;
					default:
						write_count+=n;
				}
			}
			write_data.pop_front();
		}
	}
	//}}}

	//{{{
	friend client_watcher& operator<<(client_watcher& lhs, const std::string& data)
	{
		lhs.write_data.push_back(data);
		if(!lhs.write_watcher.is_active()) lhs.write_watcher.start();

		return lhs;
	}
	//}}}

	//{{{
	bool read_n(int fd, char buffer[], int size, client_watcher& watcher)	// Read exactly size bytes
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
					delete &watcher;
					return true;
				default:
					read_count+=n;
			}
		}
		return false;
	}
	//}}}

	//{{{
	void client_watcher_cb(client_watcher& watcher, int revents)
	{
		//std::cout<<"client_watcher_cb("<<revents<<") called for fd "<<watcher.fd<<"."<<std::endl;
		(void) revents;

		struct header_t header;

		if(read_n(watcher.fd, static_cast<char*>(static_cast<void*>(&header)), sizeof(header), watcher))
		{
			return;
		}
		std::cout<<header.doodleversion<<", length: "<<header.length<<": ";

		std::string buffer(header.length, '\0');
		if(read_n(watcher.fd, &buffer[0], header.length, watcher))
		{
			return;
		}

		std::cout<<"\""<<buffer<<"\""<<std::endl;

		////////////////////////////////////////
		watcher<<buffer;	// Do something with the received data
		////////////////////////////////////////


		if(buffer=="kill")
		{
			std::cout<<"Shutting down"<<std::endl;
			//todo: delete all stuff
			watcher.loop.break_loop(ev::ALL);
		}
	}
	//}}}
};
//}}}







//{{{
Doodle::Doodle(const std::string& config_path) : conn(), config_path(config_path), current_workspace(""), nojob(), current_job(&nojob), loop(), idle(true), connection(xcb_connect(NULL, NULL)), screen(xcb_setup_roots_iterator(xcb_get_setup(connection)).data), idle_watcher_timer(loop)
{
	//{{{ Construct all members

	std::ifstream config_file(config_path+"/doodlerc");

	Json::Value configuration_root;
	Json::Reader reader;

	if( !reader.parse(config_file, configuration_root, false))
	{
		error<<reader.getFormattedErrorMessages()<<std::endl;
	}

	//{{{ Get the configuration options

	std::cout<<"retrieving config"<<std::endl;
	Json::Value config;
	if( configuration_root.isMember("config"))
	{
		config = configuration_root.get("config", "no config");
	}
	else
	{
		config = configuration_root;
	}
	settings.max_idle_time = config.get("max_idle_time", settings.MAX_IDLE_TIME_DEFAULT_VALUE).asUInt();
	settings.detect_ambiguity = config.get("detect_ambiguity", settings.DETECT_AMBIGUITY_DEFAULT_VALUE).asBool();
	settings.socket_path = config.get("socket_path", DOODLE_SOCKET_PATH_DEFAULT).asString();
	if(settings.socket_path[0] == '@') settings.socket_path[0] = '\0';
	//}}}

	//{{{ Create the individual jobs

	for( auto&f: std::experimental::filesystem::directory_iterator(config_path+"/jobs"))
	{
		if((f.path() != config_path+"/doodlerc") && (std::string::npos == f.path().string().find("_backup")) && std::experimental::filesystem::is_regular_file(f))
		{
			try
			{
				jobs.push_back({ f.path(), loop });
			}
			catch(std::runtime_error&e)
			{
				error<<"Caught exception \""<<e.what()<<"\" while constructing job "<<f.path().filename()<<". ... removing that job from the job list."<<std::endl;
			}
		}
	}
	//}}}

	nojob.start(std::chrono::steady_clock::now());										// Account for time spent on untracked jobs
	//}}}

	//{{{ Prepare for operation

	//{{{ Idle time detection

	if( settings.max_idle_time )
	{
		//{{{ Watcher for idle time

		idle_watcher_timer.set < Doodle, &Doodle::idle_time_watcher_cb > (this);
		idle_watcher_timer.set(settings.max_idle_time, settings.max_idle_time);
		idle_watcher_timer.start();
		//}}}
	}
	else
	{
		idle = false;
	}
	//}}}

	simulate_workspace_change(conn.get_workspaces());	// Inject a fake workspace change event to start tracking the first workspace.
	//simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.


	//{{{ i3 event subscriptions

	conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));

	if( !conn.subscribe(i3ipc::ET_WORKSPACE|i3ipc::ET_WINDOW))
	{
		error<<"could not connect"<<std::endl;
		throw "Could not subscribe to the workspace- and window change events.";
	}
	//}}}

	//}}}


	//{{{ Initialise socket communication



	//{{{
	for(unsigned int i = 0; i<=settings.socket_path.length(); i++)
	{
		std::cout<<"|"<<settings.socket_path[i];
	}
	std::cout<<"|"<<std::endl;
	for(unsigned int i = 0; i<=settings.socket_path.length(); i++)
	{
		std::cout<<"|"<<static_cast<int>(settings.socket_path[i]);
	}
	std::cout<<"|"<<std::endl;
	//}}}


	std::cout<<"Socket path: "<<settings.socket_path<<std::endl;

	int fd;
	if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
	{
		throw std::runtime_error("Could not create a Unix socket.");
	}

	socket_watcher.set(fd, ev::READ);
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;

	if(settings.socket_path.length() >= sizeof(addr.sun_path)-1)
	{
		throw std::runtime_error("Unix socket path \"" + settings.socket_path + "\" is too long. "
		                         "Maximum allowed size is " + std::to_string(sizeof(addr.sun_path)) + "." );
	}

	settings.socket_path.copy(addr.sun_path, settings.socket_path.length());

	unlink(&settings.socket_path[0]);

	if( bind(fd, static_cast<struct sockaddr*>(static_cast<void*>(&addr)), settings.socket_path.length()+1) == -1 )
	{
		throw std::runtime_error("Could not bind to socket " + settings.socket_path + ".");
	}

	if( listen(fd, 5) == -1 )
	{
		throw std::runtime_error("Could not listen() to socket " + settings.socket_path + ".");
	}
	socket_watcher.set<Doodle, &Doodle::socket_watcher_cb>(nullptr);
	socket_watcher.start();
	//}}}



}
//}}}

//{{{
Doodle::~Doodle(void)
{
	xcb_disconnect(connection);

	//{{{

	client_watcher* head = static_cast<client_watcher*>(socket_watcher.data);
	if(head)
	{
		client_watcher* w = head->next;
		while(w && w != head)
		{
			client_watcher* current = w;
			w = w->next;
			delete current;
		}
		delete head;
	}
	unlink(&settings.socket_path[0]);
	//}}}
}
//}}}

//{{{
inline Doodle::win_id_lookup_entry Doodle::find_job(const std::string& window_name)
{
	win_id_lookup_entry retval {
		&nojob, ""
	};

	for( Job& j : jobs )					// Search all the jobs to see, if one matches the newly focused window
	{
		if((retval.matching_name = j.match(current_workspace, window_name)) != "" )
		{
			if( !settings.detect_ambiguity )// For normal operation, just report the first match.
			{
				retval.job = &j;
				return retval;
			}
			else							// To detect ambiguity, continue searching to see if there are other matches
			{
				if( retval.job != &nojob )
				{
					error<<"Ambiguity: Window name \""<<window_name<<"\" matched "<<retval.job->get_jobname()<<" and "<<j.get_jobname()<<"."<<std::endl;
					// TODO: Show an errow window that asks to which job the window belongs to.
				}
				else
				{
					retval.job = &j;
				}
			}
		}
	}
	return retval;
}
//}}}

//{{{
void Doodle::on_window_change(const i3ipc::window_event_t& evt)
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	//{{{ Print some information about the event

	//#ifdef DEBUG
	//	logger<<"on_window_change() called "<<++win_evt_count<<"th time. Type: "<<static_cast < char > (evt.type)<<std::endl;
	//	if( evt.container != nullptr )
	//	{
	//		//logger<<"	id = "<<evt.container->id<<std::endl;
	//		logger<<"	name = "<<evt.container->name<<std::endl;
	//		//logger<<"	type = "<<evt.container->type<<std::endl;
	//		//logger<<"	urgent = "<<evt.container->urgent<<std::endl;
	//		//logger<<"	focused = "<<evt.container->focused<<std::endl;
	//	}
	//#endif
	//}}}
	if(((evt.type == i3ipc::WindowEventType::FOCUS) || (evt.type == i3ipc::WindowEventType::TITLE)) && (evt.container != nullptr))
	{
		Job* old_job = current_job;
		win_id_lookup_entry& entry = win_id_lookup[evt.container->id];

		if( !entry.job || (entry.matching_name == "") || !std::regex_search(evt.container->name, std::regex(entry.matching_name)))
		{
			entry = find_job(evt.container->name);
		}
		current_job = entry.job;
		if( old_job != current_job )
		{
			if( old_job )
			{
				old_job->stop(now);
			}
			if( current_job )
			{
				// A window change may imply user activity, so if the user is considered idle, update that status
				if( idle ) idle_time_watcher_cb(idle_watcher_timer, 2);

				if( !idle )
				{
					current_job->start(now);
				}
			}
		}
		logger<<"New current_job: "<<current_job->get_jobname()<<std::endl;
	}
	else if( evt.type == i3ipc::WindowEventType::CLOSE )
	{
		win_id_lookup.erase(evt.container->id);
		simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
	}
}
//}}}

//{{{
void Doodle::on_workspace_change(const i3ipc::workspace_event_t& evt)
{
	if( evt.type == i3ipc::WorkspaceEventType::FOCUS )
	{
		logger<<"New current_workspace: "<<evt.current->name<<std::endl;
		current_workspace = evt.current->name;
		simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
	}
#ifdef DEBUG
	else
	{
		//logger<<"Ignoring Workspace event"<<std::endl;
	}
#endif
}
//}}}

//{{{
std::ostream& operator<<(std::ostream&stream, Doodle const&doodle)
{
	stream<<"Doodle class:"<<std::endl;
	stream<<"	Current job: "<<doodle.current_job->get_jobname()<<std::endl;
	stream<<"	Current workspace: "<<doodle.current_workspace<<std::endl;
	stream<<"	Jobs:"<<std::endl;
	for( const Job& job : doodle.jobs )
	{
		stream<<"		"<<job<<std::endl;
	}
	stream<<"	Known windows:"<<std::endl<<"		win_id		jobname		matching_name"<<std::endl;
	for( auto it : doodle.win_id_lookup )
	{
		stream<<"		"<<it.first<<"	"<<it.second.job<<"	"<<it.second.matching_name<<std::endl;
	}
	return stream;
}
//}}}

//{{{
bool Doodle::simulate_workspace_change(std::vector < std::shared_ptr < i3ipc::workspace_t>>workspaces)
{	// Iterate through all workspaces and call on_workspace_change() for the focussed one.
	for( std::shared_ptr < i3ipc::workspace_t > &workspace : workspaces )
	{
		if( workspace->focused )
		{
			on_workspace_change({ i3ipc::WorkspaceEventType::FOCUS,  workspace, nullptr });
			return true;
		}
	}
	error<<"No workspace is focused."<<std::endl;
	return false;	// Should never be reached
}
//}}}

//{{{
bool Doodle::simulate_window_change(std::list < std::shared_ptr < i3ipc::container_t>>nodes)
{	// Iterate through all containers and call on_window_change() for the focussed one.
	for( std::shared_ptr < i3ipc::container_t > &container : nodes )
	{
		if( container->focused )
		{
			on_window_change({ i3ipc::WindowEventType::FOCUS,  container });
			return true;
		}
		else
		{
			if( simulate_window_change(container->nodes)) return true;
		}
	}
	return false;	// Should never be reached
}
//}}}

//{{{Callbacks

//{{{
void Doodle::SIGUSR1_cb(void)
{
	std::cout<<"Received SIGUSR1!"<<std::endl;
	//std::cout<<*this<<std::endl;
	std::cout<<"Pending count: "<<ev_pending_count(loop)<<"."<<std::endl;
}
//}}}

//{{{
void Doodle::SIGTERM_cb(void)
{
	logger<<"Shutting down doodle"<<std::endl;
	std::cout<<"Shutting down doodle"<<std::endl;
	loop.break_loop(ev::ALL);
}
//}}}

//{{{
void Doodle::idle_time_watcher_cb(ev::timer& timer, int revents)
{
	xcb_screensaver_query_info_cookie_t cookie = xcb_screensaver_query_info(connection, screen->root);
	xcb_screensaver_query_info_reply_t* info = xcb_screensaver_query_info_reply(connection, cookie, NULL);

	uint32_t idle_time = info->ms_since_user_input/1000;// use seconds

	std::cout<<"Checking idle time("<<revents<<"): "<<idle_time<<"."<<std::endl;
	free(info);

	uint32_t repeat_value = 1;
	if( !idle )
	{
		// Restart the watcher to trigger when idle_time might reach max_idle_time for the first time.
		// See solution 2 of http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Be_smart_about_timeouts
		uint32_t repeat_value = settings.max_idle_time-idle_time;
		// If the value was allowed to become zero (because of truncation), the watcher would never be started again
		repeat_value = repeat_value ? repeat_value : 1;
	}
	timer.repeat = repeat_value;
	timer.again();

	if((idle_time >= settings.max_idle_time) && !idle )
	{
		idle = true;
		logger<<"Going idle"<<std::endl;
		current_job->stop(std::chrono::steady_clock::now());
	}
	else if((idle_time < settings.max_idle_time) && idle )
	{
		idle = false;
		logger<<"Going busy again"<<std::endl;
		current_job->start(std::chrono::steady_clock::now());
	}
}
//}}}
//}}}






//{{{
void Doodle::socket_watcher_cb(ev::io& socket_watcher, int revents)
{
	std::cout<<"socket_watcher_cb called"<<std::endl;
	(void) revents;
	client_watcher* head = static_cast<client_watcher*>(socket_watcher.data);
	head = new client_watcher(socket_watcher.fd, head, socket_watcher.loop);
	socket_watcher.data = static_cast<void*>(head);
}
//}}}



//{{{
int Doodle::operator()(void)
{
	int retval = 0;

	conn.prepare_to_event_handling();

	std::cout<<"---------------Starting the event loop---------------"<<std::endl;

	//{{{ Watcher for i3 events

	ev::io i3_watcher;
	i3_watcher.set < i3ipc::connection, &i3ipc::connection::handle_event > (&conn);
	i3_watcher.set(conn.get_file_descriptor(), ev::READ);
	i3_watcher.start();
	//}}}

	//{{{ Watchers for POSIX signals

	//{{{ Watcher for the SIGUSR1 POSIX signal

	ev::sig SIGUSR1_watcher;
	SIGUSR1_watcher.set < Doodle, &Doodle::SIGUSR1_cb > (this);
	SIGUSR1_watcher.set(SIGUSR1);
	SIGUSR1_watcher.start();
	//}}}

	//{{{ Watcher for the SIGTERM POSIX signal

	ev::sig SIGTERM_watcher;
	SIGTERM_watcher.set < Doodle, &Doodle::SIGTERM_cb > (this);
	SIGTERM_watcher.set(SIGTERM);
	SIGTERM_watcher.start();
	//}}}

	//{{{ Watcher for the SIGINT POSIX signal

	ev::sig SIGINT_watcher;
	SIGINT_watcher.set < Doodle, &Doodle::SIGTERM_cb > (this);
	SIGINT_watcher.set(SIGINT);
	SIGINT_watcher.start();
	//}}}
	//}}}

	//
	//	//{{{ Listen to a posix socket
	//
	//	int max_queue = 128;
	//	struct sock_ev_serv server;
	//	struct ev_periodic every_few_seconds;
	//	std::string sock_path = "/tmp/libev-ipc-daemon.sock";
	//
	//	unlink(sock_path);
	//
	//	// Setup a unix socket listener.
	//	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	//	if( -1 == fd )
	//	{
	//		perror("echo server socket");
	//		exit(EXIT_FAILURE);
	//	}
	//
	//	// Set it non-blocking
	//	int flags = fcntl(fd, F_GETFL);
	//	flags |= O_NONBLOCK;
	//	if( -1 == fcntl(fd, F_SETFL, flags))
	//	{
	//		perror("echo server socket nonblock");
	//		exit(EXIT_FAILURE);
	//	}
	//
	//	// Set it as unix socket
	//	server.socket.sun_family = AF_UNIX;
	//	server.socket.sun_path = sock_path
	//	//strcpy(socket_un->sun_path, sock_path);
	//
	//	server.fd = fd
	//	server.socket_len = sizeof(server->socket.sun_family)+strlen(server->socket.sun_path);
	//
	//	array_init(&(server.clients), 128);
	//
	//	if( -1 == bind(server.fd, (struct sockaddr*) &(server.socket), server.socket_len))
	//	{
	//		perror("echo server bind");
	//		exit(EXIT_FAILURE);
	//	}
	//
	//	if( -1 == listen(server.fd, max_queue))
	//	{
	//		perror("listen");
	//		exit(EXIT_FAILURE);
	//	}
	//
	//
	//
	//
	//	//}}}
	//











	loop.run();



	std::cout<<"Returning from event loop"<<std::endl;


	return retval;
}
//}}}
