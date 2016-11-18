#include "job.hpp"
#include <sys/types.h>
#include <signal.h>


Job::Action::Action(Json::Value action) :
	Window_matching(action),
	kill_on_focus_loss(action.get("kill_on_focus_loss", "true").asBool()),
	pid(0)
{
	std::string cmd(action.get("command", "no_command").asString());
	std::cout<<"cmd: |"<<cmd<<"|"<<std::endl;
	std::string tmp;

	bool quoted=false;
	bool escaped=false;
	int i=0;
	for(char c : cmd)
	{
		std::cout<<"parsing(escaped: "<<escaped<<", quoted: "<<quoted<<", c: "<<c<<")"<<std::endl;
		switch(c)
		{
			case ' ' : if(!escaped){ if(quoted){tmp.append(1, c);} else { args.push_back(strdup(tmp.c_str())); tmp.clear();  }} else{ tmp.append(1, c); escaped=false; } break;
			case '\\': if(!escaped){ escaped=true; }                     else{ tmp.append(1, c); escaped=false; } break;
			case '\"': if(!escaped){ quoted=!quoted; }                      else{ tmp.append(1, c); escaped=false; } break;
			default: tmp.append(1,c);
		}
		i++;
	}
	args.push_back(strdup(tmp.c_str()));
	args.push_back(nullptr);

	for(char* arg : args)
	{
		std::cout<<"Arg: "<<arg<<std::endl;
	}
	std::cout<<std::endl<<std::endl;
}


Job::Action::Action(Action&& other) :
	Window_matching(std::move(other)),
	args(std::move(other.args)),
	kill_on_focus_loss(other.kill_on_focus_loss),
	pid(other.pid)
{}

void Job::Action::operator()(const std::string& current_workspace, const std::string& window_title)
{
	if(match(current_workspace, window_title))
	{
		switch((pid = fork()))
		{
			case -1: /* Error */
				error << "Uh-Oh! fork() failed.\n";
				exit(EXIT_FAILURE);
			case 0: /* Child process */
				execvp(args[0], args.data());
				error<< "Uh-Oh! execl() failed: "<<strerror(errno)<<std::endl; /* execl doesn't return unless there's an error */
				exit(EXIT_FAILURE);
			default: /* Parent process */
				debug << "Started command "<<args[0]<<" with pid "<<pid<<'.'<<std::endl;
		}
	}
}


void Job::Action::stop(void)
{
	if(pid && kill_on_focus_loss)
	{
		if(kill(pid, SIGINT))
		{
			error<<"Could not kill child program "<<args[0]<<". Error code: "<<strerror(errno)<<'.'<<std::endl;
		}
		else
		{
			pid=0;
		}
	}
}
