#pragma once



#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include "doodle_config.hpp"

/**
 * \brief A stream class providing formatted output
 *
 * This class wrapps around an output stream and provides automated formatted output with the
 * usual stream operators.
 * It is taken from <a href="http://stackoverflow.com/a/2212940/4360539">this post</a> at
 * Stackoverflow, with the addition of a postfix and pre- and postfix parameters for the
 * constructor.
 * Usage:
 *
 * logger << "logging some stuff" << std::endl;
 *
 * Will give something like: [      1000] (LL) logging some stuff
 * with "(LL) logging some stuff" in green, while
 *
 * error << "Oh noes, something went wrong!" << std::endl;
 *
 * Will give something like : [      8467] (EE) Oh noes, something went wrong!
 * with "(EE) Oh noes, something went wrong!" in red.
 * The number in square brackets will be the current value of the milliseconds counter.
 *
 */

#ifndef USE_NOTIFICATIONS
	class LogStream: public std::ostream
	{
		private:
			/**
			 * A stream buffer that wraps each line within prefix and postfix.
			 */
			class LogStreamBuf: public std::stringbuf
			{
				std::string prefix;
				std::ostream&   output;
				std::string postfix;
				public:
					LogStreamBuf(std::string prefix_, std::ostream&str_, std::string postfix_) : prefix(prefix_), output(str_), postfix(postfix_)
					{}

					// When we sync the stream with the output.
					// 1) Output prefix then the buffer then the postfix
					// 2) Reset the buffer
					// 3) flush the actual output stream we are using.
					virtual int sync()
					{
						output<<"["<<std::setw(10)<<
						    std::chrono::duration_cast < std::chrono::seconds > (std::chrono::high_resolution_clock::now().time_since_epoch()).count()<<
						    "] "<<prefix<<str()<<postfix;
						str("");
						output.flush();
						return 0;
					}
			};

			// My Stream just uses a version of my special buffer
			LogStreamBuf buffer;
		public:
			// \x1b[37m is an ANSI-escape sequence, see https://en.wikipedia.org/wiki/ANSI_escape_code
			LogStream(std::string prefix_, std::ostream& str = std::cout, std::string postfix_ = "\x1b[37m") : std::ostream(&buffer), buffer(prefix_, str, postfix_)
			{ }
	};
#else
#include <libnotify/notify.h>
	class LogStream: public std::ostream
	{
		private:
			/**
			 * A stream buffer that wraps each line within prefix and postfix.
			 */
			class LogStreamBuf: public std::stringbuf
			{
				bool is_first;
				std::string first;
				std::string second;
				NotifyUrgency urgency;

				public:
					LogStreamBuf(NotifyUrgency urgency) : is_first(true), first(), second(), urgency(urgency) {}

					virtual int sync()
					{
						NotifyNotification* notification = notify_notification_new(first.c_str(), second.c_str(), "dialog-information");
						notify_notification_set_timeout(notification, 3000);
						notify_notification_set_urgency(notification, urgency);
						notify_notification_show(notification, NULL);
						g_object_unref(G_OBJECT(notification));
						second.clear();
						is_first = true;
						return 0;
					}

					std::streamsize xsputn (const char_type *__s, std::streamsize __n)
					{
						if(is_first)
						{
							first.assign(__s, __n);
							is_first = false;
						}
						else
						{
							second.append(__s, __n);
						}
						return __n;
					}
			};

			// My Stream just uses a version of my special buffer
			LogStreamBuf buffer;
		public:
			LogStream(NotifyUrgency urgency = NOTIFY_URGENCY_NORMAL) : std::ostream(&buffer), buffer(urgency)
			{
			}
	};
#endif


/**
 * Use this stream for logging purposes
 */
extern LogStream logger;
/**
 * Use this stream for error output.
 */
extern LogStream error;

/**
 * This little stunt provides us with two more outputs:
 * dout is a wrapper for cout, dlog is a wrapper for logger.
 * Both will only output data, on debuggin builds.
 * Source: Randall C. Chang, February 01, 2003
 * Website: http://www.drdobbs.com/more-on-the-cc-comment-macro-for-debug-s/184401612
 */
#ifdef DEBUG
// dbgInC defined as "printf" or other custom debug function
	#define dbg printf
// dbgInCpp defined as "cout" or other custom debug class
	#define dlog logger
#else
// dbgInC defined as null [1]
	#define dbg
// dbgInCpp defined as "if(0) cerr" or "if(1); else cerr"
	#define dlog if( 0 ) std::cerr
#endif

