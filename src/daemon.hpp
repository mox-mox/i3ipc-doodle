#pragma once


#include <i3ipc++/ipc.hpp>
#include "job.hpp"
#include <xcb/screensaver.h>
#include <uvw.hpp>
#include "fixed_array.hpp"
#include "parse_date.hpp"
#include <regex>


class Daemon: public sigc::trackable
{
	using window_id = uint64_t;
	i3ipc::connection i3_conn;
	std::shared_ptr<uvw::Loop> loop;

	Names current_window;

	fixed_array<Job> jobs;

	// current_job will only ever point to entries in job, which has the same life time, so no smart pointer is needed here
	Job* current_job;

	std::map<window_id, Job*> win_id_cache;

	//{{{ Idle time detection

	bool idle;
	bool suspended;
	xcb_connection_t * xcb_conn;
	xcb_screen_t * screen;
	void on_idle_timer(const uvw::TimerEvent &, uvw::TimerHandle &);
	//}}}



	std::shared_ptr<uvw::TimerHandle> idle_timer;
	std::shared_ptr<uvw::SignalHandle> sigint;
	std::shared_ptr<uvw::PipeHandle> i3_pipe;
	std::shared_ptr<uvw::PipeHandle> client_pipe;


	void on_window_change(const i3ipc::window_event_t& evt);
	void on_workspace_change(const i3ipc::workspace_event_t& evt);

	bool simulate_window_change(std::list<std::shared_ptr<i3ipc::container_t>> nodes);
	bool simulate_workspace_change(std::vector<std::shared_ptr<i3ipc::workspace_t>> workspaces);

	inline Job* find_job(void);

	#include "commands.hpp" 		// Everything for ipc command handling

	public:
		//{{{ Constructor

		explicit Daemon(void);	// Todo: use xdg_config_path
		Daemon(const Daemon&) = delete;
		Daemon(Daemon &&) = delete;
		Daemon& operator = (const Daemon&) = delete;
		Daemon& operator = (Daemon &&) = delete;
		~Daemon(void);
		//}}}

		int operator()(void);
		friend std::ostream& operator<<(std::ostream&stream, Daemon const&doodle);
};

