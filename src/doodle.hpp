#pragma once

#include <i3ipc++/ipc.hpp>
#include <map>
#include <deque>
#include <chrono>
#include <ostream>
#include "logstream.hpp"
//#include <json/json.h>

using window_id = uint64_t;
using timepoint = std::chrono::system_clock::time_point;
using duration  = std::chrono::duration<double>;

class Doodle : public sigc::trackable
{
	private:
		//{{{
		class Timespan
		{
			public:
				Timespan(void) : start(std::chrono::system_clock::now()) {}
				void stop(void) { end=std::chrono::system_clock::now(); }
				operator duration() const  { return (end!=timepoint() ? end : std::chrono::system_clock::now()) - start; }

			private:
				timepoint start;
				timepoint end;
		};
		//}}}
		//{{{
		struct Job
		{
			void start(void)
			{
				times.push_back(Timespan());
			}
			void stop(void)
			{
				times.back().stop();
			}
			std::string jobname;

			// Time-keeping
			std::deque<Timespan> times;
			//duration aggregate_time;						// Small amounts of time, that are not tracked

			// Matching
			std::deque<std::string> window_name_segments;	// Window name segments prefixed by "!" are excluded. Note: Group exclude names first.
			std::deque<std::string> workspaces;				// Prefix by "!" to exclude specific workspaces
		};
		friend std::ostream& operator<< (std::ostream& stream, Job const& job);
		//}}}


		Job nojob;											// Special job that will not match any job. Used to keep track of unaccounted time.
		Job* current_job;
		std::deque<Job> jobs;

		// Window recognition
		//{{{
		struct win_id_lookup_entry
		{
			Job* job = nullptr;
			std::string matching_string = "";
		};
		//}}}
		std::map<window_id, win_id_lookup_entry> win_id_lookup;
		inline win_id_lookup_entry find_job(std::string window_name);
		bool simulate_window_change(std::list< std::shared_ptr<i3ipc::container_t> > nodes);
		bool simulate_workspace_change(std::vector< std::shared_ptr<i3ipc::workspace_t> > workspaces);

		i3ipc::connection& conn;
		std::string current_workspace;
		int ws_evt_count;
		int win_evt_count;




	public:
		Doodle(i3ipc::connection& conn);

		void on_window_change(const i3ipc::window_event_t& evt);
		void on_workspace_change(const i3ipc::workspace_event_t&  evt);

		friend std::ostream& operator<< (std::ostream& stream, Doodle const& doodle);
};



