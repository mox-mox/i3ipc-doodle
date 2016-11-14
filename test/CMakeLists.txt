if(DOODLE_WITH_TESTS)
	find_package(CxxTest)
	if(CXXTEST_FOUND)
		include_directories(${CXXTEST_INCLUDE_DIR})
		include_directories(src/)
		add_definitions( -DTEST_SRC_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/test")
		enable_testing()
		file(GLOB SRC_TEST *.hpp)
		CXXTEST_ADD_TEST(doodle_test doodle_test.cpp ${SRC_TEST})
		target_link_libraries(doodle_test ${I3IPCpp_LIBRARIES})
		###{{{ # configure a header file to pass some of the CMake settings to the source code

		##set(DOODLE_DAEMON_EXE $<TARGET_FILE:doodle_test>)
		##configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/test_config.hpp.in"
		##                "${CMAKE_CURRENT_SOURCE_DIR}/test_config.hpp")
		###}}}
		ADD_CUSTOM_COMMAND( TARGET doodle_test POST_BUILD COMMAND $<TARGET_FILE:doodle_test>)
	else()
		message(WARNING "CxxTest not found. Unable to run unit-tests")
	endif()
endif()
