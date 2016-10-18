#include "doodle.hpp"
#include "parse_date.hpp"


Doodle::terminal_t::terminal_t(Doodle* doodle) : doodle(doodle) {}


//{{{
Json::Value Doodle::terminal_t::run_cmd(std::string cmd, Json::Value args)
{
	try{
		Json::Value (terminal_t::* command_handler)(Json::Value) = commands.at(cmd).func;
		return (this->*command_handler)(args);
	}
	catch (std::out_of_range)
	{
		return "{\"response\":\"Unknown command\"}";
	}
}
//}}}


//{{{
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
			Json::FastWriter fastWriter;
			return fastWriter.write(run_cmd(command.get("cmd", "no cmd").asString(), command["args"]));
		}
		return "{\"response\":\"No command specified\"}";
	}
}
//}}}

//{{{
Json::Value Doodle::terminal_t::suspend(Json::Value args)
{
	logger<<"Suspending"<<std::endl;
	logger<<"Args: "<<args.toStyledString()<<std::endl;
	doodle->suspended = true;
	doodle->current_job->stop(std::chrono::steady_clock::now());

	return "{\"command\":\"suspend\",\"response\":\"suspended successfully\"}";
}
//}}}

//{{{
Json::Value Doodle::terminal_t::resume(Json::Value)
{
	logger<<"Resuming"<<std::endl;
	doodle->suspended = false;
	doodle->current_job->start(std::chrono::steady_clock::now());

	return "{\"command\":\"resume\",\"response\":\"resumed successfully\"}";
}
//}}}

//{{{
Json::Value Doodle::terminal_t::get_config_path(Json::Value)
{
	return "";
}
//}}}

//{{{
Json::Value Doodle::terminal_t::get_times_path(Json::Value)
{
	return "";
}
//}}}

//{{{
Json::Value Doodle::terminal_t::list_jobs(Json::Value)
{
	Json::Value retval;
	retval["command"] = "list_jobs";
	Json::Value response;
	for(auto& job : doodle->jobs)
	{
			response[response.size()][0] = job.get_jobname();
			response[response.size()-1][1] = job.get_total_time();
	}
	retval["response"]=response;
	return retval;
}
//}}}

//{{{
Json::Value Doodle::terminal_t::get_times(Json::Value args)
{
	Json::Value retval;
	retval["command"] = "get_times";
	std::cout<<"=======================args: "<<args<<std::endl;
	std::string jobname = args[0].asString();
	if(!jobname.empty())
	{
		std::_Deque_iterator<Job, Job&, Job*> job = std::find_if(std::begin(doodle->jobs), std::end(doodle->jobs), [&](const Job& job) -> bool { return job.get_jobname() == jobname; } );
		//const Job& job = *std::find_if(std::begin(doodle->jobs), std::end(doodle->jobs), [&](const Job& job) -> bool { return job.get_jobname() == jobname; } );
		if(job != std::end(doodle->jobs))
		{
			std::cout<<"found job: "<<*job<<std::endl;
			retval["response"].append( job->get_times(args[1].asUInt64(), args[2].asUInt64()) );
		}
		else
		{
			retval["response"] = "No job named \""+jobname+"\" found";
		}
	}
	else
	{
		retval["response"] = "No job specified";
	}
	return retval;
}
//}}}

//{{{
Json::Value Doodle::terminal_t::get_win_names(Json::Value args)
{
	(void) args;
	return "";
}
//}}}

//{{{
Json::Value Doodle::terminal_t::get_ws_names(Json::Value args)
{
	(void) args;
	return "";
}
//}}}

//{{{
Json::Value Doodle::terminal_t::detect_idle(Json::Value args)
{
	(void) args;
	return "";
}
//}}}

//{{{
Json::Value Doodle::terminal_t::detect_ambiguity(Json::Value args)
{
	(void) args;
	return "";
}
//}}}

//{{{
Json::Value Doodle::terminal_t::restart(Json::Value)
{
	fork_to_restart = true;
	doodle->loop.break_loop(ev::ALL);
	return "{\"command\":\"restart\",\"response\":\"Restarting\"}";
}
//}}}

//{{{
Json::Value Doodle::terminal_t::kill(Json::Value)
{
	logger<<"Shutting down..."<<std::endl;
	doodle->loop.break_loop(ev::ALL);

	return "{\"command\":\"kill\",\"response\":\"Apoptosis started\"}";

}
//}}}

//{{{
Json::Value Doodle::terminal_t::help(Json::Value)
{
	Json::Value retval;
	retval["command"] = "help";
	Json::Value response;
	for(auto& command : commands)
	{
			response[response.size()][0] = command.first;
			response[response.size()-1][1] = command.second.args;
			response[response.size()-1][2] = command.second.description;
	}
	retval["response"]=response;
	return retval;
}
//}}}

