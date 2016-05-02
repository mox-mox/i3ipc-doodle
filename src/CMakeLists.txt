cmake_minimum_required(VERSION 3.0)
project(i3ipc-doodle)

# Some general settings:


# Where to install the compiled package to. TODO: Remove this to install to the standard locations
SET(CMAKE_INSTALL_PREFIX /home/mox/projects/software/install)

# Version number and git information will are #define'ed in doodle_config.h.in
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



# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/doodle_config.hpp.in"
  "${PROJECT_BINARY_DIR}/doodle_config.hpp"
  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -Wextra -Wpedantic -Wfatal-errors")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=gnu++14 -Wall -Wextra -Wpedantic -Wfatal-errors -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")


# Some used libraries
set(GETOPTPP "getoptpp")

find_package(PkgConfig)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)

add_subdirectory(i3ipc++)

include_directories(
	${SIGCPP_INCLUDE_DIRS}
	${I3IPCpp_INCLUDE_DIRS}
	"${GETOPTPP}"
	"${GETOPTPP}/getoptpp"
)

link_directories(
	${SIGCPP_LIBRARY_DIRS}
	${I3IPCpp_LIBRARY_DIRS}
)

file(GLOB SRCS ../src/*.cpp ${GETOPTPP}/src/getopt_pp.cpp)

add_executable(doodle ${SRCS} ${GETOPTPP}/src/getopt_pp.cpp)
target_link_libraries(doodle ${I3IPCpp_LIBRARIES})
