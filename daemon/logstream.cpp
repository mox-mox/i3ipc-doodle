#include "logstream.hpp"

// Initialise the streams. \x1b[31m is an ANSI-escape sequence, see
// https://en.wikipedia.org/wiki/ANSI_escape_code
//
// init_priority  allows to set the initialisation order of global objects.
// Using this, it becomes possible to use the logger and error streams in the
// constructors of global objects.


#ifndef USE_NOTIFICATIONS
	__attribute__ ((init_priority (101))) LogStream logger("\x1b[32m(LL) ");
	__attribute__ ((init_priority (101))) LogStream error("\x1b[31m(EE) ");
#else
	__attribute__ ((init_priority (101))) LogStream logger(NOTIFY_URGENCY_NORMAL);
	__attribute__ ((init_priority (101))) LogStream error(NOTIFY_URGENCY_CRITICAL);
#endif
