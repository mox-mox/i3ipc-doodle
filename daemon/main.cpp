#include "main.hpp"
#include <iostream>
#include "doodle.hpp"
#include "getopt_pp.h"
#include <unistd.h>
#include <bsd/libutil.h> 
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <json/json.h>
#include <fstream>

bool show_help;
bool show_version;
bool nofork;
bool restart;
bool fork_to_restart=false;
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
	message += "	-c|--config  <path> : The path to the config file. Default: \"" DOODLE_CONFIG_PATH "\".\n";
	message += "	-s|--socket  <path> : Where to store the socket for user communication. Default: \"" + DOODLE_SOCKET_PATH_DEFAULT + "\".\n";
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
			exit(1);
		case 0: /* Child process */
			execl(program_path,
					fs::path(program_path).filename().c_str(),
					nofork?"-n":"",
					"-r",
					"-c", settings.config_path.c_str(),
					"-s", settings.socket_path.c_str(),
				   	static_cast<char*>(nullptr));
			error << "Uh-Oh! execl() failed!"; /* execl doesn't return unless there's an error */
			exit(1);
		default: /* Parent process */
			debug << "Process created with pid " << pid << "\n";
	}
}
//}}}

//{{{
void parse_config(void)
{
	std::ifstream config_file(settings.config_path+"/doodlerc");

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
	settings.socket_path = config.get("socket_path", DOODLE_SOCKET_PATH_DEFAULT).asString();
	if(settings.socket_path[0] == '@') settings.socket_path[0] = '\0';
	//}}}
}
//}}}


int main(int argc, char* argv[])
{
	int retval = -1;

	//{{{ Argument handling
	GetOpt::GetOpt_pp ops(argc, argv);

	ops.exceptions(std::ios::failbit|std::ios::eofbit);
	try
	{
		ops>>GetOpt::OptionPresent('h', "help", show_help);
		ops>>GetOpt::OptionPresent('v', "version", show_version);
		ops>>GetOpt::OptionPresent('n', "nofork", nofork);
		ops>>GetOpt::OptionPresent('r', "restart", restart);
		//ops>>GetOpt::OptionPresent('a', "allow_idle", allow_idle);
		ops>>GetOpt::Option('c', "config", settings.config_path, DOODLE_CONFIG_PATH);
		ops>>GetOpt::Option('s', "socket", settings.socket_path, DOODLE_CONFIG_PATH);
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

	{
		//Doodle doodle(settings.config_path);
		Doodle doodle(settings);

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
}
