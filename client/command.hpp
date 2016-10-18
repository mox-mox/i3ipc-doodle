#pragma once

#include <string>
#include "ipc_socket.hpp"
extern "C" {
#include <histedit.h>
}



class Command
{
	int count;
	const char* line;

	EditLine* el;
	HistEvent hist_ev;
	History *myhistory;
	const char* (*prompt)(EditLine* e);

	std::string parse_command(std::string entry);
	std::string parse_response(std::string response);
	public:
		Command(const char* (*prompt)(EditLine* e));
		Command& operator>>(IPC_socket& sock);

		friend Command& operator<<(Command& lhs, IPC_socket& rhs)
		{
			std::cout<<lhs.parse_response(rhs.receive());
			return lhs;
		}

};


