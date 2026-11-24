# --- Plushie --------------------------------------------------------------------------------------------
# Top-level CMake for scanning subdirectories that contain CMakeLists.txt
cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)
option(PLUSHIE_ROOT_CSTL_ONLY "Should plushie root only configure for cstl for smaller build" OFF)

enable_testing()
project(Plushie VERSION 0.1.0 LANGUAGES C)
include(FetchContent)

# Put every exe and DLL from every module in one folder (bin/<Config> for
# multi-config generators). Windows has no rpath, so an exe only finds its DLLs
# when they sit beside it; this makes ctest, the IDE and the build dir just work.
# Must be set before any module is added. Target output names must be unique.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# It's advised thart user's use the preset directory 
if(DEFINED PLUSHIE_PRESET_NAME)
  message(STATUS "[Plushie::Root] preset '${PLUSHIE_PRESET_NAME}' (generator: ${CMAKE_GENERATOR})")
  list(APPEND CMAKE_MODULE_PATH ${PLUSHIE_LOCAL_MODULE_DIR})
else()
  message(WARNING
    "Plushie: no preset selected. Configure with a preset instead, e.g.\n"
    "  cmake --preset windows-x64-msvc\n"
    "Run 'cmake --list-presets' to see the available ones.")
endif()

# --- Unity tests ---------------------------------------------------------------------------------------
# Unity test is the framework we use for testing
find_package(unity QUIET)
if(NOT unity_FOUND)
	# Does the user already unity checked out out on their machine? They can set it with 
	# PLUSHIE_UNITY_TEST_SRC_DIR. The usecase is for multiplatform builds without repulling
	if(DEFINED ENV{PLUSHIE_UNITY_TEST_SRC_DIR})
		message(VERBOSE "[Plushie::Root] Attempting to locate unity test from "
		"$ENV{PLUSHIE_UNITY_TEST_SRC_DIR}")

		if(NOT EXISTS "$ENV{PLUSHIE_UNITY_TEST_SRC_DIR}/CMakeLists.txt")
			message(FATAL_ERROR "[Plushie::Root] No cmake file detected in "
			"$ENV{PLUSHIE_UNITY_TEST_SRC_DIR}/CMakeLists.txt")
		endif()

		FetchContent_Declare(unity SOURCE_DIR $ENV{PLUSHIE_UNITY_TEST_SRC_DIR} 
			EXCLUDE_FROM_ALL OVERRIDE_FIND_PACKAGE)
	
	# User can define PLUSHIE_FETCH_UNITY_TEST environment variable to let cmake fetch it 
	# from github origin, this will pull for each build dir. Can be slow for multi target
	elseif(DEFINED ENV{PLUSHIE_FETCH_UNITY_TEST})
		message(VERBOSE "[Plushie::Root] Using fetch content for unity")
		FetchContent_Declare(unity GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
			GIT_TAG        v2.6.1 
			GIT_SHALLOW    TRUE
			EXCLUDE_FROM_ALL
			OVERRIDE_FIND_PACKAGE)
	
	# No specified method for unity
	else()
		message(FATAL_ERROR "No specified search method for Unity test use one of the following\n"
		"  `PLUSHIE_UNITY_TEST_SRC_DIR`: Use local source checkout\n"
		"  `PLUSHIE_FETCH_UNITY_TEST`  : Fetch from github in build tree")
	endif()
	
	# Now make it available
	FetchContent_MakeAvailable(unity)

else()
	message(VERBOSE "[Plushie::Root] Found unity test automatically")
endif()

# Useful for editing any particular properties for unity test
get_target_property(_unity_target unity::framework ALIASED_TARGET)
if(NOT _unity_target)
	set(_unity_target unity::framework)
endif()
get_target_property(_unity_imported ${_unity_target} IMPORTED)

# Clang-cl (Clang with the MSVC frontend) has an bug with not guarding noreturn
# awaiting merge: https://github.com/ThrowTheSwitch/Unity/pull/863
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
	target_compile_definitions(${_unity_target} INTERFACE UNITY_NORETURN=_Noreturn)

	# When built from source: unity's CMakeLists passes -Wall -Werror for Clang, and
	# under clang-cl -Wall means -Weverything. Silence warnings in third-party code.
	if(NOT _unity_imported)
		target_compile_definitions(${_unity_target} PRIVATE UNITY_NORETURN=_Noreturn)
		target_compile_options(${_unity_target} PRIVATE -w)
	endif()
endif()

# --- Plushie our modules -------------------------------------------------------------------------------

# We declare all of the modules first and override find_package, this way all the modules can use 
# find_package for their dependency management so that if a user has prebuilts installed for a 
# component it will still build, but the monorepo will build everything from source
#
# For example: PlushieCStl will check for PlushieStartup as an optional package via 
#  `find_package( Plushie::Startup QUIET )` but from this monorepo, it will build from source
set(_plushie_packages "")

foreach(component_dir_name modules modules_lab)
	
	set(_source_dir ${CMAKE_CURRENT_LIST_DIR}/${component_dir_name})
	if(NOT EXISTS ${_source_dir})
		continue()
	endif()
	file(GLOB _child_entries RELATIVE "${_source_dir}" "${_source_dir}/*")
	foreach(_entry IN LISTS _child_entries)
		# set abs path
		set(_path ${_source_dir}/${_entry})

		# Now we can try and get the package name
		unset(PLUSHIE_PACKAGE_NAME)
		include("${_path}/cmake/PlushiePackageName.cmake" OPTIONAL) 
		if(NOT DEFINED PLUSHIE_PACKAGE_NAME OR "${PLUSHIE_PACKAGE_NAME}" STREQUAL "")
			continue()
		endif()

		FetchContent_Declare(${PLUSHIE_PACKAGE_NAME}
        	SOURCE_DIR "${_path}"
        	OVERRIDE_FIND_PACKAGE)
    	list(APPEND _plushie_packages ${PLUSHIE_PACKAGE_NAME})

		message(STATUS "[Plushie::Root] Adding package ${PLUSHIE_PACKAGE_NAME} via "
			"${component_dir_name}/${_entry}")
		unset(PLUSHIE_PACKAGE_NAME)

	endforeach()

endforeach()

# Make all the packages available
FetchContent_MakeAvailable(${_plushie_packages})


# --- Plushie Packaging ---------------------------------------------------------------------------------

# Generate the project package name
set(PLUSHIE_PACKAGE_FILE_NAME "plushie-host-${CMAKE_HOST_SYSTEM_NAME}-target")
if(DEFINED PLUSHIE_PRESET_NAME)
  set(PLUSHIE_PACKAGE_FILE_NAME "${PLUSHIE_PACKAGE_FILE_NAME}-${PLUSHIE_PRESET_NAME}-${PROJECT_VERSION}")
else()
  set(PLUSHIE_PACKAGE_FILE_NAME "${PLUSHIE_PACKAGE_FILE_NAME}-${CMAKE_SYSTEM_NAME}-${PROJECT_VERSION}")
endif()

# Configure CPack
string(TOLOWER "${PLUSHIE_PACKAGE_FILE_NAME}" CPACK_PACKAGE_FILE_NAME )
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_NAME "plushie")
set(CPACK_PACKAGE_VENDOR "Plushie")
include(CPack)