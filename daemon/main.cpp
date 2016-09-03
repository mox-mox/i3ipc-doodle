#include "main.hpp"
#include <iostream>
#include "doodle.hpp"
#include "getopt_pp.h"
#include <unistd.h>
#include <bsd/libutil.h> 
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>

bool show_help;
bool show_version;
bool nofork;
bool restart;
std::string config_path;
std::string socket_path;


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
		ops>>GetOpt::Option('c', "config", config_path, DOODLE_CONFIG_PATH);
		ops>>GetOpt::Option('s', "socket", socket_path, DOODLE_CONFIG_PATH);
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

	//{{{Check if the socket is already in use.

	//}}}


	//{{{ Create a pidfile

	//struct pidfh* pfh = nullptr;
	//pid_t otherpid;
	//std::string pidfile;
	//if ((pidfile = getenv("XDG_RUNTIME_DIR")).empty())
	//	if ((pidfile = getenv("HOME")).empty())
	//		pidfile = getpwuid(getuid())->pw_dir;
	//pidfile.append("/.doodle.pid");
	//debug<<"PID file stored to "<<pidfile<<std::endl;

	//while(!(pfh = pidfile_open(pidfile.c_str(), 0600, &otherpid)))
	//{
	//	if(errno == EEXIST && restart)
	//	{
	//		kill(otherpid, SIGTERM);
	//		usleep(1000);
	//		continue;
	//	}

	//	if (errno == EEXIST)
	//		error<<"Daemon already running, pid: "<<(intmax_t)otherpid<<std::endl;
	//	else
	//		error<<"Cannot open or create pidfile"<<std::endl;
	//	return EXIT_FAILURE;
	//}
	//}}}

	Doodle doodle(config_path);

	//{{{ Fork to the background

	if( !nofork )
	{
		if( daemon(1, 0))
		{
			error<<"Could not daemonize."<<std::endl;
			return EXIT_FAILURE;
		}
	}
	//}}}

	retval = doodle();

	return retval;
}
