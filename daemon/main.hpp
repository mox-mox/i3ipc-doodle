#pragma once

#include "doodle_config.hpp"

#include "console_stream.hpp"
#include "notify_stream.hpp"
#include <ev++.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

extern bool show_help;
extern bool show_version;
extern bool nofork;
extern bool restart;
extern bool fork_to_restart;
//extern std::string config_path;
//extern std::string socket_path;




struct Settings
{
	static constexpr unsigned int MAX_IDLE_TIME_DEFAULT_VALUE = 60;
	unsigned int max_idle_time = MAX_IDLE_TIME_DEFAULT_VALUE;
	static constexpr bool DETECT_AMBIGUITY_DEFAULT_VALUE = false;
	bool detect_ambiguity = DETECT_AMBIGUITY_DEFAULT_VALUE;
	std::string config_path;
	std::string socket_path = DOODLE_SOCKET_PATH_DEFAULT;
};

extern Settings settings;

