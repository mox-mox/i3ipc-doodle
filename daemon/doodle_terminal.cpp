//#include "doodle.hpp"
//#include "parse_date.hpp"
//
//
//Doodle::Terminal::Terminal(Doodle* doodle) : doodle(doodle) {}
//
////{{{
//std::string Doodle::Terminal::operator()(std::string command_line_input)
//{
//	Json::Value command;
//	Json::Reader reader;
//
//	if( !reader.parse(command_line_input, command, false))
//	{
//		error<<reader.getFormattedErrorMessages()<<std::endl;
//		return "{\"response\":\"Malformed command\"}";
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
//		Json::Value (Terminal::* command_handler)(Json::Value) = commands.at(cmd).func;
//		return (this->*command_handler)(args);
//	}
//	catch (std::out_of_range)
//	{
//		return "{\"response\":\"Unknown command\"}";
//	}
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::suspend(Json::Value args)
//{
//	if(args == "help") return "Suspend operation until resume is called. Called when computer goes to sleep, or for a coffe break ;)";
//	if(args == "args") return "none";
//
//	logger<<"Suspending"<<std::endl;
//	logger<<"Args: "<<args.toStyledString()<<std::endl;
//	doodle->suspended = true;
//	doodle->current_job->stop(std::chrono::steady_clock::now());
//
//	return "{\"command\":\"suspend\",\"response\":\"suspended successfully\"}";
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::resume(Json::Value args)
//{
//	if(args == "help") return "Resume suspended operation.";
//	if(args == "args") return "none";
//
//	logger<<"Resuming"<<std::endl;
//	doodle->suspended = false;
//	doodle->current_job->start(std::chrono::steady_clock::now(), doodle->current_workspace, doodle->current_window_name);
//
//	return "{\"command\":\"resume\",\"response\":\"resumed successfully\"}";
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::get_config_path(Json::Value args)
//{
//	if(args == "help") return "Get the path to the currently used config file.";
//	if(args == "args") return "none";
//
//	std::string config_dir=settings.config_dir;
//	return config_dir;
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::get_times_path(Json::Value args)
//{
//	if(args == "help") return "Get the path to the currently used time files.";
//	if(args == "args") return "none";
//
//	std::string data_dir=settings.data_dir;
//	return data_dir;
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::list_jobs(Json::Value args)
//{
//	if(args == "help") return "List the names of all known jobs with their current total times";
//	if(args == "args") return "none";
//
//	Json::Value retval;
//	retval["command"] = "list_jobs";
//	Json::Value response;
//	for(auto& job : doodle->jobs)
//	{
//			response[response.size()][0] = job.get_jobname();
//			response[response.size()-1][1] = job.get_total_time();
//	}
//	retval["response"]=response;
//	return retval;
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::get_times(Json::Value args)
//{
//	if(args == "help") return "Get the active times for a job. If start and and are provided, only times in that interval are shown.";
//	if(args == "args") return "jobname, [start, end]";
//
//	Json::Value retval;
//	retval["command"] = "get_times";
//	std::cout<<"=======================args: "<<args<<std::endl;
//	std::string jobname = args[0].asString();
//	if(!jobname.empty())
//	{
//		std::_Deque_iterator<Job, Job&, Job*> job = std::find_if(std::begin(doodle->jobs), std::end(doodle->jobs), [&](const Job& job) -> bool { return job.get_jobname() == jobname; } );
//		if(job != std::end(doodle->jobs))
//		{
//			std::cout<<"found job: "<<*job<<std::endl;
//			retval["response"] = job->get_times(args[1].asUInt64(), args[2].asUInt64());
//		}
//		else
//		{
//			retval["response"] = "No job named \""+jobname+"\" found";
//		}
//	}
//	else
//	{
//		retval["response"] = "No job specified";
//	}
//	return retval;
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::get_win_names(Json::Value args)
//{
//	if(args == "help") return "List all window names or regular expressions for a job.";
//	if(args == "args") return "jobname";
//
//	Json::Value retval;
//	retval["command"] = "get_win_names";
//	std::cout<<"=======================args: "<<args<<std::endl;
//	std::string jobname = args[0].asString();
//	if(!jobname.empty())
//	{
//		std::_Deque_iterator<Job, Job&, Job*> job = std::find_if(std::begin(doodle->jobs), std::end(doodle->jobs), [&](const Job& job) -> bool { return job.get_jobname() == jobname; } );
//		if(job != std::end(doodle->jobs))
//		{
//			std::cout<<"found job: "<<*job<<std::endl;
//			retval["response"] = job->get_win_names();
//		}
//		else
//		{
//			retval["response"] = "No job named \""+jobname+"\" found";
//		}
//	}
//	else
//	{
//		retval["response"] = "No job specified";
//	}
//	return retval;
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::get_ws_names(Json::Value args)
//{
//	if(args == "help") return "List all workspace names or regular expressions for a job.";
//	if(args == "args") return "jobname";
//
//	Json::Value retval;
//	retval["command"] = "get_win_names";
//	std::cout<<"=======================args: "<<args<<std::endl;
//	std::string jobname = args[0].asString();
//	if(!jobname.empty())
//	{
//		std::_Deque_iterator<Job, Job&, Job*> job = std::find_if(std::begin(doodle->jobs), std::end(doodle->jobs), [&](const Job& job) -> bool { return job.get_jobname() == jobname; } );
//		if(job != std::end(doodle->jobs))
//		{
//			std::cout<<"found job: "<<*job<<std::endl;
//			retval["response"] = job->get_ws_names();
//		}
//		else
//		{
//			retval["response"] = "No job named \""+jobname+"\" found";
//		}
//	}
//	else
//	{
//		retval["response"] = "No job specified";
//	}
//	return retval;
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::detect_idle(Json::Value args)
//{
//	if(args == "help") return "Set whether to watch for idle time. If set to true, uses value set in config file.";
//	if(args == "args") return "true|false|time";
//
//	(void) args;
//	return "TODO";
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::detect_ambiguity(Json::Value args)
//{
//	if(args == "help") return "Whether to check for ambiguous matching rules. Costs a bit of performance.";
//	if(args == "args") return "true|false";
//
//	(void) args;
//	return "TODO";
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::restart(Json::Value args)
//{
//	if(args == "help") return "Restart the program to re-read the configuration.";
//	if(args == "args") return "none";
//
//	fork_to_restart = true;
//	doodle->loop.break_loop(ev::ALL);
//	return "{\"command\":\"restart\",\"response\":\"Restarting\"}";
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::kill(Json::Value args)
//{
//	if(args == "help") return "Stop the program.";
//	if(args == "args") return "none";
//
//	logger<<"Shutting down..."<<std::endl;
//	doodle->loop.break_loop(ev::ALL);
//
//	return "{\"command\":\"kill\",\"response\":\"Apoptosis started\"}";
//
//}
////}}}
//
////{{{
//Json::Value Doodle::Terminal::help(Json::Value args)
//{
//	if(args == "help") return "Show this help.";
//	if(args == "args") return "none";
//
//	Json::Value retval;
//	retval["command"] = "help";
//	Json::Value response;
//	for(auto& command : commands)
//	{
//			response[response.size()][0] = command.first;
//			response[response.size()-1][1] = (this->*command.second.func)("args");
//			response[response.size()-1][2] = (this->*command.second.func)("help");
//	}
//	retval["response"]=response;
//	return retval;
//}
////}}}
//
