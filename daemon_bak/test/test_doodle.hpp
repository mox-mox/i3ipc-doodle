#include <cxxtest/TestSuite.h>
//#include <stdlib.h>
#include "window_matching.hpp"
//#include "test_config.hpp"
//#define DOODLE_DAEMON_EXE "@DOODLE_DAEMON_EXE@"
//const std::string doodle_daemon_exe("./""@DOODLE_DAEMON_EXE@");


class doodle_daemon_test : public CxxTest::TestSuite
{
	//{{{
	constexpr static auto window_matching_full_string =
		"{                                                                  \
			\"granularity\" : 10,                                           \
		    \"window_names\" :                                              \
			[                                                               \
				\"~/projects\",                                             \
				\"!special_project\",                                       \
				\"home/mox/projects\",                                      \
				\"Vimperator\"                                              \
			],                                                              \
		    \"workspace_names\" :                                           \
			[                                                               \
				\"1\",                                                      \
				\"3\",                                                      \
				\"!private\"                                                \
			],                                                              \
			\"some_bogus_value\" : \"should not cause any problems\"        \
		}";
	//}}}

	//{{{
	constexpr static auto window_matching_no_ws_string =
		"{                                                                  \
			\"granularity\" : 10,                                           \
		    \"window_names\" :                                              \
			[                                                               \
				\"~/projects\",                                             \
				\"!special_project\",                                       \
				\"home/mox/projects\",                                      \
				\"Vimperator\"                                              \
			],                                                              \
			\"some_bogus_value\" : \"should not cause any problems\"        \
		}";
	//}}}

	//{{{
	constexpr static auto window_matching_no_wn_string =
		"{                                                                  \
			\"granularity\" : 10,                                           \
		    \"workspace_names\" :                                           \
			[                                                               \
				\"1\",                                                      \
				\"3\",                                                      \
				\"!private\"                                                \
			],                                                              \
			\"some_bogus_value\" : \"should not cause any problems\"        \
		}";
	//}}}



	public:
		//{{{
		void test_window_matching_full()
		{
			//{{{ Create a window_matching object

			Json::Value testval;
			Json::Reader reader;
			if( !reader.parse(window_matching_full_string, testval, false))
			{
				error<<"Something is REALLY wrong here: "<<reader.getFormattedErrorMessages()<<std::endl;
			}
			Window_matching winmatch(testval);
			auto& matchers = winmatch.get_matchers();

			std::deque<std::string> win_include{{"~/projects", "home/mox/projects", "Vimperator"}};
			std::deque<std::string> win_exclude{{"special_project"}};
			std::deque<std::string>  ws_include{{{"1"}, {"3"}}};
			std::deque<std::string>  ws_exclude{{"private"}};

			TS_ASSERT_EQUALS(matchers.win_names.include, win_include);
			TS_ASSERT_EQUALS(matchers.win_names.exclude, win_exclude);
			TS_ASSERT_EQUALS(matchers.ws_names.include, ws_include);
			TS_ASSERT_EQUALS(matchers.ws_names.exclude, ws_exclude);
			//}}}

			//{{{ Test the actual matching

			TS_ASSERT( winmatch.match("1", "/home/mox/projects/foobar"));
			TS_ASSERT(!winmatch.match("1", "/home/mox/scratch/foobar"));
			TS_ASSERT(!winmatch.match("private", "/home/mox/projects/foobar"));
			TS_ASSERT(!winmatch.match("2", "/home/mox/projects/foobar"));
			TS_ASSERT(!winmatch.match("2", "/home/mox/scratch/foobar"));
			//}}}
		}
		//}}}

		//{{{
		void test_window_matching_no_ws()
		{
			//{{{ Create a window_matching object

			Json::Value testval;
			Json::Reader reader;
			if( !reader.parse(window_matching_no_ws_string, testval, false))
			{
				error<<"Something is REALLY wrong here: "<<reader.getFormattedErrorMessages()<<std::endl;
			}
			Window_matching winmatch(testval);
			auto& matchers = winmatch.get_matchers();

			std::deque<std::string> win_include{{"~/projects", "home/mox/projects", "Vimperator"}};
			std::deque<std::string> win_exclude{{"special_project"}};
			std::deque<std::string>  ws_include{};
			std::deque<std::string>  ws_exclude{};

			TS_ASSERT_EQUALS(matchers.win_names.include, win_include);
			TS_ASSERT_EQUALS(matchers.win_names.exclude, win_exclude);
			TS_ASSERT_EQUALS(matchers.ws_names.include, ws_include);
			TS_ASSERT_EQUALS(matchers.ws_names.exclude, ws_exclude);
			//}}}


			//{{{ Test the actual matching

			TS_ASSERT( winmatch.match("1", "/home/mox/projects/foobar"));
			TS_ASSERT(!winmatch.match("1", "/home/mox/scratch/foobar"));
			TS_ASSERT( winmatch.match("private", "/home/mox/projects/foobar"));
			//}}}

		}
		//}}}


		//{{{
		void test_window_matching_no_wn()
		{
			//{{{ Create a window_matching object

			Json::Value testval;
			Json::Reader reader;
			if( !reader.parse(window_matching_no_wn_string, testval, false))
			{
				error<<"Something is REALLY wrong here: "<<reader.getFormattedErrorMessages()<<std::endl;
			}
			Window_matching winmatch(testval);
			auto& matchers = winmatch.get_matchers();

			std::deque<std::string> win_include{};
			std::deque<std::string> win_exclude{};
			std::deque<std::string>  ws_include{{{"1"}, {"3"}}};
			std::deque<std::string>  ws_exclude{{"private"}};

			TS_ASSERT_EQUALS(matchers.win_names.include, win_include);
			TS_ASSERT_EQUALS(matchers.win_names.exclude, win_exclude);
			TS_ASSERT_EQUALS(matchers.ws_names.include, ws_include);
			TS_ASSERT_EQUALS(matchers.ws_names.exclude, ws_exclude);
			//}}}

			//{{{ Test the actual matching

			TS_ASSERT( winmatch.match("1", "/home/mox/projects/foobar"));
			TS_ASSERT( winmatch.match("1", "/home/mox/scratch/foobar"));
			TS_ASSERT(!winmatch.match("private", "/home/mox/projects/foobar"));
			//}}}
		}
		//}}}


};
