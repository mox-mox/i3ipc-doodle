# Build script for the daemon backend

#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

file(GLOB SRCS *.cpp ../libs/getoptpp/src/getopt_pp.cpp)
set(INCDIRS "${PROJECT_BINARY_DIR}")
#set(INCDIRS_SYS "getoptpp/getoptpp")
set(LIBS -lstdc++fs)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
#}}}


#{{{GETOPTPP

set(LIBS ${LIBS} getoptpp)
set(INCDIRS_SYS ${INCDIRS_SYS} ${i3ipc-doodle_SOURCE_DIR}/libs/getoptpp/getoptpp)
#}}}

#{{{STREAMLIB

#set(LIBS ${LIBS} getoptpp)
set(INCDIRS_SYS ${INCDIRS_SYS} ${i3ipc-doodle_SOURCE_DIR}/libs/streamlib)
#}}}

#{{{LIBNOTIFY

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








#{{{CXXFLAGS

# Set build type when generating the Makefile like: cmake -DCMAKE_BUILD_TYPE=Debug ..
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -Os")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17 -lstdc++fs -Wall -Wextra -Wpedantic -Wfatal-errors -O0 -ggdb -DNDEBUG")
#}}}


add_executable(doodle_client ${SRCS})
target_link_libraries(doodle_client ${LIBS} )
target_include_directories(doodle_client PUBLIC ${INCDIRS})
target_include_directories(doodle_client SYSTEM PUBLIC ${INCDIRS_SYS})

