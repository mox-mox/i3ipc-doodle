#include "doodle.hpp"
#include <iostream>
#include <fstream>
#include <ios>
#include <regex>

//{{{
Doodle::Timespan::Timespan(Json::Value timespan)
{
	if(timespan.isValidIndex(1))
	{
		std::time_t temp;
		if(( temp = timespan.get(0u, 0).asInt64() ))
		{
			//logger<<"Start time = "<<temp<<std::endl;
			start=temp;
		}
		else
		{
			error<<"Start time invalid"<<std::endl;
		}
		if(( temp = timespan.get(1u, 0).asInt64() ))
		{
			//logger<<"End time = "<<temp<<std::endl;
			end=temp;
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
Doodle::Timespan::Timespan(void) : start(std::time(nullptr)), end(0)
{}
//}}}
//{{{
Doodle::Timespan::operator std::time_t() const
{
	return (end!=0 ? end : std::time(nullptr)) - start;
}
//}}}
//{{{
void Doodle::Timespan::stop(void)
{
	end = std::time(nullptr);
}
//}}}



//{{{
Doodle::Job::Job(Json::Value job)
{
	if((jobname = job.get("jobname", "unset").asString()) == "unset")
	{
		error<<"Jobname not found"<<std::endl;
	}

	if(job.isMember("times"))
	{
		for(auto& timespan : job.get("times", "no times"))
		{
			times.push_back(Timespan(timespan));
		}
	}
	if(job.isMember("window_name_segments"))
	{
		for(auto& window_name_segment : job.get("window_name_segments", "no window_name_segments"))
		{
			window_name_segments.push_back(window_name_segment.asString());
		}
		std::sort(window_name_segments.begin(), window_name_segments.end(), [](const std::string& lhs, const std::string& rhs) -> bool { if(lhs[0]=='!' && rhs[0]!='!') return true; else return false; }); // Sort the entries prepended with '!' first.
	}
	else
	{
		error<<"Job "<<jobname<<": No window name segments specified."<<std::endl;
	}
	if(job.isMember("workspaces"))
	{
		for(auto& workspace : job.get("workspaces", "no workspaces"))
		{
			workspaces.push_back(workspace.asString());
		}
		std::sort(workspaces.begin(), workspaces.end(), [](const std::string& lhs, const std::string& rhs) -> bool { if(lhs[0]=='!' && rhs[0]!='!') return true; else return false; }); // Sort the entries prepended with '!' first.
	}
	else
	{
		error<<"Job "<<jobname<<": No workspaces specified."<<std::endl;
	}
}
//}}}
//{{{
Doodle::Job::Job(const std::string& jobname, const std::deque<Timespan>& times, const std::deque<std::string>& window_name_segments, const std::deque<std::string>& workspaces) :
	jobname(jobname), times(times), window_name_segments(window_name_segments), workspaces(workspaces)
{
}
//}}}
//{{{
void Doodle::Job::start(void)
{
	times.push_back(Timespan());
}
//}}}
//{{{
void Doodle::Job::stop(void)
{
	times.back().stop();
}
//}}}
//{{{
void Doodle::Job::print(void)
{
	std::cout<<*this;
	std::cout<<"Window name segments: ";
	for( const std::string& segment : window_name_segments )
	{
		std::cout<<segment<<" ";
	}
	std::cout<<std::endl<<std::endl;
}
//}}}


//{{{
Doodle::Doodle(i3ipc::connection& conn, std::string config_filename)
	: nojob{"NOJOB", {}, { "!" }, { "!" }}, conn(conn), ws_evt_count(0), win_evt_count(0)
{
	Json::Value root;	// will contains the root value after parsing.
	Json::Reader reader;
	std::ifstream config_file(config_filename);
	{
		std::ofstream config_copy(config_filename+"_backup");
		config_copy<<config_file.rdbuf();
		config_file.clear();
		config_file.seekg(0, std::ios::beg);
	}
	if( !reader.parse(config_file, root, false) )
	{
		// report to the user the failure and their locations in the document.
		error<<reader.getFormattedErrorMessages() <<"\n";
	}

	if(root.isObject())
	{
		if(root.isMember("config"))
		{
			std::cout<<"retrieving config"<<std::endl;
			//std::cout<<root.get("config", "asdf")<<std::endl;
		}
		if(root.isMember("jobs"))
		{
			for(auto& job : root.get("jobs", "no jobs"))
			{
				std::cout<<"JOB "<<job<<std::endl;
				jobs.push_back(job);
			}
		}
	}

	//std::cout<<"Jobs:"<<std::endl;
	//for(auto& job : jobs)
	//{
	//	job.print();
	//	//std::cout<<"	"<<job<<std::endl;
	//}


	nojob.start();									// Account for time spend on untracked jobs
	current_job = &nojob;

	simulate_workspace_change(conn.get_workspaces());	// Inject a fake workspace change event to start tracking the first workspace.
	simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.

	conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));

	if( !conn.subscribe(i3ipc::ET_WORKSPACE|i3ipc::ET_WINDOW))
	{
		error<<"could not connect"<<std::endl;
		throw "Could not subscribe to the workspace- and window change events.";
	}
	#ifdef DEBUG
	else
	{
		logger<<"successfully subscribed"<<std::endl;
	}
	#endif
	}
//}}}

//{{{
inline Doodle::win_id_lookup_entry Doodle::find_job(std::string window_name)
{
	win_id_lookup_entry retval { &nojob, "" };

	for( Job& j : jobs )									// Search all the jobs to see, if one matches the new window
	{
		// First check if the current workspace matches the job:
		if(!j.workspaces.empty())		// If there are workspaces specified...
		{
			bool exclude_job = true;	// ... at least one workspace name segment must match the current window
			for( std::string name_segment : j.workspaces )
			{
				bool exclude_ws = false;
				if( '!' == name_segment[0] )	// Workspace name segments prepended with '!' mean that the job may not
				{								// have windows on workspaces matching the given name segment.
					exclude_ws = true;
					name_segment.erase(0, 1);
				}

				if( std::regex_search(current_workspace, std::regex(name_segment)))
				{
					if( exclude_ws )		// This job may _not_ have windows on this workspace.
					{
						break;
					}
					else					// This job may have windows on this workspace.
					{
						exclude_job = false;
					}
				}
			}
			if(exclude_job) continue; // If no workspaces matched, give up on this job and consider the next one.
		}

		// If the workspaces were no knock-out criterion, check if the window title matches:
		for( std::string name_segment : j.window_name_segments )// ... if one has a matching name segment.
		{
			bool exclude_name = false;
			if( '!' == name_segment[0] )	// Window name segments prepended with '!' mean that the job may not
			{								// have windows whose title matches the give name segment.
				exclude_name = true;
				name_segment.erase(0, 1);
			}

			if( std::regex_search(window_name, std::regex(name_segment)))
			{
				if( exclude_name )			// The window does not belong to the job.
				{
					break;					// out of the name_segment loop -> go to the next job.
				}
				else						// The window belongs to the job.
				{
					//std::cout<<"Window matched job "<<j.jobname<<", matching name segment: "<<name_segment<<". Address:"<<&j<<std::endl;
					if(!settings.detect_ambiguity) // For normal operation, just report the first match.
					{
						retval = { &j, name_segment };
						return retval;
					}
					else  // To detect ambiguity, continue searching to see if there are other matches
					{
						if( retval.job != &nojob )
						{
							error<<"Ambiguity: Window name matched "<<retval.job->jobname<<" and "<<j.jobname<<"."<<std::endl;
							// TODO: Show an errow window that asks to which job the window belongs to.
						}
						else
						{
							retval = { &j, name_segment };
						}
					}
				}
			}
		}
	}
	return retval;
}
//}}}

//{{{
void Doodle::on_window_change(const i3ipc::window_event_t& evt)
{
	//{{{ Print some information about the event

	#ifdef DEBUG
	logger<<"on_window_change() called "<<++win_evt_count<<"th time. Type: "<<static_cast<char>(evt.type)<<std::endl;;
	if(evt.container!=nullptr)
	{
		//logger<<"	id = "<<evt.container->id<<std::endl;
		logger<<"	name = "<<evt.container->name<<std::endl;
		//logger<<"	type = "<<evt.container->type<<std::endl;
		//logger<<"	urgent = "<<evt.container->urgent<<std::endl;
		//logger<<"	focused = "<<evt.container->focused<<std::endl;
	}
	#endif
	//}}}
	if(((evt.type == i3ipc::WindowEventType::FOCUS) || (evt.type == i3ipc::WindowEventType::TITLE)) && (evt.container != nullptr))
	{
		Job* old_job = current_job;
		win_id_lookup_entry& entry = win_id_lookup[evt.container->id];
		//logger<<"matching string: |"<<entry.matching_string<<"|"<<std::endl;

		if(!entry.job || entry.matching_string == "" || !std::regex_search(evt.container->name, std::regex(entry.matching_string)))
		{
			//std::cout<<"Job not found in map."<<std::endl;
			entry = find_job(evt.container->name);
		}
		#ifdef DEBUG
		else
		{
			std::cout<<"Job found in map."<<std::endl;
		}
		#endif
		current_job = entry.job;
		if( old_job != current_job )
		{
			if( old_job )
			{
				old_job->stop();
			}
			if( current_job )
			{
				current_job->start();
			}
		}
		logger<<"New current_job: "<<current_job->jobname<<std::endl;
	}
	else if( evt.type == i3ipc::WindowEventType::CLOSE )
	{
		win_id_lookup.erase(evt.container->id);
	}
}
//}}}

//{{{
void Doodle::on_workspace_change(const i3ipc::workspace_event_t& evt)
{
	//logger<<"on_workspace_change() called "<<++ws_evt_count<<"th time. Type: "<<static_cast<char>(evt.type)<<std::endl;
	//std::cout<<"	Type: "<<static_cast<char>(evt.type)<<std::endl;
	//std::cout<<"	Current.num: "<<evt.current->num<<std::endl;
	//std::cout<<"	Current.visible: "<<evt.current->visible<<std::endl;
	//std::cout<<"	Current.focused: "<<evt.current->focused<<std::endl;
	//std::cout<<"	Current.urgent: "<<evt.current->urgent<<std::endl;

	if( evt.type == i3ipc::WorkspaceEventType::FOCUS )
	{
		logger<<"New current_workspace: "<<evt.current->name<<std::endl;
		current_workspace = evt.current->name;
	}
#ifdef DEBUG
	else
	{
		logger<<"Ignoring Workspace event"<<std::endl;
	}
#endif
}
//}}}

//{{{
std::ostream& operator<<(std::ostream&stream, Doodle::Job const&job)
{
	stream<<"Job \""<<job.jobname<<"\": ";
	std::time_t total_time = 0;
	for( const Doodle::Timespan& t : job.times )
	{
		total_time += t;
	}
	stream<<total_time<<" seconds.";
	stream<<" Names:";
	for( const std::string& n : job.window_name_segments )
	{
		stream<<" |"<<n<<"|";
	}
	stream<<" workspaces:";
	for( const std::string& w : job.workspaces )
	{
		stream<<" "<<w;
	}
	stream<<std::endl;
	return stream;
}
//}}}

//{{{
std::ostream& operator<<(std::ostream&stream, Doodle const&doodle)
{
	stream<<"Doodle class:"<<std::endl;
	stream<<"	Current job: "<<doodle.current_job->jobname<<std::endl;
	stream<<"	Current workspace: "<<doodle.current_workspace<<std::endl;
	stream<<"	Jobs:"<<std::endl;
	for( const Doodle::Job& job : doodle.jobs )
	{
		stream<<"		"<<job<<std::endl;
	}
	stream<<"	Known windows:"<<std::endl<<"		win_id		jobname		matching_string"<<std::endl;
	for( auto it : doodle.win_id_lookup )
	{
		stream<<"		"<<it.first<<"	"<<it.second.job<<"	"<<it.second.matching_string<<std::endl;
	}
	return stream;
}
//}}}

//{{{
bool Doodle::simulate_workspace_change(std::vector < std::shared_ptr < i3ipc::workspace_t>>workspaces)
{	// Iterate through all workspaces and call on_workspace_change() for the focussed one.
	for( std::shared_ptr < i3ipc::workspace_t > &workspace : workspaces )
	{
		if( workspace->focused )
		{
			on_workspace_change({ i3ipc::WorkspaceEventType::FOCUS,  workspace, nullptr });
			return true;
		}
	}
	error<<"No workspace is focused."<<std::endl;
	return false;	// Should never be reached
}
//}}}

//{{{
bool Doodle::simulate_window_change(std::list < std::shared_ptr < i3ipc::container_t>>nodes)
{	// Iterate through all containers and call on_window_change() for the focussed one.
	for( std::shared_ptr < i3ipc::container_t > &container : nodes )
	{
		if( container->focused )
		{
			on_window_change({ i3ipc::WindowEventType::FOCUS,  container });
			return true;
		}
		else
		{
			if( simulate_window_change(container->nodes)) return true;
		}
	}
	return false;	// Should never be reached
}
//}}}
