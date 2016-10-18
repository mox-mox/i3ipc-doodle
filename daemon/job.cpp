#include "job.hpp"
#include <fstream>



//{{{
Job::Job(const fs::path& jobfile_path, ev::loop_ref& loop) : jobname(jobfile_path.stem()), jobfile_path(jobfile_path), timefile_path(settings.data_dir/(jobname+".times")), write_time_timer(loop)
{
	debug<<". Constructing job "<<jobname<<"."<<std::endl;
	std::ifstream jobfile(jobfile_path);
	Json::Value job;
	Json::Reader reader;
	if( !reader.parse(jobfile, job, false))
	{
		error<<reader.getFormattedErrorMessages()<<std::endl;
		throw std::runtime_error("Cannot parse job file");
	}

	job_settings.granularity = job.get("granularity", job_settings.GRANULARITY_DEFAULT_VALUE).asInt64();

	//{{{ Get window names

	if(job.isMember("window_names"))
	{
		for( auto&window_name : job.get("window_names", "no window_names"))
		{
			std::string win_name = window_name.asString();
			if( win_name == "no window_names" )
			{
				error<<"Job "<<jobname<<": Invalid window name."<<std::endl;
			}
			else
			{
				if( '!' == win_name[0] )	// Window name segments prepended with '!' mean that the job may not
				{							// have windows whose title matches the given name segment.
					win_name.erase(0, 1);		// Remove the leading '!'
					matchers.win_names_exclude.push_back(win_name);
				}
				else
				{
					matchers.win_names_include.push_back(win_name);
				}
			}
		}
	}
	else
	{
		error<<"Job "<<jobname<<": No window name segments specified."<<std::endl;
	}
	//}}}

	//{{{ Get workspace names

	if(job.isMember("workspace_names"))
	{
		for( auto&workspace_name : job.get("workspace_names", "no workspace_names"))
		{
			std::string ws_name = workspace_name.asString();
			if( ws_name == "no workspace_names" )
			{
				error<<"Job "<<jobname<<": Invalid workspace name."<<std::endl;
			}
			else
			{
				if( '!' == ws_name[0] )		// Workspace name segments prepended with '!' mean that the job may not
				{							// have windows on workspaces matching the given name segment.
					ws_name.erase(0, 1);	// Remove the leading '!'
					matchers.ws_names_exclude.push_back(ws_name);
				}
				else
				{
					matchers.ws_names_include.push_back(ws_name);
				}
			}
		}
	}
	else
	{
		error<<"Job "<<jobname<<": No workspace name segments specified."<<std::endl;
	}
	//}}}

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
	times.total = std::chrono::seconds(temp);
	//}}}

	write_time_timer.set < Job, &Job::write_time_cb > (this);
}
//}}}

//{{{
Job::Job(Job && other) noexcept : jobname(other.jobname), jobfile_path(other.jobfile_path), timefile_path(other.timefile_path), job_settings(std::move(other.job_settings)), write_time_timer(other.write_time_timer.loop)
{
	matchers = std::move(other.matchers);
	times = std::move(other.times);
	// Note that the timer is constructed with the original event loop in the intialiser list.
	// There is no real copy (or move) constructor for ev::timer, so this hack is used to re-initialise the timer
	write_time_timer.set < Job, &Job::write_time_cb > (this);

}
//}}}

//{{{
Job::Job(void) :
	jobname("NOJOB"),
	jobfile_path(),
	timefile_path(),
	times{std::chrono::seconds(0), std::chrono::steady_clock::time_point(), false,  std::chrono::seconds(0), std::chrono::system_clock::time_point(), true},
	matchers{{}, { "!" }, {}, { "!" }}
{ }
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
void Job::start(std::chrono::steady_clock::time_point start_time)
{
	times.job_start = start_time;
	times.running = true;

	if(!times.timer_active)
	{
			times.slot_start = std::chrono::system_clock::now();
			times.slot = std::chrono::seconds(0);
			times.timer_active = true;
			write_time_timer.start(job_settings.granularity);
	}
}
//}}}

//{{{
void Job::stop(std::chrono::steady_clock::time_point now)
{
	std::chrono::seconds elapsed = std::chrono::duration_cast < std::chrono::seconds > (now-times.job_start);
	times.total     += elapsed;
	times.slot      += elapsed;
	times.job_start  = std::chrono::steady_clock::time_point();	// reset to 'zero'
	times.running = false;
}
//}}}

//{{{
void Job::write_time_cb(void)
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	if( times.running )							// Account for a currently running job.
	{
		std::chrono::seconds elapsed = std::chrono::duration_cast < std::chrono::seconds > (now-times.job_start);
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
		times.slot = std::chrono::seconds(0);
	}
	else
	{
		throw std::runtime_error("Could not re-open timefile \""+timefile_path.string()+"\".");
	}

	if( times.running ) // If the job is currently not running
	{
		times.slot_start = std::chrono::system_clock::now();
		write_time_timer.start(job_settings.granularity);
	}
	else
	{
		times.timer_active = false;
	}
}
//}}}

//{{{
std::ostream& operator<<(std::ostream&stream, Job const&job)
{
	stream<<"Job \""<<job.jobname<<"\": ";
	stream<<(job.times.total+(job.times.running ? (std::chrono::duration_cast < std::chrono::seconds > (std::chrono::steady_clock::now()-job.times.job_start)) : std::chrono::seconds(0))).count()<<" seconds.";
	stream<<" Names:";
	for( const std::string& n : job.matchers.win_names_include )
	{
		stream<<" |"<<n<<"|";
	}
	for( const std::string& n : job.matchers.win_names_exclude )
	{
		stream<<" |!"<<n<<"|";
	}
	stream<<" workspaces:";
	for( const std::string& w : job.matchers.ws_names_include )
	{
		stream<<" |"<<w<<"|";
	}
	for( const std::string& w : job.matchers.ws_names_exclude )
	{
		stream<<" |!"<<w<<"|";
	}
	stream<<std::endl;
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
