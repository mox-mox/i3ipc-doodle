#pragma once
#include <i3ipc++/ipc.hpp>
//#include <json/json.h>

class Doodle : public sigc::trackable
{
	private:
		i3ipc::connection& conn;
		int count;

	public:
		Doodle(i3ipc::connection& conn);

		void on_window_change(const i3ipc::window_event_t& evt);
		void on_workspace_change(const i3ipc::workspace_event_t&  evt);


		void print_workspaces();
};
