#ifndef DOODLE_HPP
#define DOODLE_HPP
#pragma once


#include <i3ipc++/ipc.hpp>
#include <deque>
#include "job.hpp"
#include <xcb/screensaver.h>

using window_id = uint64_t;

class Doodle: public sigc::trackable
{
	i3ipc::connection i3_conn;
	std::shared_ptr<uvw::Loop> loop;

	std::vector<Job> jobs;

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


	void on_window_change(const i3ipc::window_event_t& evt);
	void on_workspace_change(const i3ipc::workspace_event_t& evt);

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
};

#endif /* DOODLE_HPP */

