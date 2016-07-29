#include "doodle_client_watcher.hpp"




	//{{{
Doodle::client_watcher::client_watcher(int main_fd, client_watcher** head, Doodle* doodle, ev::loop_ref loop) : ev::io(loop), write_watcher(loop)
	{
		this->head = head;
		*this->head = this;

		this->data = static_cast<void*>(doodle);

		//this->data = static_cast<void*>(head);
		client_watcher* next_watcher = *head;

		if( -1 == main_fd )   throw std::runtime_error("Passed invalid Unix socket");
		int client_fd = accept(main_fd, NULL, NULL);
		if( -1 == client_fd ) throw std::runtime_error("Received invalid Unix socket");
		ev::io::set(client_fd, ev::READ);
		if(!next_watcher)
		{
			this->prev = this;
			this->next = this;
		}
		else // if next is valid
		{
			this->next = next_watcher;
			this->prev = next_watcher->prev;
			this->next->prev = this;
			this->prev->next = this;
		}

		set<client_watcher, reinterpret_cast<void (client_watcher::*)(ev::io& socket_watcher, int revents)>( &client_watcher::client_watcher_cb) >(nullptr);

		start();

		write_watcher.set < client_watcher, &client_watcher::write_cb > (this);
		write_watcher.set(client_fd, ev::WRITE);
		write_watcher.start();
	}
	//}}}

	//{{{
	Doodle::client_watcher::~client_watcher(void)
	{
		close(fd);
		write_watcher.stop();
		stop();
		if(this != next && this != prev)
		{
			*head = this->next;
			next->prev = this->prev;
			prev->next = this->next;
		}
		else
		{
			*head = nullptr;
		}
	}
	//}}}

	//{{{
	void Doodle::client_watcher::write_cb(ev::io& w, int revent)
	{
		(void) revent;
		std::deque<std::string>& write_data = static_cast<client_watcher*>(w.data)->write_data;
		if(write_data.empty())
		{
			w.stop();
		}
		else while(!write_data.empty())
		{
			int write_count = 0;
			int write_size = write_data.front().length();
			while(write_count < write_size)
			{
				int n;
				switch((n=write(w.fd, &write_data.front()[write_count], write_size-write_count)))
				{
					case -1:
						throw std::runtime_error("Write error on the connection using fd." + std::to_string(w.fd) + ".");
					case  0:
						delete &w;
						return;
					default:
						write_count+=n;
				}
			}
			write_data.pop_front();
		}
	}
	//}}}

//
//	//{{{
//	Doodle::client_watcher& operator<<(Doodle::client_watcher& lhs, const std::string& data)
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
//

	//{{{
	bool Doodle::client_watcher::read_n(int fd, char buffer[], int size, client_watcher& watcher)	// Read exactly size bytes
	{
		int read_count = 0;
		while(read_count < size)
		{
			int n;
			switch((n=read(fd, &buffer[read_count], size-read_count)))
			{
				case -1:
					throw std::runtime_error("Read error on the connection using fd." + std::to_string(fd) + ".");
				case  0:
					delete &watcher;
					return false;
				default:
					read_count+=n;
			}
		}
		return true;
	}
	//}}}

	//{{{
	void Doodle::client_watcher::client_watcher_cb(client_watcher& watcher, int revents)
	{
		(void) revents;

		struct
		{
			char doodleversion[sizeof(DOODLE_PROTOCOL_VERSION)-1];
			uint16_t length;
		}  __attribute__ ((packed)) header;

		if(!read_n(watcher.fd, static_cast<char*>(static_cast<void*>(&header)), sizeof(header), watcher))
		{
			return;
		}
		std::cout<<header.doodleversion<<", length: "<<header.length<<": ";

		std::string buffer(header.length, '\0');
		if(!read_n(watcher.fd, &buffer[0], header.length, watcher))
		{
			return;
		}

		std::cout<<"\""<<buffer<<"\""<<std::endl;

		////////////////////////////////////////
		watcher<<buffer;	// Do something with the received data
		////////////////////////////////////////


		if(buffer=="kill")
		{
			std::cout<<"Shutting down"<<std::endl;
			//todo: delete all stuff
			watcher.loop.break_loop(ev::ALL);
		}
	}
	//}}}