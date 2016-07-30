#include "console_stream.hpp"

// Initialise the actual streams.
// By setting init_priority to the lowest possible level, the notifications can be safely used in constructors.


#if defined(USE_RED_STREAM)
	__attribute__ ((init_priority (101))) Console_stream red_stream_hidden("\x1b[31m ");
#endif

#if defined(USE_GREEN_STREAM)
	__attribute__ ((init_priority (101))) Console_stream green_stream_hidden("\x1b[32m ");
#endif

#if defined(USE_YELLOW_STREAM)
	__attribute__ ((init_priority (101))) Console_stream yellow_stream_hidden("\x1b[33m ");
#endif

#if defined(USE_BLUE_STREAM)
	__attribute__ ((init_priority (101))) Console_stream blue_stream_hidden("\x1b[34m ");
#endif

#if defined(USE_MAGENTA_STREAM)
	__attribute__ ((init_priority (101))) Console_stream magenta_stream_hidden("\x1b[35m ");
#endif

#if defined(USE_CYAN_STREAM)
	__attribute__ ((init_priority (101))) Console_stream cyan_stream_hidden("\x1b[36m ");
#endif

#if defined(USE_WHITE_STREAM)
	__attribute__ ((init_priority (101))) Console_stream white_stream_hidden("\x1b[37m ");
#endif



#if defined(USE_DEBUG_STREAM)
	__attribute__ ((init_priority (101))) Console_stream debug_hidden("\x1b[33m(DD) ");
#endif

#if defined(USE_LOGGER_STREAM)
	__attribute__ ((init_priority (101))) Console_stream logger_hidden("\x1b[32m(LL) ");
#endif

#if defined(USE_ERROR_STREAM)
	__attribute__ ((init_priority (101))) Console_stream error_hidden("\x1b[31m(EE) ");
#endif


