#include "doodle_config.hpp"
#include "console_stream.hpp"
#include "notify_stream.hpp"
#include "getopt_pp.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "confuse.h"
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;



extern bool show_help;
extern bool show_version;
extern bool nofork;
extern bool allow_idle;

extern std::string config_dir;
extern std::string data_dir;
extern std::string doodle_socket_path;
extern std::string i3_socket_path;

