#pragma once

#include <ev++.h>
#include <iostream>
#include <queue>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
//#include <bitset>
#include "doodle_config.hpp"

struct Args
{
	bool show_help;
	bool show_version;
	bool nofork;
	bool restart;

	bool config_set;
	bool data_set;
	bool socket_set;
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
	std::string data_path;
	std::string socket_path;
};
//}}}

extern Settings settings;
