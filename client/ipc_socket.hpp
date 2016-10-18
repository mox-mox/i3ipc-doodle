#pragma once

#include "cli.hpp"
#include "doodle_config.hpp"

class IPC_socket
{
	int fd;
	struct
	{
		char doodleversion[sizeof(DOODLE_PROTOCOL_VERSION)-1];
		uint16_t length;
	}  __attribute__((packed)) header;

	public:
		IPC_socket(std::string&socket_path);
		void send(std::string data);
		std::string receive(void);


		//IPC_socket& operator<<(std::string data);

		//IPC_socket& operator>>(std::string&buffer);
		//IPC_socket& operator>>(std::ostream&outstream);


		//std::ostream& operator<<(std::ostream& lhs, X const& rhs)
		//{
		//	  return rhs.outputToStream(lhs);
		//}
};
