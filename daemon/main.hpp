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
	bool replace;

	bool config_set;
	bool data_set;
	bool socket_set;
	std::string config_dir;
	std::string data_dir;
	std::string socket_path;
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

	fs::path config_dir;
	fs::path data_dir;
	std::string socket_path;
};
//}}}

extern Settings settings;

