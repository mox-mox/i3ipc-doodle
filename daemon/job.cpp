#include "job.hpp"
#include "INIReader.h"
#include <array>
#include <vector>

//{{{ std::vector<std::string> tokenise(std::string input)

//{{{
bool is_whitespace(char ch)
{
	const std::array<char, 2> whitespaces = {' ', ','};
	for(char c : whitespaces)
	{
		if(ch == c) return true;
	}
	return false;
}
//}}}

//{{{
char is_token_delimiter(char ch)
{
	const std::array<char, 2> token_delimiters = {'\'', '"' };
	for(char c : token_delimiters)
	{
		if(ch == c) return c;
	}
	return '\0';
}
//}}}

//{{{
std::vector<std::string> tokenise(std::string input)
{
	const char escape = '\\';
	std::vector<std::string> tokens;
	std::string current_token;
	char current_delimiter = '\0';

	bool in_token = false;
	bool escaped  = false;

	for(char c : input)
	{
		if(!in_token)
		{
			if(c=='#') break;
			if(is_whitespace(c)) continue;

			in_token=true;
			if((current_delimiter = is_token_delimiter(c))) continue;
		}

		if(escaped || (current_delimiter && c!=current_delimiter) || (!current_delimiter && !is_whitespace(c))) // We got a valid char
		{
			if(c == escape)
			{
				escaped = true;
				continue;
			}
			else
			{
				current_token.push_back(c);
				escaped = false;
			}
		}
		else
		{
			tokens.push_back(current_token);
			current_token = "";
			in_token=false;
		}
	}
	if(current_token != "")
	{
		tokens.push_back(current_token);
	}

	return tokens;
}
//}}}
//}}}





Job::Job(const fs::path& jobfile_path, std::shared_ptr<uvw::Loop> loop) :
	loop(loop),
	jobname(jobfile_path.stem())
{
	debug<<"Job::Job() : Creating job "<<jobname<<" from "<<jobfile_path<<std::endl;

	{
		//{{{ Read the configuration file

		INIReader reader(jobfile_path);

		if (reader.ParseError() < 0)
		{
			throw std::runtime_error("Cannot parse job file for job "+jobname+".");
		}

		std::string win_names_include = reader.Get("window-names", "include", "");
		std::string win_names_exclude = reader.Get("window-names", "exclude", "");

		std::string  ws_names_include = reader.Get("workspace-names", "include", "");
		std::string  ws_names_exclude = reader.Get("workspace-names", "exclude", "");

		std::string timefile_path = reader.Get("timefile", "filename", data_dir+"/"+jobname+".time");
		std::string timefile_granularity = reader.Get("timefile", "granularity", "1h");

//
//		//{{{
//		std::cout<<"win_names_include: "<<win_names_include<<std::endl;
//		std::cout<<"win_names_exclude: "<<win_names_exclude<<std::endl;
//
//
//		std::cout<<" ws_names_include: "<< ws_names_include<<std::endl;
//		std::cout<<" ws_names_exclude: "<< ws_names_exclude<<std::endl;
//
//		std::cout<<"timefile_path    : "<<timefile_path<<std::endl;
//		std::cout<<"timefile_granularity: "<<timefile_granularity<<std::endl;
//		//}}}
//


		window_include = tokenise(win_names_include);
		window_exclude = tokenise(win_names_exclude);

		workspace_include = tokenise(ws_names_include);
		workspace_exclude = tokenise(ws_names_exclude);

		debug<<"	window_include: "; for(std::string& token : window_include) debug << "\"" << token << "\""; debug << std::endl;
		debug<<"	window_exclude: "; for(std::string& token : window_exclude) debug << "\"" << token << "\""; debug << std::endl;
		debug<<"	workspace_include: "; for(std::string& token : workspace_include) debug << "\"" << token << "\""; debug << std::endl;
		debug<<"	workspace_exclude: "; for(std::string& token : workspace_exclude) debug << "\"" << token << "\""; debug << std::endl;








		//}}}

	}

}
