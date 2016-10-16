#pragma once

#include "cli.hpp"
//#include <ev++.h>
//#include <deque>
#include "doodle_config.hpp"
#include "sockets.hpp"

//{{{
struct IPC_socket
{
	//ev::io read_watcher;
	//ev::io write_watcher;
	//std::deque < std::string > write_data;
	int fd;
	struct
	{
		char doodleversion[sizeof(DOODLE_PROTOCOL_VERSION)-1];
		uint16_t length;
	}  __attribute__((packed)) header;

	//IPC_socket(std::string& socket_path, ev::loop_ref loop);
	IPC_socket(std::string& socket_path);

	//void read_cb(ev::io& watcher, int revents);
	//void write_cb(ev::io& w, int);


	//{{{
	IPC_socket& operator<<(std::string data)
	{
		//uint16_t length = data.length();
		//std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
		//credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);


		//write_n(fd, &credential[0], credential.length());

		header.length=data.length();
		write_n(fd, (static_cast<char*>(static_cast<void*>(&header))), sizeof(header));
		write_n(fd, &data[0], data.length());



		//lhs.write_data.push_back(credential);
		//lhs.write_data.push_back(data);
		//if(!lhs.write_watcher.is_active()) lhs.write_watcher.start();

		return *this;
	}
	//}}}

//
//	//{{{
//	//struct foobar {const char* string; int length;};
//	//IPC_socket& operator<<(foobar data)
//	IPC_socket& operator<<(Input data)
//	{
//		header.length=data.count;
//		write_n(fd, (static_cast<char*>(static_cast<void*>(&header))), sizeof(header));
//		write_n(fd, data.line, data.count);
//
//
//
//		//lhs.write_data.push_back(credential);
//		//lhs.write_data.push_back(data);
//		//if(!lhs.write_watcher.is_active()) lhs.write_watcher.start();
//
//		return *this;
//	}
//	//}}}
//
	//{{{
	IPC_socket& operator>>(std::string& buffer)
	{
		//struct
		//{
		//	char doodleversion[sizeof(DOODLE_PROTOCOL_VERSION)-1];
		//	uint16_t length;
		//}  __attribute__((packed)) header;

		if( read_n(fd, (static_cast < char* > (static_cast < void* > (&header))), sizeof(header)))
		{
			std::cout<<"Cannot receive data from socket"<<std::endl;
			exit(EXIT_FAILURE);
			//watcher.stop();
			//watcher.loop.break_loop(ev::ALL);
			return *this;
		}

		//std::string buffer(header.length, '\0');
		buffer.resize(header.length);
		//if( read_n(watcher.fd, &buffer[0], header.length, watcher)) { return; }
		if( read_n(fd, &buffer[0], header.length))
		{
			std::cout<<"Cannot receive data from socket"<<std::endl;
			exit(EXIT_FAILURE);
			//watcher.stop();
			//watcher.loop.break_loop(ev::ALL);
			return *this;
		}

		////////////////////////////////////////
		//std::cout<<"	Received: |"<<buffer<<"|"<<std::endl<<std::endl;			// Do something with the received data
		////////////////////////////////////////


		return *this;
	}
	//}}}

};
//}}}
