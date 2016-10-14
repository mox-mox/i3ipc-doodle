#pragma once
// This class is a nested class of doodle. This file may only be included _in_the_body_of_class_Doodle_!


struct terminal_t
{
	Doodle* const doodle;
	terminal_t(Doodle* doodle);

	std::string operator()(std::string command_line_input);

	//{{{ The terminal functions

	Json::Value suspend(Json::Value);
	Json::Value resume(Json::Value);

	Json::Value get_config_path(Json::Value);
	Json::Value get_times_path(Json::Value);



	Json::Value list_jobs(Json::Value);
	Json::Value get_times(Json::Value args);
	Json::Value get_win_names(Json::Value args);
	Json::Value get_ws_names(Json::Value args);
	Json::Value detect_idle(Json::Value args);
	Json::Value detect_ambiguity(Json::Value args);
	Json::Value restart(Json::Value);
	Json::Value kill(Json::Value);
	Json::Value help(Json::Value);
	//}}}


	struct command_t
	{
		Json::Value (terminal_t::* func)(Json::Value);
		std::string args;
		std::string description;
	};
	const std::map<std::string, command_t> commands
	{
		{"suspend",          {&terminal_t::suspend,          "none",                  "Suspend operation until resume is called. Called when computer goes to sleep, or for a coffe break ;)"}},
		{"resume",           {&terminal_t::resume,           "none",                  "Resume suspended operation."}},

		{"get_config_path",  {&terminal_t::get_config_path,  "none",                  "Get the path to the currently used config file."}},
		{"get_times_path",   {&terminal_t::get_times_path,   "none",                  "Get the path to the currently used time files."}},

		{"list_jobs",        {&terminal_t::list_jobs,        "none",                  "List the names of all known jobs with their current total times"}},
		{"get_times",        {&terminal_t::get_times,        "jobname, [start, end]", "Get the active times for a job. If start and and are provided, only times in that interval are shown."}},
		{"get_win_names",    {&terminal_t::get_win_names,    "jobname",               "List all window names or regular expressions for a job."}},
		{"get_ws_names",     {&terminal_t::get_ws_names,     "jobname",               "List all workspace names or regular expressions for a job."}},
		{"detect_idle",      {&terminal_t::detect_idle,      "true|fale|time",        "Set whether to watch for idle time. If set to true, uses value set in config file."}},
		{"detect_ambiguity", {&terminal_t::detect_ambiguity, "true|false",            "Whether to check for ambiguous matching rules. Costs a bit of performance."}},
		{"restart",          {&terminal_t::restart,          "none",                  "Restart the program to re-read the configuration."}},
		{"kill",             {&terminal_t::kill,             "none",                  "Stop the program."}},
		{"help",             {&terminal_t::help,             "none",                  "Show this help."}},
	};
};
