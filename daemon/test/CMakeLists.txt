if(DOODLE_WITH_TESTS)

#{{{Variables: SRCS INCDIRS LINKDIRS LIBS CMAKE_MODULE_PATH

#file(GLOB SRCS_LOCAL *.cpp)
file(GLOB SRCS_TEST *.hpp)
set(SRCS ${SRCS} ${SRCS_LOCAL} ${SRCS_DAEMON})
set(INCDIRS ${INCDIRS} "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/daemon")
set(INCDIRS_SYS ${INCDIRS_SYS})
set(LIBS ${LIBS} -lstdc++fs)
#}}}

	find_package(CxxTest)
	if(CXXTEST_FOUND)
		enable_testing()
		CXXTEST_ADD_TEST(doodle_test doodle_test.cpp ${SRCS_TEST} ${SRCS})
		target_compile_definitions(doodle_test PUBLIC NOTIFY_PROGRAM_NAME="${DOODLE_PROGRAM_NAME}" NOTIFY_LEVEL=0 LOGGING_LEVEL=0)
		target_include_directories(doodle_test PUBLIC ${CXXTEST_INCLUDE_DIR} ${INCDIRS})
		target_include_directories(doodle_test SYSTEM PUBLIC ${INCDIRS_SYS})
		target_link_libraries(doodle_test ${LIBS})
		ADD_CUSTOM_COMMAND( TARGET doodle_test POST_BUILD COMMAND $<TARGET_FILE:doodle_test>)
	else()
		message(WARNING "CxxTest not found. Unable to run unit-tests")
	endif()
endif()
