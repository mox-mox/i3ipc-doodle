#pragma once

#include "doodle_config.hpp"

#include "console_stream.hpp"
#include "notify_stream.hpp"
#include <ev++.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

struct Args
{
	bool show_help;
	bool show_version;
	bool nofork;
	bool restart;

};

extern Args args;

extern bool fork_to_restart;


//{{{
struct Settings
{
	static constexpr unsigned int MAX_IDLE_TIME_DEFAULT_VALUE = 60;
	unsigned int max_idle_time = MAX_IDLE_TIME_DEFAULT_VALUE;
	static constexpr bool DETECT_AMBIGUITY_DEFAULT_VALUE = false;
	bool detect_ambiguity = DETECT_AMBIGUITY_DEFAULT_VALUE;
	std::string config_path;
	std::string socket_path;
};
//}}}

extern Settings settings;

