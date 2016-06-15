#pragma once

#include <i3ipc++/ipc.hpp>
#include "job.hpp"
#include <ev++.h>

using window_id = uint64_t;

class Doodle : public sigc::trackable
{
	private:
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
		} settings;

		//{{{
		struct win_id_lookup_entry
		{
			Job* job = nullptr;
			std::string matching_name = "";
		};
		//}}}

		//}}}

		Job nojob;											// Special job that will not match any job. Used to keep track of unaccounted time.
		Job* current_job;
		std::deque<Job> jobs;

		std::map<window_id, win_id_lookup_entry> win_id_lookup;

		ev::default_loop loop;



		bool simulate_window_change(std::list< std::shared_ptr<i3ipc::container_t> > nodes);
		bool simulate_workspace_change(std::vector< std::shared_ptr<i3ipc::workspace_t> > workspaces);

		void on_window_change(const i3ipc::window_event_t& evt);
		void on_workspace_change(const i3ipc::workspace_event_t&  evt);

		inline win_id_lookup_entry find_job(const std::string& window_name);

		void SIGUSR1_cb(void);
		void SIGTERM_cb(void);

	public:
		//{{{ Constructor

		explicit Doodle(const std::string& config_path=".config/doodle"); // Todo: use xdg_config_path
		Doodle(const Doodle&) = delete;
		Doodle(Doodle&&) = delete;
		Doodle& operator=(const Doodle&) = delete;
		Doodle& operator=(Doodle&&) = delete;
		//}}}

		int operator()(void);
		friend std::ostream& operator<< (std::ostream& stream, Doodle const& doodle);
};


