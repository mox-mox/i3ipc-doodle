# Build script for the daemon backend

#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

file(GLOB SRCS *.cpp ../libs/getoptpp/src/getopt_pp.cpp)
set(INCDIRS "${PROJECT_BINARY_DIR}")
#set(INCDIRS_SYS "getoptpp/getoptpp")
set(LIBS -lstdc++fs)
#}}}


#{{{LIBEV

find_package(LIBEV REQUIRED)

set(LIBS ${LIBS} ${LIBEV_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${LIBEV_INCLUDE_DIRS})
#}}}

add_subdirectory("${LIBS_PATH}/getoptpp" "libs/getoptpp")
add_subdirectory("${LIBS_PATH}/console_stream" "libs/console_stream")
add_subdirectory("${LIBS_PATH}/notify_stream" "libs/notify_stream")

add_executable(doodle_client ${SRCS})
target_link_libraries(doodle_client ${LIBS} )
target_include_directories(doodle_client PUBLIC ${INCDIRS})
target_include_directories(doodle_client SYSTEM PUBLIC ${INCDIRS_SYS})
target_compile_options(doodle_client PUBLIC -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wfatal-errors)
target_compile_options(doodle_client PUBLIC $<$<CONFIG:DEBUG>:-O0 -ggdb -save-temps -DLOGGING_LEVEL=3>)
target_compile_options(doodle_client PUBLIC $<$<CONFIG:RELEASE>:-O3 -DLOGGING_LEVEL=1>)
