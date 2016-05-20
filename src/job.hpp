#pragma once
#include <string>
#include <deque>
#include <json/json.h>
#include "timespan.hpp"
#include <regex>
#include <thread>
#include <experimental/filesystem>
#include <mutex>


struct Job
{
	//{{{ Constructors

	Job(std::string jobname, Json::Value job, const std::experimental::filesystem::path& jobfile);
	Job(void);
	//}}}

	void start(void);
	void stop(void);

	friend std::ostream& operator<<(std::ostream&stream, Job const&job);

	inline std::string match(const std::string& current_workspace, const std::string& window_title) const;



	const std::string jobname;



	private:
		std::experimental::filesystem::path jobfile;

		std::time_t total_time;


		std::deque < Timespan > times;
		std::mutex times_mutex;

		void save_times(std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> when);


		std::deque < std::string > win_names_include;
		std::deque < std::string > win_names_exclude;
		std::deque < std::string > ws_names_include;
		std::deque < std::string > ws_names_exclude;

		inline bool ws_excluded(const std::string& current_workspace) const;
		inline bool ws_included(const std::string& current_workspace) const;

		inline bool win_excluded(const std::string& window_title) const;
		inline std::string win_included(const std::string& window_title) const;
	std::thread waker;
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
