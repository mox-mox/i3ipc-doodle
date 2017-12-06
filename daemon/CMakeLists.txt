# Build script for the daemon backend

#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

file(GLOB SRCS_LOCAL *.cpp)
set(SRCS ${SRCS} ${SRCS_LOCAL})
set(INCDIRS ${INCDIRS} "${PROJECT_BINARY_DIR}" "${PROJECT_BINARY_DIR}/daemon")
set(INCDIRS_SYS ${INCDIRS_SYS})
set(LIBS ${LIBS} -lstdc++fs)
#}}}

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

#{{{INIH

set(INCDIRS_SYS ${INCDIRS_SYS} ${LIBUV_INCLUDE_DIRS} "${LIBS_PATH}/inih")
#}}}

#{{{XCB

find_package(XCB REQUIRED)

set(LIBS ${LIBS} ${XCB_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${XCB_INCLUDE_DIRS})
#}}}



#
##{{{SIGCPP
#
#find_package(SIGCPP REQUIRED)
#set(LIBS ${LIBS} ${SIGCPP_LIBRARIES})
#set(INCDIRS_SYS ${INCDIRS_SYS} ${SIGCPP_INCLUDE_DIRS})
##}}}
#
##{{{SYSLOG
#
#if (USE_SYSLOG)
#	find_package(SYSLOG REQUIRED)
#
#	set(LIBS ${LIBS} ${SYSLOG_LIBRARIES})
#	set(INCDIRS_SYS ${INCDIRS_SYS} ${SYSLOG_INCLUDE_DIRS})
#endif (USE_SYSLOG)
##}}}
#










add_subdirectory("${LIBS_PATH}/parse_date" "libs/parse_date")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


add_custom_target( notify_start COMMAND notify-send "Starting build for doodle-daemon")

add_executable(doodle_daemon ${SRCS})
add_dependencies(doodle_daemon notify_start)
target_link_libraries(doodle_daemon ${LIBS})
target_include_directories(doodle_daemon PUBLIC ${INCDIRS})
target_include_directories(doodle_daemon SYSTEM PUBLIC ${INCDIRS_SYS})
target_compile_options(    doodle_daemon PUBLIC -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wfatal-errors)
target_compile_options(    doodle_daemon PUBLIC $<$<CONFIG:DEBUG>:-O0 -ggdb -save-temps>)
target_compile_options(    doodle_daemon PUBLIC $<$<CONFIG:RELEASE>:-O3>)
target_compile_definitions(doodle_daemon PUBLIC NOTIFY_PROGRAM_NAME="${DOODLE_PROGRAM_NAME}" ENABLE_MAIN)
target_compile_definitions(doodle_daemon PUBLIC $<$<CONFIG:DEBUG>:NOTIFY_LEVEL=3 LOGGING_LEVEL=3>)
target_compile_definitions(doodle_daemon PUBLIC $<$<CONFIG:RELEASE>:NOTIFY_LEVEL=1 LOGGING_LEVEL=1>)
# Set build type when generating the Makefile like: cmake -DCMAKE_BUILD_TYPE=Debug ../i3ipc-doodle


add_custom_target(copy_doodle_config DEPENDS ${CMAKE_SOURCE_DIR}/daemon/.config
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/daemon/.config $<TARGET_FILE_DIR:doodle_daemon>/.config)



			   #add_custom_command(TARGET doodle_daemon POST_BUILD
			   #                   COMMAND ${CMAKE_COMMAND} -E copy_directory
			   #                   ${CMAKE_SOURCE_DIR}/daemon/.config $<TARGET_FILE_DIR:doodle_daemon>/.config)


add_custom_target(do_always ALL)
add_custom_target(notify_end ALL COMMAND notify-send "Finished build for doodle-daemon" DEPENDS do_always doodle_daemon)

#add_subdirectory(test)






add_custom_target(run_doodle_daemon
	COMMAND ./doodle_daemon --no-fork; read -p "Press [Enter] key to close..."
    DEPENDS doodle_daemon copy_doodle_config
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
