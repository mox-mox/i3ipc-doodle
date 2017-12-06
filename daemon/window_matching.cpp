#include "window_matching.hpp"
#include <regex>


//{{{
bool Window_matching::search(const std::string& search_term, const std::vector<std::string>& list) const
{
	for( std::string name : list )
	{
		error<<"creating regex from \""<<name<<"\"."<<std::endl;
		std::regex my_regex(name);
		error<<"starting regex search."<<std::endl;
		if( std::regex_search(search_term, my_regex))
		{
			error<<"regex search=true."<<std::endl;
			return true;
		}
		error<<"regex search=false."<<std::endl;
	}
	return false;
}
//}}}

//{{{
bool Window_matching::match(const std::string& current_workspace, const std::string& window_title) const
{
	debug<<"Job::match(this="<<this<<")"<<std::endl;

	// To match, a window may not match an exclude item, if defined, and must match an include item, if defined.
	if(!matchers.ws_names.exclude.empty()  &&  search(current_workspace, matchers.ws_names.exclude))  return false;
	if(!matchers.ws_names.include.empty()  && !search(current_workspace, matchers.ws_names.include))  return false;
	if(!matchers.win_names.exclude.empty() &&  search(window_title,      matchers.win_names.exclude)) return false;
	if(!matchers.win_names.include.empty() && !search(window_title,      matchers.win_names.include)) return false;

	return true;
}
//}}}


//{{{
bool Window_matching::operator==(const Current& current) const
{
	debug<<"Job::match(this="<<this<<")"<<std::endl;

	// To match, a window may not match an exclude item, if defined, and must match an include item, if defined.
	if(!matchers.ws_names.exclude.empty()  &&  search(current.workspace_name, matchers.ws_names.exclude))  return false;
	if(!matchers.ws_names.include.empty()  && !search(current.workspace_name, matchers.ws_names.include))  return false;
	if(!matchers.win_names.exclude.empty() &&  search(current.window_name,    matchers.win_names.exclude)) return false;
	if(!matchers.win_names.include.empty() && !search(current.window_name,    matchers.win_names.include)) return false;

	return true;
}
bool Window_matching::operator!=(const Current& current) const
{
	return !operator==(current);
}
//}}}


//{{{
std::ostream& operator<<(std::ostream& stream, const Window_matching& job)
{
	stream<<" Names:";
	for( const std::string& n : job.matchers.win_names.include )
	{
		stream<<" |"<<n<<"|";
	}
	for( const std::string& n : job.matchers.win_names.exclude )
	{
		stream<<" |!"<<n<<"|";
	}
	stream<<" workspaces:";
	for( const std::string& w : job.matchers.ws_names.include )
	{
		stream<<" |"<<w<<"|";
	}
	for( const std::string& w : job.matchers.ws_names.exclude )
	{
		stream<<" |!"<<w<<"|";
	}
	stream<<std::endl;
	return stream;
}
//}}}



//bool Window_matching::operator==(const Window_matching& other)
//{
//	if(matchers.win_names.include != other.matchers.win_names.include) return false;
//	if(matchers.win_names.exclude != other.matchers.win_names.exclude) return false;
//	if(matchers.ws_names.include  != other.matchers.ws_names.include)  return false;
//	if(matchers.ws_names.exclude  != other.matchers.ws_names.exclude)  return false;
//	return true;
//}

