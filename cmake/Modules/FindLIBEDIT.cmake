#
#
# Tries to find libedit and libncurses and sets following variables according to found capabilities:
#
# LIBEDIT_FOUND
# LIBEDIT_INCLUDE_DIRS
# LIBEDIT_LIBRARIES

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PKG_LIBEDIT libedit)

FIND_PATH(LIBEDIT_LIBEDIT_INCLUDE_DIRS editline/readline.h
	HINTS
	${PKG_LIBEDIT_INCLUDE_DIRS}
	${PKG_LIBEDIT_INCLUDEDIR}
	PATH_SUFFIXES libedit
)

FIND_LIBRARY(LIBEDIT_LIBEDIT_LIBRARIES edit
	HINTS
	${PKG_LIBEDIT_LIBDIR}
)

FIND_LIBRARY(LIBEDIT_LIBNCURSES_LIBRARIES ncurses
	HINTS ${PKG_LIBEDIT_LIBRARY_DIRS}
)

IF(LIBEDIT_LIBEDIT_LIBRARIES AND LIBEDIT_LIBEDIT_INCLUDE_DIRS AND LIBEDIT_LIBNCURSES_LIBRARIES)
	SET(LIBEDIT_FOUND true)
ENDIF()

IF(LIBEDIT_FIND_REQUIRED AND NOT LIBEDIT_FOUND)
	MESSAGE(FATAL_ERROR "Could not find libedit or libncurses")
ENDIF()

SET(LIBEDIT_INCLUDE_DIRS ${LIBEDIT_LIBEDIT_INCLUDE_DIRS})
SET(LIBEDIT_LIBRARIES ${LIBEDIT_LIBEDIT_LIBRARIES} ${LIBEDIT_LIBNCURSES_LIBRARIES})

MARK_AS_ADVANCED(
	LIBEDIT_INCLUDE_DIRS
	LIBEDIT_LIBRARIES
	LIBEDIT_FOUND
)
