#pragma once

#include <i3ipc++/ipc.hpp>
#include <map>
#include <deque>
#include <chrono>
#include <ostream>
#include "logstream.hpp"
#include <json/json.h>
#include <ctime>

using window_id = uint64_t;

class Doodle : public sigc::trackable
{
	private:
		//{{{ Nested classes

		struct
		{
			unsigned int max_idle_time = 0;
			bool detect_ambiguity = false;
		} settings;


		//{{{
		class Timespan
		{
			std::time_t start;
			std::time_t end;
			public:
				Timespan(void);
				Timespan(Json::Value timespan);
				void stop(void);
				operator std::time_t() const;
		};
		//}}}
		//{{{
		struct Job
		{
			std::string jobname;

			std::deque<Timespan> times;
			//std::time_t aggregate_time;						// Small amounts of time, that are not tracked

			//std::deque<std::string> window_name_segments;	// Window name segments prefixed by "!" are excluded. Note: Group exclude names first.
			//std::deque<std::string> workspaces;				// Prefix by "!" to exclude specific workspaces

			std::deque<std::string> win_names_include;
			std::deque<std::string> win_names_exclude;
			std::deque<std::string> ws_names_include;
			std::deque<std::string> ws_names_exclude;
			void print(void);

			Job(Json::Value job);
			Job(void);
			//Job(const std::string& jobname,
			//    const std::deque<Timespan>& times,
			//    const std::deque<std::string>& window_name_segments,
			//    const std::deque<std::string>& workspaces);
			void start(void);
			void stop(void);
			friend std::ostream& operator<< (std::ostream& stream, Job const& job);

		};
		friend std::ostream& operator<< (std::ostream& stream, Job const& job);
		//}}}
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
		inline win_id_lookup_entry find_job(const std::string& window_name);
		bool ws_excluded(const Job& job);
		bool ws_included(const Job& job);

		bool win_excluded(const Job& job, const std::string& window_title);
		std::string win_included(const Job& job, const std::string& window_title);



		bool simulate_window_change(std::list< std::shared_ptr<i3ipc::container_t> > nodes);
		bool simulate_workspace_change(std::vector< std::shared_ptr<i3ipc::workspace_t> > workspaces);

		i3ipc::connection& conn;
		std::string current_workspace;
		int ws_evt_count;
		int win_evt_count;



	public:
		Doodle(i3ipc::connection& conn, std::string config_filename=".doodle_config");

		void on_window_change(const i3ipc::window_event_t& evt);
		void on_workspace_change(const i3ipc::workspace_event_t&  evt);

		void write_config(void);

		friend std::ostream& operator<< (std::ostream& stream, Doodle const& doodle);
};



