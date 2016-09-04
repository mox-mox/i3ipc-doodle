#pragma once
// This class is a nested class of doodle. This file may only be included _in_the_body_of_class_Doodle_!

struct Socket_watcher : ev::io
{
	Doodle* const doodle;
	Client_watcher* head;
	std::string socket_path;


	//Socket_watcher(ev::loop_ref loop, Doodle* doodle);
	Socket_watcher(ev::loop_ref loop, Doodle* doodle, std::string& socket_path);
	//void init(std::string& socket_path);
	void kill(void);

	Socket_watcher(void) = delete;
	Socket_watcher(const Socket_watcher&) = delete;
	Socket_watcher(Socket_watcher&&) = delete;
	Socket_watcher& operator=(const Socket_watcher&) = delete;
	Socket_watcher& operator=(Socket_watcher&&) = delete;

	void socket_watcher_cb(Socket_watcher& socket_watcher, int revents);

	~Socket_watcher(void);
};
