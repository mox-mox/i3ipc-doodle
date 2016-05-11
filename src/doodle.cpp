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
	if(job.isMember("window_names"))
	{
		for(auto& window_name : job.get("window_names", "no window_names"))
		{
			std::string win_name = window_name.asString();
			if(win_name == "no window_names")
			{
				error<<"Job "<<jobname<<": Invalid window name."<<std::endl;
			}
			else
			{
				if( '!' == win_name[0] )	// Window name segments prepended with '!' mean that the job may not
				{							// have windows whose title matches the given name segment.
					win_name.erase(0, 1);	// Remove the leading '!'
					win_names_exclude.push_back(win_name);
				}
				else
				{
					win_names_include.push_back(win_name);
				}
			}
		}
	}
	else
	{
		error<<"Job "<<jobname<<": No window name segments specified."<<std::endl;
	}

	if(job.isMember("workspace_names"))
	{
		for(auto& workspace_name : job.get("workspace_names", "no workspace_names"))
		{
			std::string ws_name = workspace_name.asString();
			if(ws_name == "no workspace_names")
			{
				error<<"Job "<<jobname<<": Invalid workspace name."<<std::endl;
			}
			else
			{
				if( '!' == ws_name[0] )		// Workspace name segments prepended with '!' mean that the job may not
				{							// have windows on workspaces matching the given name segment.
					ws_name.erase(0, 1);	// Remove the leading '!'
					ws_names_exclude.push_back(ws_name);
				}
				else
				{
					ws_names_include.push_back(ws_name);
				}
			}
		}
	}
	else
	{
		error<<"Job "<<jobname<<": No workspace name segments specified."<<std::endl;
	}
}
//}}}
//{{{
Doodle::Job::Job(void) : jobname("NOJOB"), times(), win_names_include(), win_names_exclude({"!"}), ws_names_include(), ws_names_exclude({"!"}) {}
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
	//std::cout<<"Window name segments: ";
	//for( const std::string& segment : window_name_segments )
	//{
	//	std::cout<<segment<<" ";
	//}
	//std::cout<<std::endl<<std::endl;
}
//}}}


//{{{
Doodle::Doodle(i3ipc::connection& conn, std::string config_filename)
	: nojob(), conn(conn), ws_evt_count(0), win_evt_count(0)
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
		}
		if(root.isMember("jobs"))
		{
			for(auto& job : root.get("jobs", "no jobs"))
			{
				jobs.push_back(job);
			}
		}
	}

	nojob.start();									// Account for time spend on untracked jobs
	current_job = &nojob;

	simulate_workspace_change(conn.get_workspaces());	// Inject a fake workspace change event to start tracking the first workspace.
	//simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.

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

//{{{ Name matching functions

//{{{
bool Doodle::ws_excluded(const Job& job)
{
	for( std::string ws_name : job.ws_names_exclude )
	{
		if( std::regex_search(current_workspace, std::regex(ws_name)))
		{
			return true;
		}
	}
	return false;
}
//}}}
//{{{
bool Doodle::ws_included(const Job& job)
{
	for( std::string ws_name : job.ws_names_include )
	{
		if( std::regex_search(current_workspace, std::regex(ws_name)))
		{
			return true;
		}
	}
	return false;
}
//}}}

//{{{
bool Doodle::win_excluded(const Job& job, const std::string& window_title)
{
	for( std::string win_name : job.win_names_exclude )
	{
		if( std::regex_search(window_title, std::regex(win_name)))
		{
			return true;
		}
	}
	return false;
}
//}}}
//{{{
std::string Doodle::win_included(const Job& job, const std::string& window_title)
{
	for( std::string win_name : job.win_names_include )
	{
		if( std::regex_search(window_title, std::regex(win_name)))
		{
			return win_name;
		}
	}
	return "";
}
//}}}
//}}}
//{{{
inline Doodle::win_id_lookup_entry Doodle::find_job(const std::string& window_name)
{
	win_id_lookup_entry retval { &nojob, "" };

	for( Job& j : jobs )									// Search all the jobs to see, if one matches the newly focused window
	{
		if( !j.ws_names_include.empty() || !j.ws_names_exclude.empty() )				// If there are workspaces specified, then ...
		{
			if(ws_excluded(j)) continue; // ... the window may not be on an excluded workspace ...
			if(!ws_included(j)) continue; // ... and it must reside on an included workspace ...
		}

		if(win_excluded(j, window_name)) continue; // If the window matches an excluded name, forget about this job and consider the next one.

		if((retval.matching_name = win_included(j, window_name)) != "")
		{
			if(!settings.detect_ambiguity) // For normal operation, just report the first match.
			{
				retval.job = &j;
				return retval;
			}
			else  // To detect ambiguity, continue searching to see if there are other matches
			{
				if( retval.job != &nojob )
				{
					error<<"Ambiguity: Window name \""<<window_name<<"\" matched "<<retval.job->jobname<<" and "<<j.jobname<<"."<<std::endl;
					// TODO: Show an errow window that asks to which job the window belongs to.
				}
				else
				{
					retval.job = &j;
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

		if(!entry.job || entry.matching_name == "" || !std::regex_search(evt.container->name, std::regex(entry.matching_name)))
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
		simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
	}
}
//}}}

//{{{
void Doodle::on_workspace_change(const i3ipc::workspace_event_t& evt)
{
	if( evt.type == i3ipc::WorkspaceEventType::FOCUS )
	{
		logger<<"New current_workspace: "<<evt.current->name<<std::endl;
		current_workspace = evt.current->name;
		simulate_window_change(conn.get_tree()->nodes);	// Inject a fake window change event to start tracking the first window.
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
	for( const std::string& n : job.win_names_include )
	{
		stream<<" |"<<n<<"|";
	}
	for( const std::string& n : job.win_names_exclude )
	{
		stream<<" |!"<<n<<"|";
	}
	stream<<" workspaces:";
	for( const std::string& w : job.ws_names_include )
	{
		stream<<" |"<<w<<"|";
	}
	for( const std::string& w : job.ws_names_exclude )
	{
		stream<<" |!"<<w<<"|";
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
	stream<<"	Known windows:"<<std::endl<<"		win_id		jobname		matching_name"<<std::endl;
	for( auto it : doodle.win_id_lookup )
	{
		stream<<"		"<<it.first<<"	"<<it.second.job<<"	"<<it.second.matching_name<<std::endl;
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
			if(simulate_window_change(container->nodes)) return true;
		}
	}
	return false;	// Should never be reached
}
//}}}
