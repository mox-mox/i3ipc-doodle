#pragma once
#include <i3ipc++/ipc.hpp>
#include <json/json.h>

class Doodle : public sigc::trackable
{
	private:
		i3ipc::I3Connection& conn;
		int count;

	public:
		Doodle(i3ipc::I3Connection& conn);

		//void on_window_change(i3ipc::WindowEventType win_evt);
		void on_window_change(const Json::Value&);


		void print_workspaces();
};
