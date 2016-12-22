#include "window_matching.hpp"


//{{{
Window_matching::Window_matching(const Json::Value& matcher)
{
	//{{{ Get window names

	if(matcher.isMember("window_names"))
	{
		parse_names(matchers.win_names, matcher.get("window_names", "no window_names"));
	}
	else
	{
		//error<<"Job "<<jobname<<": No window name segments specified."<<std::endl;
		error<<"Job : No window name segments specified."<<std::endl;
	}
	//}}}

	//{{{ Get workspace names

	if(matcher.isMember("workspace_names"))
	{
		parse_names(matchers.ws_names, matcher.get("workspace_names", "no workspace_names"));
	}
	else
	{
		//error<<"Job "<<jobname<<": No workspace name segments specified."<<std::endl;
		error<<"Job : No workspace name segments specified."<<std::endl;
	}
	//}}}
}
//}}}

Window_matching::Window_matching(Window_matching && other) : matchers(std::move(other.matchers)) {}

Window_matching::Window_matching(void) : matchers{ { {}, {"!"} }, { {}, {"!"} } }                {}


//{{{
void Window_matching::parse_names(matchers_t::names_t& lists, const Json::Value& names)
{
	for( auto& name : names)
	{
		std::string name_string = name.asString();
		if( name_string == "no window_names" )
		{
			//error<<"Job "<<jobname<<": Invalid window name."<<std::endl;
			error<<"Job : Invalid window name."<<std::endl;
		}
		else
		{
			if( '!' == name_string[0] )	// Window name segments prepended with '!' mean that the job may not
			{							// have windows whose title matches the given name segment.
				name_string.erase(0, 1);		// Remove the leading '!'
				lists.exclude.push_back(name_string);
			}
			else
			{
				lists.include.push_back(name_string);
			}
		}
	}
}
//}}}


//{{{
bool Window_matching::search(const std::string& search_term, const std::deque<std::string>& list) const
{
	for( std::string name : list )
	{
		if( std::regex_search(search_term, std::regex(name)))
		{
			return true;
		}
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
Json::Value Window_matching::get_win_names(void) const
{
	Json::Value retval;
	for(auto& win_name : matchers.win_names.include)
	{
		retval["included"].append(win_name);
	}
	for(auto& win_name : matchers.win_names.exclude)
	{
		retval["excluded"].append("!"+win_name);
	}

	return retval;
}
//}}}


//{{{
Json::Value Window_matching::get_ws_names(void) const
{
	Json::Value retval;
	for(auto& ws_name : matchers.ws_names.include)
	{
		retval["included"].append(ws_name);
	}
	for(auto& ws_name : matchers.ws_names.exclude)
	{
		retval["excluded"].append("!"+ws_name);
	}

	return retval;
}
//}}}


//{{{
std::ostream& operator<<(std::ostream&stream, const Window_matching& job)
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
