#pragma once
#include <i3ipc++/ipc.hpp>

class Doodle : public sigc::trackable
{
	private:
		i3ipc::I3Connection& conn;
		int count;

	public:
		Doodle(i3ipc::I3Connection& conn);
		//~Doodle();

		void on_window_change(i3ipc::WindowEventType win_evt);


		void print_workspaces();
};
