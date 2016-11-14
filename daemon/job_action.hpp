#pragma once
// This class is a nested class of Job. This file may only be included _in_the_body_of_class_Job!

class Action
{
	std::string action;

	//{{{ Matching

	//{{{
	struct matchers
	{
		std::deque < std::string > win_names_include;
		std::deque < std::string > win_names_exclude;
		std::deque < std::string > ws_names_include;
		std::deque < std::string > ws_names_exclude;
	} matchers;
	//}}}

	inline bool ws_excluded(const std::string& current_workspace) const;
	inline bool ws_included(const std::string& current_workspace) const;

	inline bool win_excluded(const std::string& window_title) const;
	inline bool win_included(const std::string& window_title) const;
	//}}}



};
