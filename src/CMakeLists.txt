cmake_minimum_required(VERSION 3.0)
project(i3ipc-doodle)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(DOODLE_CONFIG_PATH ".config/doodle")
set(DOODLE_SOCKET_PATH ".config/doodle")







## Where to install the compiled package to. TODO: Remove this to install to the standard locations
#SET(CMAKE_INSTALL_PREFIX /home/mox/projects/software/install)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#{{{Variables: SRCS INCDIRS LINKDIRS LIBS

file(GLOB SRCS ../src/*.cpp getoptpp/src/getopt_pp.cpp)
set(INCDIRS "${PROJECT_BINARY_DIR}")
set(INCDIRS_SYS "getoptpp/getoptpp")
set(LIBS -lpthread -lstdc++fs)
#}}}


#{{{I3IPC++

add_subdirectory(i3ipc++)
set(LIBS ${LIBS} ${I3IPCpp_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${I3IPCpp_INCLUDE_DIRS})
#}}}

#{{{SIGCPP

find_package(SIGCPP REQUIRED)
set(LIBS ${LIBS} ${SIGCPP_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${SIGCPP_INCLUDE_DIRS})
#}}}

#{{{SYSLOG

option (USE_SYSLOG "Enable logging to syslog." ON)
if (USE_SYSLOG)
	find_package(SYSLOG REQUIRED)

	set(LIBS ${LIBS} ${SYSLOG_LIBRARIES})
	set(INCDIRS_SYS ${INCDIRS_SYS} ${SYSLOG_INCLUDE_DIRS})
endif (USE_SYSLOG)
#}}}

#{{{LIBNOTIFY

option (USE_NOTIFICATIONS "Print debugging information to notification popup instead of console." ON)
# The popup notification will always be visible and will not be detected by IPC_DOODLE so viewing it does not create more output.
if (USE_NOTIFICATIONS)
	find_package(LIBNOTIFY REQUIRED)
	find_package(GLIB2 REQUIRED)
	find_package(GDK_PIXBUF2 REQUIRED)

	set(LIBS ${LIBS} ${GDK_PIXBUF2_LIBRARIES} ${GLIB2_LIBRARIES} ${NOTIFY_LIBRARIES})
	set(INCDIRS_SYS ${INCDIRS_SYS} ${NOTIFY_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS} ${GDK_PIXBUF2_INCLUDE_DIRS})
endif (USE_NOTIFICATIONS)
#}}}

#{{{LIBEV

find_package(LIBEV REQUIRED)

set(LIBS ${LIBS} ${LIBEV_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${LIBEV_INCLUDE_DIRS})
#}}}

#{{{XCB

find_package(XCB REQUIRED)

set(LIBS ${LIBS} ${XCB_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${XCB_INCLUDE_DIRS})
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

#{{{CXXFLAGS

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -Os")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DNDEBUG")
#}}}


add_executable(doodle ${SRCS})
target_include_directories(doodle PUBLIC ${INCDIRS})
target_include_directories(doodle SYSTEM PUBLIC ${INCDIRS_SYS})
target_link_libraries(doodle ${LIBS})
