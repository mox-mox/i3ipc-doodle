# Build script for the daemon backend

#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

file(GLOB SRCS *.cpp ../libs/getoptpp/src/getopt_pp.cpp)
set(INCDIRS "${PROJECT_BINARY_DIR}")
set(INCDIRS_SYS "getoptpp/getoptpp")
set(LIBS -lstdc++fs)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
#}}}

#{{{I3IPC++

add_subdirectory(${i3ipc-doodle_SOURCE_DIR}/libs/i3ipc++ i3ipc++)
set(LIBS ${LIBS} ${I3IPCpp_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${I3IPCpp_INCLUDE_DIRS})
#}}}

#{{{GETOPTPP

set(LIBS ${LIBS} getoptpp)
set(INCDIRS_SYS ${INCDIRS_SYS} ${i3ipc-doodle_SOURCE_DIR}/libs/getoptpp/getoptpp)
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


#{{{CXXFLAGS

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -Os")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DNDEBUG")
#}}}


add_executable(doodle ${SRCS})
target_link_libraries(doodle ${LIBS} )
target_include_directories(doodle PUBLIC ${INCDIRS})
target_include_directories(doodle SYSTEM PUBLIC ${INCDIRS_SYS})

add_custom_command(TARGET doodle POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/daemon/.config $<TARGET_FILE_DIR:doodle>/.config)
