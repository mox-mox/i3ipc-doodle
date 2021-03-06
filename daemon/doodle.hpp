#ifndef DOODLE_HPP
#define DOODLE_HPP
#pragma once


#include "main.hpp"
#include <i3ipc++/ipc.hpp>
#include "job.hpp"
#include <xcb/screensaver.h>
//#include <json/json.h>

using window_id = uint64_t;

class Doodle: public sigc::trackable
{
	i3ipc::connection i3_conn;
	std::string current_workspace;
	std::string current_window_name;

	//Job nojob;												// Special job that will not match any job. Used to keep track of unaccounted time.
	Job* current_job;
	std::deque<Job> jobs;

	std::map<window_id, Job*> win_id_cache;

	ev::default_loop loop;

	//{{{ Idle time detection

	bool idle;
	bool suspended;
	xcb_connection_t * xcb_conn;
	xcb_screen_t * screen;
	ev::timer idle_watcher_timer;
	void idle_time_watcher_cb(ev::timer& io_watcher, int revents);
	//}}}



	bool simulate_window_change(std::list < std::shared_ptr < i3ipc::container_t>>nodes);
	bool simulate_workspace_change(std::vector < std::shared_ptr < i3ipc::workspace_t>>workspaces);

	void on_window_change(const i3ipc::window_event_t& evt);
	void on_workspace_change(const i3ipc::workspace_event_t& evt);

	inline Job* find_job(void);




	#include "doodle_client_watcher.hpp" 		//struct Client_watcher;

	void SIGUSR1_cb(void);
	void SIGTERM_cb(void);


	#include "doodle_socket_watcher.hpp" 		//struct socket_watcher;
	Socket_watcher socket_watcher;




	#include "doodle_terminal.hpp"       		//struct Terminal;
	Terminal terminal;



	public:
		//{{{ Constructor

		explicit Doodle(void);	// Todo: use xdg_config_path
		Doodle(const Doodle&) = delete;
		Doodle(Doodle &&) = delete;
		Doodle& operator = (const Doodle&) = delete;
		Doodle& operator = (Doodle &&) = delete;
		~Doodle(void);
		//}}}

		int operator()(void);
		friend std::ostream& operator<<(std::ostream&stream, Doodle const&doodle);
		#ifdef COMPILE_UNIT_TESTS
			#include "doodle_diagnostics.hpp"		// all the testing functions for the doodle class
		#endif
};

#endif /* DOODLE_HPP */
