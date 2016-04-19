#include "doodle.hpp"
#include <iostream>




Doodle::Doodle()
{
	conn = new i3ipc::I3Connection();
}

Doodle::~Doodle()
{
	delete conn;
}


void Doodle::subscribe_to_window_change()
{
	conn->subscribe(i3ipc::ET_WINDOW);
	//mem_functor0<T_return, T_obj> sigc::mem_fun 	( 	T_return(T_obj::*)()  	_A_func	)
	//mem_functor1<T_return, T_obj, T_arg1> sigc::mem_fun 	( 	T_return(T_obj::*)(T_arg1)  	_A_func	)
	//sigc::mem_fun( 	T_return(T_obj::*)()  	_A_func	)
	//sigc::mem_fun( 	T_return(T_obj::*)(T_arg1)  	_A_func	)
	//conn->signal_window_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
	conn->signal_output_event.connect(sigc::mem_fun(*this, &Doodle::on_window_change));
}

//void Doodle::on_window_change(WindowEventType win_evt)
void Doodle::on_window_change()
{
	std::cout<<"on_window_change() called."<<std::endl;


}




void Doodle::print_workspaces()
{
	for( auto&  w : conn->get_workspaces())
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
