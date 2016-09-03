# Build script for the daemon backend

#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

file(GLOB SRCS *.cpp)
set(SRCS ${SRCS} ../doodle_config.hpp.in)
set(INCDIRS "${PROJECT_BINARY_DIR}" "${PROJECT_BINARY_DIR}/..")
set(INCDIRS_SYS "")
set(LIBS -lstdc++fs)
#}}}

#{{{I3IPCPP

add_subdirectory(${i3ipc-doodle_SOURCE_DIR}/libs/i3ipcpp "libs/i3ipcpp")
set(LIBS ${LIBS} ${I3IPCpp_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${I3IPCpp_INCLUDE_DIRS})
#}}}

#{{{SIGCPP

find_package(SIGCPP REQUIRED)
set(LIBS ${LIBS} ${SIGCPP_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${SIGCPP_INCLUDE_DIRS})
#}}}

#{{{SYSLOG

if (USE_SYSLOG)
	find_package(SYSLOG REQUIRED)

	set(LIBS ${LIBS} ${SYSLOG_LIBRARIES})
	set(INCDIRS_SYS ${INCDIRS_SYS} ${SYSLOG_INCLUDE_DIRS})
endif (USE_SYSLOG)
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

add_subdirectory("${LIBS_PATH}/getoptpp" "libs/getoptpp")
add_subdirectory("${LIBS_PATH}/console_stream" "libs/console_stream")
add_subdirectory("${LIBS_PATH}/notify_stream" "libs/notify_stream")









add_executable(doodle_daemon ${SRCS})
target_link_libraries(doodle_daemon ${LIBS} )
target_include_directories(doodle_daemon PUBLIC ${INCDIRS})
target_include_directories(doodle_daemon SYSTEM PUBLIC ${INCDIRS_SYS})
target_compile_options(doodle_daemon PUBLIC -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wfatal-errors -DNOTIFY_PROGRAM_NAME="${DOODLE_PROGRAM_NAME}")
target_compile_options(doodle_daemon PUBLIC $<$<CONFIG:DEBUG>:-DNOTIFY_LEVEL=3 -DLOGGING_LEVEL=3 -O0 -ggdb -save-temps>)
target_compile_options(doodle_daemon PUBLIC $<$<CONFIG:RELEASE>:-DNOTIFY_LEVEL=1 -DLOGGING_LEVEL=1 -O3>)
# Set build type when generating the Makefile like: cmake -DCMAKE_BUILD_TYPE=Debug ../i3ipc-doodle

add_custom_command(TARGET doodle_daemon POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/daemon/.config $<TARGET_FILE_DIR:doodle_daemon>/.config)
