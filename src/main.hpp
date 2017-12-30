#pragma once

//#define INI_INLINE_COMMENT_PREFIXES "#"
#define INI_MAX_LINE 2000
#include "INIReader.h"

#include "doodle_config.hpp"

#include "console_stream.hpp"
#include "notify_stream.hpp"
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

inline fs::path config_dir;
inline fs::path data_dir;
inline std::string user_socket_path;

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



