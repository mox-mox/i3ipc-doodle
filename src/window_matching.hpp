#pragma once
#include "main.hpp"
#include <string>

//{{{
struct Names
{
	std::string workspace_name;
	std::string window_name;
};
//}}}

class Window_matching
{
	protected:

	struct Matchers
	{
		struct Names
		{
			std::vector<std::string> include;
			std::vector<std::string> exclude;
		} win_names, ws_names;
	} matchers;

	bool search(const std::string& search_term, const std::vector<std::string>& list) const;


	public:
		bool operator ==(const Names& current) const;
		bool operator !=(const Names& current) const;

		friend std::ostream& operator<<(std::ostream& stream, const Window_matching& job);
};

