#include "job.hpp"
#include <fstream>
#include "logstream.hpp"
#include <json/json.h>


static std::streampos find_total_time(std::experimental::filesystem::path jobfile_name);

//{{{
Job::Job(const std::experimental::filesystem::path& jobfile, ev::loop_ref& loop) : jobname(jobfile.filename()), jobfile(jobfile), total_time_position(find_total_time(jobfile)), write_time_timer(loop)
{
	//logger<<"Constructing job "<<jobname<<"."<<std::endl;
	std::ifstream file(jobfile);
	Json::Value job;
	Json::Reader reader;
	if( !reader.parse(file, job, false))
	{
		error<<reader.getFormattedErrorMessages()<<std::endl;
		throw std::runtime_error("Cannot parse job file");
	}

	if(total_time_position == std::streampos(std::streamoff(-1)))
	{
		throw std::runtime_error("Jobfile does not contain a field called \"total_time\".");
	}

	sanitise_jobfile(jobfile);

	times.total = std::chrono::seconds(job.get("total_time", 0).asInt64());

	settings.granularity = job.get("granularity", settings.GRANULARITY_DEFAULT_VALUE).asInt64();

	//{{{
	if( job.isMember("window_names"))
	{
		for( auto&window_name : job.get("window_names", "no window_names"))
		{
			std::string win_name = window_name.asString();
			if( win_name == "no window_names" )
			{
				error<<"Job "<<jobname<<": Invalid window name."<<std::endl;
			}
			else
			{
				if( '!' == win_name[0] )	// Window name segments prepended with '!' mean that the job may not
				{							// have windows whose title matches the given name segment.
					win_name.erase(0, 1);		// Remove the leading '!'
					matchers.win_names_exclude.push_back(win_name);
				}
				else
				{
					matchers.win_names_include.push_back(win_name);
				}
			}
		}
	}
	else
	{
		error<<"Job "<<jobname<<": No window name segments specified."<<std::endl;
	}
	//}}}

	//{{{
	if( job.isMember("workspace_names"))
	{
		for( auto&workspace_name : job.get("workspace_names", "no workspace_names"))
		{
			std::string ws_name = workspace_name.asString();
			if( ws_name == "no workspace_names" )
			{
				error<<"Job "<<jobname<<": Invalid workspace name."<<std::endl;
			}
			else
			{
				if( '!' == ws_name[0] )		// Workspace name segments prepended with '!' mean that the job may not
				{							// have windows on workspaces matching the given name segment.
					ws_name.erase(0, 1);	// Remove the leading '!'
					matchers.ws_names_exclude.push_back(ws_name);
				}
				else
				{
					matchers.ws_names_include.push_back(ws_name);
				}
			}
		}
	}
	else
	{
		error<<"Job "<<jobname<<": No workspace name segments specified."<<std::endl;
	}
	//}}}

	write_time_timer.set < Job, &Job::write_time_cb > (this);
}
//}}}

//{{{
static std::streampos find_total_time(std::experimental::filesystem::path jobfile_name)
{	// TODO: This can probaly written in a much nicer way.
	std::ifstream jobfile(jobfile_name);
	if( !jobfile.is_open())
	{
		std::string temp = jobfile_name.c_str();
		throw std::runtime_error("Could not open jobfile "+temp+".");
	}


	std::streampos pos_start(0);

	jobfile.clear();
	jobfile.seekg(0, std::ios_base::beg);
	std::string line;
	std::regex total_time_regex("\"total_time\"");
	while( std::getline(jobfile, line))
	{
		std::smatch match;
		if( std::regex_search(line, match, total_time_regex))
		{
			jobfile.seekg(pos_start+match.position(0)+std::streampos(12));

			std::regex colon_regex(":");
			do
			{
				pos_start = jobfile.tellg();
				if( !std::getline(jobfile, line))
				{
					throw std::runtime_error("Unexpected EOF");
				}
			} while( !std::regex_search(line, match, colon_regex));

			jobfile.seekg(pos_start+match.position(0));

			std::regex chipher_regex("[0-9]");
			do
			{
				pos_start = jobfile.tellg();
				if( !std::getline(jobfile, line))
				{
					throw std::runtime_error("Unexpected EOF");
				}
			} while( !std::regex_search(line, match, chipher_regex));

			jobfile.seekg(pos_start+match.position(0));

			return jobfile.tellg();
		}
		pos_start = jobfile.tellg();
	}
	return std::streamoff(-1);	// Invalid value, total_time field was not found in the config file
}
//}}}

//{{{
void Job::sanitise_jobfile(const std::experimental::filesystem::path& jobfile)
{
	// The "total_time" field has to be 20 characters wide.
	// This is the maximum length of the string representation of a 64 bit number and the total time will be overwritten with that length when saving the time.

	std::fstream jobfile_orig(jobfile, std::ifstream::in|std::ifstream::out|std::ifstream::binary);
	if( !jobfile_orig.is_open())
	{
		throw std::runtime_error("Could not open tempory jobfile for "+jobname+".");
	}

	//std::streampos pos_start = find_total_time(jobfile_orig);
	std::streampos pos_start = this->total_time_position;
	jobfile_orig.seekg(pos_start);
	std::streampos pos_end(0);
	unsigned long long total_time;
	std::string line;

	if( !std::getline(jobfile_orig, line))
	{
		throw std::runtime_error("Unexpected EOF");
	}
	jobfile_orig.seekg(pos_start);
	total_time = std::stoull(line);

	std::smatch match;
	do
	{
		pos_end = jobfile_orig.tellg();
		line = std::string(1, jobfile_orig.get());
	} while( !jobfile_orig.eof() && std::regex_search(line, match, std::regex("[0-9]")));


	jobfile_orig.seekg(pos_end+match.position(0));
	pos_end = jobfile_orig.tellg();

	jobfile_orig.clear();
	jobfile_orig.seekg(0, std::ios::beg);


	if( 21 != (pos_end-pos_start))
	{
		std::cout<<"LENGTH too short! ("<<(pos_end-pos_start)<<")"<<std::endl;

		//{{{ Copy the jobfile verbatim
		{
			std::ofstream jobfile_copy((jobfile.string()+"_backup"));
			if( !jobfile_copy.is_open())
			{
				throw std::runtime_error("Could not open tempory jobfile for "+jobname+".");
			}
			jobfile_copy<<jobfile_orig.rdbuf();
			jobfile_orig.clear();
			jobfile_orig.seekg(0, std::ios::beg);
		}
		//}}}


		std::ifstream jobfile_copy(jobfile.string()+"_backup", std::ifstream::in|std::ifstream::binary);
		if( !jobfile_copy.is_open())
		{
			throw std::runtime_error("Could not open tempory jobfile for "+jobname+".");
		}

		jobfile_copy.clear();
		jobfile_copy.seekg(0, std::ios_base::beg);

		std::streampos one(1);
		for( std::streampos index(0); index < pos_start; index += one )
		{
			jobfile_orig<<static_cast < char > (jobfile_copy.get());
		}

		std::stringstream ss;
		ss<<std::setw(20)<<std::setfill('0')<<total_time;
		jobfile_orig<<ss.str()<<",";
		jobfile_copy.seekg(pos_end, std::ios_base::beg);


		char c;
		while( jobfile_copy.get(c))
		{
			jobfile_orig<<c;
		}
	}
}
//}}}

//{{{
Job::Job(Job && other) noexcept : jobname(other.jobname), jobfile(other.jobfile), total_time_position(std::move(other.total_time_position)), settings(std::move(other.settings)), write_time_timer(other.write_time_timer.loop)
{
	matchers = std::move(other.matchers);
	times = std::move(other.times);
	// Note that the timer is constructed with the original event loop in the intialiser list.
	// There is no real copy (or move) constructor for ev::timer, so this hack is used to re-initialise the timer
	write_time_timer.set < Job, &Job::write_time_cb > (this);

}
//}}}

//{{{
Job::Job(void) :
	jobname("NOJOB"),
	jobfile(),
	total_time_position(),
	times{std::chrono::seconds(0), std::chrono::steady_clock::time_point(), false,  std::chrono::seconds(0), std::chrono::system_clock::time_point(), true},
	matchers{{}, { "!" }, {}, { "!" }}
{ }
//}}}

//{{{
void Job::start(std::chrono::steady_clock::time_point start_time)
{
	times.job_start = start_time;
	times.running = true;

	if(!times.timer_active)
	{
			times.slot_start = std::chrono::system_clock::now();
			times.slot = std::chrono::seconds(0);
			times.timer_active = true;
			write_time_timer.start(settings.granularity);
	}
}
//}}}

//{{{
void Job::stop(std::chrono::steady_clock::time_point now)
{
	std::chrono::seconds elapsed = std::chrono::duration_cast < std::chrono::seconds > (now-times.job_start);
	times.total     += elapsed;
	times.slot      += elapsed;
	times.job_start  = std::chrono::steady_clock::time_point();	// reset to 'zero'
	times.running = false;
}
//}}}

//{{{
void Job::write_time_cb(void)
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::cout<<"Writing time for "<<jobname<<"to disk."<<std::endl;

	if( times.running )							// Account for a currently running job.
	{
		std::chrono::seconds elapsed = std::chrono::duration_cast < std::chrono::seconds > (now-times.job_start);
		times.total     += elapsed;
		times.slot      += elapsed;
		times.job_start  = now;
	}

	std::fstream joblog(jobfile);
	if(joblog.is_open())
	{
		joblog.seekg(total_time_position);		// Go to where total_time is stored
		joblog<<std::setw(20)<<std::setfill('0')<<times.total.count();

		joblog.seekg(0, std::ios_base::end);	// Go to the end of the file
		joblog<<times.slot_start.time_since_epoch().count()<<" "<<times.slot.count()<<std::endl;
		joblog.close();
		times.slot = std::chrono::seconds(0);
	}
	else
	{
		throw std::runtime_error("Could not open jobfile for "+jobname+".");
	}

	if( times.running ) // If the job is currently not running
	{
		write_time_timer.start(settings.granularity);
	}
	else
	{
		times.timer_active = false;
	}
}
//}}}

//{{{
std::ostream& operator<<(std::ostream&stream, Job const&job)
{
	stream<<"Job \""<<job.jobname<<"\": ";
	stream<<(job.times.total+(job.times.running ? (std::chrono::duration_cast < std::chrono::seconds > (std::chrono::steady_clock::now()-job.times.job_start)) : std::chrono::seconds(0))).count()<<" seconds.";
	stream<<" Names:";
	for( const std::string& n : job.matchers.win_names_include )
	{
		stream<<" |"<<n<<"|";
	}
	for( const std::string& n : job.matchers.win_names_exclude )
	{
		stream<<" |!"<<n<<"|";
	}
	stream<<" workspaces:";
	for( const std::string& w : job.matchers.ws_names_include )
	{
		stream<<" |"<<w<<"|";
	}
	for( const std::string& w : job.matchers.ws_names_exclude )
	{
		stream<<" |!"<<w<<"|";
	}
	stream<<std::endl;
	return stream;
}
//}}}

//{{{
std::string Job::get_jobname(void)
{
	return jobname;
}
//}}}
