#pragma once
#include "main.hpp"
#include <string>
#include <deque>
#include <json/json.h>
#include <regex>

class Window_matching
{
	struct matchers_t
	{
		struct names_t
		{
			std::deque<std::string> include;
			std::deque<std::string> exclude;
		} win_names, ws_names;
	} matchers;


	bool search(const std::string& search_term, const std::deque<std::string>& list) const;

	void parse_names(matchers_t::names_t& lists, const Json::Value& names);

	public:
		//{{{ Constructors

		Window_matching(const Json::Value& matchers);
		Window_matching(Window_matching && other);
		Window_matching(void);
		//}}}

		bool match(const std::string& current_workspace, const std::string& window_title) const;

		//{{{ Functions for terminal

		Json::Value get_win_names(void) const;
		Json::Value get_ws_names(void) const;
		friend std::ostream& operator<<(std::ostream&stream, const Window_matching& job);
		//}}}

		#ifdef COMPILE_UNIT_TESTS
			#include "window_matching_diagnostics.hpp"		// all the testing functions for the window_matching class
		#endif
};
