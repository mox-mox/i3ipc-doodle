#include "commands.hpp"
#include <string>
#include <sstream>




Command::Command(void)
{
	el = el_init(DOODLE_PROGRAM_NAME, stdin, stdout, stderr);
	el_set(el, EL_PROMPT, &prompt);
	el_set(el, EL_EDITOR, "vi");

	/* Initialize the history */
	myhistory = history_init();
	if (myhistory == 0)
	{
		fprintf(stderr, "history could not be initialized\n");
		return 1;
	}

	/* Set the size of the history */
	history(myhistory, &hist_ev, H_SETSIZE, 800);

	/* This sets up the call back functions for history functionality */
	el_set(el, EL_HIST, history, myhistory);
}











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
	}
	cmd += "]}";
	return cmd;
}
//}}}





//{{{
Command& Command::operator>>(IPC_socket& sock)
{
	line = el_gets(el, &count);
	std::cout<<"line = "<<line<<std::endl;

	if (count > 0)
	{
		history(myhistory, &hist_ev, H_ENTER, line);

	}
	else
	{
		std::cout<<"Invalid entry."<<std::endl;
	}




	return *this;
}
//}}}
