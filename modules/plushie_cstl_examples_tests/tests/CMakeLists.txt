# Create an executable for each test_*.c file in this directory, link it to
# Unity + PlushieCStl, and register it with CTest. Mirrors the examples layout:
# one executable per library variant that is available (header-only / static /
# shared). The header-only variant compiles the module implementations directly
# in-TU, gated by PLUSHIE_CSTL_TESTS_PROVIDE_IMPL.
file(GLOB TEST_C_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test_*.c")

function(snake_to_pascal_case input_string output_var)
    string(REPLACE "_" ";" string_list "${input_string}")
    set(result "")
    foreach(word IN LISTS string_list)
        string(SUBSTRING "${word}" 0 1 first_letter)
        string(TOUPPER "${first_letter}" first_letter)
        string(SUBSTRING "${word}" 1 -1 rest_of_word)
        string(APPEND result "${first_letter}${rest_of_word}")
    endforeach()
    set(${output_var} "${result}" PARENT_SCOPE)
endfunction()

if(${PLUSHIE_CSTL_TESTS_USE_STARTUP})
    find_package(PlushieStartupExe REQUIRED QUIET)
    include(PlushieStartupExeCMakeUtils)
endif()

foreach(_src IN LISTS TEST_C_SOURCES)

    get_filename_component(_name ${_src} NAME_WE)
    snake_to_pascal_case(${_name} _pascal)
    set(_target_base "PlushieCstl_Test_${_pascal}")

    set(_target_list "")

    # Header only version: compile the module impls into this TU.
    if(TARGET PlushieCStl)
        add_executable(${_target_base}_Header ${_src})
        target_link_libraries(${_target_base}_Header PRIVATE PlushieCStl unity::framework)
        target_compile_definitions(${_target_base}_Header PRIVATE
            PLUSHIE_CSTL_TESTS_PROVIDE_IMPL=1)
        list(APPEND _target_list ${_target_base}_Header)
        add_test(NAME ${_target_base}_Header COMMAND ${_target_base}_Header)
    endif()

    # Static version: implementations come from the compiled library.
    if(TARGET PlushieCStl_Static)
        add_executable(${_target_base}_Static ${_src})
        target_link_libraries(${_target_base}_Static PRIVATE PlushieCStl_Static unity::framework)
        list(APPEND _target_list ${_target_base}_Static)
        add_test(NAME ${_target_base}_Static COMMAND ${_target_base}_Static)
    endif()

    # Shared version: implementations come from the compiled library.
    if(TARGET PlushieCStl_Shared)
        add_executable(${_target_base}_Shared ${_src})
        target_link_libraries(${_target_base}_Shared PRIVATE PlushieCStl_Shared unity::framework)
        list(APPEND _target_list ${_target_base}_Shared)
        add_test(NAME ${_target_base}_Shared COMMAND ${_target_base}_Shared)
    endif()

    foreach(_target_entry IN LISTS _target_list)
        set_target_properties(${_target_entry} PROPERTIES
            FOLDER "plushie/plushie_cstl/tests")

        if(TARGET Plushie::StartupExe)
            target_link_libraries(${_target_entry} PRIVATE Plushie::StartupExe)
            plushie_startup_prepare_target(${_target_entry})
        endif()
    endforeach()

endforeach()
