#include "main.hpp"

#include <doodle.hpp>


bool show_help;
bool show_version;
bool nofork;
bool allow_idle;

std::string config_dir;
std::string data_dir;
std::string doodle_socket_path;
std::string i3_socket_path;









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
	//message += "	-s|--socket  <path> : Where to store the socket for user communication. Default: \"" + DOODLE_SOCKET_PATH + "\".\n";
	return message;
}

void version_message()
{
	std::cout<<DOODLE_PROGRAM_NAME<<":"<<std::endl;
	std::cout<<"Git branch: "<<GIT_BRANCH<<", git commit hash: "<<GIT_COMMIT_HASH<<", Version: "<<DOODLE_VERSION_MAJOR<<":"<<DOODLE_VERSION_MINOR<<"."<<std::endl<<std::endl;
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

	fs::path data_dir;
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
				exit(EXIT_FAILURE);
			}
			else
			{
				debug<<"Created data directory $XDG_DATA_HOME/doodle = "<<data_dir<<'.'<<std::endl;
			}
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
			exit(EXIT_FAILURE);
		}
		{
			debug<<"Created data directory $HOME/.local/share/doodle = "<<data_dir<<'.'<<std::endl;
		}
	}
	//}}}
	return data_dir;
}
//}}}







int main(int argc, char* argv[])
{
	int retval = EXIT_SUCCESS;
	//int retval = EXIT_FAILURE;

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
		//ops>>GetOpt::OptionPresent('r', "replace",    settings.replace);
		ops>>GetOpt::OptionPresent('a', "allow_idle", allow_idle);

		ops>>GetOpt::Option('c',        "config",     config_dir,         get_config_dir);
		ops>>GetOpt::Option('d',        "data",       data_dir,           get_data_dir);
		ops>>GetOpt::Option('s',        "socket",     doodle_socket_path, DOODLE_SOCKET_PATH);
		//ops>>GetOpt::Option('i',        "i3socket",   i3_socket_path,     i3ipc::get_socketpath());
	}
	catch(GetOpt::GetOptEx ex)
	{
		std::cerr<<"Error in arguments"<<std::endl;

		std::cerr<<help_message(argv[0])<<std::endl;
		return -1;
	}
	//if( show_help )
	//{
	//	std::cout<<help_message(argv[0])<<std::endl;
	//	return 0;
	//}
	//if( show_version )
	//{
	//	version_message();
	//	return 0;
	//}
	//}}}

	//{{{ Config parsing

	std::cout<<"Hello world!"<<std::endl;

	cfg_opt_t opts[] =
	{
		CFG_STR("target", "No One", CFGF_NONE),
		CFG_END()
	};
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	if(cfg_parse(cfg, "hello.conf") == CFG_PARSE_ERROR)
		return 1;

	//printf("Hello, %s!\n", cfg_getstr(cfg, "target"));

	cfg_free(cfg);
	//}}}


	{
		Doodle doodle;
		std::cout<<"asdfasdfasdafafs"<<std::endl;
		retval = doodle();
	}






//
//	//{{{
//	notify_low<<"LOW"<<"low"<<std::endl;
//	notify_normal<<sett(5000)<<"NORMAL"<<"normal"<<99<<std::endl;
//	notify_critical<<sett(10000)<<"CRITICAL"<<"critical"<<std::setw(10)<<99<<'.'<<std::endl;
//	//}}}
//

	return retval;
}
