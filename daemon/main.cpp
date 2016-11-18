#include "main.hpp"
#include "doodle.hpp"
#include "getopt_pp.h"
#include <fstream>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <pwd.h>




bool fork_to_restart=false;
bool show_help;
bool show_version;
bool nofork;
bool replace;

Settings settings;


//{{{ Help and version messages

std::string help_message(std::string progname)
{
	std::string message;
	message += "Usage: "+progname+" [options]\nOptions:\n";
	message += "	-h|--help           : Show this help and exit.\n";
	message += "	-v|--version        : Show version information and exit.\n";
	message += "	-n|--nofork         : Do not fork off into the background.\n";
	message += "	-r|--restart        : Wait for already running daemon to finish instead of aborting when another daemon is already running.\n";
	//message += "	-a|--allow_idle     : Disable idle time checking.\n";
	message += "	-c|--config  <path> : The path to the config file. Default: \"$XDG_CONFIG_HOME/doodle/\".\n";
	message += "	-s|--socket  <path> : Where to store the socket for user communication. Default: \"" + DOODLE_SOCKET_PATH + "\".\n";
	return message;
}

void version_message()
{
	std::cout<<DOODLE_PROGRAM_NAME<<":"<<std::endl;
	std::cout<<"Git branch: "<<GIT_BRANCH<<", git commit hash: "<<GIT_COMMIT_HASH<<", Version: "<<DOODLE_VERSION_MAJOR<<":"<<DOODLE_VERSION_MINOR<<"."<<std::endl<<std::endl;
}
//}}}


//{{{
void restart_doodle(void)
{
	char program_path[1024];
	ssize_t len = ::readlink("/proc/self/exe", program_path, sizeof(program_path)-1);
	if(len != -1)
	{
		program_path[len] = '\0';
	}
	else
	{
		error<<"Cannot get own path. => cannot restart."<<std::endl;
	}

	pid_t pid;
	switch ((pid = fork()))
	{
		case -1: /* Error */
			error << "Uh-Oh! fork() failed.\n";
			exit(EXIT_FAILURE);
		case 0: /* Child process */
			execl(program_path,
					fs::path(program_path).filename().c_str(),
					nofork?"-n":"",
					"-r",
					"-c", settings.config_dir.c_str(),
					"-s", settings.doodle_socket_path.c_str(),
					"-i", settings.i3_socket_path.c_str(),
				   	static_cast<char*>(nullptr));
			error<< "Uh-Oh! execl() failed!"<<std::endl; /* execl doesn't return unless there's an error */
			exit(EXIT_FAILURE);
		default: /* Parent process */
			debug << "Process created with pid " << pid << "\n";
	}
}
//}}}


//{{{
std::string get_config_dir(void)
{
	// Check order: User set value -> $XDG_CONFIG_HOME/doodle -> $HOME/.doodle -> $HOME/.config/doodle.
	// Note that $XDG_CONFIG_DIRS is not checked. There would be no sense in doing so as doodle MUST have a user configuration specifying the jobs anyway.

	//fs::path config_dir(settings.config_dir);
	fs::path config_dir;

	const char* config_dir_temp;

	//{{{ Check $XDG_CONFIG_HOME

	debug<<"No configuration directory set => checking $XDG_CONFIG_HOME"<<std::endl;
	if((config_dir_temp=getenv("XDG_CONFIG_HOME")))
	{
		debug<<"Found $XDG_CONFIG_HOME = "<<config_dir<<'.'<<std::endl;
		if(!fs::exists((config_dir=config_dir_temp)/="doodle"))
		{
			error<<"No configuration directory found at $XDG_CONFIG_HOME/doodle = "<<config_dir<<'.'<<std::endl;
		}
		debug<<"Config directory found at $XDG_CONFIG_HOME = "<<config_dir<<'.'<<std::endl;
		if(!fs::exists(config_dir/"/doodlerc"))
		{
			error<<"No config found in $XDG_CONFIG_HOME/doodle = "<<config_dir<<". Aborting..."<<std::endl;
			//TODO: Copy some default config file
			exit(EXIT_FAILURE);
		}
		debug<<"Config found in "<<config_dir<<std::endl;
		return config_dir;
	}
	//}}}

	if((config_dir_temp = getenv("HOME")) == nullptr)
	{
		config_dir_temp = getpwuid(getuid())->pw_dir;
	}

	//{{{ Check $HOME/.doodle

	if(fs::exists((config_dir=config_dir_temp)/=".doodle"))
	{
		debug<<"Config directory found at $HOME/.doodle"<<std::endl;
		if(!fs::exists(config_dir/"doodlerc"))
		{
			error<<"No config found in $HOME/.doodle. Aborting..."<<std::endl;
			//TODO: Copy some default config file
			exit(EXIT_FAILURE);
		}
		debug<<"Config found in "<<config_dir<<std::endl;
		return config_dir;
	}
	//}}}

	//{{{ Check $HOME/.config/doodle

	debug<<"No config directory found in $HOME/.doodle => try $HOME/.config/doodle"<<std::endl;
	if(fs::exists((config_dir=config_dir_temp)/=".config/doodle"))
	{
		debug<<"Config directory found at $HOME/.config/doodle"<<std::endl;
		if(!fs::exists(config_dir/"doodlerc"))
		{
			error<<"No config found in $HOME/.doodle. Aborting..."<<std::endl;
			//TODO: Copy some default config file
			exit(EXIT_FAILURE);
		}
		debug<<"Config found in "<<config_dir<<std::endl;
		return config_dir;
	}
	//}}}

	error<<"No config found. Aborting..."<<std::endl;
	exit(EXIT_FAILURE);
}
//}}}


//{{{
std::string get_data_dir(void)
{
	// Check order: User set value -> $XDG_DATA_HOME/doodle -> $HOME/.local/doodle
	// Note that $XDG_DATA_DIRS is not checked. There would be no sense in doing so as doodle MUST have a user configuration specifying the jobs anyway.

	fs::path data_dir = settings.data_dir;
	//data_dir = args.data_dir;

	////{{{
	//if(args.data_set)
	//{
	//	if(!fs::exists(data_dir))
	//	{
	//		debug<<"No data directory found at "<<settings.data_dir<<'.'<<std::endl;
	//		std::error_code fs_error;
	//		if(!fs::create_directory(data_dir, fs_error))
	//		{
	//			error<<"Could not create data directory $XDG_DATA_HOME/doodle = "<<data_dir<<": "<<fs_error.message()<<'.'<<std::endl;
	//		}
	//		debug<<"Created data directory $XDG_DATA_HOME/doodle = "<<data_dir<<'.'<<std::endl;
	//	}
	//	return;
	//}
	////}}}

	const char* data_dir_temp;

	//{{{ Check $XDG_DATA_HOME

	debug<<"No data directory set => checking $XDG_DATA_HOME"<<std::endl;
	if((data_dir_temp=getenv("XDG_DATA_HOME")))
	{
		debug<<"$XDG_DATA_HOME set => data directory should be at $XDG_DATA_HOME/doodle"<<std::endl;
		if(!fs::exists((data_dir=data_dir_temp)/="doodle"))
		{
			debug<<"No data directory found at $XDG_DATA_HOME/doodle => Create it and set it as data dir"<<std::endl;
			std::error_code fs_error;
			if(!fs::create_directory(data_dir, fs_error))
			{
				error<<"Could not create data directory $XDG_DATA_HOME/doodle = "<<data_dir<<": "<<fs_error.message()<<'.'<<std::endl;
			}
			debug<<"Created data directory $XDG_DATA_HOME/doodle = "<<data_dir<<'.'<<std::endl;
		}
		debug<<"Data directory found at "<<data_dir<<std::endl;
		return data_dir;
	}
	//}}}


	//{{{ Check $HOME/.local/share

	debug<<"$XDG_DATA_HOME not set => checking $HOME/.local/share/doodle"<<std::endl;

	if(!(data_dir_temp = getenv("HOME")))
	{
		data_dir_temp = getpwuid(getuid())->pw_dir;
	}

	if(!fs::exists((data_dir=data_dir_temp)/=".local/share/doodle"))
	{
		debug<<"No data directory found in $HOME/.local/share/doodle => create it and set it as data dir"<<std::endl;
		std::error_code fs_error;
		if(!fs::create_directory(data_dir, fs_error))
		{
			error<<"Could not create data directory $HOME/.local/share/doodle = "<<data_dir<<": "<<fs_error.message()<<'.'<<std::endl;
		}
		debug<<"Created data directory $HOME/.local/share/doodle = "<<data_dir<<'.'<<std::endl;
	}
	//}}}
	return data_dir;
}
//}}}


//{{{
void parse_config(void)
{
	std::ifstream config_file(fs::path(settings.config_dir)/"doodlerc");

	Json::Value configuration_root;
	Json::Reader reader;

	if( !reader.parse(config_file, configuration_root, false))
	{
		error<<reader.getFormattedErrorMessages()<<std::endl;
	}

	//{{{ Get the configuration options

	debug<<"retrieving config"<<std::endl;
	Json::Value config;
	if( configuration_root.isMember("config"))
	{
		config = configuration_root.get("config", "no config");
	}
	else
	{
		config = configuration_root;
	}
	settings.max_idle_time = config.get("max_idle_time", settings.MAX_IDLE_TIME_DEFAULT_VALUE).asUInt();
	settings.detect_ambiguity = config.get("detect_ambiguity", settings.DETECT_AMBIGUITY_DEFAULT_VALUE).asBool();
	settings.doodle_socket_path.append(1, '\0');
	//}}}
}
//}}}






int main(int argc, char* argv[])
{
	int retval = EXIT_FAILURE;

	//{{{ Check that we are not root.

    if (!getuid())
	{	// Doodle can run external programs. It reads from a world-writable config file which program to run. An attacker gaining access to this file could run arbitrary programs as root.
		std::cerr<<"Please never run this program as root. It is not neccessary and poses a severe security risk."<<std::endl;
		return EXIT_FAILURE;
	}
	//}}}

	//{{{ Argument handling

	GetOpt::GetOpt_pp ops(argc, argv);

	ops.exceptions(std::ios::failbit|std::ios::eofbit);
	try
	{
		ops>>GetOpt::OptionPresent('h', "help",       show_help);
		ops>>GetOpt::OptionPresent('v', "version",    show_version);
		ops>>GetOpt::OptionPresent('n', "nofork",     nofork);
		ops>>GetOpt::OptionPresent('r', "replace",    settings.replace);
		//ops>>GetOpt::OptionPresent('a', "allow_idle", allow_idle);

		ops>>GetOpt::Option('c',        "config",     settings.config_dir,         get_config_dir);
		ops>>GetOpt::Option('d',        "data",       settings.data_dir,           get_data_dir);
		ops>>GetOpt::Option('s',        "socket",     settings.doodle_socket_path, DOODLE_SOCKET_PATH);
		ops>>GetOpt::Option('i',        "i3socket",   settings.i3_socket_path,     i3ipc::get_socketpath());
	}
	catch(GetOpt::GetOptEx ex)
	{
		std::cerr<<"Error in arguments"<<std::endl;

		std::cerr<<help_message(argv[0])<<std::endl;
		return -1;
	}
	if( show_help )
	{
		std::cout<<help_message(argv[0])<<std::endl;
		return 0;
	}
	if( show_version )
	{
		version_message();
		return 0;
	}
	//}}}

	//{{{ Validate settings path

	if(!fs::exists(fs::path(settings.config_dir)/"/doodlerc"))
	{
		error<<"No config found in "<<settings.config_dir<<" set by command line switch. Aborting..."<<std::endl;
		//TODO: Copy some default config file
		exit(EXIT_FAILURE);
	}
	debug<<"Config found in "<<settings.config_dir<<'.'<<std::endl;
	//}}}

	//{{{ Validate data dir

	if(!fs::exists(fs::path(settings.data_dir)))
	{
		debug<<"No data directory found at "<<settings.data_dir<<'.'<<std::endl;
		std::error_code fs_error;
		if(!fs::create_directory(settings.data_dir, fs_error))
		{
			error<<"Could not create data directory $XDG_DATA_HOME/doodle = "<<settings.data_dir<<": "<<fs_error.message()<<'.'<<std::endl;
			exit(EXIT_FAILURE);
		}
		debug<<"Created data directory $XDG_DATA_HOME/doodle = "<<settings.data_dir<<'.'<<std::endl;
	}
	//}}}

	parse_config();

	{	// Restrict life time of the doodle object
		Doodle doodle;

		if(!nofork)
		{
			if(daemon(1, 0))
			{
				error<<"Could not daemonize."<<std::endl;
				return EXIT_FAILURE;
			}
		}

		retval = doodle();
	}

	if(fork_to_restart) restart_doodle();

	return retval;



	//TODO: Catch all exceptions here
	//	- Write to error
	//	- Write to syslog/journal
	//	- Write desktop notification
	//	- exit(EXIT_FAILURE);
}
