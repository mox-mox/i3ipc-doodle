#pragma once

#include "doodle_config.hpp"

#include "console_stream.hpp"
#include "notify_stream.hpp"
#include <ev++.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;


extern bool fork_to_restart;


//{{{
struct Settings
{
	bool replace;

	static constexpr unsigned int MAX_IDLE_TIME_DEFAULT_VALUE = 60;
	unsigned int max_idle_time = MAX_IDLE_TIME_DEFAULT_VALUE;
	static constexpr bool DETECT_AMBIGUITY_DEFAULT_VALUE = false;
	bool detect_ambiguity = DETECT_AMBIGUITY_DEFAULT_VALUE;

	//std::string testval;
	std::string config_dir;
	std::string data_dir;
	std::string doodle_socket_path;
	std::string i3_socket_path;
};
//}}}

extern Settings settings;


