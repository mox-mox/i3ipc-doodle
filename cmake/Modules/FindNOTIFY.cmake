# Try to find syslog.h for Syslog


IF (NOTIFY_INCLUDE_DIRS)
	# Already in cache, be silent
	SET(NOTIFY_FIND_QUIETLY TRUE)
ENDIF (NOTIFY_INCLUDE_DIRS)

#FIND_PATH(NOTIFY_INCLUDE_DIRS NAMES gdk-pixbuf/gdk-pixbuf.h HINTS /usr/include/gdk-pixbuf-2.0)

FIND_LIBRARY(NOTIFY_LIBRARIES notify
	PATHS /usr/lib
	HINTS ${PKG_NOTIFY_LIBDIR}
)


IF(NOTIFY_LIBRARIES) 
	SET(NOTIFY_FOUND TRUE) 
ENDIF(NOTIFY_LIBRARIES) 

IF(NOTIFY_FOUND) 
	set(CMAKE_REQUIRED_INCLUDES ${GLIB-2.0_INCLUDE_DIRS})
	set(CMAKE_REQUIRED_INCLUDES)
	IF(NOT NOTIFY_FIND_QUIETLY) 
		MESSAGE(STATUS "Found NOTIFY headers: ${NOTIFY_INCLUDE_DIRS}/glib-2.0/glib.h") 
	ENDIF(NOT NOTIFY_FIND_QUIETLY) 
ELSE(NOTIFY_FOUND) 
	IF(Syslog_FIND_REQUIRED) 
		MESSAGE(FATAL_ERROR "Could not find glib") 
	ENDIF(Syslog_FIND_REQUIRED) 
ENDIF(NOTIFY_FOUND) 

MARK_AS_ADVANCED(
	NOTIFY_FOUND
	#NOTIFY_INCLUDE_DIRS
	NOTIFY_LIBRARIES
	)
