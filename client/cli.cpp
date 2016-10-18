#include "cli.hpp"
#include <sstream>
#include <unistd.h>
#include "getopt_pp.h"
#include <iomanip>
#include "ipc_socket.hpp"
#include "command.hpp"



Args args;
Settings settings;

//{{{
const char* prompt(EditLine* e)
{
	(void) e;
	return "doodle > ";
}
//}}}

//{{{ Help and version messages

std::string help_message(std::string progname)
{
	std::string message;
	message += "Usage: "+progname+" [commands]\nOptions:\n";
	message += "	-h|--help           : Show this help and exit.\n";
	message += "	-v|--version        : Show version information and exit.\n";
	message += "	-s|--socket  <path> : Where to store the socket for user communication. Default: \"" + DOODLE_SOCKET_PATH + "\".\n";
	message += "Commands:\n";

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
	//{{{ Argument handling
	GetOpt::GetOpt_pp ops(argc, argv);

	ops.exceptions(std::ios::failbit|std::ios::eofbit);
	try
	{
		ops>>GetOpt::OptionPresent('h', "help",       args.show_help);
		ops>>GetOpt::OptionPresent('v', "version",    args.show_version);

		//ops>>GetOpt::OptionPresent('c', "config",     args.config_set);
		//ops>>GetOpt::OptionPresent('d', "data",       args.data_set);
		ops>>GetOpt::OptionPresent('s', "socket",     args.socket_set);
		//ops>>GetOpt::Option('c',        "config",     settings.config_path, DOODLE_CONFIG_PATH);
		//ops>>GetOpt::Option('d',        "data",       settings.data_path, DOODLE_CONFIG_PATH);
		ops>>GetOpt::Option('s',        "socket",     settings.socket_path, DOODLE_SOCKET_PATH);
	}
	catch(GetOpt::GetOptEx ex)
	{
		std::cerr<<"Error in arguments"<<std::endl;

		std::cerr<<help_message(argv[0])<<std::endl;
		return -1;
	}
	if( args.show_help )
	{
		std::cout<<help_message(argv[0])<<std::endl;
		return 0;
	}
	if( args.show_version )
	{
		version_message();
		return 0;
	}
	//}}}

	settings.socket_path.append(1, '\0');

	IPC_socket doodle_sock(settings.socket_path);

	Command readline(prompt);


	while(1)
	{
		readline>>doodle_sock;
		readline<<doodle_sock;
	}

	return 0;
}

