###############################################################################################
# Plushie C STL
# Entry CMakeLists file for header only additional c libs that I reuse
# It can be used as a header only, or a packaged lib if you want everything
###############################################################################################
cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)
set(PlushieCStlVersion 0.1.0)

project(PlushieCStl VERSION ${PlushieCStlVersion} LANGUAGES CXX C)
if(TARGET PlushieCstlGuard)
	return()
else()
	add_library(PlushieCstlGuard INTERFACE)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
option(PLUSHIE_CSTL_BUILD_HEADER_ONLY     "Include the header only library version"       ON )
option(PLUSHIE_CSTL_BUILD_SHARED          "Include the shared library version"            OFF)
option(PLUSHIE_CSTL_BUILD_STATIC          "Include the static library version"            OFF)
option(PLUSHIE_CSTL_INCLUDE_INSTALL       "Include the install target for Plushie Cstl"   ON )
option(PLUSHIE_CSTL_INSTALL_FOR_LOCAL_DEV "Generate find_package to use dev source files" OFF)

option(PLUSHIE_CSTL_HAS_LIBC_RUNTIME      "Build cstl using regular libc runtime"         ON )
option(PLUSHIE_CSTL_HAS_LIBC_HEADERS      "Build cstl using libc stl headers"             ON )
option(PLUSHIE_CSTL_HAS_LIBC_STARTUP      "If false includes libc startup into build"     ON )

# List of targets built in this subdirectory
set(plushie_cstl_targets "")

###############################################################################################
# Define library source files
###############################################################################################

file(GLOB_RECURSE plushie_cstl_public_headers CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_LIST_DIR}/include/plushie/*h"
	)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/include" FILES ${plushie_cstl_public_headers})

set(plushie_cstl_private_sources
	${CMAKE_CURRENT_LIST_DIR}/source/plushie_cstl_impl.c)

set(plushie_cstl_public_include_dirs
	# Used with `add_subdirectory` or `FetchContent` 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

	# Used with find_package from install 
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


###############################################################################################
# Header only library
###############################################################################################

if(${PLUSHIE_CSTL_BUILD_HEADER_ONLY})
	add_library(PlushieCStl INTERFACE)

	add_library(Plushie::CStl ALIAS PlushieCStl)
	list(APPEND plushie_cstl_targets PlushieCStl)

endif()

###############################################################################################
# Shared Lib
###############################################################################################

if(${PLUSHIE_CSTL_BUILD_SHARED})
	add_library(PlushieCStl_Shared SHARED
		${plushie_cstl_public_headers}
		${plushie_cstl_private_sources})

	add_library(Plushie::CStl_Shared ALIAS PlushieCStl_Shared)
	list(APPEND plushie_cstl_targets PlushieCStl_Shared)

	# Shared export while compiling, import for others using it
	target_compile_definitions(PlushieCStl_Shared 
		PRIVATE	  PLUSHIE_CSTL_USAGE_MODE=PLUSHIE_CSTL_USAGE_MODE_SHARED_BUILD
		INTERFACE PLUSHIE_CSTL_USAGE_MODE=PLUSHIE_CSTL_USAGE_MODE_SHARED)
endif()


###############################################################################################
# Static lib
###############################################################################################

if(${PLUSHIE_CSTL_BUILD_STATIC})
	add_library(PlushieCStl_Static STATIC
		${plushie_cstl_public_headers}
		${plushie_cstl_private_sources})

	add_library(Plushie::CStl_Static ALIAS PlushieCStl_Static)
	list(APPEND plushie_cstl_targets PlushieCStl_Static)

	target_compile_definitions(PlushieCStl_Static 
		PUBLIC PLUSHIE_CSTL_USAGE_MODE=PLUSHIE_CSTL_USAGE_MODE_STATIC)
endif()

###############################################################################################
# Set the shared properties 
###############################################################################################

foreach(target ${plushie_cstl_targets})

	# Header only libraries must use interface
	if(${target} STREQUAL "PlushieCStl")	
		target_include_directories(${target} INTERFACE ${plushie_cstl_public_include_dirs}) 
	else()
		target_include_directories(${target} PUBLIC ${plushie_cstl_public_include_dirs}) 
	endif()

	# Extract the export name
	# i.e PlushieCStl_Static -> CStl_Static
	string(REPLACE "Plushie" "" target_export_name "${target}")

	# Extract the library name
	# i.e PlushieCStl_Static -> PlushieCStl
	string(REPLACE "_" "" target_output_name "${target}")

	# Set the properties for the non header libraries that output libraries
	set_target_properties(${target} PROPERTIES 
		EXPORT_NAME             ${target_export_name}
		OUTPUT_NAME             ${target_output_name}
		FOLDER                  "plushie/plushie_cstl"
		PREFIX                  "lib"
		IMPORT_PREFIX           "lib"
		DEBUG_POSTFIX           "Dbg"
		RELEASE_POSTFIX         "Rel"
		RELWITHDEBINFO_POSTFIX  "RelDbg"
		MINSIZEREL_POSTFIX      "RelMin"
		)

endforeach()

###############################################################################################
# Install logic
###############################################################################################

if(${PLUSHIE_CSTL_INCLUDE_INSTALL} AND plushie_cstl_targets)
	include(GNUInstallDirs)
	message(STATUS "[Plushie::CStl] Including the install targets")
	
	install(TARGETS ${plushie_cstl_targets} EXPORT PlushieCStlTargets
		LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

	if(NOT ${PLUSHIE_CSTL_INSTALL_FOR_LOCAL_DEV})
		# Performing a regular portable install, this includes copying all of the 
		# header files, and generating the regular packaging files

		# Copy all header files in the include directory 
		install(DIRECTORY include/plushie 
			DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

		# Relocatable targets file
		install(EXPORT PlushieCStlTargets
 		   FILE PlushieCStlTargets.cmake
    		NAMESPACE Plushie::
    		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plushie)

		# Use the .in template to generate the exported config
		configure_package_config_file(
			"${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlushieCStlConfig.cmake.in"
			"${CMAKE_CURRENT_BINARY_DIR}/PlushieCStlConfig.cmake"
			INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plushie
			PATH_VARS CMAKE_INSTALL_INCLUDEDIR)

		# Actually export the config file
		install(FILES
			"${CMAKE_CURRENT_BINARY_DIR}/PlushieCStlConfig.cmake"
			DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plushie)

	else()

		# Performing a dev local install, skips copying any files, it just aims to 
		# place the packaging config file and point them at this source dir

		# Use regular configure_file as we're making a "good enough" fake config
		set(_plushie_cstl_root ${CMAKE_CURRENT_LIST_DIR})
		configure_file(
			"${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlushieCStlConfig_LocalDev.cmake.in"
			"${CMAKE_CURRENT_BINARY_DIR}/PlushieCStlConfig.cmake"
			@ONLY)

		# Actually export the config file
		install(FILES
			"${CMAKE_CURRENT_BINARY_DIR}/PlushieCStlConfig.cmake"
			DESTINATION ${CMAKE_CURRENT_LIST_DIR})
	endif()

endif() # End of install