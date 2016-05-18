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
		i3ipc::connection& conn;
		const std::string config_path;
		std::string current_workspace;

		//{{{ Nested classes

		struct
		{
			#define MAX_IDLE_TIME_DEFAULT_VALUE 60
			unsigned int max_idle_time = MAX_IDLE_TIME_DEFAULT_VALUE;
			#define DETECT_AMBIGUITY_DEFAULT_VALUE false
			bool detect_ambiguity = DETECT_AMBIGUITY_DEFAULT_VALUE;
			//#define JOBS_FILE_DEFAULT_VALUE ""
			//std::string jobs_file = JOBS_FILE_DEFAULT_VALUE;
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

			std::time_t total_time;


			std::deque<Timespan> times;
			//std::time_t aggregate_time;						// Small amounts of time, that are not tracked

			std::deque<std::string> win_names_include;
			std::deque<std::string> win_names_exclude;
			std::deque<std::string> ws_names_include;
			std::deque<std::string> ws_names_exclude;

			Job(std::string jobname, Json::Value job);
			Job(Json::Value job);
			Job(void);
			void start(void);
			void stop(void);

			friend std::ostream& operator<< (std::ostream& stream, Job const& job);

			inline std::string match(const std::string& current_workspace, const std::string& window_title) const;

			private:
			inline bool ws_excluded(const std::string& current_workspace) const;
			inline bool ws_included(const std::string& current_workspace) const;

			inline bool win_excluded(const std::string& window_title) const;
			inline std::string win_included(const std::string& window_title) const;

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



		bool simulate_window_change(std::list< std::shared_ptr<i3ipc::container_t> > nodes);
		bool simulate_workspace_change(std::vector< std::shared_ptr<i3ipc::workspace_t> > workspaces);

		void read_config(Json::Value config);
		//void write_config(Json::Value config);

	public:
		Doodle(i3ipc::connection& conn, const std::string& config_path=".config/doodle"); // Todo: use xdg_config_path

		void on_window_change(const i3ipc::window_event_t& evt);
		void on_workspace_change(const i3ipc::workspace_event_t&  evt);

		//void write_config(void);

		friend std::ostream& operator<< (std::ostream& stream, Doodle const& doodle);
};


