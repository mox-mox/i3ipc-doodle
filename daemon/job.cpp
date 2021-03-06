#include "job.hpp"
#include <fstream>


//{{{
Job Job::create_from_jobfile(const fs::path& jobfile_path, ev::loop_ref& loop)
{
	debug<<"Job::create_from_jobfile(jobfile_path = "<<jobfile_path<<" );"<<std::endl;
	std::ifstream jobfile(jobfile_path);
	Json::Value job;
	Json::Reader reader;
	if( !reader.parse(jobfile, job, false))
	{
		error<<reader.getFormattedErrorMessages()<<std::endl;
		throw std::runtime_error("Cannot parse job file for job "+jobfile_path.stem().string()+".");
	}

	return Job(jobfile_path.stem(), job, loop);
}
//}}}


//{{{
Job::Job(const std::string& jobname, Json::Value job, ev::loop_ref& loop) :
	Window_matching(job),
	jobname(jobname),
	timefile_path(fs::path(settings.data_dir)/(jobname+".times")),
	job_settings{job.get("granularity", job_settings.GRANULARITY_DEFAULT_VALUE).asUInt()},
	times{seconds(0), steady_clock::time_point(), false,  seconds(0), system_clock::time_point(), false},
	write_time_timer(loop)
{
	debug<<"Constructing job "<<jobname<<" at "<<this<<'.'<<std::endl;

	//{{{ Time file

	if(!fs::exists(timefile_path))
	{
		logger<<"There was no time file for job \""<<jobname<<"\". Creating one: "<<timefile_path.string()<<std::endl;
		std::ofstream timefile(timefile_path);
		if(!timefile.is_open())
		{
			error<<"Could not create time file for job \""<<jobname<<"\"."<<std::endl;
			exit(EXIT_FAILURE);
		}
		timefile<<"# Total time: "<<std::setfill('0')<<std::setw(20)<<0<<"\n# Start-time	time-spent"<<std::endl;
	}

	std::ifstream timefile(timefile_path);
	if(!timefile.is_open())
	{
		error<<"Could not open time file for job \""<<jobname<<"\"."<<std::endl;
		exit(EXIT_FAILURE);
	}
	timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// Go to where
	timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// the total time
	timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// is stored.
	uint64_t temp;
	timefile>>temp;
	times.total = seconds(temp);
	//}}}

	write_time_timer.set < Job, &Job::write_time_cb > (this);


	//{{{ Actions

	for(auto& action : job.get("actions", "no actions"))
	{
		actions.push_back({this, action});
	}
	//}}}
}
//}}}

//{{{
Job::Job(Job&& other) noexcept :
	Window_matching(std::move(other)),
	jobname(std::move(other.jobname)),
	timefile_path(std::move(other.timefile_path)),
	job_settings(std::move(other.job_settings)),
	times(std::move(other.times)),
	write_time_timer(other.write_time_timer.loop),
	actions(std::move(other.actions))
{
	debug<<"Move-Constructing job "<<jobname<<" at "<<this<<'.'<<std::endl;
	// Note that the timer is constructed with the original event loop in the intialiser list.
	// There is no real copy (or move) constructor for ev::timer, so this hack is used to re-initialise the timer
	write_time_timer.set < Job, &Job::write_time_cb > (this);
}
//}}}

//{{{
Job::Job(void) :
	Window_matching(),
	jobname("NOJOB"),
	timefile_path(),
	times{seconds(0), steady_clock::time_point(), false,  seconds(0), system_clock::time_point(), true},
	actions()
{}
//}}}

//{{{
Job::~Job(void)
{
	if(write_time_timer.is_active())
	{
		debug<<"Writing last timing for job "<<jobname<<std::endl;
		write_time_cb();
	}
}
//}}}



//{{{
void Job::start(steady_clock::time_point start_time, const std::string& current_workspace, const std::string& window_title)
{
	if(!times.running)
	{
		times.job_start = start_time;
		times.running = true;

		if(!times.timer_active)
		{
			times.slot_start = system_clock::now();
			times.slot = seconds(0);
			times.timer_active = true;
			write_time_timer.start(job_settings.granularity);
		}

		for(Action& action : actions)
		{
			action(current_workspace, window_title);
		}
	}
	else error<<"Job "<<jobname<<": Trying to start already running process."<<std::endl;
}
//}}}

//{{{
void Job::stop(steady_clock::time_point now)
{
	if(times.running)
	{
		seconds elapsed = std::chrono::duration_cast < seconds > (now-times.job_start);
		times.total     += elapsed;
		times.slot      += elapsed;
		times.job_start  = steady_clock::time_point();	// reset to 'zero'
		times.running = false;

		for(Action& action : actions)
		{
			action.stop();
		}
	}
	else error<<"Job "<<jobname<<": Trying to stop already stopped process."<<std::endl;
}
//}}}

//{{{
void Job::write_time_cb(void)
{
	steady_clock::time_point now = steady_clock::now();

	if( times.running )							// Account for a currently running job.
	{
		seconds elapsed = std::chrono::duration_cast < seconds > (now-times.job_start);
		times.total     += elapsed;
		times.slot      += elapsed;
		times.job_start  = now;
	}

	debug<<". Writing time for "<<jobname<<" to disk file "<<timefile_path<<": "<<times.slot.count()<<" seconds active in the last "<<job_settings.granularity<<" seconds."<<std::endl;

	std::fstream timefile(timefile_path);
	if(timefile.is_open())
	{
		timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// Go to where
		timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// the total time
		timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// is stored.
		timefile<<std::setw(20)<<std::setfill('0')<<times.total.count();
		timefile.seekg(0, std::ios_base::end);	// Go to the end of the jobfile
		timefile<<times.slot_start.time_since_epoch().count()<<"\t"<<times.slot.count()<<std::endl;
		timefile.close();
		times.slot = seconds(0);
	}
	else
	{
		throw std::runtime_error("Could not re-open timefile \""+timefile_path.string()+"\".");
	}

	if( times.running ) // If the job is currently not running
	{
		times.slot_start = system_clock::now();
		write_time_timer.start(job_settings.granularity);
	}
	else
	{
		times.timer_active = false;
	}
}
//}}}

//{{{
std::ostream& operator<<(std::ostream&stream, const Job& job)
{
	stream<<"Job \""<<job.jobname<<"\": ";
	stream<<(job.times.total+(job.times.running ? (std::chrono::duration_cast < std::chrono::seconds > (std::chrono::steady_clock::now()-job.times.job_start)) : std::chrono::seconds(0))).count()<<" seconds.";
	stream<<static_cast<const Window_matching&>(job);
	return stream;
}
//}}}

//{{{
std::string Job::get_jobname(void) const
{
	return jobname;
}
//}}}

//{{{
unsigned int Job::get_total_time(void) const
{
	return times.total.count();
}
//}}}

//{{{
Json::Value Job::get_times(uint64_t start, uint64_t end) const
{
	Json::Value retval;
	std::ifstream timefile(timefile_path);
	if(!timefile.is_open())
	{
		error<<"Could not open time file for job \""<<jobname<<"\"."<<std::endl;
		exit(EXIT_FAILURE);
	}
	std::string line;

	// Go to the firest times line
	while(!std::getline(timefile, line).eof())
	{
		if(line == "# Start-time	time-spent")
			break;
	}

	// Read each line
	while(!std::getline(timefile, line).eof())
	{
		std::stringstream linestream(line);
		uint64_t slot_start;
		uint64_t slot_time;
		linestream>>slot_start;
		if(end && slot_start > end)
			break;
		if(slot_start >= start)
		{
			linestream>>slot_time;
			retval[retval.size()][0] = slot_start;
			retval[retval.size()-1][1] = slot_time;
		}
	}

	return retval;
}
//}}}





