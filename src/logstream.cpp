#include "logstream.hpp"

// Initialise the streams. \x1b[31m is an ANSI-escape sequence, see
// https://en.wikipedia.org/wiki/ANSI_escape_code
//
// init_priority  allows to set the initialisation order of global objects.
// Using this, it becomes possible to use the logger and error streams in the
// constructors of global objects.
//
__attribute__ ((init_priority (101))) LogStream logger("\x1b[32m(LL) ");
__attribute__ ((init_priority (101))) LogStream error("\x1b[31m(EE) ");






//
//ios_base::Logstream_Init::Logstream_Init()
//{
//	if (__gnu_cxx::__exchange_and_add_dispatch(&_S_refcount, 1) == 0)
//	{
//		// Standard streams default to synced with "C" operations.
//		_S_synced_with_stdio = true;
//
//		new(&buf_cout_sync) stdio_sync_filebuf < char > (stdout);
//		new(&buf_cin_sync) stdio_sync_filebuf < char > (stdin);
//		new(&buf_cerr_sync) stdio_sync_filebuf < char > (stderr);
//
//		// The standard streams are constructed once only and never
//		// destroyed.
//		new(&cout) ostream(&buf_cout_sync);
//		new(&cin) istream(&buf_cin_sync);
//		new(&cerr) ostream(&buf_cerr_sync);
//		new(&clog) ostream(&buf_cerr_sync);
//		cin.tie(&cout);
//		cerr.setf(ios_base::unitbuf);
//		// _GLIBCXX_RESOLVE_LIB_DEFECTS
//		// 455. cerr::tie() and wcerr::tie() are overspecified.
//		cerr.tie(&cout);
//
//
//		// NB: Have to set refcount above one, so that standard
//		// streams are not re-initialized with uses of ios_base::Init
//		// besides <iostream> static object, ie just using <ios> with
//		// ios_base::Init objects.
//		__gnu_cxx::__atomic_add_dispatch(&_S_refcount, 1);
//	}
//}
//
//ios_base::Logstream_Init::~Logstream_Init()
//{
//	// Be race-detector-friendly.  For more info see bits/c++config.
//	_GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(&_S_refcount);
//	if (__gnu_cxx::__exchange_and_add_dispatch(&_S_refcount, -1) == 2)
//	{
//		_GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(&_S_refcount);
//		// Catch any exceptions thrown by basic_ostream::flush()
//		__try
//		{
//			// Flush standard output streams as required by 27.4.2.1.6
//			cout.flush();
//			cerr.flush();
//			clog.flush();
//
//		}
//		__catch(...)
//		{ }
//	}
//}
//
