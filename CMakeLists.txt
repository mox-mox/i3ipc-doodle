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

set_property(GLOBAL PROPERTY TARGET_MESSAGES OFF)

#set(DOODLE_CONFIG_PATH ".config/doodle")
# A leading '&' character is translated to a null character,
# mapping the socket to the invisible abstract unix socket name space.
set(DOODLE_SOCKET_PATH %doodle_user_communication)
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
set(LIBS_PATH ${LIBS_PATH} "${CMAKE_SOURCE_DIR}/libs")
SET(CMAKE_INCLUDE_CURRENT_DIR ON)


#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

set(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/doodle_config.hpp.in")
set(INCDIRS "")
set(INCDIRS_SYS "")
set(LIBS "")
#}}}

#add_subdirectory("${LIBS_PATH}/getoptpp"       "libs/getoptpp")
add_subdirectory("${LIBS_PATH}/console_stream" "libs/console_stream")
add_subdirectory("${LIBS_PATH}/notify_stream"  "libs/notify_stream")
add_subdirectory("${LIBS_PATH}/sockets"        "libs/sockets")
add_subdirectory("${LIBS_PATH}/commands"       "libs/commands")
add_subdirectory("${LIBS_PATH}/uvw"            "libs/uvw")
add_subdirectory("${LIBS_PATH}/parse_date"     "libs/parse_date")

#add_subdirectory(src)
#add_subdirectory(client)






#{{{ Build script for the daemon backend

#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

file(GLOB SRCS_LOCAL src/*.cpp)
set(SRCS ${SRCS} ${SRCS_LOCAL})
#set(INCDIRS ${INCDIRS} "${PROJECT_BINARY_DIR}" "${PROJECT_BINARY_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCDIRS ${INCDIRS} "${PROJECT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCDIRS_SYS ${INCDIRS_SYS})
set(LIBS ${LIBS} -lstdc++fs)
#}}}


MESSAGE( STATUS "PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR} )
MESSAGE( STATUS "I3IPC-DOODLE_SOURCE_DIR: " ${i3ipc-doodle_SOURCE_DIR} )
MESSAGE( STATUS "LIBS_PATH: " ${LIBS_PATH} )


#{{{LIBUV + UVW

find_package(LIBUV REQUIRED)

set(LIBS ${LIBS} ${LIBUV_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${LIBUV_INCLUDE_DIRS} "${LIBS_PATH}/uvw/src" "${LIBS_PATH}/uvw/deps/libuv/src/include")
#}}}

#{{{I3IPCPP

add_subdirectory(${i3ipc-doodle_SOURCE_DIR}/libs/i3ipcpp "libs/i3ipcpp")
set(LIBS ${LIBS} ${I3IPCpp_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${I3IPCpp_INCLUDE_DIRS})
#}}}

#{{{GETOPTPP

add_subdirectory(${i3ipc-doodle_SOURCE_DIR}/libs/getoptpp "libs/getoptpp")
set(LIBS ${LIBS} ${GETOPTPP_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${GETOPTPP_INCLUDE_DIRS})
#}}}

#{{{INIH

set(INCDIRS_SYS ${INCDIRS_SYS} ${LIBUV_INCLUDE_DIRS} "${LIBS_PATH}/inih")
#}}}

#{{{XCB

find_package(XCB REQUIRED)

set(LIBS ${LIBS} ${XCB_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${XCB_INCLUDE_DIRS})
#}}}

#{{{REPL

add_subdirectory(${i3ipc-doodle_SOURCE_DIR}/libs/repl "libs/repl")
set(LIBS ${LIBS} ${REPL_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${REPL_INCLUDE_DIRS})
#}}}



##add_subdirectory("${LIBS_PATH}/parse_date" "libs/parse_date")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")



add_executable(doodle ${SRCS})
target_link_libraries(doodle ${LIBS})
target_include_directories(doodle PUBLIC ${INCDIRS})
target_include_directories(doodle SYSTEM PUBLIC ${INCDIRS_SYS})
target_compile_options(    doodle PUBLIC -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wfatal-errors)
target_compile_options(    doodle PUBLIC $<$<CONFIG:DEBUG>:-O0 -ggdb -save-temps>)
target_compile_options(    doodle PUBLIC $<$<CONFIG:RELEASE>:-O3>)
target_compile_definitions(doodle PUBLIC NOTIFY_PROGRAM_NAME="${DOODLE_PROGRAM_NAME}" ENABLE_MAIN)
target_compile_definitions(doodle PUBLIC $<$<CONFIG:DEBUG>:NOTIFY_LEVEL=3 LOGGING_LEVEL=3>)
target_compile_definitions(doodle PUBLIC $<$<CONFIG:RELEASE>:NOTIFY_LEVEL=1 LOGGING_LEVEL=1>)
# Set build type when generating the Makefile like: cmake -DCMAKE_BUILD_TYPE=Debug ../i3ipc-doodle






#add_dependencies(doodle notify_start)
#add_custom_target( notify_start COMMAND notify-send "Starting build for doodle-daemon")


##add_custom_target(copy_doodle_config DEPENDS ${CMAKE_SOURCE_DIR}/daemon/.config
##                   COMMAND ${CMAKE_COMMAND} -E copy_directory
##                   ${CMAKE_SOURCE_DIR}/daemon/.config $<TARGET_FILE_DIR:doodle>/.config)
##
##
##
##
##add_custom_target(do_always ALL)
##add_custom_target(notify_end ALL COMMAND notify-send "Finished build for doodle-daemon" DEPENDS do_always doodle)

#add_subdirectory(test)

##add_custom_target(run_doodle
##	COMMAND ./doodle --no-fork; read -p "Press [Enter] key to close..."
##    DEPENDS doodle copy_doodle_config
##    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
##)

#}}}
