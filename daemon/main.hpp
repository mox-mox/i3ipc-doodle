#pragma once

#include "doodle_config.hpp"
#include "console_stream.hpp"
#include "notify_stream.hpp"
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

//{{{ Time handling

using milliseconds = std::chrono::duration<int64_t, std::milli>;
using steady_clock = std::chrono::steady_clock;
using system_clock = std::chrono::system_clock;

//{{{
inline std::ostream& operator<<(std::ostream& stream, milliseconds const& value)
{
	stream<<value.count()<<"ms";
	return stream;
}
//}}}

milliseconds time_string_to_milliseconds(std::string input);

//}}}

//{{{ Options

extern std::string config_dir;
extern std::string data_dir;
//extern std::string doodle_socket_path;
//extern std::string i3_socket_path;


extern milliseconds max_idle_time_ms;
extern bool stop_on_suspend;
extern bool detect_ambiguity;

//}}}

std::vector<std::string> tokenise(std::string input);


//{{{
struct Current
{
	std::string workspace_name;
	std::string window_name;
};
//}}}

