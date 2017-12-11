#include <client.hpp>
#include "main.hpp"
#include "sockets.hpp"
#include <stropts.h>
#include <sstream>


// EDITLINE

//     el_set()
//           Set editline parameters.  op determines which parameter to set, and each operation has its own parameter
//           list.  Returns 0 on success, -1 on failure.
//           EL_UNBUFFERED, int flag
//                 If flag is zero, unbuffered mode is disabled (the default).  In unbuffered mode, el_gets() will
//                 return immediately after processing a single character.


//{{{
const char* prompt(EditLine* e)
{
	(void) e;
	return "doodle > ";
}
//}}}


//{{{
Client::Client(void) :
	loop(uvw::Loop::getDefault()),
	sigint(loop->resource<uvw::SignalHandle>()),
	daemon_pipe(loop->resource<uvw::PipeHandle>()),
	//console(loop->resource<uvw::IdleHandle>())
	console(loop->resource<uvw::TTYHandle>(uvw::StdIN, true))
{

	//{{{
	el = el_init(DOODLE_PROGRAM_NAME, stdin, stdout, stderr);
	if(!el) throw std::logic_error("no el");
	el_set(el, EL_PROMPT, prompt);
	el_set(el, EL_EDITOR, "vi");

	/* Initialize the history */
	myhistory = history_init();
	if (myhistory == nullptr)
	{
		throw std::runtime_error("History could not be initialised.");
	}

	/* Set the size of the history */
	history(myhistory, &hist_ev, H_SETSIZE, 800);

	//for(auto& command : commands)
	//{
	//	history(myhistory, &hist_ev, H_ENTER, (command.first + " " + command.second.args).c_str());
	//}


	/* This sets up the call back functions for history functionality */
	el_set(el, EL_HIST, history, myhistory);



	/* Make editline return after each character */
	el_set(el, EL_UNBUFFERED, 1);

	//}}}







	//{{{ Initialise all event watchers
	
	//{{{ <Ctrl+c>/SIGINT watcher

    sigint->on<uvw::SignalEvent>([this](const auto &, auto &){
			std::cout<<"Got SIGINT"<<std::endl;
			loop->stop();
    });
	sigint->start(SIGINT);
	//}}}

	//{{{ Daemon pipe

	daemon_pipe->init();
    daemon_pipe->on<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::PipeHandle &) {
			//handle.close();
			std::cout<<"Wrote to the daemon_pipe"<<std::endl;
    });

    daemon_pipe->on<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
        auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{ 'a' });
        handle.tryWrite(std::move(dataTryWrite), 1);
        auto dataWrite = std::unique_ptr<char[]>(new char[2]{ 'b', 'c' });
        handle.write(std::move(dataWrite), 2);
    });
	daemon_pipe->on<uvw::CloseEvent>([this](const uvw::CloseEvent &, uvw::PipeHandle &) {
			loop->walk([](uvw::BaseHandle &h){ h.close(); });
			std::cout<<"Server close"<<std::endl;
	});
	daemon_pipe->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) {
			sock.close();
			std::cout<<"Socket close"<<std::endl;
	});
	
    daemon_pipe->on<uvw::DataEvent>([](const uvw::DataEvent& evt, auto&){
			std::cout<<"Got something from the daemon_pipe: "<<std::endl;
			std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
    });

	daemon_pipe->open(open_socket(doodle_socket_path));

	daemon_pipe->read();
	//}}}

//
//	//{{{ Console watcher
//
//    console->on<uvw::IdleEvent>([this](auto&, auto&){
//			//std::cout<<"Running IdleEvent"<<std::endl;
//			int n;
//
//			sleep(1);
//			std::cout<<"------------------------"<<std::endl;
//			if (ioctl(0, I_NREAD, &n) == 0 && n > 0)
//			{
//				std::cout<<"AVAILABLE"<<std::endl;
//
//				char c;
//				std::cin>>c;
//				std::cout<<"Got \""<<c<<"\"."<<std::endl;
//			}
//
//
//			//if (!feof(stdin)) // Check if the stdin is empty
//			//{
//			//	std::cout<<"-"<<std::endl;
//			//	line = el_gets(el, &count);
//			//	std::cout<<"line = "<<line<<std::endl;
//			//}
//
//			//if (count > 0)
//			//{
//			//history(myhistory, &hist_ev, H_ENTER, line);
//			////sock.send(parse_command({line, static_cast<unsigned int>(count)}));
//			//std::cout<<(parse_command({line, static_cast<unsigned int>(count)}))<<std::endl;
//			//}
//			//else
//			//{
//			//std::cout<<"Invalid entry."<<std::endl;
//			//}
//			////return *this;
//    });
//	console->start();
//	//}}}
//


	//{{{ Console watcher

    console->on<uvw::DataEvent>([this](auto& evt, auto&){
			//(void) hndl;
			//std::cout<<"Got something from STDIN: "<<std::endl;
			//std::cout<<'	'<<std::string(&evt.data[0], evt.length)<<std::endl;
			//daemon_pipe->write(&evt.data[0], evt.length);


			std::cout<<"------------------------"<<std::endl;
			//if (int n = 0; ioctl(0, I_NREAD, &n) == 0 && n > 0)

			{
			//line = el_gets(el, &count);
			//std::cout<<"line = "<<line<<std::endl;

			}












			//{{{

			// Prepare fake stringstream
			std::stringstream fake_cin;
			//fake_cin << "Hi";
			fake_cin.write(&evt.data[0], evt.length);

			// Backup and change std::cin's streambuffer
			std::streambuf *backup = std::cin.rdbuf(); // back up cin's streambuf
			std::streambuf *psbuf = fake_cin.rdbuf(); // get file's streambuf
			std::cin.rdbuf(psbuf); // assign streambuf to cin

			// Read something, will come from out stringstream
			//std::string input;
			//std::cin >> input;

			line = el_gets(el, &count);
			std::cout<<"line = "<<line<<std::endl;






			// Verify that we actually read the right text
			//std::cout << input << std::endl;

			// Restore old situation
			std::cin.rdbuf(backup); // restore cin's original streambuf
			//}}}







    });
	console->on<uvw::CloseEvent>([](const uvw::CloseEvent&, uvw::TTYHandle& console) { console.reset(), std::cout<<"TTY close"<<std::endl; });
	console->mode(uvw::details::UVTTYModeT::RAW);
	console->read();
	//}}}


	//}}}
}
//}}}

//{{{
int Client::operator()(void)
{
	int retval = 0;
	logger<<"---------------Starting the event loop---------------"<<std::endl;

    loop->run();
	// Destroy the watchers after the loop stops
	loop->walk([](uvw::BaseHandle &h){ h.close(); });

	logger<<"Returning from event loop"<<std::endl;
	return retval;
}
//}}}
