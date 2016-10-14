#include "cli.hpp"
#include <sstream>
#include <unistd.h>
#include "getopt_pp.h"
#include <iomanip>
#include "socket_watcher.hpp"
#include "commands.hpp"
#include <thread>
#include <mutex>

extern "C" {
#include <histedit.h>
}









  /* This holds all the state for our line editor */
  EditLine *el;

  /* This holds the info for our history */
  History *myhistory;

  HistEvent hist_ev;


const char* prompt(EditLine *e)
{
	(void) e;
	return "test> ";
}

int init_editline(void)
{
	/* Initialize the EditLine state to use our prompt function and
	   emacs style editing. */

	el = el_init(DOODLE_PROGRAM_NAME, stdin, stdout, stderr);
	el_set(el, EL_PROMPT, &prompt);
	el_set(el, EL_EDITOR, "vi");

	/* Initialize the history */
	myhistory = history_init();
	if (myhistory == 0) {
		fprintf(stderr, "history could not be initialized\n");
		return 1;
	}

	/* Set the size of the history */
	history(myhistory, &hist_ev, H_SETSIZE, 800);

	/* This sets up the call back functions for history functionality */
	el_set(el, EL_HIST, history, myhistory);

	return 0;
}

std::string entry;

//
////{{{
//void stdin_cb(ev::io& w, int)
//{
//	(void) w;
//	const char *line;
//	int count;
//    line = el_gets(el, &count);
//
//    if (count > 0) {
//      history(myhistory, &hist_ev, H_ENTER, line);
//      printf("You typed \"%s\"\n", line);
//    }
//
//	entry=line;
//
//	Socket_watcher& doodle_ipc = (*static_cast<Socket_watcher*>(w.data));
//	doodle_ipc<<parse_command(entry);
//}
////}}}
//


//{{{
std::mutex async_watcher_mutex;
void async_cb(ev::async& w, int)
{
	std::unique_lock<std::mutex> lock1(async_watcher_mutex);
	Socket_watcher& doodle_ipc = (*static_cast<Socket_watcher*>(w.data));
	doodle_ipc<<parse_command(entry);
}
//}}}


ev::default_loop loop;
ev::async async_watcher(loop);


void loop_thread(void)
{
	std::cout<<"Inside loop 2"<<std::endl;;

	Socket_watcher socket_watcher(settings.socket_path, loop);

	////{{{ Create a libev io watcher to respond to terminal input

	//ev::io stdin_watcher(loop);
	//stdin_watcher.set<stdin_cb>(static_cast<void*>(&socket_watcher));
	//stdin_watcher.set(STDIN_FILENO, ev::READ);
	//stdin_watcher.start();
	////}}}


	//{{{ Create a libev async watcher to respond to terminal input

	//ev::async async_watcher(loop);
	async_watcher.set<async_cb>(static_cast<void*>(&socket_watcher));
	async_watcher.start();
	//}}}


	std::cout<<"> "<<std::flush;
	loop.run();
	exit(EXIT_SUCCESS);

	return;
}










Args args;
Settings settings;

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

	init_editline();




	std::thread socket_communication(loop_thread);



	const char *line;
	int count;




	while(1)
	{
		std::cout<<"*"<<std::endl;
		line = el_gets(el, &count);

		if (count > 0) {
			history(myhistory, &hist_ev, H_ENTER, line);
			printf("You typed \"%s\"\n", line);
		}

		std::unique_lock<std::mutex> lock1(async_watcher_mutex);
		entry=line;
		async_watcher.send();
	}


	socket_communication.join();

	return 0;
}

