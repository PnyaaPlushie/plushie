###############################################################################################
# Plushie Startup
# Replaces the libc premain functionality to call into the main function 
###############################################################################################

option(PLUSHIE_STARTUP_BUILD_TESTS    "include install target for tests"     ON )
option(PLUSHIE_STARTUP_INSTALL_TESTS    "include install target for tests"     ON )
option(PLUSHIE_STARTUP_INCLUDE_CXX_TESTS "" OFF )

# --- Derive the default assembly language dialect -------------------------------------------
if(MSVC)
	if(CMAKE_C_COMPILER_ARCHITECTURE_ID STREQUAL "ARM64")
		set(_plushie_asm_default ASM_MARMASM) # armasm64, cmake 3.26+
	else()
		set(_plushie_asm_default ASM_MASM)    # ml64
	endif()
else()
	set(_plushie_asm_default ASM)             # .S through c compiler driver
endif()


set(PLUSHIE_STARTUP_ASM_DIALECT ${_plushie_asm_default} CACHE STRING "Dialect to use for assembly")
enable_language(${PLUSHIE_STARTUP_ASM_DIALECT})

# --- Select input file ----------------------------------------------------------------------

if(PLUSHIE_STARTUP_ASM_DIALECT STREQUAL "ASM_MARMASM")
	set(_plushie_startup_asm_file ${CMAKE_CURRENT_LIST_DIR}/source/startup_arm64_masm.asm)
elseif(PLUSHIE_STARTUP_ASM_DIALECT STREQUAL "ASM_MASM")
	set(_plushie_startup_asm_file ${CMAKE_CURRENT_LIST_DIR}/source/startup_x64_masm.asm)
elseif(PLUSHIE_STARTUP_ASM_DIALECT STREQUAL "ASM")
	set(_plushie_startup_asm_file ${CMAKE_CURRENT_LIST_DIR}/source/startup_via_cc.S)
else()
	message(FATAL_ERROR "[Plushie::Startup] no asm input file for dialect "
		"\"${PLUSHIE_STARTUP_ASM_DIALECT}\"")
endif()

# --- Define the library ----------------------------------------------------------------------

add_library(PlushieStartup OBJECT ${_plushie_startup_asm_file})

set_target_properties(PlushieStartup PROPERTIES 
	FOLDER                  "plushie/plushie_startup"
	PREFIX                  "lib"
	IMPORT_PREFIX           "lib"
	DEBUG_POSTFIX           "Dbg"
	RELEASE_POSTFIX         "Rel"
	RELWITHDEBINFO_POSTFIX  "RelDbg"
	MINSIZEREL_POSTFIX      "RelMin"

	# We pin the linker language as this object gets linked into user projects
	# We don't want cmake to accidentally pick the wrong kind
	LINKER_LANGUAGE         C
	)

# Add the helpers for anyone that tries to call us 
include("${CMAKE_CURRENT_LIST_DIR}/cmake/PlushieStartupHelpers.cmake")

# For msvc and masm, Visual studio seems to incorrectly not generate debug information 
target_compile_options(PlushieStartup PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:ASM_MASM>,$<CONFIG:Debug,RelWithDebInfo>>:/Zi>)

# --- Add tests ----------------------------------------------------------------------
 if(PLUSHIE_STARTUP_BUILD_TESTS)
 	add_subdirectory(tests)
 endif()