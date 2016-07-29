#include "doodle.hpp"
#include <fstream>
#include <experimental/filesystem>
#include "logstream.hpp"
#include <functional>
#include <json/json.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <map>

//{{{
struct Doodle::client_watcher : ev::io
{
	client_watcher* prev;
	client_watcher* next;
	client_watcher** head;

	ev::io write_watcher;
	std::deque<std::string> write_data;

	client_watcher(int main_fd, client_watcher** head, Doodle* doodle, ev::loop_ref loop);

	~client_watcher(void);

	void write_cb(ev::io& w, int revent);

	friend client_watcher& operator<<(client_watcher& lhs, const std::string& data)
	{
		uint16_t length = data.length();
		std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
		credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);

		lhs.write_data.push_back(credential);
		lhs.write_data.push_back(data);
		if(!lhs.write_watcher.is_active()) lhs.write_watcher.start();

		return lhs;
	}

	bool read_n(int fd, char buffer[], int size, client_watcher& watcher);	// Read exactly size bytes

	void client_watcher_cb(client_watcher& watcher, int revents);
};
//}}}

