#include "doodle.hpp"


Doodle::terminal_t::terminal_t(Doodle* doodle) : doodle(doodle) {}

std::string Doodle::terminal_t::operator()(std::string command_line_input)
{
	Json::Value command;
	Json::Reader reader;

	if( !reader.parse(command_line_input, command, false))
	{
		error<<reader.getFormattedErrorMessages()<<std::endl;
		return "{\"response\":\"Malformed command\"}";
	}
	else
	{
		if( command.isMember("cmd"))
		{
			try{
				std::string cmd = command.get("cmd", "no cmd").asString();
				Json::Value (terminal_t::* command_handler)(Json::Value) = commands.at(cmd).func;

				return (this->*command_handler)(command["args"]).toStyledString();
			}
			catch (std::out_of_range)
			{
				return "{\"response\":\"Unknown command\"}";
			}
		}
		return "{\"response\":\"No command specified\"}";
	}
}

Json::Value Doodle::terminal_t::suspend(Json::Value args)
{
	logger<<"Suspending"<<std::endl;
	logger<<"Args: "<<args.toStyledString()<<std::endl;
	doodle->suspended = true;
	doodle->current_job->stop(std::chrono::steady_clock::now());

	return "{\"command\":\"suspend\",\"response\":\"suspended successfully\"}";
}
Json::Value Doodle::terminal_t::resume(Json::Value)
{
	logger<<"Resuming"<<std::endl;
	doodle->suspended = false;
	doodle->current_job->start(std::chrono::steady_clock::now());

	return "{\"command\":\"resume\",\"response\":\"resumed successfully\"}";
}

Json::Value Doodle::terminal_t::list_jobs(Json::Value)
{
	return "";
}
Json::Value Doodle::terminal_t::get_times(Json::Value args)
{
	(void) args;
	return "";
}
Json::Value Doodle::terminal_t::get_win_names(Json::Value args)
{
	(void) args;
	return "";
}
Json::Value Doodle::terminal_t::get_ws_names(Json::Value args)
{
	(void) args;
	return "";
}
Json::Value Doodle::terminal_t::detect_idle(Json::Value args)
{
	(void) args;
	return "";
}
Json::Value Doodle::terminal_t::detect_ambiguity(Json::Value args)
{
	(void) args;
	return "";
}
Json::Value Doodle::terminal_t::restart(Json::Value)
{
	logger<<"<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>"<<std::endl;
	char program_path[1024];
	ssize_t len = ::readlink("/proc/self/exe", program_path, sizeof(program_path)-1);
	if (len != -1)
	{
		program_path[len] = '\0';
	}
	else
	{
		error<<"Cannot get own path. => cannot restart."<<std::endl;
		return "{\"response\":\"Cannot restart\"}";
	}

	pid_t pid;

	switch ((pid = fork()))
	{
		case -1: /* Error */
			std::cerr << "Uh-Oh! fork() failed.\n";
			exit(1);
		case 0: /* Child process */
			execl(program_path, std::experimental::filesystem::path(program_path).filename().c_str(), "-r", nofork?"-n":"", "-c", doodle->config_path.c_str(), "-s", socket_path.c_str(),  static_cast<char*>(nullptr)); /* Execute the program */
			std::cerr << "Uh-Oh! execl() failed!"; /* execl doesn't return unless there's an error */
			exit(1);
		default: /* Parent process */
			debug << "Process created with pid " << pid << "\n";
			return "{\"response\":\"Restarting\"}";
	}
}
Json::Value Doodle::terminal_t::kill(Json::Value)
{
	logger<<"Shutting down..."<<std::endl;
	doodle->loop.break_loop(ev::ALL);

	return "{\"command\":\"kill\",\"response\":\"Apoptosis started\"}";

}
Json::Value Doodle::terminal_t::help(Json::Value)
{
	Json::Value retval;
	retval["command"] = "help";
	for(auto& command : commands)
	{
		Json::Value entry;
		entry[command.first].append(command.second.args);
		entry[command.first].append(command.second.description);
		retval["response"].append(entry);
	}
	return retval;




}

