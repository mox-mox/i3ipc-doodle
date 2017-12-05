#include "job.hpp"
#include "INIReader.h"
#include <array>
#include <vector>
#include <cctype>
#include <fstream>


//{{{
fs::path set_timefile_path(std::string dir, std::string jobname)
{
	fs::path timefile_path;
	if(dir == "default")
		timefile_path = data_dir + "/" + jobname + ".times";
	else
		timefile_path = dir;

	if(timefile_path.filename() != jobname+".times")
	{
		timefile_path += jobname+".times";
	}

	fs::path timefile_dir = timefile_path;
	timefile_dir.remove_filename();

	//{{{ Create timefile if neccessary

	if(!fs::exists(timefile_dir))
	{
		std::error_code fs_error;
		if(!fs::create_directory(timefile_dir, fs_error))
		{
			notify_critical<<"Cannot create data dir"<<"Plese check the logs!"<<std::endl;
			error<<"Could not create data directory \""<<timefile_dir.string()<<"\" for job \""<<jobname<<"\": "<<fs_error.message()<<'.'<<std::endl;
			throw std::runtime_error("Could not create data directory \""+timefile_dir.string()+"\" for job \""+jobname+"\": "+fs_error.message()+'.');
		}
		else
		{
			debug<<"Created data directory \""<<timefile_dir.string()<<"\" for job \""<<jobname<<"\"."<<std::endl;
		}
	}

	if(!fs::exists(timefile_path))
	{
		debug<<"There was no time file for job \""<<jobname<<"\". Creating one: "<<timefile_path.string()<<std::endl;
		std::ofstream timefile(timefile_path);
		if(!timefile.is_open())
		{
			notify_critical<<"Cannot create timefile"<<"Plese check the logs!"<<std::endl;
			error<<"Could not create time file \""<<timefile_dir.string()<<"\" for job \""<<jobname<<"\"."<<std::endl;
			throw std::runtime_error("Could not create time_file \""+timefile_path.string()+"\" for job \""+jobname+"\".");
		}
		timefile<<"# Total time: "<<std::setfill('0')<<std::setw(20)<<0<<"\n# Start-time	time-spent"<<std::endl;
	}
	//}}}

	return timefile_path;
}
//}}}


//{{{
Job::Job(const fs::path& jobfile_path, std::shared_ptr<uvw::Loop> loop) :
	loop(loop),
	jobname(jobfile_path.stem())
{
	debug<<"Creating job \""<<jobname<<"\" from "<<jobfile_path<<std::endl;

		//{{{ Read the configuration file

		INIReader reader(jobfile_path);
		if (reader.ParseError() < 0)
		{
			error<<"Cannot parse job file for job \""<<jobname<<"\"."<<std::endl;
			notify_critical<<"Cannot parse job file for job"<<jobname<<std::endl;
			throw std::runtime_error("Cannot parse job file for job "+jobname+".");
		}

		//{{{ Include/Exclude strings

		window_include = tokenise(reader.Get("window-names", "include", ""));
		window_exclude = tokenise(reader.Get("window-names", "exclude", ""));

		workspace_include = tokenise(reader.Get("workspace-names", "include", ""));
		workspace_exclude = tokenise(reader.Get("workspace-names", "exclude", ""));

		debug<<"	window_include: "; for(std::string& token : window_include) debug << "\"" << token << "\""; debug << std::endl;
		debug<<"	window_exclude: "; for(std::string& token : window_exclude) debug << "\"" << token << "\""; debug << std::endl;
		debug<<"	workspace_include: "; for(std::string& token : workspace_include) debug << "\"" << token << "\""; debug << std::endl;
		debug<<"	workspace_exclude: "; for(std::string& token : workspace_exclude) debug << "\"" << token << "\""; debug << std::endl;
		//}}}

		//{{{ Timefile

		timefile_path = set_timefile_path(reader.Get("timefile", "filename", "default"), jobname);
		debug<<"	timefile_path = "<<timefile_path<<std::endl;

		timefile_granularity = time_string_to_seconds(reader.Get("timefile", "granularity", ""));
		timefile_granularity = timefile_granularity ? timefile_granularity : default_timefile_granularity;
		debug<<"	timefile_granularity: "<<timefile_granularity<<std::endl;
		if(timefile_granularity < 1800)
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

}
//}}}
