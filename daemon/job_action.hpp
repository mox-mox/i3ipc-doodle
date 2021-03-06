#pragma once
// This class is a nested class of Job. This file may only be included _in_the_body_of_class_Job!
//#include <unistd.h>

class Action : private Window_matching
{
	const Job* job;
	std::vector<char*> command;
	bool kill_on_focus_loss;
	pid_t pid;


	public:
	Action(const Job* job, Json::Value action);
	Action(Action&& other);
	void operator()(const std::string& current_workspace, const std::string& window_title);
	void stop(void);
};
