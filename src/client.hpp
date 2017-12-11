#pragma once
#include <uvw.hpp>



//{{{ Redirect the read() function to read from an std::string called "stdin_buffer" instead

std::string stdin_buffer;
ssize_t read_stdin_from_string(int fd, void* buf, size_t count)
{
	if( fd != 0 )
	{
		return read(fd, buf, count);
	}
	else
	{
		size_t n =  count < stdin_buffer.length() ? count : stdin_buffer.length();
		std::strncpy(reinterpret_cast<char*>(buf), stdin_buffer.c_str(), n);
		stdin_buffer.erase(0, n);
		return n;
	}
}
//}}}

#define read read_stdin_from_string
// Take care to ever only include the editline functions in one place
// TODO: verify if this also works with a dynamic library
#include <histedit.h>
#undef read



class Client
{
	//{{{ Event handling

	std::shared_ptr<uvw::Loop> loop;
	std::shared_ptr<uvw::SignalHandle> sigint;
	std::shared_ptr<uvw::PipeHandle> daemon_pipe;
	//std::shared_ptr<uvw::IdleHandle> console;
	std::shared_ptr<uvw::TTYHandle> console;
	//}}}




	//{{{ Editline
	int count;
	const char* line;

	EditLine* el;
	HistEvent hist_ev;
	History *myhistory;
	//}}}

	std::string parse_command(std::string entry);
	std::string parse_response(std::string response);



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


