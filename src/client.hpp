#pragma once

#include "main.hpp"
#include "repl.hpp"

#include <uvw.hpp>
#include <string>

class Client
{
	//{{{ Event handling

	std::shared_ptr<uvw::Loop> loop;
	std::shared_ptr<uvw::SignalHandle> sigint;
	std::shared_ptr<uvw::PipeHandle> daemon_pipe;
	std::shared_ptr<uvw::TTYHandle> console;
	//}}}

	//{{{ REPL
	Repl repl;
	//}}}

	std::string parse_command(std::string entry);
	std::string parse_response(std::string response);

	public:
		//{{{ Constructor

		explicit Client(INIReader& config_reader);
		Client(const Client&) = delete;
		Client(Client &&) = delete;
		Client& operator = (const Client&) = delete;
		Client& operator = (Client &&) = delete;
		~Client(void);
		//}}}

		int operator()(void);
};


