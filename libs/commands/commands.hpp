// This file is nested inside doodle_terminal.hpp and command.hpp. This file may only be included _in_the_body_of_a_class_!


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


//typedef Json::Value (CLASSNAME::* func)(Json::Value);
//const std::map<std::string, func> commands
//{
//	{"suspend",          &CLASSNAME::suspend},
//	{"resume",           &CLASSNAME::resume},
//
//	{"get_config_path",  &CLASSNAME::get_config_path},
//	{"get_times_path",   &CLASSNAME::get_times_path},
//
//	{"list_jobs",        &CLASSNAME::list_jobs},
//	{"get_times",        &CLASSNAME::get_times},
//	{"get_win_names",    &CLASSNAME::get_win_names},
//	{"get_ws_names",     &CLASSNAME::get_ws_names},
//	{"detect_idle",      &CLASSNAME::detect_idle},
//	{"detect_ambiguity", &CLASSNAME::detect_ambiguity},
//	{"restart",          &CLASSNAME::restart},
//	{"kill",             &CLASSNAME::kill},
//	{"help",             &CLASSNAME::help},
//};






struct command_t
{
	Json::Value (CLASSNAME::* func)(Json::Value);
	std::string args;
};
const std::map<std::string, command_t> commands
{
	{"suspend",          {&CLASSNAME::suspend,          ""}},
	{"resume",           {&CLASSNAME::resume,           ""}},
	{"get_config_path",  {&CLASSNAME::get_config_path,  ""}},
	{"get_times_path",   {&CLASSNAME::get_times_path,   ""}},
	{"list_jobs",        {&CLASSNAME::list_jobs,        ""}},
	{"get_times",        {&CLASSNAME::get_times,        "<jobname>, [<start>, <end>]"}},
	{"get_win_names",    {&CLASSNAME::get_win_names,    "<jobname>"}},
	{"get_ws_names",     {&CLASSNAME::get_ws_names,     "<jobname>"}},
	{"detect_idle",      {&CLASSNAME::detect_idle,      "true|fale|time"}},
	{"detect_ambiguity", {&CLASSNAME::detect_ambiguity, "true|false"}},
	{"restart",          {&CLASSNAME::restart,          ""}},
	{"kill",             {&CLASSNAME::kill,             ""}},
	{"help",             {&CLASSNAME::help,             ""}},
};
