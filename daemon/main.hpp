#pragma once

#include "doodle_config.hpp"
#include "console_stream.hpp"
#include "notify_stream.hpp"
#include <uvw.hpp>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;



//extern bool allow_idle;

extern std::string config_dir;
extern std::string data_dir;
//extern std::string doodle_socket_path;
//extern std::string i3_socket_path;


extern uint32_t max_idle_time_ms;
extern bool stop_on_suspend;
extern bool detect_ambiguity;





std::vector<std::string> tokenise(std::string input);

uint32_t time_string_to_seconds(std::string input);
