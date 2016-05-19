#include "job.hpp"
#include "logstream.hpp"


//{{{
Job::Job(std::string jobname, Json::Value job) : jobname(jobname)
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
		for( auto&timespan : job.get("times", "no times"))
		{
			times.push_back(Timespan(timespan));
		}
	}
}
//}}}
//{{{
Job::Job(Json::Value job)
{
	if((jobname = job.get("jobname", "unset").asString()) == "unset" )
	{
		error<<"Jobname not found"<<std::endl;
	}

	if( job.isMember("times"))
	{
		for( auto&timespan : job.get("times", "no times"))
		{
			times.push_back(Timespan(timespan));
		}
	}
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
}
//}}}
//{{{
Job::Job(void) : jobname("NOJOB"), times(), win_names_include(), win_names_exclude(
{
	"!"
}), ws_names_include(), ws_names_exclude({ "!" }) {}
//}}}
//{{{
void Job::start(void)
{
	times.push_back(Timespan());
}
//}}}
//{{{
void Job::stop(void)
{
	times.back().stop();
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
