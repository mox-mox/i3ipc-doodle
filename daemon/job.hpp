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
	std::shared_ptr<uvw::TimerHandle> write_timer;
	void on_write_timer(void);

	//{{{ Timekeeping

	//{{{ Explanation
	//
	//{{{
	// Goal:
	// We want to keep track on how much time is spent on certain jobs and be able to reconstruct when time was spent on that jobs.
	//}}}
	//
	//{{{
	// Theory of operation:
	// When a job matches the current window- and workspace titles (so gets activated), two things happen:
	//	- A stopwatch is started.
	// 		When the job is de-activated, the stopwatch is stopped.
	//	- If not already running, a wait timer is started.
	//		When the wait timer runs out the stopwatch value is added to the Timefile on disk and the stopwatch is reset to zero.
	//		-> The stopwatch value is the time the job was active since the wait timer was started.
	//		-> The wait time of the wait timer is called slot length
	//	The Timefile for a job will contain the absolute amount of time that was spent on that job, as well as a list of when the job was active.
	//	Whenever the wait timer runs out one line is appended to the list stating when the time slot started and how much time was spent on the
	//	job during that time slot. Therefore the absolute time spent on the job is the sum of the second column of the timefile.
	//	Additionally, that sum is maintained in a special field in the beginning of the file, to allow for faster access.
	//}}}
	//
	//{{{
	// Example:
	// Operation for a Job A
	//												|
	// 	Some other Job is active					|	
	// 		|										|
	//		V										|
	//	Job A becomes active for the first time		|	The stopwatch is started from zero.
	// 		|	(At time 12341234, runs for 49 s)	|	The wait timer is started.
	//		V										|
	// 	Job A becomes inactive						|	The stopwatch is halted.
	// 		|										|	The wait timer continues.
	//		V										|
	//	Job A becomes active again					|	The stopwatch is started again, continuing from the old (non-zero) value.
	//		.	(runs 17 s)							|
	//		.										|
	//		.										|
	//		.										|
	//	The wait timer runs our						|	The stopwatch value is written to disk, the stopwatch is reset.
	//												|	If Job A is currently active, the stopwatch continues running from zero, else stays stopped.
	//												|	If Job A is currently active, the wait timer is started again.
	//												|	-> Total Time entry in Timefileis increased by (49s+17s)*1000 = 66000ms
	//												|	-> A line with <slot start time> <run time> is added to Timefile:
	//												|		12341234 66000
	// 												|
	//}}}
	//
	//{{{
	// Format: <File Job A>
	//
	//	# Jobname: "Job A"							|
	//	# Total time: 00000000000000082694 ms		|	The absolute time ever spent on Job A
	//	# Start-time	time-spent					|	
	//	1512649290601227311	2231					|	Time the job was started (first column) and how long it was active after that time (second column).
	//	1512654817286713262	53298					|	Start time is system clock ticks since epoch aka Unix Time, the active time is in milliseconds.
	//	1512654877288645310	27165					|
	//	1512654877288645310	3487					|
	//	1512654877288645310	9828898					|
	//	1512654877288645310	82349					|
	//	1512654877288645310	2340					|
	//	1512654877288645310	529						|
	//	1512654877288645310	32467492				|
	//	...
	//	...
	//}}}
	//
	//{{{
	// Logic:
	// Job running times: 1.) total_time  2.) runtime_since_slot_start 3.) runtime_since_job_start
	//		Construction:
	//			total_time := from timefile
	//
	//		Job_start():
	//			job_start_time := steady_clock::now()
	//			if no slot is running:
	//				Slot_start()
	//
	//		Job_stop():
	//			runtime_since_job_start := steady_clock::now() - job_start_time
	//			runtime_since_slot_start := runtime_since_slot_start + runtime_since_job_start
	//			//total_time := total_time + runtime_since_job_start
	//
	//		Slot_start():
	//			slot_start_time := system_clock::now()
	//			write_timer := slot_start_time + granularity
	//
	//		Slot stop/timeout_reached:
	//			runtime_since_job_start := steady_clock::now() - job_start_time; job_start_time := steady_clock::now()
	//			runtime_since_slot_start := runtime_since_slot_start + runtime_since_job_start
	//			total_time := total_time + runtime_since_slot_start
	//			write_times_to_disk()
	//}}}
	//
	//}}}

	milliseconds total_run_time;             // Set at startup (from file), updated on event
	steady_clock::time_point job_start;      // Get run_time with now-job_start
	system_clock::time_point slot_start;     // When the current slot was started, printed to the timefile
	milliseconds slot_run_time;              // The time spent on the job during this slot

	//{{{
	class Timefile
	{
		const uint8_t timefile_time_width = 20;
		fs::path path;
		milliseconds granularity;
		bool check_and_create_file(void);

		public:
		void set_path(std::string dir, std::string jobname);
		void set_granularity(milliseconds new_granularity);

		void add_time(milliseconds new_total, system_clock::time_point slot_start, milliseconds slot);

		milliseconds get_total_time(void);
		inline const milliseconds& get_granularity(void) { return granularity; }
		inline const fs::path get_path(void) { return path; }
	} timefile;
	//}}}

	//{{{
	inline milliseconds runtime_since_job_start(steady_clock::time_point now) const
	{
		// When the job is not active, runtime since activation should be 0
		return is_active ? std::chrono::duration_cast<milliseconds>(now-job_start) : milliseconds(0);
	}
	//}}}

	void start_slot();

	//}}}




	public:
	//{{{ Constructors

	explicit Job(const fs::path& jobfile_path, std::shared_ptr<uvw::Loop> loop);
	Job(const Job& other) = delete;
	Job(Job& other) = delete;
	Job(Job&& other) = delete;
	Job(void) = delete;
	~Job(void);
	//}}}

	void start(steady_clock::time_point now);
	void stop(steady_clock::time_point now);

	const std::string& get_jobname(void) const;
	milliseconds get_total_time(void) const;

	friend std::ostream& operator<<(std::ostream&stream, const Job& job);
};

