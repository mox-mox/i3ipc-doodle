#pragma once

#include <string>
#include "socket_watcher.hpp"
extern "C" {
#include <histedit.h>
}



class Command
{
	int count;
	const char* line;

	EditLine *el;
	HistEvent hist_ev;
	History *myhistory;

	std::string parse_command(std::string entry);
	public:
		Command& operator>>(IPC_socket& sock);
		Command(void);








};







	//std::ostream& operator<<(std::ostream& lhs, X const& rhs)
	//{
	//	  return rhs.outputToStream(lhs);
	//}
