#include "doodle.hpp"
#include <iostream>




Doodle::Doodle(i3ipc::I3Connection& conn) : conn(conn), count(0)
{
	conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	if(!conn.subscribe(i3ipc::ET_WINDOW))
	{
		std::cout<<"could not connect"<<std::endl;
	}
	else
	{
		std::cout<<"successfully subscribed"<<std::endl;
	}
}


void Doodle::on_window_change(const Json::Value& root)
//void Doodle::on_window_change(i3ipc::WindowEventType win_evt)
{
	std::cout<<"(LL): on_window_change() called "<<++count<<"th time."<<std::endl;
			std::string  change = root["change"].asString();
			std::cout<<"Change: "<<change<<std::endl;
			unsigned int current = root["container"]["id"].asUInt();
			std::cout<<"Current: "<<current<<std::endl;
			std::cout<<"ASDF"<<std::endl;
			std::cout<<root<<std::endl;
}






void Doodle::print_workspaces()
{
	for( auto&  w : conn.get_workspaces())
	{
		std::cout<<'#'<<std::hex<<w.num<<std::dec
		         <<"\n\tName: "<<w.name
		         <<"\n\tVisible: "<<w.visible
		         <<"\n\tFocused: "<<w.focused
		         <<"\n\tUrgent: "<<w.urgent
		         <<"\n\tRect: "
		         <<"\n\t\tX: "<<w.rect.x
		         <<"\n\t\tY: "<<w.rect.y
		         <<"\n\t\tWidth: "<<w.rect.width
		         <<"\n\t\tHeight: "<<w.rect.height
		         <<"\n\tOutput: "<<w.output
		         <<std::endl;
	}
}
