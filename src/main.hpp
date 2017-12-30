#pragma once

//#define INI_INLINE_COMMENT_PREFIXES "#"
#define INI_MAX_LINE 2000
#include "INIReader.h"

#include "doodle_config.hpp"

#include "console_stream.hpp"
#include "notify_stream.hpp"
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

inline std::string config_dir;
inline std::string data_dir;

//Time handling
using milliseconds = std::chrono::duration<int64_t, std::milli>;
using steady_clock = std::chrono::steady_clock;
using system_clock = std::chrono::system_clock;

//{{{
inline std::string ms_to_string(milliseconds value)
{
	return std::string(std::to_string(value.count())+"ms");
}

inline std::ostream& operator<<(std::ostream& stream, milliseconds value)
{
	//stream<<value.count()<<"ms";
	stream<<ms_to_string(value);
	return stream;
}
//}}}

milliseconds string_to_ms(std::string input);




//extern std::string data_dir;
//extern std::string doodle_socket_path;
//extern std::string i3_socket_path;

//
//
//extern milliseconds max_idle_time_ms;
//extern bool stop_on_suspend;
//extern bool detect_ambiguity;





