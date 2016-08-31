#ifndef DOODLE_HPP
#define DOODLE_HPP
#pragma once

//#warning "<<<<<<<<<<<<<<<<<<<< doodle.hpp: include  doodle_config.hpp >>>>>>>>>>>>>>>>>>>>"

//#include <iostream>
//#include <sstream>
//#include <iomanip>
//#include <chrono>
//#include <ctime>

#include "doodle_config.hpp"
//#warning "++++++++++++++++++++ doodle.hpp: include  doodle_config.hpp ++++++++++++++++++++"
//#include <ev++.h>
#include <i3ipc++/ipc.hpp>
#include "job.hpp"
#include <xcb/screensaver.h>
#include <json/json.h>
//#include "console_stream.hpp"

using window_id = uint64_t;

class Doodle: public sigc::trackable
{
	i3ipc::connection conn;
	const std::string config_path;
	std::string current_workspace;

	//{{{ Nested classes

	struct settings
	{
		static constexpr unsigned int MAX_IDLE_TIME_DEFAULT_VALUE = 60;
		unsigned int max_idle_time = MAX_IDLE_TIME_DEFAULT_VALUE;
		static constexpr bool DETECT_AMBIGUITY_DEFAULT_VALUE = false;
		bool detect_ambiguity = DETECT_AMBIGUITY_DEFAULT_VALUE;
		std::string socket_path = DOODLE_SOCKET_PATH_DEFAULT;
	} settings;

	//{{{
	struct win_id_lookup_entry
	{
		Job* job = nullptr;
		std::string matching_name = "";
	};
	//}}}

	//}}}

	Job nojob;												// Special job that will not match any job. Used to keep track of unaccounted time.
	Job* current_job;
	std::deque<Job> jobs;

	std::map<window_id, win_id_lookup_entry> win_id_lookup;

	ev::default_loop loop;

	//{{{ Idle time detection

	bool idle;
	bool suspended;
	xcb_connection_t * connection;
	xcb_screen_t * screen;
	ev::timer idle_watcher_timer;
	void idle_time_watcher_cb(ev::timer& io_watcher, int revents);
	//}}}



	bool simulate_window_change(std::list < std::shared_ptr < i3ipc::container_t>>nodes);
	bool simulate_workspace_change(std::vector < std::shared_ptr < i3ipc::workspace_t>>workspaces);

	void on_window_change(const i3ipc::window_event_t& evt);
	void on_workspace_change(const i3ipc::workspace_event_t& evt);

	inline win_id_lookup_entry find_job(const std::string& window_name);




	#include "doodle_client_watcher.hpp" //struct Client_watcher;

	void SIGUSR1_cb(void);
	void SIGTERM_cb(void);


	#include "doodle_socket_watcher.hpp" //struct socket_watcher;
	Socket_watcher socket_watcher;




	#include "doodle_terminal.hpp" //struct terminal_t;
	terminal_t terminal;



	public:
		//{{{ Constructor

		explicit Doodle(const std::string& config_path = ".config/doodle");	// Todo: use xdg_config_path
		Doodle(const Doodle&) = delete;
		Doodle(Doodle &&) = delete;
		Doodle& operator = (const Doodle&) = delete;
		Doodle& operator = (Doodle &&) = delete;
		~Doodle(void);
		//}}}

		int operator()(void);
		friend std::ostream& operator<<(std::ostream&stream, Doodle const&doodle);
};

#endif /* DOODLE_HPP */
