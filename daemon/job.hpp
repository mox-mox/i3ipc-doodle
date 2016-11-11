#pragma once
#include "main.hpp"
#include <string>
#include <deque>
#include <regex>
#include <experimental/filesystem>
#include <json/json.h>


class Job
{
	const std::string jobname;
	const fs::path jobfile_path;
	const fs::path timefile_path;
	//const std::streampos total_time_position;
	struct ev_loop* loop;


	//{{{
	struct job_settings
	{
		static constexpr unsigned int GRANULARITY_DEFAULT_VALUE = 3600;
		unsigned int granularity;
	} job_settings;
	//}}}


	//{{{ Timekeeping

	struct timekeeping
	{
		std::chrono::seconds total;								// The total elapsed time of the job
		std::chrono::steady_clock::time_point job_start;		// For calculation of the passed time since the job was started
		bool running = false;

		std::chrono::seconds slot;								// The elapsed time in the current time slot is this + (now-job_start)
		std::chrono::system_clock::time_point slot_start;	// = std::chrono::system_clock::now();
		bool timer_active = false;
	} times;
	//}}}

	ev::timer write_time_timer;
	void write_time_cb(void);



	//{{{ Job matching

	//{{{
	struct matchers
	{
		std::deque < std::string > win_names_include;
		std::deque < std::string > win_names_exclude;
		std::deque < std::string > ws_names_include;
		std::deque < std::string > ws_names_exclude;
	} matchers;
	//}}}

	inline bool ws_excluded(const std::string& current_workspace) const;
	inline bool ws_included(const std::string& current_workspace) const;

	inline bool win_excluded(const std::string& window_title) const;
	inline bool win_included(const std::string& window_title) const;
	//}}}

	public:
		//{{{ Constructors

		Job(const fs::path&jobfile_path, ev::loop_ref&loop);
		Job(Job && o) noexcept;
		Job(void);
		~Job(void);
		//}}}

		void start(std::chrono::steady_clock::time_point now);
		void stop(std::chrono::steady_clock::time_point now);

		friend std::ostream& operator<<(std::ostream&stream, Job const&job);

		inline bool match(const std::string& current_workspace, const std::string& window_title) const;

		std::string get_jobname(void) const;
		unsigned int get_total_time(void) const;
		Json::Value get_times(uint64_t start=0, uint64_t end=0) const;
		Json::Value get_win_names(void) const;
		Json::Value get_ws_names(void) const;

		//static Job no_job;												// Special job that will not match any job. Used to keep track of unaccounted time.
		//static Job* nojob;
};


//{{{ Name matching functions ( inline )

//{{{
bool Job::ws_excluded(const std::string& current_workspace) const
{
	for( std::string ws_name : matchers.ws_names_exclude )
	{
		if( std::regex_search(current_workspace, std::regex(ws_name)))
		{
			return true;
		}
	}
	return false;
}
//}}}
//{{{
bool Job::ws_included(const std::string& current_workspace) const
{
	for( std::string ws_name : matchers.ws_names_include )
	{
		if( std::regex_search(current_workspace, std::regex(ws_name)))
		{
			return true;
		}
	}
	return false;
}
//}}}

//{{{
bool Job::win_excluded(const std::string& window_title) const
{
	for( std::string win_name : matchers.win_names_exclude )
	{
		if( std::regex_search(window_title, std::regex(win_name)))
		{
			return true;
		}
	}
	return false;
}
//}}}
//{{{
bool Job::win_included(const std::string& window_title) const
{
	for( std::string win_name : matchers.win_names_include )
	{
		if( std::regex_search(window_title, std::regex(win_name)))
		{
			return true;
		}
	}
	return false;
}
//}}}
//{{{
bool Job::match(const std::string& current_workspace, const std::string& window_title) const
{
	std::cout<<"Job::match(this="<<this<<")"<<std::endl;
	if(!matchers.ws_names_exclude.empty())
	   	if(ws_excluded(current_workspace))
			return false;	// ... the window may not be on an excluded workspace ...

	if(!matchers.ws_names_include.empty() && !ws_included(current_workspace)) return false;	// ... and it must reside on an included workspace ...

	if( win_excluded(window_title)) return false;	// If the window matches an excluded name, forget about this job and consider the next one.

	return win_included(window_title);
}
//}}}
//}}}
