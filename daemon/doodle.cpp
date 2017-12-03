#include <doodle.hpp>
#include "main.hpp"



//{{{
Doodle::Doodle(void) : i3_conn()
{
	//{{{ i3 event subscriptions

	std::cout<<"1"<<std::endl;
	i3_conn.signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	std::cout<<"2"<<std::endl;
	i3_conn.signal_workspace_event.connect(sigc::mem_fun(*this, &Doodle::on_workspace_change));
	std::cout<<"3"<<std::endl;

	if( !i3_conn.subscribe(i3ipc::ET_WORKSPACE|i3ipc::ET_WINDOW))
	{
		error<<"could not connect"<<std::endl;
		throw "Could not subscribe to the workspace- and window change events.";
	}
	std::cout<<"4"<<std::endl;
	//}}}
}
//}}}

//{{{
void Doodle::on_window_change(const i3ipc::window_event_t& evt)
{
	notify_normal<<sett(5000)<<"New current window: "<< evt.container->name <<std::endl;
}
//}}}

//{{{
void Doodle::on_workspace_change(const i3ipc::workspace_event_t& evt)
{
	if( evt.type == i3ipc::WorkspaceEventType::FOCUS )
	{
		notify_normal<<sett(5000)<<"New current_workspace: "<<evt.current->name<<std::endl;
	}
}
//}}}

//{{{
int Doodle::operator()(void)
{
	int retval = 0;
	std::cout<<"5"<<std::endl;

	i3_conn.connect_event_socket();
	std::cout<<"6"<<std::endl;

	logger<<"---------------Starting the event loop---------------"<<std::endl;

	while( true )
	{
		std::cout<<"7"<<std::endl;
		i3_conn.handle_event();
		std::cout<<"8"<<std::endl;
	}

	logger<<"Returning from event loop"<<std::endl;

	return retval;
}
//}}}

