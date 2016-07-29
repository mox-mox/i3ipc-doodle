#include "doodle.hpp"
#include "logstream.hpp"

Doodle::terminal_t::terminal_t(Doodle* doodle) : doodle(doodle)
{
	std::cout<<"terminal at "<<this<<" ++>>"<<std::endl;
	std::cout<<"<<++ doodle = "<<doodle<<std::endl;
}

std::string Doodle::terminal_t::operator()(std::string command_line_input)
{
	std::cout<<"terminal_t::operator() at"<<this<<" ++>>"<<std::endl;
	std::cout<<"<<++ doodle = "<<doodle<<std::endl;

		Json::Value cmd;
		Json::Reader reader;

		if( !reader.parse(command_line_input, cmd, false))
		{
			error<<reader.getFormattedErrorMessages()<<std::endl;
			return "{\"response\":\"invalid format\"}";
		}
		else
		{
			std::cout<<"starting the terminal evaluation"<<std::endl;
			std::cout<<"this: "<<this<<std::endl;
			//return "{\"command\":\"suspend\",\"response\":\"suspended successfully\"}";
			//return suspend(cmd).toStyledString();
			//return (this->*func)(cmd).toStyledString();
			if( cmd.isMember("command"))
			{
				try{

				return (this->*func)(cmd).toStyledString();
				}
				catch (std::out_of_range)
				{
					return "{\"response\":\"Unknown command\"}";
				}
			}
			return "{\"response\":\"No command specified\"}";

		}
}

Json::Value Doodle::terminal_t::suspend(Json::Value)
{
	std::cout<<"SUSPENDING"<<std::endl;
	logger<<"Suspending"<<std::endl;
	std::cout<<"doodle = "<<doodle<<std::endl;
	doodle->suspended = true;
	std::cout<<"doodle = "<<doodle<<std::endl;
	//doodle->current_job->stop(std::chrono::steady_clock::now());

	return "{\"command\":\"suspend\",\"response\":\"suspended successfully\"}";
}
Json::Value Doodle::terminal_t::resume(Json::Value)
{
	logger<<"Resuming"<<std::endl;
	doodle->suspended = false;
	doodle->current_job->start(std::chrono::steady_clock::now());
	return "{\"command\":\"resume\",\"response\":\"resumed successfully\"}";
}

//Json::Value Doodle::terminal_t::list_jobs(Json::Value);
//Json::Value Doodle::terminal_t::get_times(Json::Value args);
//Json::Value Doodle::terminal_t::get_win_names(Json::Value args);
//Json::Value Doodle::terminal_t::get_ws_names(Json::Value args);
//Json::Value Doodle::terminal_t::detect_idle(Json::Value args);
//Json::Value Doodle::terminal_t::detect_ambiguity(Json::Value args);
//Json::Value Doodle::terminal_t::restart(Json::Value);
//Json::Value Doodle::terminal_t::kill(Json::Value);
//Json::Value Doodle::terminal_t::help(Json::Value);

