//#include "timer.hpp"
//#include "logstream.hpp"
//
////template < class Sleeper >
////Timer::Timer(void)
////{
////}
//
//
//template < class Sleeper >
//Timer::~Timer(void)
//{
//	if(waker.joinable()) waker.join();
//}
//
//
//
//template < class Sleeper >
//void Timer::awakener(void)
//{
//	while(!sleepers.empty())
//	{
//		Timer_entry current = sleepers.pop();
//		std::this_thread::sleep_until(current.waketime);
//		std::cout<<"Awakener called for Job: "<<current.sleeper<<std::endl;
//		sleeper.wakeup();
//	}
//	logger<<"No more sleepers, stopping waker thread."<<std::endl;
//}
//
//
//template < class Sleeper >
//void Timer::awakener(Sleeper& sleeper, std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>)
//{
//	sleepers.push({ sleeper, waketime });
//	if(sleepers.size() == 1)
//	{
//		waker = std::thread(awakener);
//		logger<<"No more sleepers, stopping waker thread."<<std::endl;
//	}
//}
