#include <iostream>
#include "doodle_config.hpp"
#include "doodle.hpp"
#include "getopt_pp.h"

#ifdef USE_NOTIFICATIONS
	#include <libnotify/notify.h>
#endif
#ifdef USE_SYSLOG
	#include <syslog.h>
#endif



//{{{ Help and version messages

std::string help_message(std::string progname)
{
	std::string message;
	message += "Usage: "+progname+" [options]\nOptions:\n";
	message += "	-h|--help           : Show this help and exit.\n";
	message += "	-v|--version        : Show version information and exit.\n";
	message += "	-n|--nofork         : Do not fork off into the background.\n";
	message += "	-c|--config  <path> : The path to the config file. Default: \"" DOODLE_CONFIG_PATH "\".\n";
	message += "	-s|--socket  <path> : Where to store the socket for user communication. Default: \"" DOODLE_SOCKET_PATH "\".\n";
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

	bool show_help;
	bool show_version;
	bool nofork;
	std::string config;
	std::string socket;


	GetOpt::GetOpt_pp ops(argc, argv);

	ops.exceptions(std::ios::failbit|std::ios::eofbit);
	try
	{
		ops>>GetOpt::OptionPresent('h', "help", show_help);
		ops>>GetOpt::OptionPresent('v', "version", show_version);
		ops>>GetOpt::OptionPresent('n', "nofork", nofork);
		ops>>GetOpt::Option('c', "config", config, DOODLE_CONFIG_PATH);
		ops>>GetOpt::Option('s', "socket", config, DOODLE_CONFIG_PATH);
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

	//{{{ Fork to the background

	//std::cout<<"Orig. PID is "<<getpid()<<"."<<std::endl;
	if( !nofork )
	{
		if( daemon(1, 0))
		{
			std::cerr<<"Could not fork."<<std::endl;
			return -1;
		}
	}
	//std::cout<<"New PID is "<<getpid()<<"."<<std::endl;
	//}}}

	//{{{
#ifdef USE_NOTIFICATIONS
		notify_init("Doodle");
		//NotifyNotification * Hello = notify_notification_new ("Hello world", "This is an example notification.", "dialog-information");
		//notify_notification_show (Hello, NULL);
		//g_object_unref(G_OBJECT(Hello));
#endif
	//}}}

	//{{{
#ifdef USE_SYSLOG
		setlogmask(LOG_UPTO(LOG_NOTICE));
		openlog("DOODLE", LOG_CONS|LOG_PID|LOG_NDELAY, LOG_LOCAL1);
		//syslog(LOG_NOTICE, "Writing to my Syslog");
#endif
	//}}}

	Doodle doodle(config);
	retval = doodle();

	#ifdef USE_NOTIFICATIONS
		notify_uninit();
	#endif
	#ifdef USE_SYSLOG
		closelog();
	#endif

	return retval;
}
