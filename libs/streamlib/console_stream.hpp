#pragma once


#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

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

class Console_stream: public std::ostream
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

		LogStreamBuf buffer;
	public:
		// \x1b[37m is an ANSI-escape sequence, see https://en.wikipedia.org/wiki/ANSI_escape_code
		Console_stream(std::string prefix_, std::ostream& str = std::cout, std::string postfix_ = "\x1b[37m") :
			std::ostream(&buffer), buffer(prefix_, str, postfix_)
		{ }
};

extern Console_stream red_stream_hidden;
extern Console_stream green_stream_hidden;
extern Console_stream yellow_stream_hidden;
extern Console_stream blue_stream_hidden;
extern Console_stream magenta_stream_hidden;
extern Console_stream cyan_stream_hidden;
extern Console_stream white_stream_hidden;

extern Console_stream debug_hidden;
extern Console_stream logger_hidden;
extern Console_stream error_hidden;

/**
 * This little stund will only enable the output streams that are actually needed.
 * Source: Randall C. Chang, February 01, 2003
 * Website: http://www.drdobbs.com/more-on-the-cc-comment-macro-for-debug-s/184401612
 */
#if defined(USE_RED_STREAM)
	#define red_stream red_stream_hidden
#else
	#define red_stream if(0)std::cout
#endif

#if defined(USE_GREEN_STREAM)
	#define green_stream green_stream_hidden
#else
	#define green_stream if(0)std::cout
#endif

#if defined(USE_YELLOW_STREAM)
	#define yellow_stream yellow_stream_hidden
#else
	#define yellow_stream if(0)std::cout
#endif

#if defined(USE_BLUE_STREAM)
	#define blue_stream blue_stream_hidden
#else
	#define blue_stream if(0)std::cout
#endif

#if defined(USE_MAGENTA_STREAM)
	#define magenta_stream magenta_stream_hidden
#else
	#define magenta_stream if(0)std::cout
#endif

#if defined(USE_CYAN_STREAM)
	#define cyan_stream cyan_stream_hidden
#else
	#define cyan_stream if(0)std::cout
#endif

#if defined(USE_WHITE_STREAM)
	#define white_stream white_stream_hidden
#else
	#define white_stream if(0)std::cout
#endif



#if defined(USE_DEBUG_STREAM)
	#define debug debug_hidden
#else
	#define debug if(0)std::cout
#endif

#if defined(USE_LOGGER_STREAM)
	#define logger logger_hidden
#else
	#define logger if(0)std::cout
#endif

#if defined(USE_ERROR_STREAM)
	#define error error_hidden
#else
	#define error if(0)std::cout
#endif

