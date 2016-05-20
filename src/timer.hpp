//#pragma once
//
//#include <queue>
//#include <ctime>
//#include <thread>
//
//// TODO: Thread safety...
//// Thread safety will also become neccessary for the other classes, when they use the Timer...
//
//template < class Sleeper >
//class Timer
//{
//	struct Timer_entry
//	{
//		Sleeper& sleeper;
//		std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> waketime;
//	};
//
//	struct less { bool operator() (const Timer_entry& lhs, const Timer_entry& rhs) const { return lhs.waketime < rhs.waketime; } };
//
//	std::priority_queue<Timer_entry, std::list<Timer_entry>, less> sleepers;
//
//	void awakener(void);
//	std::thread waker;
//
//	public:
//	//Timer(void);
//	~Timer(void);
//
//
//};
