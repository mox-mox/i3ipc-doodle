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
	fork_to_restart = true;
	doodle->loop.break_loop(ev::ALL);
	return "{\"command\":\"restart\",\"response\":\"Restarting\"}";
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

