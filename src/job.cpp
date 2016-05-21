#include "job.hpp"
#include "logstream.hpp"
#include  <sstream>
       #include <unistd.h>


//{{{
Job::Job(std::string jobname, Json::Value job, const std::experimental::filesystem::path& jobfile) : jobname(jobname), jobfile(jobfile)
{
	logger<<"Constructing job "<<jobname<<"."<<std::endl;
	//{{{
	if( job.isMember("window_names"))
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
					win_names_exclude.push_back(win_name);
				}
				else
				{
					win_names_include.push_back(win_name);
				}
			}
		}
	}
	else
	{
		error<<"Job "<<jobname<<": No window name segments specified."<<std::endl;
	}
	//}}}

	//{{{
	if( job.isMember("workspace_names"))
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
					ws_names_exclude.push_back(ws_name);
				}
				else
				{
					ws_names_include.push_back(ws_name);
				}
			}
		}
	}
	else
	{
		error<<"Job "<<jobname<<": No workspace name segments specified."<<std::endl;
	}
	//}}}

	times.total = std::chrono::seconds(job.get("total_time", 0).asInt64());

	settings.granularity = std::chrono::seconds(job.get("granularity", settings.GRANULARITY_DEFAULT_VALUE).asInt64());
	//waker = std::thread(&Job::save_times, this); // this is a hack: The real jobs are move-constructed into the container, so start the thread only after move-construction
}
//}}}

//{{{
Job::Job(Job&& other) noexcept
{
	std::unique_lock<std::mutex> other_lk(other.times_mutex);

	jobname = std::move(other.jobname);
	jobfile = std::move(other.jobfile);


	settings = std::move(other.settings);

	times = std::move(other.times);
	waker = std::move(other.waker);

	win_names_include = std::move(other.win_names_include);
	win_names_exclude = std::move(other.win_names_exclude);
	ws_names_include  = std::move(other.ws_names_include);
	ws_names_exclude  = std::move(other.ws_names_exclude);
	waker = std::thread(&Job::save_times, this);
}
//}}}

//{{{
Job::Job(void) : jobname("NOJOB"), times(), win_names_include(), win_names_exclude( {"!"}), ws_names_include(), ws_names_exclude({ "!" }) {times.waker_running = true;}
//}}}
//
//{{{
Job::~Job(void)
{
	if(waker.joinable()) waker.join();
}
//}}}

//{{{
void Job::start(std::chrono::steady_clock::time_point start_time)
{
	//std::cout<<"	"<<jobname<<".start() called."<<std::endl;
	{
		std::lock_guard<std::mutex> lock(times_mutex);
		times.job_start=start_time;
		times.job_running = true;
	}

	if(!times.waker_running)	// Get the saver to start.
	{
		error<<"Starting the saver for Job "<<jobname<<std::endl;
		times_cv.notify_one();
	}
}
//}}}

//{{{
void Job::stop(std::chrono::steady_clock::time_point stop_time)
{
	std::lock_guard<std::mutex> lock(times_mutex);
	std::chrono::seconds elapsed = std::chrono::duration_cast<std::chrono::seconds>(stop_time - times.job_start);
	times.job_start=std::chrono::steady_clock::time_point(); // reset to 'zero'
	times.total += elapsed;
	times.slot  += elapsed;
	times.job_running = false;
}
//}}}

void Job::save_times(void)
{
	std::unique_lock<std::mutex> lock(times_mutex);	// Grab the mutex. Whenever the thread is sleeping, it will free the mutex.
	for(;;)
	{
		if(times.destructor_called) { lock.unlock(); return; }

		// Wait until the job is activated the (first) time.
		if(!times.job_running)	// No need to wait, when the job is already running.
		{
			times.waker_running = false;
			times_cv.wait(lock, [this]{return times.job_running || times.destructor_called;});	// Wait until the job is started or destructor is called.
		}

		if(times.destructor_called) { lock.unlock(); return; }

		// Remember the date when the tims_slot started.
		std::chrono::system_clock::time_point slot_start = std::chrono::system_clock::now();

		times.waker_running = true;

		// Wait until it is time to write the time slot to disk or the destructor has been called.
		times_cv.wait_until(lock, std::chrono::steady_clock::now()+settings.granularity, [this](){return times.destructor_called;});


		std::cerr<<"TODO: Write the time of the last hour to file."<<std::endl;


		if(times.job_running) // Account for a currently running job.
		{
			std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
			std::chrono::seconds current_runtime = std::chrono::duration_cast<std::chrono::seconds>(now - times.job_start);
			times.total += current_runtime;
			times.slot += current_runtime;
			times.job_start = now;
		}

		std::stringstream ss;
		ss << std::setw(20) << std::setfill('0') << times.total.count();
		std::cerr<<"	New total_time for job "<<jobname<<": "<<ss.str()<<std::endl;
		std::cerr<<"	[ "<<slot_start.time_since_epoch().count()<<", "<<times.slot.count()<<" ]"<<std::endl;
		times.slot = std::chrono::seconds(0);
		std::cerr<<std::endl;
		std::cerr<<std::endl;
	}
}




//{{{
std::ostream& operator<<(std::ostream&stream, Job const&job)
{
    stream<<"Job \""<<job.jobname<<"\": ";
    stream<<(job.times.total + (job.times.job_running ? (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - job.times.job_start)):std::chrono::seconds(0))).count()   <<" seconds.";
    stream<<" Names:";
    for( const std::string& n : job.win_names_include )
	{
		stream<<" |"<<n<<"|";
	}
    for( const std::string& n : job.win_names_exclude )
	{
		stream<<" |!"<<n<<"|";
	}
    stream<<" workspaces:";
    for( const std::string& w : job.ws_names_include )
	{
		stream<<" |"<<w<<"|";
	}
    for( const std::string& w : job.ws_names_exclude )
	{
		stream<<" |!"<<w<<"|";
	}
    stream<<std::endl;
    return stream;
}
//}}}




//void Doodle::write_config(Json::Value config)
//{
//	config["max_idle_time"] = settings.max_idle_time;
//	config["detect_ambiguity"] = settings.detect_ambiguity;
//
//	Json::StyledWriter writer;
//	// Make a new JSON document for the configuration. Preserve original comments.
//	std::string outputConfig = writer.write( config );
//	std::cout<< outputConfig <<std::endl;
//}


//std::ifstream config_file(config_filename);
//{
//	std::ofstream config_copy(config_filename+"_backup");
//	config_copy<<config_file.rdbuf();
//	config_file.clear();
//	config_file.seekg(0, std::ios::beg);
//}
