#include "doodle.hpp"
#include <iostream>




Doodle::Doodle(i3ipc::connection& conn) : conn(conn), count(0)
{
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


void Doodle::on_window_change(const i3ipc::window_event_t& evt)
{
	std::cout<<"(LL): on_window_change() called "<<++count<<"th time."<<std::endl;
	std::cout<<"	type: "<<static_cast<char>(evt.type)<<std::endl;
	if(evt.container!=nullptr)
	{
		std::cout<<"	id = "<<evt.container->id<<std::endl;
		std::cout<<"	xwindow_id = "<<evt.container->xwindow_id<<std::endl;
		std::cout<<"	name = "<<evt.container->name<<std::endl;
		std::cout<<"	type = "<<evt.container->type<<std::endl;
		std::cout<<"	border = "<<static_cast<char>(evt.container->border)<<std::endl;
		std::cout<<"	border_raw = "<<evt.container->border_raw<<std::endl;
		std::cout<<"	current_border_width = "<<evt.container->current_border_width<<std::endl;
		std::cout<<"	layout = "<<static_cast<char>(evt.container->layout)<<std::endl;
		std::cout<<"	layout_raw = "<<evt.container->layout_raw<<std::endl;
		std::cout<<"	percent = "<<evt.container->percent<<std::endl;
		std::cout<<"	rect.x = "<<evt.container->rect.x<<std::endl;
		std::cout<<"	rect.y = "<<evt.container->rect.y<<std::endl;
		std::cout<<"	rect.width = "<<evt.container->rect.width<<std::endl;
		std::cout<<"	rect.height = "<<evt.container->rect.height<<std::endl;
		std::cout<<"	window_rect.x = "<<evt.container->window_rect.x<<std::endl;
		std::cout<<"	window_rect.y = "<<evt.container->window_rect.y<<std::endl;
		std::cout<<"	window_rect.width = "<<evt.container->window_rect.width<<std::endl;
		std::cout<<"	window_rect.height = "<<evt.container->window_rect.height<<std::endl;
		std::cout<<"	deco_rect.x = "<<evt.container->deco_rect.x<<std::endl;
		std::cout<<"	deco_rect.y = "<<evt.container->deco_rect.y<<std::endl;
		std::cout<<"	deco_rect.width = "<<evt.container->deco_rect.width<<std::endl;
		std::cout<<"	deco_rect.height = "<<evt.container->deco_rect.height<<std::endl;
		std::cout<<"	geometry.x = "<<evt.container->geometry.x<<std::endl;
		std::cout<<"	geometry.y = "<<evt.container->geometry.y<<std::endl;
		std::cout<<"	geometry.width = "<<evt.container->geometry.width<<std::endl;
		std::cout<<"	geometry.height = "<<evt.container->geometry.height<<std::endl;
		std::cout<<"	urgent = "<<evt.container->urgent<<std::endl;
		std::cout<<"	focused = "<<evt.container->focused<<std::endl;

	}

}

void Doodle::on_workspace_change(const i3ipc::workspace_event_t&  evt)
{
	std::cout<<"(LL): on_workspace_change() called "<<++count<<"th time."<<std::endl;
}



void  dump_tree_container(const i3ipc::container_t&  c, std::string&  prefix) {
	std::cout << prefix << "ID: " << c.id << " (i3's; X11's - " << c.xwindow_id << ")" << std::endl;
	prefix.push_back('\t');
	std::cout << prefix << "name = \"" << c.name << "\"" << std::endl;
	std::cout << prefix << "type = \"" << c.type << "\"" << std::endl;
	std::cout << prefix << "border = \"" << c.border_raw << "\"" << std::endl;
	std::cout << prefix << "current_border_width = " << c.current_border_width << std::endl;
	std::cout << prefix << "layout = \"" << c.layout_raw << "\"" << std::endl;
	std::cout << prefix << "percent = " << c.percent << std::endl;
	if (c.urgent) {
		std::cout << prefix << "urgent" << std::endl;
	}
	if (c.focused) {
		std::cout << prefix << "focused" << std::endl;
	}
	prefix.push_back('\t');
	for (auto&  n : c.nodes) {
		dump_tree_container(*n, prefix);
	}
	prefix.pop_back();
	prefix.pop_back();
}


void Doodle::print_workspaces()
{
	for (auto&  w : conn.get_workspaces()) {
		std::cout << '#' << std::hex << w->num << std::dec
			<< "\n\tName: " << w->name
			<< "\n\tVisible: " << w->visible
			<< "\n\tFocused: " << w->focused
			<< "\n\tUrgent: " << w->urgent
			<< "\n\tRect: "
			<< "\n\t\tX: " << w->rect.x
			<< "\n\t\tY: " << w->rect.y
			<< "\n\t\tWidth: " << w->rect.width
			<< "\n\t\tHeight: " << w->rect.height
			<< "\n\tOutput: " << w->output
			<< std::endl;
	}
	std::string  prefix_buf;
	dump_tree_container(*conn.get_tree(), prefix_buf);
	//for( auto&  w : conn.get_workspaces())
	//{
	//	std::cout<<'#'<<std::hex<<w.num<<std::dec
	//	         <<"\n\tName: "<<w.name
	//	         <<"\n\tVisible: "<<w.visible
	//	         <<"\n\tFocused: "<<w.focused
	//	         <<"\n\tUrgent: "<<w.urgent
	//	         <<"\n\tRect: "
	//	         <<"\n\t\tX: "<<w.rect.x
	//	         <<"\n\t\tY: "<<w.rect.y
	//	         <<"\n\t\tWidth: "<<w.rect.width
	//	         <<"\n\t\tHeight: "<<w.rect.height
	//	         <<"\n\tOutput: "<<w.output
	//	         <<std::endl;
	//}
}
