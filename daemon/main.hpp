#pragma once

#include "doodle_config.hpp"

#include "console_stream.hpp"
#include "notify_stream.hpp"
#include <ev++.h>

extern bool show_help;
extern bool show_version;
extern bool nofork;
extern bool restart;
extern std::string config_path;
extern std::string socket_path;
