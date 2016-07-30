#include "notify_stream.hpp"

// Initialise the actual streams.
// By setting init_priority to the lowest possible level, the notifications can be safely used in constructors.


#ifdef USE_NOTIFICATIONS
	unsigned int Notify_stream::instance_count = 0;


#if defined(USE_NOTIFY_LOW)
	__attribute__ ((init_priority (101))) Notify_stream notify_low_hidden(NOTIFY_URGENCY_LOW);
#endif


#if defined(USE_NOTIFY_LOW) || defined(USE_NOTIFY_NORMAL)
	__attribute__ ((init_priority (101))) Notify_stream notify_normal_hidden(NOTIFY_URGENCY_NORMAL);
#endif


#if defined(USE_NOTIFY_LOW) || defined(USE_NOTIFY_NORMAL) || defined(USE_NOTIFY_CRITICAL)
	__attribute__ ((init_priority (101))) Notify_stream notify_critical_hidden(NOTIFY_URGENCY_CRITICAL);
#endif



#endif

