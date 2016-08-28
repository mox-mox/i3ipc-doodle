#ifndef doodle_socket_watcher_hpp
#define doodle_socket_watcher_hpp
#pragma once
// This class is a nested class of doodle. This file may only be included _in_the_body_of_class_Doodle_!
//#warning "<<<<<<<<<<<<<<<<<<<< doodle_socket_watcher.hpp: include  doodle_config.hpp >>>>>>>>>>>>>>>>>>>>"
//#include "doodle_config.hpp"
//#warning "++++++++++++++++++++ doodle_socket_watcher.hpp: include  doodle_config.hpp ++++++++++++++++++++"
//#include <ev++.h>

//{{{
struct Socket_watcher : ev::io
{
	Doodle* const doodle;
	Client_watcher* head;
	std::string socket_path;


	Socket_watcher(ev::loop_ref loop, Doodle* doodle);
	void init(std::string& socket_path);

	//Socket_watcher(void) = delete;
	//Socket_watcher(const Socket_watcher&) = delete;
	//Socket_watcher(Socket_watcher&&) = delete;
	//Socket_watcher& operator=(const Socket_watcher&) = delete;
	//Socket_watcher& operator=(Socket_watcher&&) = delete;

	void socket_watcher_cb(Socket_watcher& socket_watcher, int revents);

	~Socket_watcher(void);
};
//}}}

#endif /* doodle_socket_watcher_hpp */
