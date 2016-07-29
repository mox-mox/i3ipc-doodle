#include "doodle.hpp"
#include <fstream>
#include <experimental/filesystem>
#include "logstream.hpp"
#include <functional>
#include <json/json.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <map>


//{{{
struct Doodle::terminal
{
	Doodle& doodle;
	terminal(Doodle& doodle);
	Json::Value operator()(Json::Value command_line_input);
	std::string suspend(Json::Value);
	std::string resume(Json::Value);
	//std::string list_jobs(Json::Value);
	//std::string get_times(Json::Value args);
	//std::string get_win_names(Json::Value args);
	//std::string get_ws_names(Json::Value args);
	//std::string detect_idle(Json::Value args);
	//std::string detect_ambiguity(Json::Value args);
	//std::string restart(Json::Value);
	//std::string kill(Json::Value);
	//std::string help(Json::Value);




	struct command_t
	{
		std::string (terminal::* func)(Json::Value);
		std::string args;
		std::string description;
	};
	std::map < std::string, command_t > commands
	{
		{"suspend",          {&terminal::suspend,          "none",                  "Suspend operation until resume is called. Called when computer goes to sleep, or for a coffe break ;)"}},
		{"resume",           {&terminal::resume,           "none",                  "Resume suspended operation."}},
		//{"list_jobs",        {&terminal::list_jobs,        "none",                  "List the names of all known jobs with their current total times"}},
		//{"get_times",        {&terminal::get_times,        "jobname, [start, end]", "Get the active times for a job. If start and and are provided, only times in that interval are shown."}},
		//{"get_win_names",    {&terminal::get_win_names,    "jobname",               "List all window names or regular expressions for a job."}},
		//{"get_ws_names",     {&terminal::get_ws_names,     "jobname",               "List all workspace names or regular expressions for a job."}},
		//{"detect_idle",      {&terminal::detect_idle,      "true|fale|time",        "Set whether to watch for idle time. If set to true, uses value set in config file."}},
		//{"detect_ambiguity", {&terminal::detect_ambiguity, "true|false",            "Whether to check for ambiguous matching rules. Costs a bit of performance."}},
		//{"restart",          {&terminal::restart,          "none",                  "Restart the program to re-read the configuration."}},
		//{"kill",             {&terminal::kill,             "none",                  "Stop the program."}},
		//{"help",             {&terminal::help,             "none",                  "Show this help."}},
	};




};
//}}}

