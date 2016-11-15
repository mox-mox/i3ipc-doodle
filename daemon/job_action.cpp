#include "job.hpp"
#include <sys/types.h>
#include <signal.h>


Job::Action::Action(Json::Value action) :
	Window_matching(action),
	command(action.get("command", "no_command").asString()),
	kill_on_focus_loss(action.get("kill_on_focus_loss", "true").asBool()),
	pid(0)
{}


Job::Action::Action(Action&& other) :
	Window_matching(std::move(other)),
	command(std::move(other.command)),
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
				execlp(command.c_str(), command.c_str(), static_cast<char*>(nullptr));
				error<< "Uh-Oh! execl() failed!"<<std::endl; /* execl doesn't return unless there's an error */
				exit(EXIT_FAILURE);
			default: /* Parent process */
				debug << "Started command "<<command<<" with pid "<<pid<<'.'<<std::endl;
		}
	}
}


void Job::Action::stop(void)
{
	if(pid && kill_on_focus_loss)
	{
		if(kill(pid, SIGINT))
		{
			error<<"Could not kill child program "<<command<<". Error code: "<<strerror(errno)<<'.'<<std::endl;
		}
		else
		{
			pid=0;
		}
	}
}
