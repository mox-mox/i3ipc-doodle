#include "doodle.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "sockets.hpp"


//{{{
Doodle::Client_watcher::Client_watcher(int main_fd, Client_watcher** head, Doodle* doodle, ev::loop_ref loop) : doodle(doodle), head(head), read_watcher(loop), write_watcher(loop)
{
	debug<<"Doodle::Client_watcher::Client_watcher(Client_watcher** head at "<<head<<" points to "<<*head<<", Doodle* doodle="<<doodle<<") at "<<this<<std::endl;
	*this->head = this;
	Client_watcher* next_watcher = *head;

	if( -1 == main_fd )   throw std::runtime_error("Passed invalid Unix socket");
	int client_fd = accept(main_fd, NULL, NULL);
	if( -1 == client_fd ) throw std::runtime_error("Received invalid Unix socket");
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


	read_watcher.set<Client_watcher, &Client_watcher::read_cb>(this);
	read_watcher.set(client_fd, ev::READ);
	read_watcher.start();

	write_watcher.set<Client_watcher, &Client_watcher::write_cb>(this);
	write_watcher.set(client_fd, ev::WRITE);
	write_watcher.start();
}
//}}}


//{{{
Doodle::Client_watcher::~Client_watcher(void)
{
	debug<<"Doodle::Client_watcher::~Client_watcher() at "<<this<<".Client_watcher** head at "<<head<<" points to "<<*head<<", Doodle* doodle="<<doodle<<")"<<std::endl;
	close(read_watcher.fd);
	read_watcher.stop();
	write_watcher.stop();
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
void Doodle::Client_watcher::write_cb(ev::io& watcher, int)
{
	if(write_data.empty())
	{
		watcher.stop();
	}
	else while(!write_data.empty())
	{
		write_n(watcher.fd, &write_data.front()[0], write_data.front().length());
		write_data.pop_front();
	}
}
//}}}


//{{{
void Doodle::Client_watcher::read_cb(ev::io& watcher, int)
{
	debug<<"void Doodle::Client_watcher::read_cb(Client_watcher& watcher at "<<&watcher<<") at "<<this<<std::endl;


	std::string buffer = read_socket(watcher.fd);

	if(buffer.empty())
	{
		delete this;
		return;
	}

	debug<<"\""<<buffer<<"\""<<std::endl;

	*this<<doodle->terminal(buffer);
}
//}}}
