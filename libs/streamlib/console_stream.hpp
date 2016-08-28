#ifndef CONSOLE_STREAM_HPP
#define CONSOLE_STREAM_HPP
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

template< class CharT, class Traits = std::char_traits<CharT> >
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



		//{{{

		//basic_ostream& operator<<( short value )
		//{
		//	std::cout<<" basic_ostream& operator<<( short value ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( unsigned short value )
		//{
		//	std::cout<<" basic_ostream& operator<<( unsigned short value ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( int value )
		//{
		//	std::cout<<" basic_ostream& operator<<( int value ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( unsigned int value )
		//{
		//	std::cout<<" basic_ostream& operator<<( unsigned int value ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( long value )
		//{
		//	std::cout<<" basic_ostream& operator<<( long value ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( unsigned long value )
		//{
		//	std::cout<<" basic_ostream& operator<<( unsigned long value ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( long long value )
		//{
		//	std::cout<<" basic_ostream& operator<<( long long value ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( unsigned long long value )
		//{
		//	std::cout<<" basic_ostream& operator<<( unsigned long long value ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( float value )
		//{
		//	std::cout<<" basic_ostream& operator<<( float value ) "<<std::endl;
		//	return *this;
		//}

		//basic_ostream& operator<<( double value )
		//{
		//	std::cout<<" basic_ostream& operator<<( double value ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( long double value )
		//{
		//	std::cout<<" basic_ostream& operator<<( long double value ) "<<std::endl;
		//	return *this;
		//}
		////basic_ostream& operator<<( bool value )
		////{
		////	std::cout<<" basic_ostream& operator<<( bool value ) "<<std::endl;
		////	return *this;
		////}
		////basic_ostream& operator<<( const void* value )
		////{
		////	std::cout<<" basic_ostream& operator<<( const void* value ) "<<std::endl;
		////	return *this;
		////}
		//basic_ostream& operator<<( std::basic_streambuf<CharT, Traits>* sb)
		//{
		//	std::cout<<" basic_ostream& operator<<( std::basic_streambuf<CharT, Traits>* sb) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( std::ios_base& (*func)(std::ios_base&) )
		//{
		//	std::cout<<" basic_ostream& operator<<( std::ios_base& (*func)(std::ios_base&) ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( std::basic_ios<CharT,Traits>& (*func)(std::basic_ios<CharT,Traits>&) )
		//{
		//	std::cout<<" basic_ostream& operator<<( std::basic_ios<CharT,Traits>& (*func)(std::basic_ios<CharT,Traits>&) ) "<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( std::basic_ostream<CharT,Traits>& (*func)(std::basic_ostream<CharT,Traits>&) )
		//{
		//	std::cout<<" basic_ostream& operator<<( std::basic_ostream<CharT,Traits>& (*func)(std::basic_ostream<CharT,Traits>&) ) "<<std::endl;
		//	return *this;
		//}

		basic_ostream& flush()
		{
			std::cout<<"Console_stream: flush() called"<<std::endl;
			return *this;
		}
		//int sync()
		//{
		//	std::cout<<"Console_stream: sync() called"<<std::endl;
		//	return 0;
		//}


		//}}}






//
////{{{
//
//  template<typename _CharT, typename _Traits>
//    basic_ostream<_CharT, _Traits>&
//    operator<<(basic_ostream<_CharT, _Traits>& __os, const error_code& __e)
//    { return (__os << __e.category().name() << ':' << __e.value()); }
//
//
//      __ostream_type&
//      operator<<(__ostream_type& (*__pf)(__ostream_type&))
//      {
//
//
//
// return __pf(*this);
//      }
//
//      __ostream_type&
//      operator<<(__ios_type& (*__pf)(__ios_type&))
//      {
//
//
//
// __pf(*this);
// return *this;
//      }
//
//      __ostream_type&
//      operator<<(ios_base& (*__pf) (ios_base&))
//      {
//
//
//
// __pf(*this);
// return *this;
//      }
//
//      __ostream_type&
//      operator<<(long __n)
//      { return _M_insert(__n); }
//
//      __ostream_type&
//      operator<<(unsigned long __n)
//      { return _M_insert(__n); }
//
//      __ostream_type&
//      operator<<(bool __n)
//      { return _M_insert(__n); }
//
//      __ostream_type&
//      operator<<(short __n);
//
//      __ostream_type&
//      operator<<(unsigned short __n)
//      {
//
//
// return _M_insert(static_cast<unsigned long>(__n));
//      }
//
//      __ostream_type&
//      operator<<(int __n);
//
//      __ostream_type&
//      operator<<(unsigned int __n)
//      {
//
//
// return _M_insert(static_cast<unsigned long>(__n));
//      }
//
//
//      __ostream_type&
//      operator<<(long long __n)
//      { return _M_insert(__n); }
//
//      __ostream_type&
//      operator<<(unsigned long long __n)
//      { return _M_insert(__n); }
//# 219 "/usr/include/c++/6.1.1/ostream" 3
//      __ostream_type&
//      operator<<(double __f)
//      { return _M_insert(__f); }
//
//      __ostream_type&
//      operator<<(float __f)
//      {
//
//
// return _M_insert(static_cast<double>(__f));
//      }
//
//
//  template<typename _CharT, typename _Traits>
//    inline basic_ostream<_CharT, _Traits>&
//    operator<<(basic_ostream<_CharT, _Traits>& __out, _CharT __c)
//    { return __ostream_insert(__out, &__c, 1); }
//
//  template<typename _CharT, typename _Traits>
//    inline basic_ostream<_CharT, _Traits>&
//    operator<<(basic_ostream<_CharT, _Traits>& __out, char __c)
//    { return (__out << __out.widen(__c)); }
//
//
//  template <class _Traits>
//    inline basic_ostream<char, _Traits>&
//    operator<<(basic_ostream<char, _Traits>& __out, char __c)
//    { return __ostream_insert(__out, &__c, 1); }
//
//
//  template<class _Traits>
//    inline basic_ostream<char, _Traits>&
//    operator<<(basic_ostream<char, _Traits>& __out, signed char __c)
//    { return (__out << static_cast<char>(__c)); }
//
//  template<class _Traits>
//    inline basic_ostream<char, _Traits>&
//    operator<<(basic_ostream<char, _Traits>& __out, unsigned char __c)
//    { return (__out << static_cast<char>(__c)); }
//# 537 "/usr/include/c++/6.1.1/ostream" 3
//  template<typename _CharT, typename _Traits>
//    inline basic_ostream<_CharT, _Traits>&
//    operator<<(basic_ostream<_CharT, _Traits>& __out, const _CharT* __s)
//    {
//      if (!__s)
// __out.setstate(ios_base::badbit);
//      else
// __ostream_insert(__out, __s,
//    static_cast<streamsize>(_Traits::length(__s)));
//      return __out;
//    }
//
//  template<typename _CharT, typename _Traits>
//    basic_ostream<_CharT, _Traits> &
//    operator<<(basic_ostream<_CharT, _Traits>& __out, const char* __s);
//
//
//  template<class _Traits>
//    inline basic_ostream<char, _Traits>&
//    operator<<(basic_ostream<char, _Traits>& __out, const char* __s)
//    {
//      if (!__s)
// __out.setstate(ios_base::badbit);
//      else
// __ostream_insert(__out, __s,
//    static_cast<streamsize>(_Traits::length(__s)));
//      return __out;
//    }
//
//
//  template<class _Traits>
//    inline basic_ostream<char, _Traits>&
//    operator<<(basic_ostream<char, _Traits>& __out, const signed char* __s)
//    { return (__out << reinterpret_cast<const char*>(__s)); }
//
//  template<class _Traits>
//    inline basic_ostream<char, _Traits> &
//    operator<<(basic_ostream<char, _Traits>& __out, const unsigned char* __s)
//    { return (__out << reinterpret_cast<const char*>(__s)); }
//
//
//
//
//  //}}}
//















		//template<typename T>
		//basic_ostream& operator<<( T value )
		//{
		//	std::cout<<"Console_stream: operator<<(T) called"<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( std::basic_streambuf<CharT, Traits>* sb)
		//{
		//	std::cout<<"Console_stream: operator<<(streambuf) called"<<std::endl;
		//	return *this;
		//}

		//basic_ostream& operator<<(std::ios_base& (*func)(std::ios_base&) )
		//{
		//	std::cout<<"Console_stream: operator<<(func1) called"<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( std::basic_ios<CharT,Traits>& (*func)(std::basic_ios<CharT,Traits>&) )
		//{
		//	std::cout<<"Console_stream: operator<<(func2) called"<<std::endl;
		//	return *this;
		//}
		//basic_ostream& operator<<( std::basic_ostream<CharT,Traits>& (*func)(std::basic_ostream<CharT,Traits>&) )
		//{
		//	std::cout<<"Console_stream: operator<<(func3) called"<<std::endl;
		//	return *this;
		//}




};

//extern Console_stream red_stream;
//extern Console_stream green_stream;
//extern Console_stream yellow_stream;
//extern Console_stream blue_stream;
//extern Console_stream magenta_stream;
//extern Console_stream cyan_stream;
//extern Console_stream white_stream;

extern Console_stream<char> debug;
extern Console_stream<char> logger;
extern Console_stream<char> my_error;




//extern Console_stream red_stream_hidden;
//extern Console_stream green_stream_hidden;
//extern Console_stream yellow_stream_hidden;
//extern Console_stream blue_stream_hidden;
//extern Console_stream magenta_stream_hidden;
//extern Console_stream cyan_stream_hidden;
//extern Console_stream white_stream_hidden;
//
//extern Console_stream debug_hidden;
//extern Console_stream logger_hidden;
//extern Console_stream error_hidden;
//
///**
// * This little stund will only enable the output streams that are actually needed.
// * Source: Randall C. Chang, February 01, 2003
// * Website: http://www.drdobbs.com/more-on-the-cc-comment-macro-for-debug-s/184401612
// */
//#if defined(USE_RED_STREAM)
//	#define red_stream red_stream_hidden
//#else
//	#define red_stream if(0)std::cout
//#endif
//
//#if defined(USE_GREEN_STREAM)
//	#define green_stream green_stream_hidden
//#else
//	#define green_stream if(0)std::cout
//#endif
//
//#if defined(USE_YELLOW_STREAM)
//	#define yellow_stream yellow_stream_hidden
//#else
//	#define yellow_stream if(0)std::cout
//#endif
//
//#if defined(USE_BLUE_STREAM)
//	#define blue_stream blue_stream_hidden
//#else
//	#define blue_stream if(0)std::cout
//#endif
//
//#if defined(USE_MAGENTA_STREAM)
//	#define magenta_stream magenta_stream_hidden
//#else
//	#define magenta_stream if(0)std::cout
//#endif
//
//#if defined(USE_CYAN_STREAM)
//	#define cyan_stream cyan_stream_hidden
//#else
//	#define cyan_stream if(0)std::cout
//#endif
//
//#if defined(USE_WHITE_STREAM)
//	#define white_stream white_stream_hidden
//#else
//	#define white_stream if(0)std::cout
//#endif
//
//
//
//#if defined(USE_DEBUG_STREAM)
//	#define debug debug_hidden
//#else
//	#define debug if(0)std::cout
//#endif
//
//#if defined(USE_LOGGER_STREAM)
//	#define logger logger_hidden
//#else
//	#define logger if(0)std::cout
//#endif
//
//#if defined(USE_ERROR_STREAM)
//	#define error error_hidden
//#else
//	#define error if(0)std::cout
//#endif

#endif /* CONSOLE_STREAM_HPP */
