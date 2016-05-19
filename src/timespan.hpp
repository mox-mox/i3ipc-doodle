#pragma once
#include <ctime>
#include <json/json.h>


class Timespan
{
	std::time_t start;
	std::time_t end;
	public:
		Timespan(void);
		Timespan(Json::Value timespan);
		void stop(void);
		operator std::time_t() const;
};
