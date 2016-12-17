#include <cxxtest/TestSuite.h>
//#include <stdlib.h>
//#include "window_matching.hpp"
//#include "test_config.hpp"
//#define DOODLE_DAEMON_EXE "@DOODLE_DAEMON_EXE@"
//const std::string doodle_daemon_exe("./""@DOODLE_DAEMON_EXE@");


class doodle_daemon_test : public CxxTest::TestSuite
{
	public:
		void test_pack()
		{
			//Window_matching winmatch("");
			//int doodle_daemon_exit_status = system("/bin/bash -c echo ../daemon/doodle_daemon");
			//TS_ASSERT_EQUALS(doodle_daemon_exit_status, 0)

			//TODO: Implement a real check
			TS_ASSERT_EQUALS(1+1, 2)
		}
};
