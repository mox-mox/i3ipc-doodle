#include "commands.hpp"
#include <string>
#include <sstream>
//#include <unistd.h>

//{{{
std::string parse_command(std::string entry)
{
	std::stringstream ss(entry);
	std::string token;

	ss>>token;
	std::string cmd = "{\"cmd\":\""+token+"\",\"args\":[";

	bool first = true;
	token = "";
	while(ss>>token)
	{
		if( !first )
		{
			cmd += ',';
		}
		first = false;
		cmd += "\"";
		cmd += token;
		cmd += "\"";
		token = "";
		//usleep(100000);
	}
	cmd += "]}";
	return cmd;
}
//}}}
