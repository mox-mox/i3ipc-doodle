#include "doodle_terminal.hpp"

Doodle::terminal::terminal(Doodle& doodle) : doodle(doodle)
{
}

//Json::Value Doodle::terminal::operator()(Json::Value command_line_input);

std::string Doodle::terminal::suspend(Json::Value)
{
	doodle.suspended = true;
	logger<<"Suspending"<<std::endl;
	doodle.current_job->stop(std::chrono::steady_clock::now());
	return "success";
}
std::string Doodle::terminal::resume(Json::Value)
{
	doodle.suspended = false;
	logger<<"Resuming"<<std::endl;
	doodle.current_job->start(std::chrono::steady_clock::now());
	return "success";
}

//std::string Doodle::terminal::list_jobs(Json::Value);
//std::string Doodle::terminal::get_times(Json::Value args);
//std::string Doodle::terminal::get_win_names(Json::Value args);
//std::string Doodle::terminal::get_ws_names(Json::Value args);
//std::string Doodle::terminal::detect_idle(Json::Value args);
//std::string Doodle::terminal::detect_ambiguity(Json::Value args);
//std::string Doodle::terminal::restart(Json::Value);
//std::string Doodle::terminal::kill(Json::Value);
//std::string Doodle::terminal::help(Json::Value);

