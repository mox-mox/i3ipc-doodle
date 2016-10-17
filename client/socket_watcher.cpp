#include "socket_watcher.hpp"
#include "sockets.hpp"


//{{{
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
}
//}}}

//{{{
IPC_socket& IPC_socket::operator<<(std::string data)
{
	header.length=data.length();
	write_n(fd, (static_cast<char*>(static_cast<void*>(&header))), sizeof(header));
	write_n(fd, &data[0], data.length());

	return *this;
}
//}}}

//{{{
IPC_socket& IPC_socket::operator>>(std::string& buffer)
{
	if( read_n(fd, (static_cast < char* > (static_cast < void* > (&header))), sizeof(header)))
	{
		std::cout<<"Received EOF (Server has closed the connection)."<<std::endl;
		exit(EXIT_SUCCESS);
		return *this;
	}

	buffer.resize(header.length);
	if( read_n(fd, &buffer[0], header.length))
	{
		std::cout<<"Received EOF (Server has closed the connection)."<<std::endl;
		exit(EXIT_SUCCESS);
		return *this;
	}

	return *this;
}
//}}}

//{{{
IPC_socket& IPC_socket::operator>>(std::ostream& outstream)
{
	if( read_n(fd, (static_cast < char* > (static_cast < void* > (&header))), sizeof(header)))
	{
		std::cout<<"Received EOF (Server has closed the connection)."<<std::endl;
		exit(EXIT_SUCCESS);
		return *this;
	}

	std::string buffer(header.length, '\0');
	if( read_n(fd, &buffer[0], header.length))
	{
		std::cout<<"Received EOF (Server has closed the connection)."<<std::endl;
		exit(EXIT_SUCCESS);
		return *this;
	}

	outstream<<buffer;
	return *this;
}
//}}}
