#pragma once
// This class is a nested class of doodle. This file may only be included _in_the_body_of_class_Doodle_!


class terminal_t
{

	Doodle* const doodle;

	#define CLASSNAME terminal_t
	#include "commands.hpp"
	#undef CLASSNAME

	Json::Value run_cmd(std::string, Json::Value);

	public:
	terminal_t(Doodle* doodle);

	std::string operator()(std::string command_line_input);

};
