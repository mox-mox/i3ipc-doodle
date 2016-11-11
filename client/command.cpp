#include "command.hpp"
#include <string>
#include <sstream>
#include <iomanip>




//{{{
Terminal::Terminal(const char* (*prompt)(EditLine* e))
{
	if(!prompt) throw std::logic_error("No prompt function defined");
	el = el_init(DOODLE_PROGRAM_NAME, stdin, stdout, stderr);
	if(!el) throw std::logic_error("no el");
	el_set(el, EL_PROMPT, prompt);
	el_set(el, EL_EDITOR, "vi");

	/* Initialize the history */
	myhistory = history_init();
	if (myhistory == nullptr)
	{
		throw std::runtime_error("History could not be initialised.");
	}

	/* Set the size of the history */
	history(myhistory, &hist_ev, H_SETSIZE, 800);

	for(auto& command : commands)
	{
		history(myhistory, &hist_ev, H_ENTER, (command.first + " " + command.second.args).c_str());
	}


	/* This sets up the call back functions for history functionality */
	el_set(el, EL_HIST, history, myhistory);
}
//}}}


//
////{{{
//std::string Terminal::parse_response(std::string entry)
//{
//	Json::Value command;
//	Json::Reader reader;
//
//	if( !reader.parse(entry, command, false))
//	{
//		error<<reader.getFormattedErrorMessages()<<std::endl;
//		return "Invalid response\n";
//	}
//	else
//	{
//		if( command.isMember("cmd"))
//		{
//			Json::FastWriter fastWriter;
//			return fastWriter.write(run_cmd(command.get("cmd", "no cmd").asString(), command["args"]));
//		}
//		return "{\"response\":\"No command specified\"}";
//	}
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::run_cmd(std::string cmd, Json::Value args)
//{
//	try{
//		Json::Value (Terminal::* command_handler)(Json::Value) = commands.at(cmd);
//		return (this->*command_handler)(args);
//	}
//	catch (std::out_of_range)
//	{
//		return "{\"response\":\"Unknown command\"}";
//	}
//}
////}}}
//








//{{{
std::string Terminal::parse_command(std::string entry)
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
std::string Terminal::parse_response(std::string entry)
{
	//for(unsigned int i=0; i<entry.length(); i++)
	//{
	//	std::cout<<"|"<<entry[i]<<"|"<<std::endl;
	//}
	std::cout<<entry<<std::endl;
	Json::Value root;
	Json::Reader reader;

	if(entry == "\"\"\n" || !reader.parse(entry, root, false))
	{
		std::cout<<reader.getFormattedErrorMessages()<<std::endl;
		return "Invalid response\n";
	}

	Json::Value response = root.get("response", "no response");

	std::stringstream retval;
	if(root.get("command", "invalid").asString() == "help")
	{
		retval<<"Available commands:\n";
		for( auto& commands_line : response)
		{
			retval<<'\t'<<std::left<<std::setw(20)<<commands_line[0].asString()<<std::left<<std::setw(25)<<commands_line[1].asString()<<commands_line[2].asString()<<'\n';
		}
	}


	return retval.str();
	//return entry;
}
//}}}



//{{{
Terminal& Terminal::operator>>(IPC_socket& sock)
{
	line = el_gets(el, &count);
	std::cout<<"line = "<<line<<std::endl;

	if (count > 0)
	{
		history(myhistory, &hist_ev, H_ENTER, line);
		sock.send(parse_command({line, static_cast<unsigned int>(count)}));
	}
	else
	{
		std::cout<<"Invalid entry."<<std::endl;
	}




	return *this;
}
//}}}




Json::Value Terminal::suspend(Json::Value) { return ""; }
Json::Value Terminal::resume(Json::Value) { return ""; }
Json::Value Terminal::get_config_path(Json::Value) { return ""; }
Json::Value Terminal::get_times_path(Json::Value) { return ""; }
Json::Value Terminal::list_jobs(Json::Value) { return ""; }
Json::Value Terminal::get_times(Json::Value) { return ""; }
Json::Value Terminal::get_win_names(Json::Value) { return ""; }
Json::Value Terminal::get_ws_names(Json::Value) { return ""; }
Json::Value Terminal::detect_idle(Json::Value) { return ""; }
Json::Value Terminal::detect_ambiguity(Json::Value) { return ""; }
Json::Value Terminal::restart(Json::Value) { return ""; }
Json::Value Terminal::kill(Json::Value) { return ""; }
Json::Value Terminal::help(Json::Value) { return ""; }


