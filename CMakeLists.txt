# Top-level CMake for scanning subdirectories that contain CMakeLists.txt
cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)
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

# Override the find package behaviour for the modules which we build 
include(FetchContent)
FetchContent_Declare(PlushieCStl SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/modules/plushie_cstl OVERRIDE_FIND_PACKAGE)
FetchContent_MakeAvailable(PlushieCStl)




set(plushie_search_dirs
	modules
	modules_lab
	games)

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


set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_NAME "plushie")
set(CPACK_PACKAGE_VENDOR "Plushie")
include(CPack)