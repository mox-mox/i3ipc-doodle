#include "socket_watcher.hpp"


//{{{
//IPC_socket::IPC_socket(std::string& socket_path, ev::loop_ref loop) : read_watcher(loop), write_watcher(loop)
IPC_socket::IPC_socket(std::string& socket_path)
{
	//int fd;
	try
	{
		fd = open_client_socket(socket_path);
	} catch(const std::length_error& e){
		std::cerr<<e.what()<<std::endl;
		exit(EXIT_FAILURE);
	} catch(const std::runtime_error& e){
		std::cerr<<e.what()<<std::endl;
		exit(EXIT_FAILURE);
	}

	//read_watcher.set(fd, ev::READ);
	//read_watcher.set<IPC_socket, &IPC_socket::read_cb>(this);

	//write_watcher.set(fd, ev::WRITE);
	//write_watcher.set<IPC_socket, &IPC_socket::write_cb>(this);

	//write_watcher.start();
	//read_watcher.start();
}
//}}}





//
////{{{
//void IPC_socket::read_cb(ev::io& watcher, int revents)
//{
//	(void) revents;
//	struct
//	{
//		char doodleversion[sizeof(DOODLE_PROTOCOL_VERSION)-1];
//		uint16_t length;
//	}  __attribute__((packed)) header;
//
//	if( read_n(watcher.fd, (static_cast < char* > (static_cast < void* > (&header))), sizeof(header)))
//	{
//		watcher.stop();
//		watcher.loop.break_loop(ev::ALL);
//		return;
//	}
//
//	std::string buffer(header.length, '\0');
//	//if( read_n(watcher.fd, &buffer[0], header.length, watcher)) { return; }
//	if( read_n(watcher.fd, &buffer[0], header.length))
//	{
//		watcher.stop();
//		watcher.loop.break_loop(ev::ALL);
//		return;
//	}
//
//	////////////////////////////////////////
//	std::cout<<"	Received: |"<<buffer<<"|"<<std::endl<<std::endl;			// Do something with the received data
//	////////////////////////////////////////
//}
////}}}
//
//
////{{{
//void IPC_socket::write_cb(ev::io& w, int)
//{
//	if( write_data.empty())
//	{
//		w.stop();
//	}
//	else
//	{
//		write_n(w.fd, &write_data.front()[0], write_data.front().length());
//		write_data.pop_front();
//	}
//}
////}}}
//
