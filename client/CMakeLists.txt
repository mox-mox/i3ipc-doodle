# Build script for the daemon backend

#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

file(GLOB SRCS_LOCAL *.cpp)
set(SRCS ${SRCS} ${SRCS_LOCAL})
set(INCDIRS ${INCDIRS} "${PROJECT_BINARY_DIR}" "${PROJECT_BINARY_DIR}/..")
set(INCDIRS_SYS ${INCDIRS_SYS})
set(LIBS ${LIBS} -lstdc++fs)
#}}}

#set_property( GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS FALSE )

#{{{LIBEDIT

find_package(LIBEDIT REQUIRED)

set(LIBS ${LIBS} ${LIBEDIT_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${LIBEDIT_INCLUDE_DIRS})
#}}}

#{{{JSONCPP

#find_package(JSONCPP REQUIRED)
set(LIBJSONCPP_LIBRARIES "/usr/lib/libjsoncpp.so")
set(LIBJSONCPP_INCLUDE_DIRS "/usr/include")

set(LIBS ${LIBS} ${LIBJSONCPP_LIBRARIES})
set(INCDIRS_SYS ${INCDIRS_SYS} ${LIBJSONCPP_INCLUDE_DIRS})
#}}}

add_executable(doodle_client ${SRCS})
target_link_libraries(doodle_client ${LIBS} )
target_include_directories(doodle_client PUBLIC ${INCDIRS})
target_include_directories(doodle_client SYSTEM PUBLIC ${INCDIRS_SYS})
target_compile_options(doodle_client PUBLIC -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wfatal-errors -DNOTIFY_LEVEL=0 -DNOTIFY_PROGRAM_NAME="${DOODLE_PROGRAM_NAME}")
target_compile_options(doodle_client PUBLIC $<$<CONFIG:DEBUG>:-O0 -ggdb -save-temps -DLOGGING_LEVEL=3>)
target_compile_options(doodle_client PUBLIC $<$<CONFIG:RELEASE>:-O3 -DLOGGING_LEVEL=1>)
