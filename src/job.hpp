#pragma once
#include <string>
#include <deque>
#include <json/json.h>
//#include "timespan.hpp"
#include <regex>
#include <thread>
#include <experimental/filesystem>
#include <mutex>
#include <condition_variable>


struct Job
{
	//{{{ Constructors

	Job(std::string jobname, Json::Value job, const std::experimental::filesystem::path& jobfile);
	Job(void);
	//Job(Job&&) = default;
    Job(Job&& o) noexcept;// : s(std::move(o.s)) { }
	~Job(void);
	//}}}

	void start(std::chrono::steady_clock::time_point now);
	void stop(std::chrono::steady_clock::time_point now);

	friend std::ostream& operator<<(std::ostream&stream, Job const&job);

	inline std::string match(const std::string& current_workspace, const std::string& window_title) const;

	std::string get_jobname(void) {return jobname;}




	private:
	std::string jobname;

		struct settings
		{
			static constexpr unsigned int  GRANULARITY_DEFAULT_VALUE = 3600;
			std::chrono::seconds granularity;
		} settings;
		std::experimental::filesystem::path jobfile;


		std::mutex times_mutex;
		std::condition_variable times_cv;
		struct timekeeping
		{
			std::chrono::seconds total;							// The total elapsed time of the job

			bool job_running = false;
			std::chrono::steady_clock::time_point job_start;	// For calculation of the passed time since the job was started
			std::chrono::seconds slot;							// The elapsed time in the current time slot is this + (now-job_start)
			bool waker_running = false;
			bool destructor_called = false;
		} times;






		void save_times(void);
		std::thread waker;


		std::deque < std::string > win_names_include;
		std::deque < std::string > win_names_exclude;
		std::deque < std::string > ws_names_include;
		std::deque < std::string > ws_names_exclude;

		inline bool ws_excluded(const std::string& current_workspace) const;
		inline bool ws_included(const std::string& current_workspace) const;

		inline bool win_excluded(const std::string& window_title) const;
		inline std::string win_included(const std::string& window_title) const;
};

//{{{ Name matching functions ( inline )

//{{{
bool Job::ws_excluded(const std::string& current_workspace) const
{
	for( std::string ws_name : ws_names_exclude )
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
	for( std::string ws_name : ws_names_include )
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
	for( std::string win_name : win_names_exclude )
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
std::string Job::win_included(const std::string& window_title) const
{
	for( std::string win_name : win_names_include )
	{
		if( std::regex_search(window_title, std::regex(win_name)))
		{
			return win_name;
		}
	}
	return "";
}
//}}}
//{{{
std::string Job::match(const std::string& current_workspace, const std::string& window_title) const
{
	if( !ws_names_include.empty() || !ws_names_exclude.empty())					// If there are workspaces specified, then ...
	{
		if( ws_excluded(current_workspace)) return "";	// ... the window may not be on an excluded workspace ...

		if( !ws_included(current_workspace)) return "";	// ... and it must reside on an included workspace ...
	}

	if( win_excluded(window_title)) return "";	// If the window matches an excluded name, forget about this job and consider the next one.

	return win_included(window_title);
}
//}}}
//}}}
