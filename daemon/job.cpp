#include "job.hpp"
#include "INIReader.h"
//#include <array>
//#include <vector>
//#include <cctype>
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
		timefile<<slot_start.time_since_epoch().count()<<"\t"<<slot.count()<<std::endl;
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

//}}}



//{{{ Constructors

//{{{
Job::Job(const fs::path& jobconfig_path, std::shared_ptr<uvw::Loop> loop) :
	jobname(jobconfig_path.stem()),
	is_active(false),
	loop(loop),
	times{milliseconds(0), steady_clock::time_point(),  milliseconds(0), system_clock::time_point()},
	write_timer(loop->resource<uvw::TimerHandle>())
{
	debug<<"Creating job \""<<jobname<<"\" from "<<jobconfig_path<<std::endl;

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
	debug<<"	timefile path = "<<timefile.path<<std::endl;

	timefile.granularity = time_string_to_milliseconds(reader.Get("timefile", "granularity", default_timefile_granularity));
	debug<<"	timefile.granularity: "<<timefile.granularity<<std::endl;
	if(timefile.granularity < milliseconds(1800))
		logger<<"	Very small timefile granularity. This may create some serious amout of data on disk. Consider using a greater granularity."<<std::endl;
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
	}
	//}}}

	//}}}

	// idle time watcher
	write_timer->on<uvw::TimerEvent>(std::bind( &Job::on_write_timer, this));
	//error<<"	this = "<<this<<std::endl;
	//error<<"	write_timer = "<<write_timer<<std::endl;

	times.total = timefile.get_total_time();
}
//}}}

//
////{{{
//Job::Job(Job&& other) noexcept :
//	Window_matching(std::move(other)),
//	jobname(std::move(other.jobname)),
//	timefile_path(std::move(other.timefile_path)),
//	job_settings(std::move(other.job_settings)),
//	times(std::move(other.times)),
//	write_time_timer(other.write_time_timer.loop),
//	actions(std::move(other.actions))
//{
//	debug<<"Move-Constructing job "<<jobname<<" at "<<this<<'.'<<std::endl;
//	// Note that the timer is constructed with the original event loop in the intialiser list.
//	// There is no real copy (or move) constructor for ev::timer, so this hack is used to re-initialise the timer
//	write_time_timer.set < Job, &Job::write_time_cb > (this);
//}
////}}}
//
////{{{
//Job::Job(void) :
//	Window_matching(),
//	jobname("NOJOB"),
//	timefile_path(),
//	times{seconds(0), steady_clock::time_point(), false,  seconds(0), system_clock::time_point(), true},
//	actions()
//{}
////}}}
//

//{{{
Job::~Job(void)
{
	if(write_timer->active())
	{
		debug<<"Writing last timing for job "<<jobname<<std::endl;
		on_write_timer();
		write_timer->stop();
	}
}
//}}}
//}}}

//{{{
void Job::on_write_timer(void)
{
	steady_clock::time_point now = steady_clock::now();

	if(is_active)							// Account for a currently running job.
	{
		milliseconds elapsed = std::chrono::duration_cast<milliseconds>(now-times.job_start);
		times.total     += elapsed;
		times.slot      += elapsed;
		times.job_start  = now;
	}

	timefile.add_time(times.total, times.slot_start, times.slot);
	times.slot = milliseconds(0);

	if(is_active) // If the job is currently running
	{
		times.slot_start = system_clock::now();
		write_timer->start(timefile.granularity, timefile.granularity);
	}
}
//}}}

//{{{
void Job::start(steady_clock::time_point start_time)
{
	if(!is_active)
	{
		debug<<"Starting job \""<<jobname<<"\"."<<std::endl;
		is_active = true;
		times.job_start = start_time;

		//error<<"	this = "<<this<<std::endl;
		//error<<"	write_timer = "<<write_timer<<std::endl;
		if(!write_timer->active())
		{
			times.slot_start = system_clock::now();
			times.slot = milliseconds(0);
			write_timer->start(timefile.granularity, timefile.granularity);
		}

		//for(Action& action : actions)
		//{
		//	action(current_workspace, window_title);
		//}
	}
	else error<<"Job "<<jobname<<": Trying to start already running process."<<std::endl;
}
//}}}

//{{{
void Job::stop(steady_clock::time_point now)
{
	if(is_active)
	{
		debug<<"Stopping job \""<<jobname<<"\"."<<std::endl;
		milliseconds elapsed = std::chrono::duration_cast<milliseconds>(now-times.job_start);
		times.total     += elapsed;
		times.slot      += elapsed;
		times.job_start  = steady_clock::time_point();	// reset to 'zero'
		is_active        = false;

		//for(Action& action : actions)
		//{
		//	action.stop();
		//}
	}
	else error<<"Job "<<jobname<<": Trying to stop already stopped process."<<std::endl;
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
	return times.total;
}
//}}}

//{{{
std::ostream& operator<<(std::ostream&stream, const Job& job)
{
	stream<<"Job \""<<job.jobname<<"\" "<<(job.is_active?"[active]":"[inactive]");
	stream<<(job.times.total+(job.is_active ? (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-job.times.job_start)) : std::chrono::seconds(0)))<<".";
	return stream;
}
//}}}


