#include "window_matching.hpp"
#include <regex>

//{{{
static std::string regex_error_to_string(std::regex_constants::error_type error)
{
	switch(error)
	{
		case std::regex_constants::error_collate:    return "error_collate"; break;
		case std::regex_constants::error_ctype:      return "error_ctype"; break;
		case std::regex_constants::error_escape:     return "error_escape"; break;
		case std::regex_constants::error_backref:    return "error_backref"; break;
		case std::regex_constants::error_brack:      return "error_brack"; break;
		case std::regex_constants::error_paren:      return "error_paren"; break;
		case std::regex_constants::error_brace:      return "error_brace"; break;
		case std::regex_constants::error_badbrace:   return "error_badbrace"; break;
		case std::regex_constants::error_range:      return "error_range"; break;
		case std::regex_constants::error_space:      return "error_space"; break;
		case std::regex_constants::error_badrepeat:  return "error_badrepeat"; break;
		case std::regex_constants::error_complexity: return "error_complexity"; break;
		case std::regex_constants::error_stack:      return "error_stack"; break;
		default:                                     return "unknown error type";
	}
}
//}}}

//{{{
bool Window_matching::search(const std::string& search_term, const std::vector<std::string>& list) const
{
	for( std::string name : list )
	{
		try
		{
			std::regex search_word(name);
			if( std::regex_search(search_term, search_word))
			{
				return true;
			}
		}
		catch (const std::regex_error& e) { // caught by reference to base
			std::string error_type=regex_error_to_string(e.code());
			error<<"Cannot create regular expression from string \""<<name<<"\": "<<e.what()<<" Error type: \""<<error_type<<"\"."<<std::endl;
			continue;
		}
	}
	return false;
}
//}}}

//{{{
bool Window_matching::operator==(const Current& current) const
{
	//debug<<"Job::match(this="<<this<<")"<<std::endl;

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

