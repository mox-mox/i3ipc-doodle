cmake_minimum_required(VERSION 3.0)
# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()
project(i3ipc-doodle)
find_package( PythonLibs 3 REQUIRED )
find_package( PythonInterp 3 REQUIRED )


#set(DOODLE_CONFIG_PATH ".config/doodle")
# A leading '&' character is translated to a null character,
# mapping the socket to the invisible abstract unix socket name space.
set(DOODLE_SOCKET_PATH "%doodle_user_communication")
#set(DOODLE_SOCKET_PATH "./doodle_user_communication")


set(DOODLE_PROTOCL_VERSION "Doodle01")


## Where to install the compiled package to. TODO: Remove this to install to the standard locations
#SET(CMAKE_INSTALL_PREFIX /home/mox/projects/software/install)



#SET (NOTIFICATION_LEVEL "Show desktop notifications about program operation? 0 = disable notifications, 1 = show critical information only, 2 = also show debugging information, 3 = be verbose" 1)
SET (NOTIFICATION_LEVEL 3) # Show desktop notifications about program operation? 0 = disable notifications, 1 = show critical information only, 2 = also show debugging information, 3 = be verbose
#option (USE_NOTIFICATIONS "Print debugging information to notification popup instead of console." ON)
option (USE_SYSLOG "Enable logging to syslog." OFF)

option(DOODLE_WITH_TESTS "Build unit tests executables" ON)


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







set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(LIBS_PATH ${LIBS_PATH} "${CMAKE_SOURCE_DIR}/libs/")


#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

set(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/doodle_config.hpp.in")
set(INCDIRS "")
set(INCDIRS_SYS "")
set(LIBS "")
#}}}

add_subdirectory("${LIBS_PATH}/getoptpp"       "libs/getoptpp")
add_subdirectory("${LIBS_PATH}/console_stream" "libs/console_stream")
add_subdirectory("${LIBS_PATH}/notify_stream"  "libs/notify_stream")
add_subdirectory("${LIBS_PATH}/sockets"        "libs/sockets")
add_subdirectory("${LIBS_PATH}/commands"       "libs/commands")

add_subdirectory(daemon)
#add_subdirectory(client)
