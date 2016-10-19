// This file is nested inside doodle_terminal.hpp and command.hpp. This file may only be included _in_the_body_of_a_class_!

typedef Json::Value (CLASSNAME::* func)(Json::Value);
const std::map<std::string, func> commands
{
	{"suspend",          &CLASSNAME::suspend},
	{"resume",           &CLASSNAME::resume},

	{"get_config_path",  &CLASSNAME::get_config_path},
	{"get_times_path",   &CLASSNAME::get_times_path},

	{"list_jobs",        &CLASSNAME::list_jobs},
	{"get_times",        &CLASSNAME::get_times},
	{"get_win_names",    &CLASSNAME::get_win_names},
	{"get_ws_names",     &CLASSNAME::get_ws_names},
	{"detect_idle",      &CLASSNAME::detect_idle},
	{"detect_ambiguity", &CLASSNAME::detect_ambiguity},
	{"restart",          &CLASSNAME::restart},
	{"kill",             &CLASSNAME::kill},
	{"help",             &CLASSNAME::help},
};
