#include "doodle.hpp"
#include <iostream>




//{{{
Doodle::Doodle(i3ipc::connection& conn) : conn(conn), evt_count(0)
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

			for( Job& j : jobs)
			{
				std::cout<<"Address of "<<j.jobname<<" is "<<&j<<"."<<std::endl;
			}
	#pragma GCC diagnostic pop

	conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));
	if(!conn.subscribe(i3ipc::ET_WORKSPACE | i3ipc::ET_WINDOW))
	{
		std::cout<<"could not connect"<<std::endl;
	}
	else
	{
		std::cout<<"successfully subscribed"<<std::endl;
	}
}
//}}}


//{{{
inline Doodle::win_id_lookup_entry Doodle::find_job(std::string window_name)
{
	win_id_lookup_entry retval;
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
					std::cout<<"Window matched job "<<j.jobname<<", matching name segment: "<<name_segment<<". Address:"<<&j<<std::endl;
					#ifndef DEBUG		// For normal operation, just report the first match.
					retval = { &j, name_segment};
					return retval;
					#else				// When debugging, continue searching to see if there are other matches
					if(retval.job)		// e.g. if there is ambiguity in the window_name_segments.
					{
						std::cerr<<"(EE): Window name matched "<<retval->jobname<<" and "<<j.jobname<<"."<<std::endl;
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
	std::cout<<"(LL): on_window_change() called "<<++evt_count<<"th time."<<std::endl;
	std::cout<<"	type: "<<static_cast<char>(evt.type)<<std::endl;
	if(evt.container!=nullptr)
	{
		std::cout<<"	id = "<<evt.container->id<<std::endl;
		//std::cout<<"	xwindow_id = "<<evt.container->xwindow_id<<std::endl;
		std::cout<<"	name = "<<evt.container->name<<std::endl;
		std::cout<<"	type = "<<evt.container->type<<std::endl;
		std::cout<<"	border = "<<static_cast<char>(evt.container->border)<<std::endl;
		//std::cout<<"	border_raw = "<<evt.container->border_raw<<std::endl;
		//std::cout<<"	current_border_width = "<<evt.container->current_border_width<<std::endl;
		//std::cout<<"	layout = "<<static_cast<char>(evt.container->layout)<<std::endl;
		//std::cout<<"	layout_raw = "<<evt.container->layout_raw<<std::endl;
		//std::cout<<"	percent = "<<evt.container->percent<<std::endl;
		//std::cout<<"	rect.x = "<<evt.container->rect.x<<std::endl;
		//std::cout<<"	rect.y = "<<evt.container->rect.y<<std::endl;
		//std::cout<<"	rect.width = "<<evt.container->rect.width<<std::endl;
		//std::cout<<"	rect.height = "<<evt.container->rect.height<<std::endl;
		//std::cout<<"	window_rect.x = "<<evt.container->window_rect.x<<std::endl;
		//std::cout<<"	window_rect.y = "<<evt.container->window_rect.y<<std::endl;
		//std::cout<<"	window_rect.width = "<<evt.container->window_rect.width<<std::endl;
		//std::cout<<"	window_rect.height = "<<evt.container->window_rect.height<<std::endl;
		//std::cout<<"	deco_rect.x = "<<evt.container->deco_rect.x<<std::endl;
		//std::cout<<"	deco_rect.y = "<<evt.container->deco_rect.y<<std::endl;
		//std::cout<<"	deco_rect.width = "<<evt.container->deco_rect.width<<std::endl;
		//std::cout<<"	deco_rect.height = "<<evt.container->deco_rect.height<<std::endl;
		//std::cout<<"	geometry.x = "<<evt.container->geometry.x<<std::endl;
		//std::cout<<"	geometry.y = "<<evt.container->geometry.y<<std::endl;
		//std::cout<<"	geometry.width = "<<evt.container->geometry.width<<std::endl;
		//std::cout<<"	geometry.height = "<<evt.container->geometry.height<<std::endl;
		std::cout<<"	urgent = "<<evt.container->urgent<<std::endl;
		std::cout<<"	focused = "<<evt.container->focused<<std::endl;
	}
	if((evt.type == i3ipc::WindowEventType::FOCUS || evt.type == i3ipc::WindowEventType::TITLE) && evt.container!=nullptr)
	{
		Job* old_job = current_job;
		win_id_lookup_entry& entry = win_id_lookup[evt.container->id];

		if(!entry.job || std::string::npos == evt.container->name.find(entry.matching_string)) // Window not yet associated with a job
		{																						// or needs re-association
			entry = find_job(evt.container->name);
		}
		else
		{
			std::cout<<"Job found in map."<<std::endl;
		}
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
	std::cout<<"(LL): on_workspace_change() called "<<++evt_count<<"th time."<<std::endl;
	(void)evt;
}
//}}}




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


std::ostream& operator<< (std::ostream& stream, Doodle const& doodle)
{
	stream<<"Doodle class:\n";
	stream<<"	Current job: "<<doodle.current_job->jobname<<std::endl;
	stream<<"	Current workspace: "<<doodle.current_workspace<<std::endl;
	stream<<"	Jobs:"<<std::endl;
	for(const Doodle::Job& job : doodle.jobs)
	{
		stream<<"		"<<job<<std::endl;
	}
	stream<<"	Known windows:\n		win_id		jobname		matching_string"<<std::endl;
	for (auto it : doodle.win_id_lookup)
	{
		std::cout<<"		"<<it.first<<"	"<<it.second.job<<"	"<<it.second.matching_string<<std::endl;
	}
	return stream;
}




//{{{
//void  dump_tree_container(const i3ipc::container_t&  c, std::string&  prefix) {
//	std::cout << prefix << "ID: " << c.id << " (i3's; X11's - " << c.xwindow_id << ")" << std::endl;
//	prefix.push_back('\t');
//	std::cout << prefix << "name = \"" << c.name << "\"" << std::endl;
//	std::cout << prefix << "type = \"" << c.type << "\"" << std::endl;
//	std::cout << prefix << "border = \"" << c.border_raw << "\"" << std::endl;
//	std::cout << prefix << "current_border_width = " << c.current_border_width << std::endl;
//	std::cout << prefix << "layout = \"" << c.layout_raw << "\"" << std::endl;
//	std::cout << prefix << "percent = " << c.percent << std::endl;
//	if (c.urgent) {
//		std::cout << prefix << "urgent" << std::endl;
//	}
//	if (c.focused) {
//		std::cout << prefix << "focused" << std::endl;
//	}
//	prefix.push_back('\t');
//	for (auto&  n : c.nodes) {
//		dump_tree_container(*n, prefix);
//	}
//	prefix.pop_back();
//	prefix.pop_back();
//}


//void Doodle::print_workspaces()
//{
//	for (auto&  w : conn.get_workspaces()) {
//		std::cout << '#' << std::hex << w->num << std::dec
//			<< "\n\tName: " << w->name
//			<< "\n\tVisible: " << w->visible
//			<< "\n\tFocused: " << w->focused
//			<< "\n\tUrgent: " << w->urgent
//			<< "\n\tRect: "
//			<< "\n\t\tX: " << w->rect.x
//			<< "\n\t\tY: " << w->rect.y
//			<< "\n\t\tWidth: " << w->rect.width
//			<< "\n\t\tHeight: " << w->rect.height
//			<< "\n\tOutput: " << w->output
//			<< std::endl;
//	}
//	std::string  prefix_buf;
//	//dump_tree_container(*conn.get_tree(), prefix_buf);
//
//	//for( auto&  w : conn.get_workspaces())
//	//{
//	//	std::cout<<'#'<<std::hex<<w.num<<std::dec
//	//	         <<"\n\tName: "<<w.name
//	//	         <<"\n\tVisible: "<<w.visible
//	//	         <<"\n\tFocused: "<<w.focused
//	//	         <<"\n\tUrgent: "<<w.urgent
//	//	         <<"\n\tRect: "
//	//	         <<"\n\t\tX: "<<w.rect.x
//	//	         <<"\n\t\tY: "<<w.rect.y
//	//	         <<"\n\t\tWidth: "<<w.rect.width
//	//	         <<"\n\t\tHeight: "<<w.rect.height
//	//	         <<"\n\tOutput: "<<w.output
//	//	         <<std::endl;
//	//}
//}
//}}}
