cmake_minimum_required(VERSION 3.0)
project(i3ipc-doodle)


set(DOODLE_CONFIG_PATH ".config/doodle")
# A leading '@' character is translated to a null character,
# mapping the socket to the invisible abstract unix socket name space.
set(DOODLE_SOCKET_PATH "@doodle_user_communication")
#set(DOODLE_SOCKET_PATH "./doodle_user_communication")


## Where to install the compiled package to. TODO: Remove this to install to the standard locations
#SET(CMAKE_INSTALL_PREFIX /home/mox/projects/software/install)



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


subdirs(daemon client libs)
