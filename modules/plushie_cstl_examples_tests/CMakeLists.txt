###############################################################################################
# Plushie C STL::(Examples and Tests)
# Provides the examples and tests for plushie_cstl. This is placed in a seperate repo 
# too keep the contents of the CStl streamlined, that way there's less content to download
# when using fetch_content or something similar
###############################################################################################
cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)
project(PlushieCStlExamples VERSION 0.1.0 LANGUAGES C)

option(PLUSHIE_CSTL_BUILD_EXAMPLES   "Build examples for plushie::cstl"     ON )
option(PLUSHIE_CSTL_BUILD_TESTS      "Build tests for plushie::cstl"        ON )
option(PLUSHIE_CSTL_INSTALL_EXAMPLES "include install target for examples"  ON )
option(PLUSHIE_CSTL_INSTALL_TESTS    "include install target for tests"     ON )
option(PLUSHIE_CSTL_INCLUDE_CXX      "include C++ for tests and examples"   OFF)

if(${PLUSHIE_CSTL_INCLUDE_CXX})
	message(STATUS "ENABLING CXX")
	enable_language(CXX)
endif()

# Subhelpers for if we should use libPlushieStartupExe to test not using libc
option(PLUSHIE_CSTL_EXAMPLES_USE_STARTUP "Use custom premain function" OFF)
option(PLUSHIE_CSTL_TESTS_USE_STARTUP    "Use custom premain function" OFF)

find_package(PlushieCStl QUIET REQUIRED)

###############################################################################################
# Examples
###############################################################################################

if(${PLUSHIE_CSTL_BUILD_EXAMPLES})
	add_subdirectory(examples)
endif()

###############################################################################################
# Tests
###############################################################################################

if(${PLUSHIE_CSTL_BUILD_TESTS})

	# We use unity test for the native tests
	find_package(unity REQUIRED QUIET)
	enable_testing()

	# List of targets used
	set(TEST_TARGETS "")

	# Glob all the test files
	file(GLOB TEST_C_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_*.c")
	if(${PLUSHIE_CSTL_INCLUDE_CXX})
		file(GLOB TEST_CPP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_*.cpp")
	else()
		set(TEST_CPP_SOURCES "")
	endif()
	set(TEST_SOURCES "tests/main.c" "tests/tests_shared.h" ${TEST_C_SOURCES} ${TEST_CPP_SOURCES})

	if(TARGET PlushieCStl)
		list(APPEND TEST_TARGETS PlushieCStl_Test_HeaderOnly)
		add_executable(PlushieCStl_Test_HeaderOnly ${TEST_SOURCES})
		target_link_libraries(PlushieCStl_Test_HeaderOnly PRIVATE PlushieCStl)
		target_compile_definitions(PlushieCStl_Test_HeaderOnly PRIVATE PLUSHIE_CSTL_TESTS_PROVIDE_IMPL=1)
	endif()

	if(TARGET PlushieCStl_Static)
		list(APPEND TEST_TARGETS PlushieCStl_Test_Static)
		add_executable(PlushieCStl_Test_Static ${TEST_SOURCES})
		target_link_libraries(PlushieCStl_Test_Static PRIVATE PlushieCStl_Static)
	endif()

	if(TARGET PlushieCStl_Shared)
		list(APPEND TEST_TARGETS PlushieCStl_Test_Shared)
		add_executable(PlushieCStl_Test_Shared ${TEST_SOURCES})
		target_link_libraries(PlushieCStl_Test_Shared PRIVATE PlushieCStl_Shared)
	endif()

	# Apply the shared testing properties 
	foreach(target ${TEST_TARGETS})
		set_target_properties(${target} PROPERTIES FOLDER "plushie/plushie_cstl/tests")
		target_link_libraries(${target} PRIVATE unity::framework)

		add_test(NAME ${target} COMMAND ${target})
	endforeach()
	


endif()