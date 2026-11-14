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
get_target_property(_unity_imported ${_unity_target} IMPORTED)
if(NOT _unity_target)
	set(_unity_target unity::framework)
endif()

# --- Plushie Packages Override -------------------------------------------------------------------------

# Override the find package behaviour for the modules which we build 
FetchContent_Declare(PlushieCStl SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/modules/plushie_cstl OVERRIDE_FIND_PACKAGE)
FetchContent_MakeAvailable(PlushieCStl)


# --- Plushie our modules -------------------------------------------------------------------------------
set(plushie_search_dirs
	modules
	modules_lab
	games)

if(${PLUSHIE_ROOT_CSTL_ONLY})
	add_subdirectory("modules/plushie_cstl")
	add_subdirectory("modules/plushie_cstl_examples_tests")
else()
	foreach(search_subdir ${plushie_search_dirs})
		
		# Search for different modules 
		set(source_dir ${CMAKE_CURRENT_LIST_DIR}/${search_subdir})
		if(EXISTS ${source_dir})
			message(STATUS "[Plushie::Root] Loading subdir ${search_subdir}")
		else()
			message(STATUS "[Plushie::Root] Skipping empty subdir ${search_subdir}")
			continue()
		endif()
	
		# Gather all of the subfolders of the modules
		file(GLOB CHILD_ENTRIES RELATIVE "${source_dir}" "${source_dir}/*")
	
		# For each subfile
		foreach(_entry IN LISTS CHILD_ENTRIES)
	
			# This should be the path to a subproject 
			set(_path "${source_dir}/${_entry}")		
			if(NOT (IS_DIRECTORY "${_path}"))
				continue()
			endif()
	
			# Skip obvious build/artifact directories
			if(_entry MATCHES "^(build|build_.*|\.git|\.vs)$")
				continue()
			endif()
	
			# Does this project have a CMake list file?
			if(NOT EXISTS "${_path}/CMakeLists.txt")
				continue()
			endif()
	
			message(STATUS "[Plushie::${search_subdir}] Adding subdirectory: ${_entry}")
			add_subdirectory(${_path})
	
	
		endforeach()
	endforeach()
endif()

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