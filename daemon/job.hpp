#pragma once
#include "main.hpp"
#include "window_matching.hpp"
#include <uvw.hpp>

// File jobname.job:
//
// [window-names]
// include="foo*", "bar", "baz" # Regular expressions that are used to match the window title
// exclude="foo", "bar", "baz"
//
// [workspace-names]
// include="foo", "bar", "baz"
// exclude="foo", "bar", "baz"
//
// [timefile]
// path=/path/to/file
// granularity=1h
//
// [actions]
// # TODO
//









class Job final : public Window_matching
{
	const std::string jobname;
	bool is_active;

	std::shared_ptr<uvw::Loop> loop;

	//std::vector<std::string> window_include;
	//std::vector<std::string> window_exclude;

	//std::vector<std::string> workspace_include;
	//std::vector<std::string> workspace_exclude;

	//{{{
	struct Timefile
	{
		const uint8_t timefile_time_width = 20;
		fs::path path;
		milliseconds granularity;
		bool check_and_create_file(void);

		void set_path(std::string dir, std::string jobname);
		milliseconds get_total_time(void);
		void add_time(milliseconds new_total, system_clock::time_point slot_start, milliseconds slot);
	} timefile;
	//}}}

	//{{{ Timekeeping

	struct timekeeping
	{
		milliseconds total;                     // The total elapsed time of the job
		steady_clock::time_point job_start;     // For calculation of the passed time since the job was started

		milliseconds slot;                      // The elapsed time in the current time slot is this + (now-job_start)
		system_clock::time_point slot_start;    // = system_clock::now();
	} times;
	//}}}

	std::shared_ptr<uvw::TimerHandle> write_timer;
	void on_write_timer(void);


	public:
	Job(const fs::path& jobfile_path, std::shared_ptr<uvw::Loop> loop);
	~Job(void);

	void start(steady_clock::time_point now);
	void stop(steady_clock::time_point now);

	const std::string& get_jobname(void) const;
	milliseconds get_total_time(void) const;

	friend std::ostream& operator<<(std::ostream&stream, const Job& job);
};

