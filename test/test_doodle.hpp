#include <cxxtest/TestSuite.h>

class doodle_daemon_test : public CxxTest::TestSuite
{
	public:
		void test_pack()
		{
			{
				//TODO: Implement a real check
				TS_ASSERT_EQUALS(1+1, 2)
			}
		}
};
