#include "job.hpp"
#include "logstream.hpp"


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
	total_time = job.get("total_time", 0).asInt64();
	//}

	if( job.isMember("times"))
	{
		std::lock_guard<std::mutex> lock(times_mutex);
		for( auto&timespan : job.get("times", "no times"))
		{
			times.push_back(Timespan(timespan));
		}
	}
}
//}}}
//{{{
Job::Job(void) : jobname("NOJOB"), times(), win_names_include(), win_names_exclude( {"!"}), ws_names_include(), ws_names_exclude({ "!" }) {}
//}}}
//{{{
void Job::start(void)
{
	std::lock_guard<std::mutex> lock(times_mutex);
	times.push_back(Timespan());
}
//}}}
//{{{
void Job::stop(void)
{
	std::lock_guard<std::mutex> lock(times_mutex);
	times.back().stop();
}
//}}}

void Job::save_times(std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> when)
{
	std::this_thread::sleep_until(when);

	std::lock_guard<std::mutex> lock(times_mutex);
	error<<"TODO: Write the time of the last hour to file."<<std::endl;
}








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
