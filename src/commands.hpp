// This file is nested inside doodle.hpp. This file may only be included _in_the_body_of_that_class_!


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

using command_function   = std::function<std::string(const std::vector<std::string>&)>;
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

	{ "help",{"                     <none>|<jobname>",                                          "Print this help",
			[this](const std::vector<std::string>& args)//{{{
			{
				using namespace std::string_literals;
				std::string response;
				if(args.size() > 0)
				{
					// If there is a mapping, execute the mapped function...
					if(auto action = actions.find(args[0]); action != actions.end())
					{
						response += "	" + action->first + " " + std::regex_replace(action->second.arguments, std::regex("^\\s+|\\s+$"), "", std::regex_constants::format_default) + "\n	" + action->second.explanation + "\n";
					}
					else // ... or return an error code.
					{
						return "Command not found: \"" + args[0] + "\"";
					}
				}
				else
				{
					for(auto& action : actions)
					{
						response += "	" + action.first + action.second.arguments + "\n";
					}
				}
				return response;
			}}},
	{ "suspend",{"                  <none>",                                                    "If \'stop_on_suspend\' is set, wrtie times to disk, stop the current job and halt operation until resumed. Else ignored.",
			[this](const std::vector<std::string>&)//{{{
			{
				std::string response;
				if(stop_on_suspend)
				{
					response = "Suspending: stopped jobs, wrote times";
					// TODO
				}
				else
				{
					response = "Suspending";
				}
				return response;
			}}},
	{ "resume",{"                   <none>",                                                    "If suspended, restart operation, e.g. re-start the current job. Else ignored.",
			[this](const std::vector<std::string>&)//{{{
			{
				std::string response;
				if(stop_on_suspend)
				{
					response = "Waking up: Activating current job";
					// TODO
				}
				else
				{
					response = "Waking up: already awake";
				}
				return response;
			}}},
	{ "reload",{"                   <none>",                                                    "Reload the configuration.",
			[this](const std::vector<std::string>&)//{{{
			{
				std::string response;
				response = "Not implemented yet. Just restart doodle manually";
				// TODO
				return response;
			}}},
	{ "kill",{"                     <none>",                                                    "Stop the daemon.",
			[this](const std::vector<std::string>&)//{{{
			{
				loop->stop();
				return "Killing daemon";
			}}},
	//}}}
	//{{{ Information commands

	//{{{ Getting information

	{ "get-config-path",{"          <none>",                                                    "Return the path to the configuration files used by the daemon.",
			[this](const std::vector<std::string>&)//{{{
			{
				return config_dir;
			}}},
	{ "get-data-path",{"            <none>",                                                    "Return the path to the data/times files used by the daemon.",
			[this](const std::vector<std::string>&)//{{{
			{
				return data_dir;
			}}},
	{ "get-max-idle-time",{"        <none>",                                                    "Return the time since last user action (type a key somewhere or move the mouse) before the user is considered idle and the active job is stopped.",
			[this](const std::vector<std::string>&)//{{{
			{
				return ms_to_string(max_idle_time_ms);
			}}},
	{ "get-detect-ambiguity",{"     <none>",                                                    "Return wether the daemon checks for ambigous job- and window-name matchers.",
			[this](const std::vector<std::string>&)//{{{
			{
				return detect_ambiguity?"Checking for ambigous matchers":"Not checking for ambigous natchers";
			}}},
	{ "get-jobs",{"                 <none>",                                                    "Return a list of all jobs known to doodle.",
			[this](const std::vector<std::string>&)//{{{
			{
				std::string response = "Jobs: \n";
				for(auto& job : jobs)
				{
					response += "	" + std::string(job) + "\n";
				}
				return response;
			}}},
	//}}}
	//{{{ Setting commands

	{ "set-max-idle-time",{"        <maximum time> | 0",                                        "Set the time since last user action (type a key somewhere or move the mouse) before the user is considered idle and the active job is stopped. Setting to zero disables the checking.",
			[this](const std::vector<std::string>& args)//{{{
			{
				using namespace std::string_literals;
				if(args.size() < 1) return "Set max_idle_time needs one argument, the new idle time"s;
				milliseconds old_max_idle_time_ms = max_idle_time_ms;
				max_idle_time_ms = time_string_to_milliseconds(args[0]);

				if(max_idle_time_ms > milliseconds(0))
				{
					if(old_max_idle_time_ms == milliseconds(0)) idle_timer->start(milliseconds(20),milliseconds(0));
					else on_idle_timer(uvw::TimerEvent(), *idle_timer);
				}
				else
				{
					idle_timer->stop();
				}

				return "Set maximum idle time to "+ms_to_string(max_idle_time_ms);
			}}},
	{ "set-detect-ambiguity",{"     <true|false>",                                              "Set wether the daemon should check for ambigous job- and window-name matchers. This is a runtime check so will cost some additional processing time.",
			[this](const std::vector<std::string>& args)//{{{
			{
				using namespace std::string_literals;
				if(args.size() < 1) return "Set detect_ambiguity needs one argument"s;
				if(args[0] == "true" || args[0] == "True" || args[0] == "TRUE" || args[0] == "1")
					detect_ambiguity = true;
				else if(args[0] == "false" || args[0] == "False" || args[0] == "FALSE" || args[0] == "0")
					detect_ambiguity = false;
				else
					return "Cannot decide if \"" + args[0] + "\" means true or false. Leving detect_ambiguity unchanged.";



				return detect_ambiguity ? "Enabled run-time checking for ambigous window- and workspace-name watchers"s : "Enabled run-time checking for ambigous window- and workspace-name watchers"s;
			}}},
	//}}}
	//}}}
	//{{{ Job commands

	//{{{ General commands
	{ "job-reload",{"               <jobname>",                                                 "Reload configuration for the job.",
			[this](const std::vector<std::string>& args)//{{{
			{
				//{{{
				using namespace std::string_literals;
				if(args.size() < 1) return "job_* commands need at least one argument, the jobname"s;
				std::string jobname = args[0];
				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
				if(it == jobs.end()) return "No job named \"" + jobname + "\" found.";
				//}}}

				const fs::path jobconfig_path = it->jobconfig_path;
				std::shared_ptr<uvw::Loop> loop = it->loop;
				try
				{
				jobs.replace(it, jobconfig_path, loop);
				}
				catch (const std::exception& e)
				{
					return "Could not reload job at "s + jobconfig_path.string() + ": " + e.what();
				}
				catch(...)
				{
					return "Could not reload job at "s + jobconfig_path.string() + ".";
				}

				return "Reloaded job "s + std::string(*it);
			}}},
	//}}}
	//{{{ Getters
	{ "job-get-granularity",{"      <jobname>",                                                 "Return the minimum time intervall between writing the next timestamp to disk.",
			[this](const std::vector<std::string>& args)//{{{
			{
				//{{{
				using namespace std::string_literals;
				if(args.size() < 1) return "job_* commands need at least one argument, the jobname"s;
				std::string jobname = args[0];
				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
				if(it == jobs.end()) return "No job named \"" + jobname + "\" found.";
				//}}}
				return "Granularity is " + ms_to_string(it->timefile.get_granularity());
			}}},
	{ "job-get-suppress-time",{"     <jobname>",                                                 "Return the time a job must have been active to actually be considered in the timing.",
			[this](const std::vector<std::string>& args)//{{{
			{
			//{{{
				using namespace std::string_literals;
				if(args.size() < 1) return "job_* commands need at least one argument, the jobname"s;
				std::string jobname = args[0];
				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
				if(it == jobs.end()) return "No job named \"" + jobname + "\" found.";
				//}}}
				return "Suppression time is " + ms_to_string(it->suppress);
			}}},
	{ "job-get-times",{"            <jobname> [<intervall start time> [<intervall stop time>]]","Return a list of all timestamps in the time intervall for a job. To list from the begin set Start time to 0, the start time defaults to 0, the stop time to now if omitted.",
			[this](const std::vector<std::string>& args)//{{{
			{
				//{{{
				using namespace std::string_literals;
				if(args.size() < 1) return "job_* commands need at least one argument, the jobname"s;
				std::string jobname = args[0];
				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
				if(it == jobs.end()) return "No job named \"" + jobname + "\" found.";
				//}}}

				std::time_t start_time;
				std::time_t stop_time;
				//{{{
				if(args.size() >= 2)
				{
					start_time = parse_time(args[1]);
				}
				else
				{
					start_time = 0;
				}
				//}}}
				//{{{
				if(args.size() >= 3)
				{
					stop_time = parse_time(args[2]);
				}
				else
				{
					stop_time = 0;
				}
				//}}}

				//std::cout<<"job-get-times("<<start_time<<", "<<stop_time<<");"<<std::endl;
				return it->timefile.get_times(start_time, stop_time);
			}}},
	{ "job-get-win-names",{"        <jobname>",                                                 "Return a list of all window name matchers for the job.",
			[this](const std::vector<std::string>& args)//{{{
			{
				//{{{
				using namespace std::string_literals;
				if(args.size() < 1) return "job_* commands need at least one argument, the jobname"s;
				std::string jobname = args[0];
				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
				if(it == jobs.end()) return "No job named \"" + jobname + "\" found.";
				//}}}

				std::string response;
				for(std::string& win_name : it->matchers.win_names.include)
				{
					response += win_name + ", ";
				}
				for(std::string& win_name : it->matchers.win_names.exclude)
				{
					response += "!" + win_name + ", ";
				}
				if(response.length() > 2)
				{
					response.pop_back();
					response.pop_back();
				}
				return response;
			}}},
	{ "job-get-ws-names",{"         <jobname>",                                                 "Return a list of all workspace name matchers for the job.",
			[this](const std::vector<std::string>& args)//{{{
			{
				//{{{
				using namespace std::string_literals;
				if(args.size() < 1) return "job_* commands need at least one argument, the jobname"s;
				std::string jobname = args[0];
				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
				if(it == jobs.end()) return "No job named \"" + jobname + "\" found.";
				//}}}

				std::string response;
				for(std::string& ws_name : it->matchers.ws_names.include)
				{
					response += ws_name + ", ";
				}
				for(std::string& ws_name : it->matchers.ws_names.exclude)
				{
					response += "!" + ws_name + ", ";
				}
				if(response.length() > 2)
				{
					response.pop_back();
					response.pop_back();
				}
				return response;
			}}},
	//}}}
//	//{{{ Adders
//
//	{ "job-add-times",{"            <jobname> {(<start_time> <duration> [#comment])}",          "Write additional time stamps to the time file, postfixed with the comment.",
//			[this](const std::vector<std::string>& args)//{{{
//			{
//			}}},
//	{ "job-add-win-names",{"        <jobname> {<name to include>|!<name to exclude>}",          "Temporarily (untile the daemon stops), add window name matchers to a job. Prefix exclusion matchers with a bang. To permanently add matchers, use the config file for the job.",
//			[this](const std::vector<std::string>& args)//{{{
//			{
//				//{{{
//				std::string jobname = arg.asString();
//				auto it = std::find_if(jobs.begin(), jobs.end(), [jobname](const Job& job){return job.jobname == jobname; });
//				//}}}
//				if(it != jobs.end())
//				{
//				}
//				else
//				{
//				}
//				Json::Value retval;
//				retval["command"] = "help";
//				Json::Value response;
//				response[response.size()][0] = "Added window names";
//				retval["response"]=response;
//				return retval;
//			}}},
//	{ "job-add-ws-names",{"         <jobname> {<name to include>|!<name to exclude>}",          "Temporarily (untile the daemon stops), add workspace name matchers to a job. Prefix exclusion matchers with a bang. To permanently add matchers, use the config file for the job.",
//			[this](const std::vector<std::string>& args)//{{{
//			{
//			}}},
//	//}}}
//	//{{{ Removers
//
//	{ "job-remove-times",{"         <jobname> {(<start_time> <duration>)}",                     "Remove the given time stamps from the time file.",
//			[this](const std::vector<std::string>& args)//{{{
//			{
//			}
//	} },
//	{ "job-remove-win-names",{"     <jobname> {<name not to include>|!<name not to exclude>}",  "Temporarily (untile the daemon stops), remove window name matchers from a job. Prefix exclusion matchers with a bang. To permanently remove matchers, use the config file.",
//			[this](const std::vector<std::string>& args)//{{{
//			{
//			}}},
//	{ "job-remove-ws-names",{"      <jobname> {<name not to include>|!<name not to exclude>}",  "Temporarily (untile the daemon stops), remove workspace name matchers from a job. Prefix exclusion matchers with a bang. To permanently remove matchers, use the config file.",
//			[this](const std::vector<std::string>& args)//{{{
//			{
//			}}},
//	//}}}
	//}}}

	//}}}

};

#pragma GCC diagnostic pop




std::string run_command(std::string commandline)
{
	// Split the line into words
	std::istringstream iss(commandline);
	std::vector<std::string> args(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

	std::string command(args[0]);
	args.erase(args.begin());

	// If there is a mapping, execute the mapped function...
	if(auto fun = actions.find(command); fun != actions.end())
	{
		return std::invoke(fun->second.function, args);
	}
	else // ... or return an error code.
	{
		return "Command not found: \"" + command + "\"";
	}
}




