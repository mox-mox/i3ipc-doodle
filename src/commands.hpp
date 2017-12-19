// This file is nested inside doodle.hpp. This file may only be included _in_the_body_of_that_class_!

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

using command_function   = std::function<Json::Value(Json::Value&)>;
//{{{
struct command_t
{
	std::string arguments;
	std::string explanation;
	command_function function;
};
//}}}
using action_map = std::map<std::string, command_t>;

// All the terminal commands. Yes, this is one huge std::map of lambdas. C++ does not offer reflection and this way there is only one definition.
action_map actions = {
	//{{{ General commands

	{ "help",{"                     <none>",                                                    "Print this help",
			[this](Json::Value&)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				for(auto& action : actions)
				{
					response[response.size()][0] = action.first + action.second.arguments + action.second.explanation;
				}
				retval["response"]=response;
				return retval;
			}}},
	{ "suspend",{"                  <none>",                                                    "If \'stop_on_suspend\' is set, wrtie times to disk, stop the current job and halt operation until resumed. Else ignored.",
			[this](Json::Value&)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				if(stop_on_suspend)
				{
					response[response.size()][0] = "Suspending: stopped jobs, wrote times";
				}
				else
				{
					response[response.size()][0] = "Suspending";
				}
				retval["response"]=response;
				return retval;
			}}},
	{ "resume",{"                   <none>",                                                    "If suspended, restart operation, e.g. re-start the current job. Else ignored.",
			[this](Json::Value&)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				if(suspended)
				{
					response[response.size()][0] = "Waking up: Activating current job";
					suspended = false;
				}
				else
				{
					response[response.size()][0] = "Waking up: already awake";
				}
				retval["response"]=response;
				return retval;
			}}},
	{ "reload",{"                   <none>",                                                    "Reload the configuration.",
			[this](Json::Value&)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = "No implemented yet. Please restart doodle manually";
				retval["response"]=response;
				return retval;
			}}},
	{ "kill",{"                     <none>",                                                    "Stop the daemon.",
			[this](Json::Value&)//{{{
			{
				loop->stop();
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = "Killing daemon";
				retval["response"]=response;
				return retval;
			}}},
	//}}}
	//{{{ Information commands

	//{{{ Getting information

	{ "get_config_path",{"          <none>",                                                    "Return the path to the configuration files used by the daemon.",
			[this](Json::Value&)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = config_dir;
				retval["response"]=response;
				return retval;
			}}},
	{ "get_data_path",{"            <none>",                                                    "Return the path to the data/times files used by the daemon.",
			[this](Json::Value&)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = data_dir;
				retval["response"]=response;
				return retval;
			}}},
	{ "get_max_idle_time",{"        <none>",                                                    "Return the time since last user action (type a key somewhere or move the mouse) before the user is considered idle and the active job is stopped.",
			[this](Json::Value&)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = ms_to_string(max_idle_time_ms);
				retval["response"]=response;
				return retval;
			}}},
	{ "get_detect_ambiguity",{"     <none>",                                                    "Return wether the daemon checks for ambigous job- and window-name matchers.",
			[this](Json::Value&)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = detect_ambiguity?"Checking for ambigous matchers":"Not checking for ambigous natchers";
				retval["response"]=response;
				return retval;
			}}},
	{ "get_jobs",{"                 <none>",                                                    "Return a list of all jobs known to doodle.",
			[this](Json::Value&)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				for(auto& job : jobs)
				{
					response[response.size()][0] = std::string(job);
				}
				retval["response"]=response;
				return retval;
			}}},
	//}}}
	//{{{ Setting commands

	{ "set_max_idle_time",{"        <maximum time> | 0",                                        "Set the time since last user action (type a key somewhere or move the mouse) before the user is considered idle and the active job is stopped. Setting to zero disables the checking.",
			[this](Json::Value& arg)//{{{
			{
				milliseconds old_max_idle_time_ms = max_idle_time_ms;
				max_idle_time_ms = time_string_to_milliseconds(arg.asString());

				if(max_idle_time_ms > milliseconds(0))
				{
					if(old_max_idle_time_ms == milliseconds(0)) idle_timer->start(milliseconds(20),milliseconds(0));
					else on_idle_timer(uvw::TimerEvent(), *idle_timer);
				}
				else
				{
					idle_timer->stop();
				}

				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = "Set maximum idle time to "+ms_to_string(max_idle_time_ms);
				retval["response"]=response;
				return retval;
			}}},
	{ "set_detect_ambiguity",{"     <true|false>",                                              "Set wether the daemon should check for ambigous job- and window-name matchers. This is a runtime check so will cost some additional processing time.",
			[this](Json::Value& arg)//{{{
			{
				detect_ambiguity = arg.asBool();

				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = detect_ambiguity ? "Enabled run-time checking for ambigous window- and workspace-name watchers" : "Enabled run-time checking for ambigous window- and workspace-name watchers";
				retval["response"]=response;
				return retval;
			}}},
	//}}}
	//}}}
	//{{{ Job commands

	//{{{ General commands
	{ "job_reload",{"               <jobname>",                                                 "Reload configuration for the job.",
			[this](Json::Value& arg)//{{{
			{
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = "No implemented yet. Please restart doodle manually";
				retval["response"]=response;
				return retval;
			}}},
	//}}}
	//{{{ Getters
	{ "job_get_granularity",{"      <jobname>",                                                 "Return the minimum time intervall between writing the next timestamp to disk.",
			[this](Json::Value& arg)//{{{
			{
				//{{{
				std::string jobname = arg.asString();
				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
				//}}}
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = it != jobs.end() ? "Granularity is " + ms_to_string(it->timefile.get_granularity()) : "No job named \"" + jobname + "\" found";
				retval["response"]=response;
				return retval;
			}}},
	{ "job_get_supress_time",{"     <jobname>",                                                 "Return the time a job must have been active to actually be considered in the timing.",
			[this](Json::Value& arg)//{{{
			{
				//{{{
				std::string jobname = arg.asString();
				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
				//}}}
				Json::Value retval;
				retval["command"] = "help";
				Json::Value response;
				response[response.size()][0] = it != jobs.end() ? "Granularity is " + ms_to_string(it->suppress) : "No job named \"" + jobname + "\" found";
				retval["response"]=response;
				return retval;
			}}},
	{ "job_get_times",{"            <jobname> [<intervall start time> [<intervall stop time>]]","Return a list of all timestamps in the time intervall for a job. To list from the begin set Start time to 0, the start time defaults to 0, the stop time to now if omitted.",
			[this](Json::Value& arg)//{{{
			{
				//{{{
				std::string jobname = arg.asString();
				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
				//}}}
				std::time_t start_time;
				std::time_t stop_time;
				//{{{
				if(std::string start_time_string(arg.get("start_time", "0").asString()); start_time_string != "0")
				{
					start_time = parse_time(start_time_string);
				}
				else
				{
					start_time = 0;
				}
				//}}}
				//{{{
				if(std::string stop_time_string(arg.get("stop_time", "now").asString()); stop_time_string != "now")
				{
					stop_time = parse_time(stop_time_string);
				}
				else
				{
					stop_time = 0;
				}
				//}}}
				Json::Value retval;
				retval["command"] = "help";
				//Json::Value response;
				//response[response.size()][0] = it != jobs.end() ? "Granularity is " + ms_to_string(it->suppress) : "No job named \"" + jobname + "\" found";
				retval["response"]=it != jobs.end() ?  it->timefile.get_times(start_time, stop_time) : "No job named \"" + jobname + "\" found";
				return retval;
			}}},
//	{ "job_get_win_names",{"        <jobname>",                                                 "Return a list of all window name matchers for the job.",
//			[this](Json::Value& arg)//{{{
//			{
//			}}},
//	{ "job_get_ws_names",{"         <jobname>",                                                 "Return a list of all workspace name matchers for the job.",
//			[this](Json::Value& arg)//{{{
//			{
//			}}},
//	//}}}
//	//{{{ Adders
//
//	{ "job_add_times",{"            <jobname> {(<start_time> <duration> [#comment])}",          "Write additional time stamps to the time file, postfixed with the comment.",
//			[this](Json::Value& arg)//{{{
//			{
//			}}},
//	{ "job_add_win_names",{"        <jobname> {<name to include>|!<name to exclude>}",          "Temporarily (untile the daemon stops), add window name matchers to a job. Prefix exclusion matchers with a bang. To permanently add matchers, use the config file for the job.",
//			[this](Json::Value& arg)//{{{
//			{
//			}}},
//	{ "job_add_ws_names",{"         <jobname> {<name to include>|!<name to exclude>}",          "Temporarily (untile the daemon stops), add workspace name matchers to a job. Prefix exclusion matchers with a bang. To permanently add matchers, use the config file for the job.",
//			[this](Json::Value& arg)//{{{
//			{
//			}}},
//	//}}}
//	//{{{ Removers
//
//	{ "job_remove_times",{"         <jobname> {(<start_time> <duration>)}",                     "Remove the given time stamps from the time file.",
//			[this](Json::Value& arg)//{{{
//			{
//			}
//	} },
//	{ "job_remove_win_names",{"     <jobname> {<name not to include>|!<name not to exclude>}",  "Temporarily (untile the daemon stops), remove window name matchers from a job. Prefix exclusion matchers with a bang. To permanently remove matchers, use the config file.",
//			[this](Json::Value& arg)//{{{
//			{
//			}}},
//	{ "job_remove_ws_names",{"      <jobname> {<name not to include>|!<name not to exclude>}",  "Temporarily (untile the daemon stops), remove workspace name matchers from a job. Prefix exclusion matchers with a bang. To permanently remove matchers, use the config file.",
//			[this](Json::Value& arg)//{{{
//			{
//			}}},
	//}}}
	//}}}

};

#pragma GCC diagnostic pop







