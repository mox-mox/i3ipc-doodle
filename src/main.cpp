// This is just the workplaces example from i3ipc++ that I will use as a starting point.
// Right now, I use it to test, if I can sucessfully compile the project.


#include <iostream>
#include "doodle_config.hpp"
#include <signal.h>

//#include <i3ipc++/ipc.hpp>
#include <i3ipc++/ipc.hpp>
#include "auss.hpp"
#include "getopt_pp.h"
#include "doodle.hpp"
#include "logstream.hpp"
//#include "jason.hpp"

#ifdef USE_NOTIFICATIONS
#include <libnotify/notify.h>
#endif


//{{{ Help and version messages

std::string help_message(std::string progname)
{
	std::string message;
	message+="Usage: "+progname+" [options]\nOptions:";
	//std::cerr << "-s|--steps   <NUM>: Set the number of simulation steps."<<std::endl;
	//std::cerr << "-r|--radius  <NUM>: Set the radius of the stimuli."<<std::endl;
	//std::cerr << "-h|--heat    <NUM>: Set the heat of the stimuli."<<std::endl;
	return message;
}

void version_message()
{
	std::cout<<DOODLE_PROGRAM_NAME<<":"<<std::endl;
	std::cout<<"Git branch: "<<GIT_BRANCH<<", git commit hash: "<<GIT_COMMIT_HASH<<", Version: "<<DOODLE_VERSION_MAJOR<<":"<<DOODLE_VERSION_MINOR<<"."<<std::endl<<std::endl;
}
//}}}

Doodle* doodle;


void signal_handler(int signum)
{
	if (signum == SIGUSR1)
	{
		std::cout<<"Received SIGUSR1!"<<std::endl;
		if(doodle)
		{
			std::cout<<*doodle<<std::endl;
		}
		else
		{
			error<<"Doodle not initialised."<<std::endl;
		}
	}
}




int main(int argc, char* argv[])
{
	//{{{ Argument handling

	bool show_help;
	bool show_version;
	//bool jason;

	GetOpt::GetOpt_pp ops(argc, argv);

	ops.exceptions(std::ios::failbit|std::ios::eofbit);
	try
	{
		//ops >> GetOpt::Option('s', "steps", steps, 100);
		//ops >> GetOpt::Option('r', "radius", radius, 3);
		//ops >> GetOpt::Option('h', "heat", heat, 127.0);
		//ops>>GetOpt::OptionPresent('j', "jason", jason);
		ops>>GetOpt::OptionPresent('h', "help", show_help);
		ops>>GetOpt::OptionPresent('v', "version", show_version);
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

	//if(jason)
	//{
	//	write_json();
	//	return 0;
	//}

	//}}}


#ifdef USE_NOTIFICATIONS
	notify_init ("Hello world!");
	NotifyNotification * Hello = notify_notification_new ("Hello world", "This is an example notification.", "dialog-information");
	notify_notification_show (Hello, NULL);
	g_object_unref(G_OBJECT(Hello));
	notify_uninit();
#endif

	i3ipc::connection conn;

	doodle = new Doodle(conn);

	signal(SIGUSR1, signal_handler);
	conn.prepare_to_event_handling();


	for(;;)
	{
		conn.handle_event();
	}

	return 0;
}








//class Emitter
//{
//	public:
//		Emitter(std::string const&n) : _number(0), _name(n) {}
//		void increaseNumber(int by)
//		{
//			_number += by;
//			signal_number_changed.emit(by);
//			//signal_number_changed.emit();
//		}
//		int getNumber()
//		{
//			return _number;
//		}
//		std::string const& getName()
//		{
//			return _name;
//		}
//		sigc::signal < void, int > signal_number_changed;
//		sigc::signal <void, i3ipc::WindowEventType>  signal_window_event; /**< Window event signal */
//		//sigc::signal <void >  signal_window_event; /**< Window event signal */
//	private:
//		int _number;
//		std::string _name;
//};
//
//std::ostream& operator<<(std::ostream&o, Emitter&e)
//{
//    o<<e.getNumber();
//    return o;
//}
//
//
//
//
//
//class Receiver : public sigc::trackable
//{
//	public:
//		Receiver(Emitter&emitter)
//		{
//			emitter.signal_number_changed.connect(sigc::mem_fun(*this, &Receiver::_handleNumberChange));
//			emitter.signal_window_event.connect(sigc::mem_fun(*this, &Receiver::_handleWindowChange));
//		}
//	private:
//		void _handleNumberChange(int increment)
//		//void _handleNumberChange()
//		{
//			//int increment = 10;
//			std::cout<<"The number increased by "<<increment<<std::endl;
//		}
//		void _handleWindowChange(i3ipc::WindowEventType increment)
//		{
//			std::cout<<"Window changed"<<std::endl;
//		}
//};
//
//
//
//int main()
//{
//	Emitter emil("emil");
//	Receiver richard(emil);
//
//	emil.increaseNumber(20);
//
//
//	return 0;
//}



