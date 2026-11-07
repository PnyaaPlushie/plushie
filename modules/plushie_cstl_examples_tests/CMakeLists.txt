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
	add_subdirectory(tests)
endif()