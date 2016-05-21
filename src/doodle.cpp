#include "doodle.hpp"
#include <ios>
#include <regex>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include "logstream.hpp"







//{{{
Doodle::Doodle(i3ipc::connection& conn, const std::string& config_path)
	: conn(conn), config_path(config_path), nojob()
{
	std::ifstream config_file(config_path+"/doodlerc");


	//error<<"config file: "<<config_path+"/doodlerc"<<std::endl;
	//std::cout<<config_file.rdbuf();
	//config_file.clear();
	//config_file.seekg(0, std::ios::beg);


	Json::Value configuration_root;
	Json::Reader reader;

	if( !reader.parse(config_file, configuration_root, false))
	{
		error<<reader.getFormattedErrorMessages()<<std::endl;
	}
	std::cout<<"retrieving config"<<std::endl;
	read_config(configuration_root);

	if( configuration_root.isMember("config"))
	{
		std::cout<<"retrieving config"<<std::endl;
		read_config(configuration_root.get("config", "no config"));
	}
	for( auto&f: std::experimental::filesystem::directory_iterator(config_path+"/jobs"))
	{
		if((f.path() != config_path+"/doodlerc") && std::experimental::filesystem::is_regular_file(f))
		{
			std::ifstream jobfile(f.path());
			Json::Value job;
			if( !reader.parse(jobfile, job, false))
			{
				error<<reader.getFormattedErrorMessages()<<std::endl;
			}
			else
			{
				jobs.push_back(Job(f.path().filename(), job, f.path()));
			}
		}
	}

	nojob.start(std::chrono::steady_clock::now());										// Account for time spent on untracked jobs
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

//{{{
void Doodle::read_config(Json::Value config)
{
	settings.max_idle_time = config.get("max_idle_time", settings.MAX_IDLE_TIME_DEFAULT_VALUE).asUInt();
	std::cout<<"	max_idle_time = "<<settings.max_idle_time<<std::endl;
	settings.detect_ambiguity = config.get("detect_ambiguity", settings.DETECT_AMBIGUITY_DEFAULT_VALUE).asBool();
	std::cout<<"	detect_ambiguity = "<<settings.detect_ambiguity<<std::endl;
}
//}}}

//{{{
inline Doodle::win_id_lookup_entry Doodle::find_job(const std::string& window_name)
{
	win_id_lookup_entry retval {
		&nojob, ""
	};

	for( Job& j : jobs )									// Search all the jobs to see, if one matches the newly focused window
	{
		if((retval.matching_name = j.match(current_workspace, window_name)) != "" )
		{
			if( !settings.detect_ambiguity )// For normal operation, just report the first match.
			{
				retval.job = &j;
				return retval;
			}
			else	// To detect ambiguity, continue searching to see if there are other matches
			{
				if( retval.job != &nojob )
				{
					error<<"Ambiguity: Window name \""<<window_name<<"\" matched "<<retval.job->get_jobname()<<" and "<<j.get_jobname()<<"."<<std::endl;
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
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	//{{{ Print some information about the event

	#ifdef DEBUG
		logger<<"on_window_change() called "<<++win_evt_count<<"th time. Type: "<<static_cast < char > (evt.type)<<std::endl;
		;
		if( evt.container != nullptr )
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

		if( !entry.job || (entry.matching_name == "") || !std::regex_search(evt.container->name, std::regex(entry.matching_name)))
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
				old_job->stop(now);
			}
			if( current_job )
			{
				current_job->start(now);
			}
		}
		logger<<"New current_job: "<<current_job->get_jobname()<<std::endl;
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
std::ostream& operator<<(std::ostream&stream, Doodle const&doodle)
{
	stream<<"Doodle class:"<<std::endl;
	stream<<"	Current job: "<<doodle.current_job->get_jobname()<<std::endl;
	stream<<"	Current workspace: "<<doodle.current_workspace<<std::endl;
	stream<<"	Jobs:"<<std::endl;
	for( const Job& job : doodle.jobs )
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
			if( simulate_window_change(container->nodes)) return true;
		}
	}
	return false;	// Should never be reached
}
//}}}
