#pragma once

#include <ev++.h>
#include <deque>
#include "doodle_config.hpp"
#include "sockets.hpp"

//{{{

struct Socket_watcher
{
	ev::io read_watcher;
	ev::io write_watcher;
	std::deque < std::string > write_data;

	Socket_watcher(std::string& socket_path, ev::loop_ref loop);

	void read_cb(ev::io& watcher, int revents);
	void write_cb(ev::io& w, int);

	//{{{
	friend Socket_watcher& operator<<(Socket_watcher& lhs, std::string data)
	{
		uint16_t length = data.length();
		std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
		credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);

		lhs.write_data.push_back(credential);
		lhs.write_data.push_back(data);
		if(!lhs.write_watcher.is_active()) lhs.write_watcher.start();

		return lhs;
	}
	//}}}
};
//}}}
