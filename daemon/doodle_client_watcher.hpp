#pragma once
// This class is a nested class of doodle. This file may only be included _in_the_body_of_class_Doodle_!

//struct Client_watcher// : ev::io
//{
//	Doodle* const doodle;
//	Client_watcher** head;
//	Client_watcher* prev;
//	Client_watcher* next;
//
//	ev::io read_watcher;
//	ev::io write_watcher;
//	std::deque<std::string> write_data;
//
//	//{{{ Constructors
//
//	Client_watcher(int main_fd, Client_watcher** head, Doodle* doodle, ev::loop_ref loop);
//	~Client_watcher(void);
//	Client_watcher() = delete;
//	Client_watcher(const Client_watcher&) = delete;
//	Client_watcher(Client_watcher&&) = delete;
//	Client_watcher& operator=(const Client_watcher&) = delete;
//	Client_watcher& operator=(Client_watcher&&) = delete;
//	//}}}
//
//
//	void write_cb(ev::io& watcher, int);
//	void read_cb(ev::io& watcher, int);
//
//
//	//{{{
//	friend Client_watcher& operator<<(Client_watcher& lhs, std::string data)
//	{
//		uint16_t length = data.length();
//		std::string credential(DOODLE_PROTOCOL_VERSION, 0, sizeof(DOODLE_PROTOCOL_VERSION)-1);
//		credential.append(static_cast<char*>(static_cast<void*>(&length)), 2);
//
//		lhs.write_data.push_back(credential);
//		lhs.write_data.push_back(data);
//		if(!lhs.write_watcher.is_active()) lhs.write_watcher.start();
//
//		return lhs;
//	}
//	//}}}
//};
