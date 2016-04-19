#pragma once
#include <i3ipc++/ipc.hpp>

class Doodle : public sigc::trackable
{
	private:
		i3ipc::I3Connection* conn;

	public:
		Doodle();
		~Doodle();

		void subscribe_to_window_change();
		//void on_window_change(WindowEventType win_evt);
		void on_window_change();


		void print_workspaces();
};
