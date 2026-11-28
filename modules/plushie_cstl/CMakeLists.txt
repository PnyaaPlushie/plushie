###############################################################################################
# Plushie C STL
# Entry CMakeLists file for header only additional c libs that I reuse
# It can be used as a header only, or a packaged lib if you want everything
#
# The different configurations of using PlushieCstl are called flavours, they define an axis
# of availability. 
#
# PlushieOutputFlavour:
# 1. Header
# 2. Static
# 3. Shared
#
# PlushieLibcFlabour
# 1. Full    : Use libc for implementation and startup
# 2. PreMain : Use libc for startup but not for implementation
# 3. Header  : No linking of libc but common headers are available i.e <stdint.h>
# 4. None    : No linking or headers for libc available at all i.e Kernel
#
#
# However the header only implementations do not define a Libc flavour as that is up to the 
# end user application 
###############################################################################################
cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)
set(PlushieCStlVersion 0.1.0)


option(PLUSHIE_CSTL_BUILD_OUTPUT_FLAVOUR_HEADER  "Include Header only flavours"              ON  )
option(PLUSHIE_CSTL_BUILD_OUTPUT_FLAVOUR_STATIC  "Include Static library flavours"           OFF )
option(PLUSHIE_CSTL_BUILD_OUTPUT_FLAVOUR_SHARED  "Include Shared library flavours"           OFF )
option(PLUSHIE_CSTL_BUILD_LIBC_FLAVOUR_FULL      "Use libc for init and implementation"      ON  )
option(PLUSHIE_CSTL_BUILD_LIBC_FLAVOUR_PREMAIN   "Use libc for init, not for implementation" OFF )
option(PLUSHIE_CSTL_BUILD_LIBC_FLAVOUR_HEADER    "Don't link to libc, provide headers"       OFF )
option(PLUSHIE_CSTL_BUILD_LIBC_FLAVOUR_NONE      "Don't link to libc or provide headers"     OFF )

option(PLUSHIE_CSTL_INCLUDE_INSTALL       "Include the install target for Plushie Cstl"   ON )
option(PLUSHIE_CSTL_INSTALL_FOR_LOCAL_DEV "Generate find_package to use dev source files" OFF)

project(PlushieCStl VERSION ${PlushieCStlVersion} LANGUAGES CXX C)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

###############################################################################################
# Helpers
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

# List of targets built in this subdirectory
set(plushie_cstl_targets "")

###############################################################################################
# Function to add one target
###############################################################################################

# cstl_add_lib
# Function which is used to one version of the library to be globbed
# CSTL_FLAVOUR
function(cstl_add_lib)
	cmake_parse_arguments(T "" "CSTL_FLAVOUR;LIBC_FLAVOUR" "" ${ARGN})

	set(target_name PlushieCStl_${T_CSTL_FLAVOUR})
	if(DEFINED T_LIBC_FLAVOUR)
		set(target_name "${target_name}_Libc${T_LIBC_FLAVOUR}")
	endif()

	if("${T_CSTL_FLAVOUR}" STREQUAL "Header")  
	# --- Header ------------------------------------------------------
		add_library(${target_name} INTERFACE)
		target_include_directories(${target_name} INTERFACE ${plushie_cstl_public_include_dirs}) 
	else()
	# --- Static ------------------------------------------------------
		if("${T_CSTL_FLAVOUR}" STREQUAL "Static")
			add_library(${target_name} STATIC 
				${plushie_cstl_public_headers} ${plushie_cstl_private_sources})	
			target_compile_definitions(${target_name} 
				PUBLIC PLUSHIE_CSTL_USAGE_MODE=PLUSHIE_CSTL_USAGE_MODE_STATIC)
		
	# --- Shared ------------------------------------------------------
		elseif("${T_CSTL_FLAVOUR}" STREQUAL "Shared")
			add_library(${target_name} SHARED 
				${plushie_cstl_public_headers} ${plushie_cstl_private_sources})
			target_compile_definitions(${target_name} 
				PRIVATE	  PLUSHIE_CSTL_USAGE_MODE=PLUSHIE_CSTL_USAGE_MODE_SHARED_BUILD
				INTERFACE PLUSHIE_CSTL_USAGE_MODE=PLUSHIE_CSTL_USAGE_MODE_SHARED)

		else()
			message(FATAL_ERROR "[Plushie::CStl] Unkown flavour")
		endif()
	# --- Static + Shared ---------------------------------------------
		target_include_directories(${target_name} PUBLIC ${plushie_cstl_public_include_dirs}) 
	endif()

	# --- Header + Static + Shared ------------------------------------
	message(STATUS "[Plushie::CStl] ${target_name}")
	# Extract the export name
	# i.e PlushieCStl_Static -> CStl_Static
	string(REPLACE "Plushie" "" target_export_name "${target_name}")

	# Extract the library name
	# i.e PlushieCStl_Static -> PlushieCStlStatic
	string(REPLACE "_" "" target_output_name "${target_name}")

	set_target_properties(${target_name} PROPERTIES 
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

	# Same name in tree as the installed version
	add_library(Plushie::${target_export_name} ALIAS ${target_name})

	if(${PLUSHIE_CSTL_INCLUDE_INSTALL})
		install(TARGETS ${target_name} EXPORT PlushieCStlTargets
			LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
			ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
			RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
			INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	endif()
	
	# Add to the lists of targets
	set(plushie_cstl_targets ${plushie_cstl_targets} ${target_name} PARENT_SCOPE)

endfunction()

# Alias targets aren't exported so we add a real interface target so the user can
# have different names for default accesses
# cstl_define_alias(CSTL_FULL_NAME PlushieCStl_Static_LibcFull ALIAS_LIST PlushieCStl_Static)
#   Each alias becomes a real INTERFACE target linking the full target, it gets a
#   Plushie:: alias in tree, and is exported as Plushie::<name without Plushie>
#   Skipped if the full target was not built
function(cstl_define_alias)
	cmake_parse_arguments(T "" "CSTL_FULL_NAME" "ALIAS_LIST" ${ARGN})
	if(NOT TARGET ${T_CSTL_FULL_NAME})
		return()
	endif()

	foreach(_alias ${T_ALIAS_LIST})
		add_library(${_alias} INTERFACE)
		target_link_libraries(${_alias} INTERFACE ${T_CSTL_FULL_NAME})

		# i.e PlushieCStl_Static -> CStl_Static
		string(REPLACE "Plushie" "" _alias_export_name "${_alias}")
		set_target_properties(${_alias} PROPERTIES
			EXPORT_NAME ${_alias_export_name}
			FOLDER      "plushie/plushie_cstl")

		# Same name in tree as the installed version
		add_library(Plushie::${_alias_export_name} ALIAS ${_alias})

		message(STATUS "[Plushie::CStl] ${_alias} -> ${T_CSTL_FULL_NAME}")

		if(${PLUSHIE_CSTL_INCLUDE_INSTALL})
			install(TARGETS ${_alias} EXPORT PlushieCStlTargets)
		endif()
	endforeach()

endfunction()

###############################################################################################
# Define library source files
###############################################################################################

if(${PLUSHIE_CSTL_BUILD_OUTPUT_FLAVOUR_HEADER})
cstl_add_lib(
	CSTL_FLAVOUR Header
)
endif()



foreach(libc_flavour Full Premain Header)
	
	string(TOUPPER ${libc_flavour} UPPER_LIBC_FLAVOUR)
	if(NOT ${PLUSHIE_CSTL_BUILD_LIBC_FLAVOUR_${UPPER_LIBC_FLAVOUR}})
		continue()
	endif()
	if(${PLUSHIE_CSTL_BUILD_OUTPUT_FLAVOUR_STATIC})
	cstl_add_lib(
		CSTL_FLAVOUR Static
		LIBC_FLAVOUR ${libc_flavour}
	)
	endif()

	if(${PLUSHIE_CSTL_BUILD_OUTPUT_FLAVOUR_SHARED})
	cstl_add_lib(
		CSTL_FLAVOUR Shared
		LIBC_FLAVOUR ${libc_flavour}
	)
	endif()

endforeach()

###############################################################################################
# Default names, forward to the most common flavour of each output
###############################################################################################

cstl_define_alias(
	CSTL_FULL_NAME PlushieCStl_Header
	ALIAS_LIST     PlushieCStl
)

cstl_define_alias(
	CSTL_FULL_NAME PlushieCStl_Static_LibcFull
	ALIAS_LIST     PlushieCStl_Static
)

cstl_define_alias(
	CSTL_FULL_NAME PlushieCStl_Shared_LibcFull
	ALIAS_LIST     PlushieCStl_Shared
)

###############################################################################################
# Install logic
###############################################################################################

if(${PLUSHIE_CSTL_INCLUDE_INSTALL} AND plushie_cstl_targets)
	message(STATUS "[Plushie::CStl] Including the install targets")
	

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