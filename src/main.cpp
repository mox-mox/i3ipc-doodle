#include <iostream>
#include "doodle_config.hpp"
#include <signal.h>

#include <i3ipc++/ipc.hpp>
//#include "auss.hpp"
#include "getopt_pp.h"
#include "doodle.hpp"
#include "logstream.hpp"
#include <cstdlib>
#include <algorithm>	// std::max

#ifdef USE_NOTIFICATIONS
	#include <libnotify/notify.h>
#endif
#ifdef USE_SYSLOG
	#include <syslog.h>
#endif



Doodle* doodle = nullptr;

//{{{ Help and version messages

std::string help_message(std::string progname)
{
	std::string message;
	message += "Usage: "+progname+" [options]\nOptions:";
	message += "	-h|--help    : Show this help and exit.\n";
	message += "	-v|--version : Show version information and exit.\n";
	message += "	-n|--nofork  : Do not fork off into the background.\n";
	return message;
}

void version_message()
{
	std::cout<<DOODLE_PROGRAM_NAME<<":"<<std::endl;
	std::cout<<"Git branch: "<<GIT_BRANCH<<", git commit hash: "<<GIT_COMMIT_HASH<<", Version: "<<DOODLE_VERSION_MAJOR<<":"<<DOODLE_VERSION_MINOR<<"."<<std::endl<<std::endl;
}
//}}}

//{{{ Signal handlers

void SIGUSR1_handler(int signum)
{
	(void) signum;
	std::cout<<"Received SIGUSR1!"<<std::endl;
	if( doodle )
	{
		std::cout<<*doodle<<std::endl;
	}
	else
	{
		error<<"Doodle not initialised."<<std::endl;
	}
}
void SIGTERM_handler(int signum)
{
	(void) signum;
	std::cout<<"Received SIGTERM!"<<std::endl;
	logger<<"shutting down"<<std::endl;
	exit(0);
}

void atexit_handler()
{
	delete doodle;

	#ifdef USE_NOTIFICATIONS
		notify_uninit();
	#endif
	#ifdef USE_SYSLOG
		closelog();
	#endif
}
//}}}


int main(int argc, char* argv[])
{
	//{{{ Argument handling

	bool show_help;
	bool show_version;
	bool nofork;

	GetOpt::GetOpt_pp ops(argc, argv);

	ops.exceptions(std::ios::failbit|std::ios::eofbit);
	try
	{
		ops>>GetOpt::OptionPresent('h', "help", show_help);
		ops>>GetOpt::OptionPresent('v', "version", show_version);
		ops>>GetOpt::OptionPresent('n', "nofork", nofork);
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

	std::cout<<"Orig. PID is "<<getpid()<<"."<<std::endl;
	if( !nofork )
	{
		if( daemon(1, 0))
		{
			std::cerr<<"Could not fork."<<std::endl;
			return -1;
		}
	}
	std::cout<<"New PID is "<<getpid()<<"."<<std::endl;
	//}}}




	const int atexit_registration_failed = std::atexit(atexit_handler);
	if( atexit_registration_failed )
	{
		std::cerr<<"Could not register the atexit function. Aborting."<<std::endl;
		return -1;
	}

	//{{{
#ifdef USE_NOTIFICATIONS
		notify_init("Doodle");
		//NotifyNotification * Hello = notify_notification_new ("Hello world", "This is an example notification.", "dialog-information");
		//notify_notification_show (Hello, NULL);
		//g_object_unref(G_OBJECT(Hello));
#endif

#ifdef USE_SYSLOG
		setlogmask(LOG_UPTO(LOG_NOTICE));
		openlog("DOODLE", LOG_CONS|LOG_PID|LOG_NDELAY, LOG_LOCAL1);
		syslog(LOG_NOTICE, "Writing to my Syslog");
#endif
	//}}}


	signal(SIGUSR1, SIGUSR1_handler);
	signal(SIGTERM, SIGTERM_handler);
	signal(SIGINT, SIGTERM_handler);



	i3ipc::connection conn;
	doodle = new Doodle(conn);

	conn.prepare_to_event_handling();








	for( ; ; )
	{
		std::cout<<"---------------Starting the event loop---------------"<<std::endl;
		int s1, s2, n, rv;
		fd_set readfds;
		struct timeval tv;
		s1 = 1;
		s2 = conn.get_file_descriptor();


		//// clear the set ahead of time
		FD_ZERO(&readfds);

		//// add our descriptors to the set
		FD_SET(s1, &readfds);
		FD_SET(s2, &readfds);

		n = std::max(s1, s2)+1;



		// wait until either socket has data ready to be recv()d (timeout 10.5 secs)
		tv.tv_sec = 10;
		tv.tv_usec = 500000;
		rv = select(n, &readfds, NULL, NULL, &tv);
		if( rv == -1 )
		{
			error<<"Select returned error."<<std::endl;
		}
		else if( rv == 0 )
		{
			logger<<"Timeout occurred! No data after 10.5 seconds."<<std::endl;
		}
		else
		{
			std::cout<<"---------------Data on one or more file descriptors---------------"<<std::endl;
			if( FD_ISSET(s2, &readfds))
			{
				std::cout<<"---------------Data on i3 socket connection---------------"<<std::endl;
				//recv(s2, buf2, sizeof buf2, 0);
				conn.handle_event();
			}
			if( FD_ISSET(s1, &readfds))
			{
				std::cout<<"---------------Data on stdin---------------"<<std::endl;
				//recv(s1, buf1, sizeof buf1, 0);
				std::string temp;
				std::cin>>temp;
				std::cout<<"Received |"<<temp<<"| on stdin."<<std::endl;
			}
		}
	}
	//for(;;)
	//{
	//	conn.handle_event();
	//}


	return 0;
}
