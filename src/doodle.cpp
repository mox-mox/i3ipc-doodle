// This is just the workplaces example from i3ipc++ that I will use as a starting point.
// Right now, I use it to test, if I can sucessfully compile the project.


#include <iostream>
#include "doodle_config.hpp"

#include <i3ipc++/ipc.hpp>

int  main()
{
	std::cout<<"Git branch: "<<GIT_BRANCH<<", git commit hash: "<<GIT_COMMIT_HASH<<", Version: "<<DOODLE_VERSION_MAJOR<<":"<<DOODLE_VERSION_MINOR<<"."<<std::endl<<std::endl;

	i3ipc::I3Connection conn;

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
	return 0;
}
