#pragma once
// This class is a nested class of doodle. This file may only be included _in_the_body_of_class_Doodle_!

struct Socket_watcher : ev::io
{
	Doodle* const doodle;
	Client_watcher* head;
	std::string doodle_socket_path;

	//{{{ Constructors

	Socket_watcher(ev::loop_ref loop, Doodle* doodle, std::string& doodle_socket_path);
	~Socket_watcher(void);
	Socket_watcher(void) = delete;
	Socket_watcher(const Socket_watcher&) = delete;
	Socket_watcher(Socket_watcher&&) = delete;
	Socket_watcher& operator=(const Socket_watcher&) = delete;
	Socket_watcher& operator=(Socket_watcher&&) = delete;
	//}}}

	void kill_other_daemon(void);
	void socket_watcher_cb(Socket_watcher& socket_watcher, int revents);
};
