cmake_minimum_required(VERSION 3.0)
project(i3ipc-doodle)
MESSAGE( STATUS "\n\n\n\n\n\n\n\n==== STARTING CMAKE ===========================================\n\n")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")


## Where to install the compiled package to. TODO: Remove this to install to the standard locations
#SET(CMAKE_INSTALL_PREFIX /home/mox/projects/software/install)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")




# Some used libraries
set(GETOPTPP "getoptpp")

find_package(PkgConfig)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)

add_subdirectory(i3ipc++)



# Variables: SRCS INCDIRS LINKDIRS LIBS
file(GLOB SRCS ../src/*.cpp ${GETOPTPP}/src/getopt_pp.cpp)

set(INCDIRS
	"${PROJECT_BINARY_DIR}"
	${SIGCPP_INCLUDE_DIRS}
	${I3IPCpp_INCLUDE_DIRS}
	"${GETOPTPP}"
	"${GETOPTPP}/getoptpp"
	)

set(LIBS
	${I3IPCpp_LIBRARIES}
	-lpthread -lstdc++fs
	)

set(LINKDIRS ${LINKDIRS}
	${LIBS}
	${SIGCPP_LIBRARY_DIRS}
	${I3IPCpp_LIBRARY_DIRS}
	)




#{{{
option (USE_SYSLOG "Enable logging to syslog." ON)
if (USE_SYSLOG)
	find_package(SYSLOG REQUIRED)

	set(LIBS ${LIBS} ${SYSLOG_LIBRARIES})
	set(INCDIRS ${INCDIRS} ${SYSLOG_INCLUDE_DIRS})
endif (USE_SYSLOG)
#}}}


#{{{
option (USE_NOTIFICATIONS "Print debugging information to notification popup instead of console." ON)
# The popup notification will always be visible and will not be detected by IPC_DOODLE so viewing it does not create more output.
if (USE_NOTIFICATIONS)
	find_package(Libnotify REQUIRED)
	find_package(GLIB2 REQUIRED)
	find_package(GDK_PIXBUF2 REQUIRED)

	set(LIBS ${LIBS} ${GDK_PIXBUF2_LIBRARIES} ${GLIB2_LIBRARIES} ${NOTIFY_LIBRARIES})
	set(INCDIRS ${INCDIRS} ${NOTIFY_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS} ${GDK_PIXBUF2_INCLUDE_DIRS})
endif (USE_NOTIFICATIONS)
#}}}



#{{{
# Version number and git information
# will be #define'ed in doodle_config.h.in
# The version number.
set (DOODLE_VERSION_MAJOR 0)
set (DOODLE_VERSION_MINOR 1)
set (DOODLE_PROGRAM_NAME IPC_DOODLE)

#{{{ Get the current working branch

execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
#}}}

#{{{ Get the latest abbreviated commit hash of the working branch

execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
#}}}
#}}}
#{{{
# configure a header file to pass some of the CMake settings to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/doodle_config.hpp.in"
  "${PROJECT_BINARY_DIR}/doodle_config.hpp"
  )
#}}}



#{{{ CXXFLAGS

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -lstdc++fs  -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DNDEBUG")
#}}}


link_directories( ${LINKDIRS})
add_executable(doodle ${SRCS})
target_include_directories(doodle SYSTEM PUBLIC ${INCDIRS})
target_link_libraries(doodle ${LIBS})






###############################################################################################################################################################
###############################################################################################################################################################
###############################################################################################################################################################
###############################################################################################################################################################








#cmake_minimum_required(VERSION 3.0)
#project(i3ipc-doodle)
#
#MESSAGE( STATUS "\n\n\n\n\n\n\n\n==== STARTING CMAKE ===========================================\n\n")
#
## Some general settings:
#
#
## Where to install the compiled package to. TODO: Remove this to install to the standard locations
#SET(CMAKE_INSTALL_PREFIX /home/mox/projects/software/install)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
#
#MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH} )
#
## Version number and git information will are #define'ed in doodle_config.h.in
## The version number.
#set (DOODLE_VERSION_MAJOR 0)
#set (DOODLE_VERSION_MINOR 1)
#set (DOODLE_PROGRAM_NAME IPC_DOODLE)
#
##{{{ Get the current working branch
#
#execute_process(
#COMMAND git rev-parse --abbrev-ref HEAD
#WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#OUTPUT_VARIABLE GIT_BRANCH
#OUTPUT_STRIP_TRAILING_WHITESPACE
#)
##}}}
#
##{{{ Get the latest abbreviated commit hash of the working branch
#
#execute_process(
#COMMAND git log -1 --format=%h
#WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#OUTPUT_VARIABLE GIT_COMMIT_HASH
#OUTPUT_STRIP_TRAILING_WHITESPACE
#)
##}}}
#
#
#
#
## configure a header file to pass some of the CMake settings
## to the source code
#configure_file (
#  "${PROJECT_SOURCE_DIR}/doodle_config.hpp.in"
#  "${PROJECT_BINARY_DIR}/doodle_config.hpp"
#  )
#
#
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -lstdc++fs  -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -ggdb -DDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DNDEBUG")


## Some used libraries
#set(GETOPTPP "getoptpp")
#
#find_package(PkgConfig)
#pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
#
#add_subdirectory(i3ipc++)
#
#set(INCDIRS ${INCDIRS}
#	"${PROJECT_BINARY_DIR}"
#	${SIGCPP_INCLUDE_DIRS}
#	${LIBNOTIFY_INCLUDE_DIR}
#	${I3IPCpp_INCLUDE_DIRS}
#	"${GETOPTPP}"
#	"${GETOPTPP}/getoptpp"
#	)
#
#MESSAGE( STATUS "Include directories:")
#foreach(dir ${INCDIRS})
#	MESSAGE( STATUS "	" ${dir})
#endforeach(dir)
#
#include_directories( ${INCDIRS})
#
#
#	set(LIBS ${LIBS} -lpthread -lstdc++fs)
#
#
#set(LINKDIRS ${LINKDIRS}
#	${LIBS}
#	${SIGCPP_LIBRARY_DIRS}
#	${LIBNOTIFY_LIBRARIES}
#	${I3IPCpp_LIBRARY_DIRS}
#	)
#MESSAGE( STATUS "Link directories:")
#foreach(dir ${LINKDIRS})
#	MESSAGE( STATUS "	" ${dir})
#endforeach(dir)
#
#link_directories(
#	${LINKDIRS}
#)
#
#file(GLOB SRCS ../src/*.cpp ${GETOPTPP}/src/getopt_pp.cpp)

#add_executable(doodle ${SRCS} ${GETOPTPP}/src/getopt_pp.cpp)
#target_link_libraries(doodle ${LIBS} ${I3IPCpp_LIBRARIES})








#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -lnotify -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0            -std=gnu++14 -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DDEBUG")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -pthread -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -lnotify -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0            -std=gnu++14 -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DDEBUG")
