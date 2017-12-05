#include "job.hpp"
#include "INIReader.h"
#include <array>
#include <vector>
#include <cctype>
#include <fstream>

//{{{ Parsing functions

// Performance really is no concern here, so prefer simple to read code

//
////{{{ std::vector<std::string> tokenise(std::string input)
//
////{{{
//bool is_whitespace(char ch)
//{
//	const std::array<char, 2> whitespaces = {' ', ','};
//	for(char c : whitespaces)
//	{
//		if(ch == c) return true;
//	}
//	return false;
//}
////}}}
//
////{{{
//char is_token_delimiter(char ch)
//{
//	const std::array<char, 2> token_delimiters = {'\'', '"' };
//	for(char c : token_delimiters)
//	{
//		if(ch == c) return c;
//	}
//	return '\0';
//}
////}}}
//
////{{{
//std::vector<std::string> tokenise(std::string input)
//{
//	const char escape = '\\';
//	std::vector<std::string> tokens;
//	std::string current_token;
//	char current_delimiter = '\0';
//
//	bool in_token = false;
//	bool escaped  = false;
//
//	for(char c : input)
//	{
//		if(!in_token)
//		{
//			if(c=='#') break;
//			if(is_whitespace(c)) continue;
//
//			in_token=true;
//			if((current_delimiter = is_token_delimiter(c))) continue;
//		}
//
//		if(escaped || (current_delimiter && c!=current_delimiter) || (!current_delimiter && !is_whitespace(c))) // We got a valid char
//		{
//			if(c == escape)
//			{
//				escaped = true;
//				continue;
//			}
//			else
//			{
//				current_token.push_back(c);
//				escaped = false;
//			}
//		}
//		else
//		{
//			tokens.push_back(current_token);
//			current_token = "";
//			in_token=false;
//		}
//	}
//	if(current_token != "")
//	{
//		tokens.push_back(current_token);
//	}
//
//	return tokens;
//}
////}}}
////}}}
//
////{{{
//uint32_t time_string_to_seconds(std::string input)
//{
//	uint32_t seconds = 0;
//	uint32_t temp_number = 0;
//
//	bool in_number = false;
//	for(char c : input)
//	{
//		if(std::isdigit(c))
//		{
//			if(!in_number)
//			{
//				temp_number = (c-'0');
//				in_number = true;
//			}
//			else
//			{
//				temp_number = temp_number*10 + (c-'0');
//			}
//		}
//		else
//		{
//			if(!in_number) continue;
//			in_number = false;
//			switch(c)
//			{
//				case 'y': seconds += temp_number*365*24*60*60; break;
//				case 'd': seconds += temp_number    *24*60*60; break;
//				case 'h': seconds += temp_number       *60*60; break;
//				case 'm': seconds += temp_number          *60; break;
//				default:
//				case 's': seconds += temp_number           *1;
//			}
//			temp_number = 0;
//		}
//	}
//	seconds += temp_number;
//
//	return seconds;
//}
////}}}
//

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
