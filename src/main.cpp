// This is just the workplaces example from i3ipc++ that I will use as a starting point.
// Right now, I use it to test, if I can sucessfully compile the project.


#include <iostream>
#include "doodle_config.hpp"

//#include <i3ipc++/ipc.hpp>
#include "getopt_pp.h"
#include "doodle.hpp"



//{{{ Help and version messages

void help_message(std::string progname)
{
	std::cerr<<"Usage: "<<progname<<" [options]"<<std::endl;
	std::cerr<<"Options:"<<std::endl;
	//std::cerr << "-s|--steps   <NUM>: Set the number of simulation steps."<<std::endl;
	//std::cerr << "-r|--radius  <NUM>: Set the radius of the stimuli."<<std::endl;
	//std::cerr << "-h|--heat    <NUM>: Set the heat of the stimuli."<<std::endl;
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

	bool show_help;
	bool show_version;

	GetOpt::GetOpt_pp ops(argc, argv);

	ops.exceptions(std::ios::failbit|std::ios::eofbit);
	try
	{
		//ops >> GetOpt::Option('s', "steps", steps, 100);
		//ops >> GetOpt::Option('r', "radius", radius, 3);
		//ops >> GetOpt::Option('h', "heat", heat, 127.0);
		ops>>GetOpt::OptionPresent('h', "help", show_help);
		ops>>GetOpt::OptionPresent('v', "version", show_version);
	}
	catch(GetOpt::GetOptEx ex)
	{
		std::cerr<<"Error in arguments"<<std::endl;

		help_message(argv[0]);
		return -1;
	}
	if( show_help )
	{
		help_message(argv[0]);
		return 0;
	}
	if( show_version )
	{
		version_message();
		return 0;
	}

	//}}}

	i3ipc::I3Connection conn;
	Doodle mydoodle(conn);
	conn.prepare_to_event_handling();

	//mydoodle.subscribe_to_window_change();
	for(;;)
	{
		conn.handle_event();
	}

	//mydoodle.print_workspaces();



	////conn.signal_window_event.connect(/*TODO*/);
	////conn.subscribe(i3ipc::ET_WINDOW);

	//for( auto&  w : conn.get_workspaces())
	//{
	//	std::cout<<'#'<<std::hex<<w.num<<std::dec
	//	         <<"\n\tName: "<<w.name
	//	         <<"\n\tVisible: "<<w.visible
	//	         <<"\n\tFocused: "<<w.focused
	//	         <<"\n\tUrgent: "<<w.urgent
	//	         <<"\n\tRect: "
	//	         <<"\n\t\tX: "<<w.rect.x
	//	         <<"\n\t\tY: "<<w.rect.y
	//	         <<"\n\t\tWidth: "<<w.rect.width
	//	         <<"\n\t\tHeight: "<<w.rect.height
	//	         <<"\n\tOutput: "<<w.output
	//	         <<std::endl;
	//}
	return 0;
}




#include <i3ipc++/ipc.hpp>




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



