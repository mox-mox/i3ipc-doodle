#pragma once


//#include <i3ipc++/ipc.hpp>
#include <uvw.hpp>

class Client
{
	std::shared_ptr<uvw::Loop> loop;

	std::shared_ptr<uvw::SignalHandle> sigint;
	std::shared_ptr<uvw::PipeHandle> daemon_pipe;
	std::shared_ptr<uvw::TTYHandle> console;

	public:
		//{{{ Constructor

		explicit Client(void);	// Todo: use xdg_config_path
		Client(const Client&) = delete;
		Client(Client &&) = delete;
		Client& operator = (const Client&) = delete;
		Client& operator = (Client &&) = delete;
		//~Client(void);
		//}}}

		int operator()(void);
};


