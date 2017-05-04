#include "main.hpp"
//#include "getopt_pp.h"
//#include "doodle.hpp"


int main(void)
{
	std::cout<<"Hello World!"<<std::endl;
	return 0;
}


////#include <uvw.hpp>
////#include <fstream>
////#include <stdlib.h>
////#include <unistd.h>
////
////#include <sys/types.h>
////#include <pwd.h>
////#include <json/json.h>
////#include <i3ipc++/ipc.hpp>
////#include <iostream>
//
//
//
////
////Cmd_args cmd_args;
////
////
//////{{{ Help and version messages
////
////std::string help_message(std::string progname)
////{
////	std::string message;
////	message += "Usage: "+progname+" [options]\nOptions:\n";
////	message += "	-h|--help           : Show this help and exit.\n";
////	message += "	-v|--version        : Show version information and exit.\n";
////	message += "	-n|--nofork         : Do not fork off into the background.\n";
////	message += "	-c|--config  <path> : The path to the config file. Default: \"$XDG_CONFIG_HOME/doodle/\".\n";
////	message += "	-s|--socket  <path> : Where to store the socket for user communication. Default: \"" + DOODLE_SOCKET_PATH + "\".\n";
////	return message;
////}
////
////void version_message()
////{
////	std::cout<<DOODLE_PROGRAM_NAME<<":"<<std::endl;
////	std::cout<<"Git branch: "<<GIT_BRANCH<<", git commit hash: "<<GIT_COMMIT_HASH<<", Version: "<<DOODLE_VERSION_MAJOR<<":"<<DOODLE_VERSION_MINOR<<"."<<std::endl<<std::endl;
////}
//////}}}
////
//
//
//
//
//#ifdef ENABLE_MAIN
//int main(int argc, char* argv[])
////try
//{
//	int retval = EXIT_FAILURE;
////
////	//{{{ Check that we are not root.
////
////    if (!getuid())
////	{	// Doodle can run external programs. It reads from a world-writable config file which program to run. An attacker gaining access to this file could run arbitrary programs as root.
////		std::cerr<<"Please never run this program as root. It is not neccessary and poses a severe security risk."<<std::endl;
////		return EXIT_FAILURE;
////	}
////	//}}}
////
////	//{{{// Argument handling
////
////	{
////		bool show_help;
////		bool show_version;
////		GetOpt::GetOpt_pp ops(argc, argv);
////
////		ops.exceptions(std::ios::failbit|std::ios::eofbit);
////		try
////		{
////			ops>>GetOpt::OptionPresent('h', "help",       show_help);
////			ops>>GetOpt::OptionPresent('v', "version",    show_version);
////			ops>>GetOpt::OptionPresent('n', "nofork",     cmd_args.nofork);
////
////			ops>>GetOpt::Option('c',        "config",     cmd_args.config_dir,         "foo");
////			ops>>GetOpt::Option('d',        "data",       cmd_args.data_dir,           "bar");
////			//ops>>GetOpt::Option('s',        "socket",     cmd_args.doodle_socket_path, "@foobar.sock");
////			//ops>>GetOpt::Option('i',        "i3socket",   cmd_args.i3_socket_path,     i3ipc::get_socketpath());
////		}
////		catch(GetOpt::GetOptEx ex)
////		{
////			std::cerr<<"Error in arguments"<<std::endl;
////
////			std::cerr<<help_message(argv[0])<<std::endl;
////			return -1;
////		}
////		if( show_help )
////		{
////			std::cout<<help_message(argv[0])<<std::endl;
////			return 0;
////		}
////		if( show_version )
////		{
////			version_message();
////			return 0;
////		}
////	}
////	//}}}
////
//
//	(void) argc; (void) argv;
//
//
//	//{	// Restrict life time of the doodle object
//	//	Doodle doodle;
//
//	//	//if(!cmd_args.nofork)
//	//	//{
//	//	//	if(daemon(1, 0))
//	//	//	{
//	//	//		error<<"Could not daemonize."<<std::endl;
//	//	//		return EXIT_FAILURE;
//	//	//	}
//	//	//}
//
//	//	retval = doodle();
//	//}
//
//
//	//{
//	//	// Create a loop
//	//	auto loop = uvw::Loop::getDefault();
//
//	//	// Create a resource that will listen to STDIN
//	//	auto console = loop->resource<uvw::TTYHandle>(uvw::StdIN, true);
//
//	//	// Set the callback function
//	//	console->on<uvw::DataEvent>([](auto& evt, auto&){
//	//			std::cout<<"Got something: "<<std::endl;
//	//			// The event argument is a struct with only two members: A unique_ptr
//	//			// to a char array and an integer with the length.
//	//			std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
//	//			});
//	//	console->read();
//	//	loop->run();
//	//}
//
//	return retval;
//}
////catch (const std::exception& e) { // caught by reference to base
////	std::cout << " Unhandled exception was encountered, with message '" << e.what() << "'\n...Aborting"<<std::endl;
////	exit(EXIT_FAILURE);
////}
//#endif
