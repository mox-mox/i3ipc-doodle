#include "main.hpp"
#include "getopt_pp.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <daemon.hpp>
#include <client.hpp>



//{{{ Help and version messages, config copy function

std::string help_message(std::string progname)
{
	std::string message;
	message += "Usage: "+progname+" [options]\nOptions:\n";
	message += "	-h|--help               : Show this help and exit.\n";
	message += "	-v|--version            : Show version information and exit.\n";
	message += "	-q|--quiet              : Mute output.\n";
	message += "	   --verbose            : Enable debugging output.\n";

	//message += "	-a|--allow-idle         : Disable idle time checking.\n";

	message += "	-c|--config  <path>     : The path to the config files. Default: \"$XDG_CONFIG_HOME/doodle/\".\n";
	message += "	-d|--data  <path>       : The path to the data files. Default: \"$XDG_DATA_HOME/doodle/\".\n";

	//message += "	-u|--usersocket  <path> : Where to store the socket for user communication. Default: \"" + DOODLE_SOCKET_PATH + "\".\n";
	message += "	-i|--i3socket  <path>   : The socket for communication with i3. Default: \"" + i3ipc::get_socketpath() + "\".\n";

	message += "	   --client             : Run the terminal client to send commands to a running doodle daemon.\n";
	return message;
}

void version_message()
{
	std::cout<<DOODLE_PROGRAM_NAME<<":"<<std::endl;
	std::cout<<"Git branch: "<<GIT_BRANCH<<", git commit hash: "<<GIT_COMMIT_HASH<<", Version: "<<DOODLE_VERSION_MAJOR<<":"<<DOODLE_VERSION_MINOR<<"."<<std::endl<<std::endl;
}


void config_copy()
{
	std::cout<<"copy_config is not yet implemented."<<std::endl;
	//fs::copy(EXAMPLE_CONFIG_PATH, 

}
//}}}

//{{{
fs::path check_config_dir(fs::path dir = "default")
{
	//{{{ Check user provided value

	if(dir != "default")
	{
		debug<<"Checking user provided config path = "<<dir<<'.'<<std::endl;
		if(fs::exists(dir) && fs::exists(dir/"doodlerc"))
		{
			debug<<"Config found in "<<dir<<std::endl;
			return dir;
		}
		else
		{
			error<<"No config found in user provided config dir "<<dir<<": "
				<<(fs::exists(dir)?"No configuration file.":"Path does not exist.")<<std::endl;
			//TODO: Copy some default config file
			exit(EXIT_FAILURE);
		}
	}
	//}}}

	//{{{ Check $XDG_CONFIG_HOME/doodle

	// Note that $XDG_CONFIG_DIRS (system wide configuration) is not checked.
	// There would be no sense in doing so as doodle MUST have a user configuration specifying the jobs anyway.
	debug<<"No configuration directory set => checking $XDG_CONFIG_HOME"<<std::endl;
	//if((config_dir_temp=getenv("XDG_CONFIG_HOME")))
	if(const char* xdg_config_home=getenv("XDG_CONFIG_HOME"); xdg_config_home)
	{
		debug<<"Checking $XDG_CONFIG_HOME = "<<xdg_config_home<<'.'<<std::endl;
		if(fs::path config_dir(xdg_config_home); fs::exists(config_dir) && fs::exists(config_dir/"doodle") && fs::exists(config_dir/"doodle/doodle.conf"))
		{
			debug<<"Config found in "<<config_dir<<std::endl;
			return config_dir;
		}
		else
		{
			error<<"No config found in $XDG_CONFIG_HOME: "<<config_dir<<": "
				<<(fs::exists(config_dir/"doodle")?"No configuration file.":"Path does not exist.")<<std::endl;
			//TODO: Copy some default config file
			exit(EXIT_FAILURE);
		}
	}
	//}}}

	//{{{ Check $HOME/.doodle and $HOME/.config/doodle

	//{{{ Get $HOME

	debug<<"$XDG_CONFIG_HOME not set => Checking $HOME/.doodle"<<std::endl;
	const char* home_dir_tmp;
	if(home_dir_tmp = getenv("HOME"); home_dir_tmp)
	{
		debug<<"Found $HOME = "<<home_dir_tmp<<'.'<<std::endl;
	}
	else if(home_dir_tmp = getpwuid(getuid())->pw_dir; home_dir_tmp)
	{
		debug<<"Found $HOME = "<<home_dir_tmp<<'.'<<std::endl;
	}
	else
	{
		error<<"No home directory found."<<std::endl;
		exit(EXIT_FAILURE);
	}
	//}}}

	if(fs::path home_dir(home_dir_tmp); fs::exists(home_dir))
	{
		//debug<<"Checking $HOME/.doodle = "<<home_dir/".doodle"<<" and $HOME/.config/doodle = "<<home_dir/".config/doodle"<<"."std::endl;
		debug<<"Checking $HOME/.doodle = "<<home_dir/".doodle"<<" and $HOME/.config/doodle = "<<home_dir/".config/doodle"<<"."<<std::endl;
		if(fs::exists(home_dir/".doodle") && fs::exists(home_dir/".doodle/doodle.conf"))
		{
			debug<<"Config found in "<<home_dir/".doodle"<<std::endl;
			return home_dir/".doodle";
		}
		else if(fs::exists(home_dir/".config") && fs::exists(home_dir/".config/doodle") && fs::exists(home_dir/".config/doodle/doodle.conf"))
		{
			debug<<"Config found in "<<home_dir/".config/doodle"<<std::endl;
			return home_dir/".config/doodle";
		}
		else
		{
			error<<"No config found in $HOME/.doodle = "<<home_dir/".doodle"<<" or $HOME/.config/doodle = "<<home_dir/".config/doodle"<<": "
				<<(fs::exists(home_dir/".doodle")||fs::exists(home_dir/".config/doodle") ?"No configuration file.":"Path does not exist.")<<std::endl;
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		error<<"$HOME directory = "<<home_dir<<" does not exist."<<std::endl;
		exit(EXIT_FAILURE);
	}
	//}}}

	return "";
}
//}}}

//{{{
fs::path check_data_dir(fs::path dir = "default")
{
	//{{{ Check user provided value

	if(dir != "default")
	{
		debug<<"Checking user provided data path = "<<dir<<'.'<<std::endl;
		if(dir.filename() != "doodle") dir /= "doodle";

		if(fs::exists(dir))
		{
			debug<<"Data directory found in "<<dir<<std::endl;
			return dir;
		}
		else
		{
			logger<<"No data directory found in user provided data dir "<<dir<<" => Create it and set it as data dir"<<std::endl;
			if(std::error_code fs_error; !fs::create_directories(dir, fs_error))
			{
				error<<"Could not create user provided data directory = "<<dir<<": "<<fs_error.message()<<'.'<<std::endl;
				exit(EXIT_FAILURE);
			}
			else
			{
				debug<<"Created user provided data directory = "<<dir<<'.'<<std::endl;
				return dir;
			}
		}
	}
	//}}}

	//{{{ Check $XDG_DATA_HOME/doodle

	// Note that $XDG_CONFIG_DIRS (system wide configuration) is not checked.
	// There would be no sense in doing so as doodle MUST have a user configuration specifying the jobs anyway.
	debug<<"No data directory set => checking $XDG_DATA_HOME"<<std::endl;
	if(const char* xdg_data_home=getenv("XDG_DATA_HOME"); xdg_data_home)
	{
		debug<<"Checking $XDG_DATA_HOME = "<<xdg_data_home<<'.'<<std::endl;
		if(fs::path data_dir(xdg_data_home); fs::exists(data_dir) && fs::exists(data_dir/"doodle"))
		{
			debug<<"Config found in "<<data_dir<<std::endl;
			return data_dir;
		}
		else
		{
			logger<<"No data directory found at $XDG_DATA_HOME/doodle => Create it and set it as data dir"<<std::endl;
			if(std::error_code fs_error; !fs::create_directories(data_dir, fs_error))
			{
				error<<"Could not create data directory $XDG_DATA_HOME/doodle = "<<data_dir<<": "<<fs_error.message()<<'.'<<std::endl;
				exit(EXIT_FAILURE);
			}
			else
			{
				debug<<"Created data directory $XDG_DATA_HOME/doodle = "<<data_dir<<'.'<<std::endl;
				return dir;
			}
		}
	}
	//}}}

	//{{{ Check $HOME/.local/share

	//{{{ Get $HOME

	debug<<"$XDG_DATA_HOME not set => Checking $HOME/.local/share/doodle"<<std::endl;
	const char* home_dir_tmp;
	if(home_dir_tmp = getenv("HOME"); home_dir_tmp)
	{
		debug<<"Found $HOME = "<<home_dir_tmp<<'.'<<std::endl;
	}
	else if(home_dir_tmp = getpwuid(getuid())->pw_dir; home_dir_tmp)
	{
		debug<<"Found $HOME = "<<home_dir_tmp<<'.'<<std::endl;
	}
	else
	{
		error<<"No home directory found."<<std::endl;
		exit(EXIT_FAILURE);
	}
	//}}}

	if(fs::path home_dir(home_dir_tmp); fs::exists(home_dir))
	{
		debug<<"Checking $HOME/.local/share/doodle = "<<home_dir/".local/share/doodle"<<"."<<std::endl;
		if(fs::path data_dir(home_dir/".local/share/doodle"); fs::exists(data_dir))
		{
			debug<<"Config found in "<<data_dir<<std::endl;
			return data_dir;
		}
		else
		{
			logger<<"No data directory found at $HOME/.local/share/doodle => Create it and set it as data dir"<<std::endl;
			if(std::error_code fs_error; !fs::create_directories(data_dir, fs_error))
			{
				error<<"Could not create data directory $HOME/.local/share/doodle = "<<data_dir<<": "<<fs_error.message()<<'.'<<std::endl;
				exit(EXIT_FAILURE);
			}
			else
			{
				debug<<"Created data directory $HOME/.local/share/doodle = "<<data_dir<<'.'<<std::endl;
				return dir;
			}
		}
	}
	else
	{
		error<<"$HOME directory = "<<home_dir<<" does not exist."<<std::endl;
		exit(EXIT_FAILURE);
	}
	//}}}

	return "";
}
//}}}


//{{{ Parsing functions

// Performance really is no concern here, so prefer simple to read code

//{{{
milliseconds string_to_ms(std::string input)
{
	uint64_t seconds = 0;
	uint64_t temp_number = 0;

	bool in_number = false;
	for(char c : input)
	{
		if(std::isdigit(c))
		{
			if(!in_number)
			{
				temp_number = (c-'0');
				in_number = true;
			}
			else
			{
				temp_number = temp_number*10 + (c-'0');
			}
		}
		else
		{
			if(!in_number) continue;
			in_number = false;
			switch(c)
			{
				case 'y': seconds += temp_number*365*24*60*60; break;
				case 'd': seconds += temp_number    *24*60*60; break;
				case 'h': seconds += temp_number       *60*60; break;
				case 'm': seconds += temp_number          *60; break;
				default:
				case 's': seconds += temp_number           *1;
			}
			temp_number = 0;
		}
	}
	seconds += temp_number;

	return milliseconds(1000*seconds);
}
//}}}

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

	bool run_client;
	std::string i3_socket_path;

	{
		//{{{ Internal variables

		bool show_help;
		bool show_version;
		bool copy_config;
		bool quiet;
		bool verbose;
		//}}}

		GetOpt::GetOpt_pp ops(argc, argv);
		ops.exceptions(std::ios::failbit|std::ios::eofbit);
		try
		{
			ops>>GetOpt::OptionPresent('h',       "help",           show_help);
			ops>>GetOpt::OptionPresent('v',       "version",        show_version);
			ops>>GetOpt::OptionPresent('q',       "quiet",          quiet);
			ops>>GetOpt::OptionPresent(           "verbose",        verbose);

			ops>>GetOpt::OptionPresent(           "client",         run_client);
			ops>>GetOpt::OptionPresent(           "copy-config",    copy_config);

			ops>>GetOpt::Option(       'c',       "config",         config_dir,  "default");
			ops>>GetOpt::Option(       'd',       "data",           data_dir,    "default");

			ops>>GetOpt::Option(       's',       "socket",         user_socket_path,   "default");
			ops>>GetOpt::Option(       'i',       "i3socket",       i3_socket_path,     "default");
			//ops>>GetOpt::Option(     'l',       "logfile",        logfile,            "default");
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

		//{{{ Loglevel

		logger<<setloglevel(2);
		if(run_client) quiet = true;
		if(quiet) logger<<setloglevel(1);
		if(verbose) logger<<setloglevel(3);
		//}}}

		config_dir = check_config_dir(config_dir);

		if( copy_config )
		{
			config_copy();
			return 0;
		}
	}
	//}}}



	INIReader config_reader(config_dir/"doodle.conf");
	if (config_reader.ParseError() < 0)
	{
		error<<"Cannot parse doodle config at "<< (config_dir/"doodle.conf") <<"."<<std::endl;
		notify_critical<<"Cannot parse doodle config file"<<(config_dir/"doodle.conf")<<std::endl;
		throw std::runtime_error("Cannot parse doodle config file at " + (config_dir/"doodle.conf").string() +".");
	}

	data_dir = check_data_dir( data_dir != "default" ? data_dir : fs::path(config_reader.Get("", "data_dir", "default")));
	user_socket_path = user_socket_path != "default" ? user_socket_path : config_reader.Get("", "user_socket_path", USER_SOCKET_PATH);

	if(run_client)
	{
		Client client(config_reader);
		retval = client();
	}
	else
	{
		Daemon daemon(config_reader);
		retval = daemon();
	}

	return retval;
}
////{{{
//notify_low<<"LOW"<<"low"<<std::endl;
//notify_normal<<sett(5000)<<"NORMAL"<<"normal"<<99<<std::endl;
//notify_critical<<sett(10000)<<"CRITICAL"<<"critical"<<std::setw(10)<<99<<'.'<<std::endl;
////}}}
