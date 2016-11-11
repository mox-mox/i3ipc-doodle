#pragma once

#include <string>
#include "ipc_socket.hpp"
extern "C" {
#include <histedit.h>
}
#include <json/json.h>



class Terminal
{
	int count;
	const char* line;

	EditLine* el;
	HistEvent hist_ev;
	History *myhistory;

	std::string parse_command(std::string entry);
	std::string parse_response(std::string response);


	#define CLASSNAME Terminal
	#include "commands.hpp"
	#undef CLASSNAME







	public:
		Terminal(const char* (*prompt)(EditLine* e));
		Terminal& operator>>(IPC_socket& sock);

		friend Terminal& operator<<(Terminal& lhs, IPC_socket& rhs)
		{
			std::cout<<lhs.parse_response(rhs.receive());
			return lhs;
		}

};


