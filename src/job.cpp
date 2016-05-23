#include "job.hpp"
#include <fstream>
#include  <sstream>
#include "logstream.hpp"
#include <exception>

//{{{
Job::Job(const std::experimental::filesystem::path& jobfile) : jobname(jobfile.filename()), jobfile(jobfile)
{
	logger<<"Constructing job "<<jobname<<"."<<std::endl;
	std::ifstream file(jobfile);
	Json::Value job;
	Json::Reader reader;
	if( !reader.parse(file, job, false))
	{
		error<<reader.getFormattedErrorMessages()<<std::endl;
		throw std::runtime_error("Cannot parse job file");
	}

	sanitise_jobfile(jobfile);




	//std::cout<<"Job "<<jobname<<": Total time as string is: "<<job["total_time"]<<std::endl;
	//if(job.get("total_time", 0).asString().length() != 20)
	//{
	//	error<<"total_time to short! ("<<job.get("total_time", 0).asString().length()<<")"<<std::endl;
	//}

	times.total = std::chrono::seconds(job.get("total_time", 0).asInt64());

	settings.granularity = std::chrono::seconds(job.get("granularity", settings.GRANULARITY_DEFAULT_VALUE).asInt64());





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
}
//}}}

//{{{
static std::streampos find_total_time(std::fstream& jobfile)
{
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
	return 0;
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

	std::streampos pos_start = find_total_time(jobfile_orig);
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
Job::Job(Job && other) noexcept
{
	std::unique_lock < std::mutex > other_lk(other.times_mutex);

	jobname = std::move(other.jobname);
	jobfile = std::move(other.jobfile);

	settings = std::move(other.settings);
	matchers = std::move(other.matchers);

	times = std::move(other.times);
	saver_thread = std::move(other.saver_thread);
}
//}}}

//{{{
//Job::Job(void) : jobname("NOJOB"), times(), matchers({.win_names_include(), .win_names_exclude( {"!"}), .ws_names_include(), .ws_names_exclude({ "!" })}) {times.saver_thread_running = true;}
Job::Job(void) :
	jobname("NOJOB"),
	times{std::chrono::seconds(0), std::chrono::steady_clock::time_point(), std::chrono::seconds(0), false, true, true},
	matchers{{}, {
		"!"
	}, {}, {
		"!"
	}}
{ }
//}}}

//{{{
void Job::start_saver_thread(void)
{
	if( !saver_thread.joinable())
	{
		saver_thread = std::thread(&Job::save_times, this);
	}
	else
	{
		throw "Trying to start already running saver_thread for "+jobname+".";
	}
}
//}}}

//{{{
Job::~Job(void)
{
	if( saver_thread.joinable()) saver_thread.join();
}
//}}}

//{{{
void Job::start(std::chrono::steady_clock::time_point start_time)
{
	//std::cout<<"	"<<jobname<<".start() called."<<std::endl;
	{
		std::lock_guard < std::mutex > lock(times_mutex);
		times.job_start = start_time;
		times.job_currently_running = true;
	}

	if( !times.saver_thread_running )	// Get the saver to start.
	{
		//error<<"Starting the saver for Job "<<jobname<<std::endl;
		times_cv.notify_one();
	}
}
//}}}

//{{{
void Job::stop(std::chrono::steady_clock::time_point stop_time)
{
	std::lock_guard < std::mutex > lock(times_mutex);
	std::chrono::seconds elapsed = std::chrono::duration_cast < std::chrono::seconds > (stop_time-times.job_start);
	times.job_start = std::chrono::steady_clock::time_point();	// reset to 'zero'
	times.total += elapsed;
	times.slot  += elapsed;
	times.job_currently_running = false;
}
//}}}

//{{{
void Job::save_times(void)	// !! ASYNCHRONOUS !!
{
	std::unique_lock < std::mutex > lock(times_mutex);	// Grab the mutex. Whenever the thread is sleeping, it will free the mutex.
	for( ; ; )
	{
		if( times.destructor_called ) { lock.unlock(); return; }

		// Wait until the job is activated the (first) time.
		if( !times.job_currently_running )		// No need to wait, when the job is already running.
		{
			times.saver_thread_running = false;
			// Wait until the job is started or destructor is called.
			times_cv.wait(lock, [this] { return times.job_currently_running || times.destructor_called; });
		}

		if( times.destructor_called ) { lock.unlock(); return; }

		// Remember the date when the tims_slot started.
		std::chrono::system_clock::time_point slot_start = std::chrono::system_clock::now();

		times.saver_thread_running = true;

		// Wait until it is time to write the time slot to disk or the destructor has been called.
		times_cv.wait_until(lock, std::chrono::steady_clock::now()+settings.granularity, [this](){ return times.destructor_called; });


		std::cerr<<"TODO: Write the time of the last hour to file."<<std::endl;


		if( times.job_currently_running )	// Account for a currently running job.
		{
			std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
			std::chrono::seconds current_runtime = std::chrono::duration_cast < std::chrono::seconds > (now-times.job_start);
			times.total += current_runtime;
			times.slot += current_runtime;
			times.job_start = now;
		}

		std::stringstream ss;
		ss<<std::setw(20)<<std::setfill('0')<<times.total.count();



		std::fstream joblog(jobfile);
		if(!joblog.is_open())
		{
			throw std::runtime_error("Could not open jobfile for "+jobname+".");
		}
		else
		{
			(void) find_total_time(joblog);
			joblog<<ss.str();
			joblog.seekg(0, std::ios_base::end);
			joblog<<slot_start.time_since_epoch().count()<<" "<<times.slot.count()<<std::endl;


		}

		joblog.close();
	}
}
//}}}

//{{{
std::ostream& operator<<(std::ostream&stream, Job const&job)
	    {
	    stream<<"Job \""<<job.jobname<<"\": ";
	    stream<<(job.times.total+(job.times.job_currently_running ? (std::chrono::duration_cast < std::chrono::seconds > (std::chrono::steady_clock::now()-job.times.job_start)) : std::chrono::seconds(0))).count()<<" seconds.";
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




//void Doodle::write_config(Json::Value config)
//{
//	config["max_idle_time"] = settings.max_idle_time;
//	config["detect_ambiguity"] = settings.detect_ambiguity;
//
//	Json::StyledWriter writer;
//	// Make a new JSON document for the configuration. Preserve original comments.
//	std::string outputConfig = writer.write( config );
//	std::cout<< outputConfig <<std::endl;
//}


//std::ifstream config_file(config_filename);
//{
//	std::ofstream config_copy(config_filename+"_backup");
//	config_copy<<config_file.rdbuf();
//	config_file.clear();
//	config_file.seekg(0, std::ios::beg);
//}
