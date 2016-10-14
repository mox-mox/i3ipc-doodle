#pragma once

#include <ev++.h>
#include "sockets.hpp"

//{{{ Extend an IO watcher with a wrtie-queue

namespace ev
{
	struct Socket
	{
		ev::io read_watcher;
		ev::io write_watcher;
		std::deque < std::string > write_data;

		//{{{
		void read_cb(ev::io& watcher, int revents)
		{
			(void) revents;
			struct
			{
				char doodleversion[sizeof(DOODLE_PROTOCOL_VERSION)-1];
				uint16_t length;
			}  __attribute__((packed)) header;

			//if( read_n(watcher.fd, (static_cast < char* > (static_cast < void* > (&header))), sizeof(header), watcher))
			if( read_n(watcher.fd, (static_cast < char* > (static_cast < void* > (&header))), sizeof(header)))
			{
				watcher.stop();
				watcher.loop.break_loop(ev::ALL);
				return;
			}

			std::string buffer(header.length, '\0');
			//if( read_n(watcher.fd, &buffer[0], header.length, watcher)) { return; }
			if(read_n(watcher.fd, &buffer[0], header.length))
			{
				watcher.stop();
				watcher.loop.break_loop(ev::ALL);
				return;
			}

			////////////////////////////////////////
			std::cout<<"	Received: |"<<buffer<<"|"<<std::endl;	// Do something with the received data
			////////////////////////////////////////
		}
		//}}}

		//{{{
		void write_cb(ev::io& w, int revent)
		{
			(void) revent;
			if(write_data.empty())
			{
				w.stop();
			}
			else
			{
				write_n(w.fd, &write_data.front()[0], write_data.front().length());
				write_data.pop_front();
			}
		}
		//}}}

		//{{{
		Socket(std::string& socket_path, ev::loop_ref loop) : read_watcher(loop), write_watcher(loop)
		{
			int fd = open_client_socket(socket_path);

			read_watcher.set(fd, ev::READ);
			read_watcher.set<Socket, &Socket::read_cb>(this);

			write_watcher.set(fd, ev::WRITE);
			write_watcher.set<Socket, &Socket::write_cb>(this);

			write_watcher.start();
			read_watcher.start();
		}
		//}}}

		//{{{
		friend Socket& operator<<(Socket&lhs, std::string data)
		{
			uint16_t length = data.length();
			std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
			credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);

			std::cout<<"Writing credential: "<<credential<<std::endl;
			lhs.write_data.push_back(credential);
			std::cout<<"Writing cmd: "<<data<<std::endl;
			lhs.write_data.push_back(data);
			if( !lhs.write_watcher.is_active()) lhs.write_watcher.start();
			std::cout<<"...done"<<std::endl;

			return lhs;
		}
		//}}}
	};
}
//}}}
