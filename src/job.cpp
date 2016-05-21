#include "job.hpp"
#include "logstream.hpp"
#include  <sstream>


//{{{
Job::Job(std::string jobname, Json::Value job, const std::experimental::filesystem::path& jobfile) : jobname(jobname), jobfile(jobfile)
{
	//if((jobname = job.get("jobname", "unset").asString()) == "unset" )
	//{
	//	error<<"Jobname not found"<<std::endl;
	//}

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

	//if( job.isMember("total_time"))
	//{
	times.total = std::chrono::seconds(job.get("total_time", 0).asInt64());
	//}

	//if( job.isMember("times"))
	//{
	//	std::lock_guard<std::mutex> lock(times_mutex);
	//	for( auto&timespan : job.get("times", "no times"))
	//	{
	//		times.push_back(Timespan(timespan));
	//	}
	//}


	settings.granularity = std::chrono::seconds(job.get("granularity", settings.GRANULARITY_DEFAULT_VALUE).asInt64());
	//std::thread(&Job::save_times, this);
}
//}}}

//{{{
Job::Job(Job&& other) noexcept
{
	std::unique_lock<std::mutex> other_lk(other.times_mutex);

	jobname = std::move(other.jobname);
	jobfile = std::move(other.jobfile);

	times = std::move(other.times);

	//times.total_time = std::move(other.times.total_time);
	//times = std::move(other.times);

	win_names_include = std::move(other.win_names_include);
	win_names_exclude = std::move(other.win_names_exclude);
	ws_names_include  = std::move(other.ws_names_include);
	ws_names_exclude  = std::move(other.ws_names_exclude);
}
//}}}

//{{{
Job::Job(void) : jobname("NOJOB"), times(), win_names_include(), win_names_exclude( {"!"}), ws_names_include(), ws_names_exclude({ "!" }) {}
//}}}

//{{{
void Job::start(std::chrono::steady_clock::time_point start_time)
{
	//std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::lock_guard<std::mutex> lock(times_mutex);
	times.job_start=start_time;
	times.running = true;

	//times.push_back(Timespan());
	//if(!waker.joinable())
	//{
	//	//std::thread waker(save_times, std::chrono::high_resolution_clock::now()+std::chrono::seconds(2));
	//}
}
//}}}

//{{{
void Job::stop(std::chrono::steady_clock::time_point stop_time)
{
	//std::chrono::steady_clock::time_point job_stop = std::chrono::steady_clock::now();
	std::lock_guard<std::mutex> lock(times_mutex);
	std::chrono::seconds elapsed = std::chrono::duration_cast<std::chrono::seconds>(stop_time - times.job_start);
	times.total += elapsed;
	times.slot  += elapsed;
	times.running = false;
}
//}}}

//void Job::save_times(std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> when)
void Job::save_times(void)
{
	for(;;)
	{
		// Wait until the job is activated the (first) time
		std::unique_lock<std::mutex> lk(running_mutex);
		running_cv.wait(lk, [this]{return running;});




		std::chrono::system_clock::time_point slot_start = std::chrono::system_clock::now();

		std::this_thread::sleep_until(std::chrono::steady_clock::now()+settings.granularity);
		std::lock_guard<std::mutex> lock(times_mutex);

		(void) slot_start;
		error<<"TODO: Write the time of the last hour to file."<<std::endl;

	}
	//std::this_thread::sleep_until(when);

	//std::lock_guard<std::mutex> lock(times_mutex);
	//error<<"TODO: Write the time of the last hour to file."<<std::endl;
	//std::stringstream ss;
	//ss << std::setw(20) << std::setfill('0') << 10;
	//std::string s = ss.str();
	//std::cout<<"total_time is "<<s<<std::endl;;
}

//void Job::



//{{{
std::ostream& operator<<(std::ostream&stream, Job const&job)
{
    stream<<"Job \""<<job.jobname<<"\": ";
    stream<<(job.times.total + (job.times.running ? (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - job.times.job_start)):std::chrono::seconds(0))).count()   <<" seconds.";
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
