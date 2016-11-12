if(DOODLE_WITH_TESTS)
	find_package(CxxTest)
	if(CXXTEST_FOUND)
		include_directories(${CXXTEST_INCLUDE_DIR})
		include_directories(src/)
		add_definitions( -DTEST_SRC_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/test")

		enable_testing()
		file(GLOB SRC_TEST *.hpp)
		CXXTEST_ADD_TEST(doodle_check test.cpp ${SRC_TEST})
		target_link_libraries(doodle_check ${I3IPCpp_LIBRARIES})
        ADD_CUSTOM_COMMAND( TARGET doodle_check
                            POST_BUILD
                            COMMAND $<TARGET_FILE:doodle_check>)
	else()
		message(WARNING "CxxTest not found. Unable to run unit-tests")
	endif()
endif()
