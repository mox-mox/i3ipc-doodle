#include "timespan.hpp"
#include "logstream.hpp"

//{{{
Timespan::Timespan(Json::Value timespan)
{
	if( timespan.isValidIndex(1))
	{
		std::time_t temp;
		if((temp = timespan.get(0u, 0).asInt64()))
		{
			//logger<<"Start time = "<<temp<<std::endl;
			start = temp;
		}
		else
		{
			error<<"Start time invalid"<<std::endl;
		}
		if((temp = timespan.get(1u, 0).asInt64()))
		{
			//logger<<"End time = "<<temp<<std::endl;
			end = temp;
		}
		else
		{
			error<<"End time invalid"<<std::endl;
		}
	}
	else
	{
		error<<"Could not get the times for this Timespan"<<std::endl;
	}
}
//}}}
//{{{
Timespan::Timespan(void) : start(std::time(nullptr)), end(0)
{}
//}}}
//{{{
Timespan::operator std::time_t() const
{
	return (end != 0 ? end : std::time(nullptr))-start;
}
//}}}
//{{{
void Timespan::stop(void)
{
	end = std::time(nullptr);
}
//}}}
