//#include "job.hpp"
//#include <sys/types.h>
//#include <signal.h>
//
//
////{{{
//Job::Action::Action(const Job* job, Json::Value action) :
//	Window_matching(action),
//	job(job),
//	kill_on_focus_loss(action.get("kill_on_focus_loss", "true").asBool()),
//	pid(0)
//{
//	std::string cmd(action.get ("command", "no_command").asString());
//	debug<<"cmd: |"<<cmd<<"|"<<std::endl;
//	if(cmd == "no_command")
//	{
//		throw std::logic_error("Job \""+job->get_jobname()+"\": Action without command was specified.");
//	}
//
//	//{{{ Split the command string in tokens for execvp
//
//	std::string tmp;
//	bool quoted = false;
//	bool escaped = false;
//	int i = 0;
//	for( char c : cmd )
//	{
//		//std::cout<<"parsing(escaped: "<<escaped<<", quoted: "<<quoted<<", c: "<<c<<")"<<std::endl;
//		switch( c )
//		{
//			case ' ':
//				if( !escaped )
//				{
//					if( quoted )
//					{
//						tmp.append(1, c);
//					}
//					else
//					{
//						command.push_back(strdup(tmp.c_str()));
//						tmp.clear();
//					}
//				}
//				else
//				{
//					tmp.append(1, c);
//					escaped = false;
//				}
//				break;
//			case '\\':
//				if(
//					!escaped )
//				{
//					escaped = true;
//				}
//				else
//				{
//					tmp.append(1, c);
//					escaped = false;
//				} break;
//			case '\"':
//				if( !escaped )
//				{
//					quoted = !quoted;
//				}
//				else
//				{
//					tmp.append(1, c);
//					escaped = false;
//				} break;
//			default:
//				tmp.append(1, c);
//		}
//		i++;
//	}
//	command.push_back(strdup(tmp.c_str()));
//	//}}}
//	command.push_back(nullptr);
//}
////}}}
//
//
////{{{
//Job::Action::Action(Action && other) :
//	Window_matching(std::move(other)),
//	job(std::move(other.job)),
//	command(std::move(other.command)),
//	kill_on_focus_loss(other.kill_on_focus_loss),
//	pid(other.pid)
//{}
////}}}
//
//
////{{{
//void Job::Action::operator()(const std::string&current_workspace, const std::string&window_title)
//{
//	if( match(current_workspace, window_title))
//	{
//		switch((pid = fork()))
//		{
//			case -1:/* Error */
//				error<<"Job \""<<job->get_jobname()<<"\"::Action: fork() failed. Error:"<<strerror(errno)<<std::endl;
//				exit(EXIT_FAILURE);
//			case 0:	/* Child process */
//				execvp(command[0], command.data());
//				error<<"Job \""<<job->get_jobname()<<"\"::Action: execl() failed. Error: "<<strerror(errno)<<std::endl;	/* execl doesn't return unless there's an error */
//				exit(EXIT_FAILURE);
//			default:/* Parent process */
//				debug<<"Job::Action: Started command "<<command[0]<<" with pid "<<pid<<'.'<<std::endl;
//		}
//	}
//}
////}}}
//
//
////{{{
//void Job::Action::stop(void)
//{
//	if( pid && kill_on_focus_loss )
//	{
//		if( kill(pid, SIGINT))
//		{
//			if(errno != ESRCH) // ESRCH = Job does not exist, which is normal for actions that have terminated
//			error<<"Job \""<<job->get_jobname()<<"\"::Action: Could not kill child program "<<command[0]<<". Error code: "<<strerror(errno)<<'.'<<std::endl;
//			else
//			debug<<"Job \""<<job->get_jobname()<<"\"::Action: Could not kill child program "<<command[0]<<". Error code: "<<strerror(errno)<<'.'<<std::endl;
//		}
//		else
//		{
//			pid = 0;
//		}
//	}
//}
////}}}
//
//
//
