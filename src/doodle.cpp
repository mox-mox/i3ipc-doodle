#include "doodle.hpp"
#include <iostream>




Doodle::Doodle(i3ipc::I3Connection& conn) : conn(conn), count(0)
{
	//conn = new i3ipc::I3Connection();
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

//Doodle::~Doodle()
//{
//	delete conn;
//}



void Doodle::on_window_change(i3ipc::WindowEventType increment)
{
	std::cout<<"on_window_change() called "<<++count<<"th time."<<std::endl;
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
