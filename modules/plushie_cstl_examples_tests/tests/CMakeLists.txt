###############################################################################################
# Expects to be able to find the PlushieCStl, but will check the flavour of cstl from 
# the targets provided (Header, Static, Shared)
#
# We want to primarily use the header flavour for testing as it checks the implementations,
# and is also the primary expected use case,
#
# For the tests, we want to handle a series of given permutations/combinations of PlushieCStl
# and the majority of that is focused on the header flavour
# - Header only flavour gets the most testing, because it also implies the testing of the
#   actual implementation, AND our code will end up in the user's translation units which
#   and the code generation can be effected by more settings, such as the standard etc.
#
# - PlushieCStl will be designed to work without libc for the full bringup, so one of the
#   flavours to check should also the libc availability
#    - Full    : Has full libc available
#    - Startup : Has premain available, but no runtime
#    - Headers : Only has libc headers for types
#    - None    : Has 0 libc, no headers, no runtime
#
# --- Testing Matrix --------------------------------------------------
# We test a matrix of possible configurations for each of the applications.
# 1. Compiler needs to be configured in the workflow
# 2. Available C/C++ standards can be configured
# 3. Linking format (header, static, shared)
# 4. libc availability (full, just start, none)
###############################################################################################

include(GNUInstallDirs)
find_package(PlushieCStl    QUIET REQUIRED )
find_package(PlushieStartup QUIET          )
find_package(unity          QUIET REQUIRED )
enable_testing()

# Startup lib is not always supplied, but we should test it when we can. if we do not
# have startup, then we can still check what happens when we don't use libc in our 
# components, but still use the _startup code from the vendor
set(cstl_libc_flavours Full Premain)
if(PlushieStartup_FOUND)
    list(APPEND cstl_libc_flavours None)
endif()

###############################################################################################
# 0. Helpers
###############################################################################################

# Available languages and their file extension
get_property(_enabled_langs GLOBAL PROPERTY ENABLED_LANGUAGES)
set(_ext_C   c)
set(_ext_CXX cpp)

# Get the list of supported c/cxx stls for the compiler. 
# MSVC will produce identical exes since it doesn't have strict standard support
# especially for the earlier and newer versions, but we chosse to duplicate rather
# than under test some given combination
foreach(lang C CXX)
	set(_stds "${CMAKE_${lang}_COMPILE_FEATURES}")
	list(FILTER _stds INCLUDE REGEX "^c(xx)?_std_[0-9]+$")
	list(TRANSFORM _stds REPLACE "^c(xx)?_std_" "")
	message(STATUS "[Plushie::Cstl] ${lang} standards (${CMAKE_${lang}_COMPILER_ID} " 
	"${CMAKE_${lang}_COMPILER_VERSION}): ${_stds}")
	set(_stds_${lang} ${_stds})
endforeach()

###############################################################################################
# 0. Shared test inputs
###############################################################################################

# Every test_*.c is registered through main, so each exe gets main + all of the tests
file(GLOB cstl_test_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test_*.c")

if(${PLUSHIE_CSTL_TESTS_USE_STARTUP})
	find_package(PlushieStartupExe REQUIRED QUIET)
	include(PlushieStartupExeCMakeUtils)
endif()

# Filled by cstl_add_test, used by the install logic
set(cstl_test_targets        "") # Tests that don't need any extra runtime files
set(cstl_test_shared_targets "") # Tests that need the CStl shared lib next to them
set(cstl_test_shared_libs    "") # The CStl shared libs those tests use

###############################################################################################
# 0. Target creation function
###############################################################################################

# Add one test executable with it's CTest entries and install options
# cstl_add_test(CSTL_LINK_FLAVOUR Header|Static|Shared LIBC_FLAVOUR f LANG C|CXX [STD 17]
#               [SOURCES ...] [LABELS ...])
#   STD omitted = compiler default
#   SOURCES are extra sources on top of main + test_*.c
function(cstl_add_test)
    cmake_parse_arguments(T "" "CSTL_LINK_FLAVOUR;LIBC_FLAVOUR;LANG;STD" "SOURCES;LABELS" ${ARGN})

    # Full name of the CStl target to link against
    # Header : libc is chosen by the consumer (this test), so there is one target
    # Static / Shared : libc is baked into the library, so each libc flavour is its own target
    if(T_CSTL_LINK_FLAVOUR STREQUAL "Header")
        set(cstl_target PlushieCStl_Header)
    else()
        set(cstl_target PlushieCStl_${T_CSTL_LINK_FLAVOUR}_Libc${T_LIBC_FLAVOUR})
    endif()

    # Skip if that flavour of CStl was not built
    if(NOT TARGET ${cstl_target})
        message(VERBOSE "[Plushie::CStl::Tests] Skipping non-present target ${cstl_target}")
        return()
    endif()

    set(target_name "PlushieCStl_Test_${T_CSTL_LINK_FLAVOUR}_${T_LANG}${T_STD}_Libc${T_LIBC_FLAVOUR}")
    message(VERBOSE "[Plushie::CStl::Tests] ${target_name} -> ${cstl_target}")

    # --- Executable ------------------------------------------------------
    add_executable(${target_name}
        main.${_ext_${T_LANG}}
        tests_shared.h
        ${cstl_test_sources}
        ${T_SOURCES})

    target_link_libraries(${target_name} PRIVATE ${cstl_target} unity::framework)

    # Header only : the module implementations are compiled into the test TUs
    if(T_CSTL_LINK_FLAVOUR STREQUAL "Header")
        target_compile_definitions(${target_name} PRIVATE PLUSHIE_CSTL_TESTS_PROVIDE_IMPL=1)
    endif()

    if(DEFINED T_STD)
        set_target_properties(${target_name} PROPERTIES
            ${T_LANG}_STANDARD          ${T_STD}
            ${T_LANG}_STANDARD_REQUIRED ON
            ${T_LANG}_EXTENSIONS        OFF)
    endif()

    set_target_properties(${target_name} PROPERTIES
        FOLDER                  "plushie/plushie_cstl/tests"
        DEBUG_POSTFIX           "Dbg"
        RELEASE_POSTFIX         "Rel"
        RELWITHDEBINFO_POSTFIX  "RelDbg"
        MINSIZEREL_POSTFIX      "RelMin")

    if(TARGET Plushie::StartupExe)
        target_link_libraries(${target_name} PRIVATE Plushie::StartupExe)
        plushie_startup_prepare_target(${target_name})
    endif()

    # --- CTest -----------------------------------------------------------
    add_test(NAME ${target_name} COMMAND ${target_name})
    set_tests_properties(${target_name} PROPERTIES
        LABELS "${T_CSTL_LINK_FLAVOUR};${T_LANG};Libc${T_LIBC_FLAVOUR};${T_LABELS}")

    # --- Install lists ---------------------------------------------------
    if(T_CSTL_LINK_FLAVOUR STREQUAL "Shared")
        set(cstl_test_shared_targets ${cstl_test_shared_targets} ${target_name} PARENT_SCOPE)
        if(NOT cstl_target IN_LIST cstl_test_shared_libs)
            set(cstl_test_shared_libs ${cstl_test_shared_libs} ${cstl_target} PARENT_SCOPE)
        endif()
    else()
        set(cstl_test_targets ${cstl_test_targets} ${target_name} PARENT_SCOPE)
    endif()

endfunction()

###############################################################################################
# 1. Test matrix
# Header : every standard, the code is compiled in the consumers TU so the standard matters
# Static : One C consumer and one CXX consumer, oldest c, newest cxx
# Shared : Same as static
###############################################################################################

# Flavours that were not built are skipped inside cstl_add_test
foreach(cstl_link_flavour Header Static Shared)
    foreach(lang C CXX)
        # Check the language was actually enabled first
        if(NOT lang IN_LIST _enabled_langs)
            continue()
        endif()

        # Select the standards to test for this link flavour
        if(cstl_link_flavour STREQUAL "Header")
            set(_stds ${_stds_${lang}})
        elseif(lang STREQUAL "C")
            list(GET _stds_C 0 _stds)
        else()
            list(GET _stds_CXX -1 _stds)
        endif()

        foreach(std ${_stds})
            foreach(libc_flavour ${cstl_libc_flavours})

                cstl_add_test(
                    CSTL_LINK_FLAVOUR ${cstl_link_flavour}
                    LIBC_FLAVOUR      ${libc_flavour}
                    LANG              ${lang}
                    STD               ${std})

            endforeach()
        endforeach()
    endforeach()
endforeach()

list(LENGTH cstl_test_targets        _n_tests)
list(LENGTH cstl_test_shared_targets _n_shared_tests)
math(EXPR _n_tests "${_n_tests} + ${_n_shared_tests}")
message(STATUS "[Plushie::CStl::Tests] ${_n_tests} test executables")

###############################################################################################
# 2. Install
# Install the test exes along with the DLLs they need, so the installed / packaged copies
# run standalone (the build tree doesn't need this, the root CMakeLists puts all exes and
# DLLs in one bin folder).
###############################################################################################

if(POLICY CMP0207)
	cmake_policy(SET CMP0207 NEW) # Prevents warning on path seperator conversions
endif()

if(${PLUSHIE_CSTL_INSTALL_TESTS})

	if(cstl_test_targets)
		install(TARGETS ${cstl_test_targets}
			RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests)
	endif()

	if(cstl_test_shared_targets)
		# Runtime dependencies seem to fail to resolve
		if(CMAKE_CROSSCOMPILING)
			# Backup for cross compiling
			install(TARGETS ${cstl_test_shared_targets}
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests)

			foreach(_shared_test ${cstl_test_shared_targets})
				install(FILES $<TARGET_RUNTIME_DLLS:${_shared_test}>
					DESTINATION ${CMAKE_INSTALL_BINDIR}/tests)
			endforeach()
		else()
			install(TARGETS ${cstl_test_shared_libs} ${cstl_test_shared_targets}
				RUNTIME_DEPENDENCY_SET plushie_cstl_runtime_deps
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests)

			install(RUNTIME_DEPENDENCY_SET plushie_cstl_runtime_deps
				PRE_EXCLUDE_REGEXES  "api-ms-.*" "ext-ms-.*"            # Windows API sets
				POST_EXCLUDE_REGEXES ".*[/\\\\][Ss]ystem32[/\\\\].*"    # Windows system DLLs
				                     "^/lib/.*" "^/usr/lib/.*"          # Linux system libraries
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests)
		endif()
	endif()

endif()
