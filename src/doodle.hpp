#pragma once
#include <i3ipc++/ipc.hpp>
#include <map>
#include <deque>
#include <chrono>
//#include <json/json.h>

using window_id = uint64_t;

class Doodle : public sigc::trackable
{
	private:
		i3ipc::connection& conn;
		std::string current_ws_name;
		int evt_count;


		struct Job
		{
			std::string jobname;
			// Accounting
			//{{{
			class Timespan
			{
				public:
					Timespan(std::chrono::system_clock::time_point start, std::chrono::system_clock::time_point end) : start(start), end(end)
					{
						#ifdef DEBUG
						assert(start < end);
						#endif
					}
					operator std::chrono::duration<double>()
					{
						std::chrono::duration<double> diff = end-start;
						return diff;
					}

				private:
					std::chrono::system_clock::time_point start;
					std::chrono::system_clock::time_point end;
			};
			//}}}

			// Time-keeping
			std::deque<Timespan> times;

			// Matching
			std::deque<std::string> window_name_segments;	// Window name segments prefixed by "!" are excluded. Note: Group exclude names first.
			//std::deque<std::string> workspaces;			// Prefix by "!" to exclude specific workspaces
		};

		// Window recognition
		std::map<window_id, Job*> win_id_lookup;
		std::deque<Job> jobs;

		inline Job* find_job(std::string window_name);

	public:
		Doodle(i3ipc::connection& conn);

		void on_window_change(const i3ipc::window_event_t& evt);
		void on_workspace_change(const i3ipc::workspace_event_t&  evt);


		void print_workspaces();
};
