#pragma once
#include "main.hpp"
#include <string>

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
		bool match(const std::string& current_workspace, const std::string& window_title) const;

		bool operator ==(const Current& current) const;
		bool operator !=(const Current& current) const;

		friend std::ostream& operator<<(std::ostream& stream, const Window_matching& job);
};

