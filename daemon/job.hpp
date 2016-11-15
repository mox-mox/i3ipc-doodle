#pragma once
#include "window_matching.hpp"
#include <experimental/filesystem>
#include <unistd.h>


class Job : public Window_matching
{
	const std::string jobname;
	const fs::path timefile_path;


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

	#include "job_action.hpp"
	std::deque<Action> actions;


	public:
		//{{{ Constructors

		static Job create_from_jobfile(const fs::path&jobfile_path, ev::loop_ref&loop);
		Job(const std::string& jobname, Json::Value job, ev::loop_ref&loop);
		Job(Job&& other) noexcept;
		Job(void);
		~Job(void);
		//}}}

		void start(std::chrono::steady_clock::time_point now, const std::string& current_workspace, const std::string& window_title);
		void stop(std::chrono::steady_clock::time_point now);

		friend std::ostream& operator<<(std::ostream&stream, const Job& job);

		std::string get_jobname(void) const;
		unsigned int get_total_time(void) const;
		Json::Value get_times(uint64_t start=0, uint64_t end=0) const;
};
