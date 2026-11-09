# Top-level CMake for scanning subdirectories that contain CMakeLists.txt
cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)
option(PLUSHIE_ROOT_CSTL_ONLY "Should plushie root only configure for cstl for smaller build" OFF)

enable_testing()
project(Plushie VERSION 0.1.0 LANGUAGES C)

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


include(FetchContent)

# Use the environment variables to check if the user wants to fetch unitytest
# as part of the build, useful in CI, since we don't have to manually fetch
if(DEFINED ENV{PLUSHIE_FETCH_UNITY_TEST})
	message(STATUS ["Plushie::Root Using fetch content for unity test"])
	FetchContent_Declare(unity GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
		GIT_TAG        v2.6.1 
		GIT_SHALLOW    TRUE
		EXCLUDE_FROM_ALL
		OVERRIDE_FIND_PACKAGE)
  FetchContent_MakeAvailable(unity)
endif()
find_package(unity REQUIRED QUIET)

# Override the find package behaviour for the modules which we build 
FetchContent_Declare(PlushieCStl SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/modules/plushie_cstl OVERRIDE_FIND_PACKAGE)
FetchContent_MakeAvailable(PlushieCStl)


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