#include "doodle.hpp"
#include <iostream>

//{{{
Doodle::Doodle(i3ipc::connection& conn) : nojob{"NOJOB", {}, {"!"}, {"!"}}, conn(conn), ws_evt_count(0), win_evt_count(0)
{
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	jobs = {
		{
			.jobname = "project",
			.times = {},
			.window_name_segments = { "home/mox/projects", "~/projects", "zathura" },
			.workspaces = { "" }
		},
		{
			.jobname = "scratch",
			.times = {},
			.window_name_segments = { "home/mox/scratch", "~/scratch", "okular" },
			.workspaces = { "" }
		}
	};
	#pragma GCC diagnostic pop

	nojob.start();										// Account for time spend on untracked jobs
	current_job = &nojob;

	simulate_workspace_change(conn.get_workspaces());	// Inject a fake workspace change event to start tracking the first workspace.
	simulate_window_change(conn.get_tree()->nodes); 	// Inject a fake window change event to start tracking the first window.

	conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));

	if(!conn.subscribe(i3ipc::ET_WORKSPACE | i3ipc::ET_WINDOW))
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
	//win_id_lookup_entry retval;
	win_id_lookup_entry retval{&nojob, ""};
	for( Job& j : jobs)											// Search all the jobs to see, ...
	{
		for( std::string name_segment : j.window_name_segments)	// ... if one has a matching name segment.
		{
			bool exclude = false;
			if('!' == name_segment[0])	// If the name segment is prepended with a '!', finding the name segment
			{							// in the window name means that the window does NOT belong to the job.
				exclude = true;
				name_segment.erase(0, 1);
			}

			if(std::string::npos != window_name.find(name_segment))
			{
				if(exclude)				// The window does not belong to the job.
				{
					break;				// out of the name_segment loop -> go to the next job.
				}
				else					// The window belongs to the job.
				{
					//std::cout<<"Window matched job "<<j.jobname<<", matching name segment: "<<name_segment<<". Address:"<<&j<<std::endl;
					#ifndef DEBUG		// For normal operation, just report the first match.
					retval = { &j, name_segment};
					return retval;
					#else				// When debugging, continue searching to see if there are other matches
					if(retval.job != &nojob)		// e.g. if there is ambiguity in the window_name_segments.
					{
						error<<"Window name matched "<<retval.job->jobname<<" and "<<j.jobname<<"."<<std::endl;
					}
					else
					{
						retval = { &j, name_segment};
					}
					#endif
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
	//logger<<"on_window_change() called "<<++win_evt_count<<"th time. Type: "<<static_cast<char>(evt.type)<<std::endl;;
	//if(evt.container!=nullptr)
	//{
	//	std::cout<<"	id = "<<evt.container->id<<std::endl;
	//	std::cout<<"	name = "<<evt.container->name<<std::endl;
	//	std::cout<<"	type = "<<evt.container->type<<std::endl;
	//	std::cout<<"	urgent = "<<evt.container->urgent<<std::endl;
	//	std::cout<<"	focused = "<<evt.container->focused<<std::endl;
	//}
	#endif
	//}}}
	if((evt.type == i3ipc::WindowEventType::FOCUS || evt.type == i3ipc::WindowEventType::TITLE) && evt.container!=nullptr)
	{
		Job* old_job = current_job;
		win_id_lookup_entry& entry = win_id_lookup[evt.container->id];

		if(!entry.job || std::string::npos == evt.container->name.find(entry.matching_string)) // Window not yet associated with a job
		{																						// or needs re-association
			entry = find_job(evt.container->name);
		}
		#ifdef DEBUG
		else
		{
			std::cout<<"Job found in map."<<std::endl;
		}
		#endif
		current_job = entry.job;
		if(old_job != current_job)
		{
			if(old_job)
			{
				old_job->stop();
			}
			if(current_job)
			{
				current_job->start();
			}
		}
		logger<<"New current_job: "<<current_job->jobname<<std::endl;
	}
	else if(evt.type == i3ipc::WindowEventType::CLOSE)
	{
		win_id_lookup.erase(evt.container->id);
	}
}
//}}}

//{{{
void Doodle::on_workspace_change(const i3ipc::workspace_event_t&  evt)
{
	//logger<<"on_workspace_change() called "<<++ws_evt_count<<"th time. Type: "<<static_cast<char>(evt.type)<<std::endl;
	//std::cout<<"	Type: "<<static_cast<char>(evt.type)<<std::endl;
	//std::cout<<"	Current.num: "<<evt.current->num<<std::endl;
	//std::cout<<"	Current.visible: "<<evt.current->visible<<std::endl;
	//std::cout<<"	Current.focused: "<<evt.current->focused<<std::endl;
	//std::cout<<"	Current.urgent: "<<evt.current->urgent<<std::endl;

	if(evt.type == i3ipc::WorkspaceEventType::FOCUS)
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
std::ostream& operator<< (std::ostream& stream, Doodle::Job const& job)
{
	stream<<"Job \""<<job.jobname<<"\": ";
	duration total_time;
	for(const Doodle::Timespan& t : job.times)
	{
		total_time += t;
	}
	stream << std::chrono::duration_cast<std::chrono::seconds>(total_time).count() << " seconds.";
	return stream;
}
//}}}

//{{{
std::ostream& operator<< (std::ostream& stream, Doodle const& doodle)
{
	stream<<"Doodle class:"<<std::endl;
	stream<<"	Current job: "<<doodle.current_job->jobname<<std::endl;
	stream<<"	Current workspace: "<<doodle.current_workspace<<std::endl;
	stream<<"	Jobs:"<<std::endl;
	for(const Doodle::Job& job : doodle.jobs)
	{
		stream<<"		"<<job<<std::endl;
	}
	stream<<"	Known windows:"<<std::endl<<"		win_id		jobname		matching_string"<<std::endl;
	for (auto it : doodle.win_id_lookup)
	{
		stream<<"		"<<it.first<<"	"<<it.second.job<<"	"<<it.second.matching_string<<std::endl;
	}
	return stream;
}
//}}}

//{{{
bool Doodle::simulate_workspace_change(std::vector< std::shared_ptr<i3ipc::workspace_t> > workspaces)
{	// Iterate through all workspaces and call on_workspace_change() for the focussed one.
	for(std::shared_ptr<i3ipc::workspace_t>& workspace : workspaces)
	{
		if(workspace->focused)
		{
			on_workspace_change( { i3ipc::WorkspaceEventType::FOCUS,  workspace, nullptr } );
			return true;
		}
	}
	error<<"No workspace is focused."<<std::endl;
	return false; // Should never be reached
}
//}}}

//{{{
bool Doodle::simulate_window_change(std::list< std::shared_ptr<i3ipc::container_t > > nodes )
{	// Iterate through all containers and call on_window_change() for the focussed one.
	for(std::shared_ptr<i3ipc::container_t>& container : nodes)
	{
		if(container->focused)
		{
			on_window_change( { i3ipc::WindowEventType::FOCUS,  container } );
			return true;
		}
		else
		{
			if(simulate_window_change(container->nodes))
				return true;
		}
	}
	return false; // Should never be reached
}
//}}}
