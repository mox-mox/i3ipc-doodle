#include "job.hpp"
#include "INIReader.h"
#include <fstream>

//{{{ Timefile

//{{{
void Job::Timefile::set_path(std::string dir, std::string jobname)
{
	if(dir == "default")
		path = data_dir + "/" + jobname + ".times";
	else
		path = dir;

	if(path.filename() != jobname+".times")
	{
		path += jobname+".times";
	}
}
//}}}

//{{{
bool Job::Timefile::check_and_create_file(void)
{
	fs::path dir = path;
	dir.remove_filename();

	//{{{ Create timefile if neccessary

	if(!fs::exists(dir))
	{
		std::error_code fs_error;
		if(!fs::create_directory(dir, fs_error))
		{
			notify_critical<<"Cannot create timefile dir"<<"Plese check the logs!"<<std::endl;
			error<<"Could not create timefile directory \""<<dir.string()<<"\": "<<fs_error.message()<<'.'<<std::endl;
			//error<<"Could not create timefile directory \""<<dir.string()<<"\" for job \""<<jobname<<"\": "<<fs_error.message()<<'.'<<std::endl;
			//throw std::runtime_error("Could not create timefile directory \""+dir.string()+"\" for job \""+jobname+"\": "+fs_error.message()+'.');
			return false;
		}
		else
		{
			debug<<"Created timefile directory \""<<dir.string()<<"\"."<<std::endl;
			//debug<<"Created timefile directory \""<<dir.string()<<"\" for job \""<<jobname<<"\"."<<std::endl;
		}
	}

	if(!fs::exists(path))
	{
		std::ofstream timefile(path);
		if(!timefile.is_open())
		{
			notify_critical<<"Cannot create timefile"<<"Plese check the logs!"<<std::endl;
			error<<"Could not create timefile \""<<dir.string()<<"\"."<<std::endl;
			//error<<"Could not create timefile \""<<dir.string()<<"\" for job \""<<jobname<<"\"."<<std::endl;
			//throw std::runtime_error("Could not create timefile \""+path.string()+"\" for job \""+jobname+"\".");
			return false;
		}
		else
		{
			timefile<<"# Total time: "<<std::setfill('0')<<std::setw(timefile_time_width)<<0<<" ms\n# Start-time	time-spent"<<std::endl;
			debug<<"Created timefile \""<<path.string()<<"\"."<<std::endl;
			//debug<<"Created timefile \""<<path.string()<<"\" for job \""<<jobname<<"\"."<<std::endl;
		}
	}
	//}}}
	
	return true;
}
//}}}

//{{{
milliseconds Job::Timefile::get_total_time(void)
{
	//{{{
	if(!check_and_create_file())
	{
		notify_critical<<"Cannot open/create timefile"<<"Plese check the logs!"<<std::endl;
		error<<"Could not open/create timefile \""<<path.string()<<"\"."<<std::endl;
		throw std::runtime_error("Could not open/create timefile \""+path.string()+"\".");
		//error<<"Could not open/create timefile \""<<path.string()<<"\" for job \""<<jobname<<"\"."<<std::endl;
		//throw std::runtime_error("Could not open/create timefile \""+path.string()+"\" for job \""+jobname+"\".");
	}
	//}}}

	std::fstream timefile(path); // Not ifstream so we'll also know if the file is writable
	if(!timefile.is_open())
	{
		notify_critical<<"Cannot open timefile"<<"Plese check the logs!"<<std::endl;
		error<<"Could not open time file \""<<path.string()<<"\"."<<std::endl;
		throw std::runtime_error("Could not create time_file \""+path.string()+"\".");
		//error<<"Could not open time file \""<<path.string()<<"\" for job \""<<jobname<<"\"."<<std::endl;
		//throw std::runtime_error("Could not create time_file \""+path.string()+"\" for job \""+jobname+"\".");
	}
	timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// Go to where
	timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// the total time
	timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// is stored.

	//milliseconds::rep temp; // TODO
	uint64_t temp;
	timefile>>temp;
	return milliseconds(temp);
}
//}}}

//{{{
void Job::Timefile::add_time(milliseconds new_total, system_clock::time_point slot_start, milliseconds slot)
{
	debug<<"Writing time to disk file "<<path<<": "<<new_total<<" total, "<<slot<<" active in the last slot."<<std::endl;
	std::fstream timefile(path);
	if(timefile.is_open())
	{
		timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// Go to where
		timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// the total time
		timefile.ignore(std::numeric_limits<std::streamsize>::max(), ' ');	// is stored.
		timefile<<std::setfill('0')<<std::setw(timefile_time_width)<<new_total.count();
		timefile.seekg(0, std::ios_base::end);	// Go to the end of the jobfile
		timefile<<std::chrono::system_clock::to_time_t(slot_start)<<"\t"<<slot.count()<<std::endl;
		timefile.close();
	}
	else
	{
		error<<"Cannot re-open timefile \""<<path.string()<<"\"."<<std::endl;
		notify_critical<<"Cannot re-open timefile "<<path.string()<<std::endl;
		throw std::runtime_error("Could not re-open timefile \""+path.string()+"\".");
	}
}
//}}}

//{{{
void Job::Timefile::set_granularity(milliseconds new_granularity)
{
	granularity = new_granularity;
	if(granularity < milliseconds(1800))
		logger<<"	Very small timefile granularity. This may create some serious amout of data on disk. Consider using a greater granularity."<<std::endl;
}
//}}}

//{{{
std::string Job::Timefile::get_times(std::time_t start, std::time_t end) const
{
	std::ifstream timefile(path);
	if(!timefile.is_open())
	{
		error<<"Could not open time file \""<<path<<"\"."<<std::endl;
		return "Could not open time file \"" + path.string() + "\".";
	}


	std::string retval = "Slot start time      Time spent\n";

	// Go to the first times line
	std::string line;
	while(!std::getline(timefile, line).eof())
	{
		if(line == "# Start-time	time-spent")
			break;
	}

	// Read each line
	while(!std::getline(timefile, line).eof())
	{
		std::stringstream linestream(line);
		std::time_t slot_start;
		std::time_t slot_time;
		linestream>>slot_start;
		if(end && slot_start > end)
			break;
		if(slot_start >= start)
		{
			linestream>>slot_time;
			std::stringstream out;
			out << std::put_time(std::localtime(&slot_start), "%d.%m.%y %T");

			retval += out.str() + "    " + std::to_string(slot_time) + "\n";
		}
	}

	return retval;
}
//}}}

//}}}


//{{{
Job::Job(const fs::path& jobconfig_path, std::shared_ptr<uvw::Loop> loop) :
	jobconfig_path(jobconfig_path),
	jobname(jobconfig_path.stem()),
	is_active(false),
	loop(loop),
	write_timer(loop->resource<uvw::TimerHandle>())
{
	debug<<"Creating job \""<<jobname<<"\" from "<<jobconfig_path<<" at "<<this<<std::endl;

	//{{{ Read the configuration file

	INIReader reader(jobconfig_path);
	if (reader.ParseError() < 0)
	{
		error<<"Cannot parse job file for job \""<<jobname<<"\"."<<std::endl;
		notify_critical<<"Cannot parse job file for job"<<jobname<<std::endl;
		throw std::runtime_error("Cannot parse job file for job "+jobname+".");
	}

	//{{{ Include/Exclude strings

	matchers.win_names.include = tokenise(reader.Get("window-names", "include", ""));
	matchers.win_names.exclude = tokenise(reader.Get("window-names", "exclude", ""));
	matchers. ws_names.include = tokenise(reader.Get("workspace-names", "include", ""));
	matchers. ws_names.exclude = tokenise(reader.Get("workspace-names", "exclude", ""));

	debug<<"	window_include: ";    for(std::string& token : matchers.win_names.include) debug << "\"" << token << "\""; debug << std::endl;
	debug<<"	window_exclude: ";    for(std::string& token : matchers.win_names.exclude) debug << "\"" << token << "\""; debug << std::endl;
	debug<<"	workspace_include: "; for(std::string& token : matchers. ws_names.include) debug << "\"" << token << "\""; debug << std::endl;
	debug<<"	workspace_exclude: "; for(std::string& token : matchers. ws_names.exclude) debug << "\"" << token << "\""; debug << std::endl;
	//}}}

	//{{{ Timefile

	timefile.set_path(reader.Get("timefile", "path", "default"), jobname);
	debug<<"	timefile path = "<<timefile.get_path()<<std::endl;

	timefile.set_granularity(time_string_to_milliseconds(reader.Get("timefile", "granularity", default_timefile_granularity)));
	debug<<"	timefile.granularity: "<<timefile.get_granularity()<<std::endl;

	suppress = time_string_to_milliseconds(reader.Get("timefile", "suppress", default_timefile_suppress));
	debug<<"	timefile.suppress: "<<suppress<<std::endl;
	if(suppress*2 > timefile.get_granularity())
	{
		error<<"	Timefile.granularity must be at least twice as long as timefile.suppress. Setting to timefile.suppress to timefile.granularity/2 = "<<timefile.get_granularity()/2<<"."<<std::endl;
		suppress = timefile.get_granularity()/2;
	}
	//}}}

	//{{{ TODO: Actions

	for(const std::string& section : reader.Sections())
	{
		if(section == "window-names" || section == "workspace-names" || section == "timefile") continue;
		if(section.find_first_of("action-") != 0)
		{
			logger<<"Unknown section \""<<section<<"\" in config."<<std::endl;
			continue;
		}
		//TODO: Handle all stuff pertaining to different actions
		error<<"Reading action-section \""<<section<<"\": Well, actions are not implemented yet..."<<std::endl;
	}
	//}}}

	//}}}

	// write time watcher
	write_timer->on<uvw::TimerEvent>(std::bind( &Job::on_write_timer, this));

	total_run_time = timefile.get_total_time();
}
//}}}

//{{{
Job::~Job(void)
{
	debug<<"Destroying Job \""<<jobname<<"\"."<<std::endl;
	//if(write_timer->active())
	if(slot_run_time != milliseconds(0) || is_active)
	{
		debug<<"Writing last timing for job \""<<jobname<<"\"."<<std::endl;
		on_write_timer();
		write_timer->stop();
	}
}
//}}}

//{{{
void Job::on_write_timer(void)
{
	steady_clock::time_point now = steady_clock::now();

	debug<<"Job \""<<jobname<<"\": Writing timing to disk."<<std::endl;

	// Update the timers before writing to disk...
	slot_run_time  += runtime_since_job_start(now);
	// We just added the runtime since job_start to the slot and total run time,
	// so to avoid counting the time from job start to now twice, set job start to now.
	job_start = now;
	total_run_time += slot_run_time;
	// ... and write the timers to disk
	timefile.add_time(total_run_time, slot_start, slot_run_time);
	slot_run_time = milliseconds(0);

	// If the job is active re-start the slot timer (if not it will be started when the job becomes active the next time)
	if(is_active) start_slot();
}
//}}}

//{{{
void Job::start(steady_clock::time_point start_time)
{
	if(!is_active)
	{
		debug<<"Starting job \""<<jobname<<"\"."<<std::endl;
		is_active = true;
		job_start = start_time;
		if(!write_timer->active()) start_slot();

		//for(Action& action : actions) action(current_workspace, window_title);
	}
	else error<<"Job "<<jobname<<": Trying to start already running process."<<std::endl;
}
//}}}

//{{{
void Job::stop(steady_clock::time_point stop_time)
{
	if(is_active)
	{
		milliseconds runtime = runtime_since_job_start(stop_time);
		if(runtime > suppress)
		{
			slot_run_time += runtime;
			debug<<"Stopping job \""<<jobname<<"\". Runtime since job start: "<<runtime<<", slot_run_time: "<<slot_run_time<<"."<<std::endl;
		}
		else
		{
			debug<<"Stopping job \""<<jobname<<"\". Runtime since job start: "<<runtime<<", slot_run_time: "<<slot_run_time<<" (suppressed)."<<std::endl;
			if(write_timer->active() && slot_run_time < suppress)
			{
				// If the slot was just started for a short suppressed activation, un-start it. If the slot has accumulated more time, don't touch it.
				write_timer->stop();
				debug<<"	... aborting running now-redundant write_timer."<<std::endl;
			}
		}

		is_active = false; // Has to be after runtime_since_job_start()

		//for(Action& action : actions) action.stop();
	}
	else error<<"Job "<<jobname<<": Trying to stop already stopped process."<<std::endl;
}
//}}}

//{{{
void Job::suspend(void)
{
	logger<<"Suspending job \""<<jobname<<"\" because the computer will be suspended."<<std::endl;
	if(is_active)
	{
		stop(steady_clock::now());
		if(write_timer->active())
		{
			//debug<<"Writing timing for job \""<<jobname<<"\" because the computer will be suspended."<<std::endl;
			on_write_timer();
			write_timer->stop();
		}
	}
	//else error<<"Job "<<jobname<<": Trying to suspend already stopped process."<<std::endl;
}
//}}}

//{{{
void Job::resume(void)
{
	logger<<"Resuming job \""<<jobname<<"\"."<<std::endl;
	start(steady_clock::now());
}
//}}}

//{{{
void Job::start_slot()
{
	// Slot start is not used for calculating passed times but only to know when the user was active.
	// Steady clock is not fixed to a particular start time, so to get meaningfull timestamps we use system clock/wall clock time here.
	slot_start = system_clock::now();
	slot_run_time = milliseconds(0);
	write_timer->start(timefile.get_granularity(), timefile.get_granularity());
}
//}}}

//{{{
const std::string& Job::get_jobname(void) const
{
	return jobname;
}
//}}}

//{{{
milliseconds Job::get_total_time(void) const
{
	return total_run_time;
}
//}}}

//{{{
Job::operator std::string() const
{
	std::string retval("Job \"" + jobname + "\" " + (is_active?"[active]":"[inactive]"));
	retval += ms_to_string(total_run_time+runtime_since_job_start(steady_clock::now())) + ".";
	return retval;
}

std::ostream& operator<<(std::ostream&stream, const Job& job)
{
	//stream<<"Job \""<<job.jobname<<"\" "<<(job.is_active?"[active]":"[inactive]");
	//stream<<(job.total_run_time+job.runtime_since_job_start(steady_clock::now()))<<".";
	stream<<static_cast<std::string>(job);
	return stream;
}
//}}}


