# Create an executable for each .c file in this directory and link to PlushieCStl
file(GLOB EXAMPLE_C_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")


function(snake_to_pascal_case input_string output_var)
    # 1. Replace underscores with semicolons to create a CMake list
    string(REPLACE "_" ";" string_list "${input_string}")
    
    set(result "")
    
    foreach(word IN LISTS string_list)
        # 2. Get the first letter and capitalize it
        string(SUBSTRING "${word}" 0 1 first_letter)
        string(TOUPPER "${first_letter}" first_letter)
        
        # 3. Get the rest of the word
        string(SUBSTRING "${word}" 1 -1 rest_of_word)
        
        # 4. Append capitalized word to the result
        string(APPEND result "${first_letter}${rest_of_word}")
    endforeach()
    
    set(${output_var} "${result}" PARENT_SCOPE)
endfunction()

if(${PLUSHIE_CSTL_EXAMPLES_USE_STARTUP})
    find_package(PlushieStartupExe REQUIRED QUIET)
    include(PlushieStartupExeCMakeUtils)
endif()


foreach(_src IN LISTS EXAMPLE_C_SOURCES)

	get_filename_component(_name ${_src} NAME_WE)
    snake_to_pascal_case(${_name} _target_name)
    set(_target_name "PlushieCstl_Example_${_target_name}")

    # Keep a list of targets
    set(_target_list "")

    # Header only version
    if(TARGET PlushieCStl)
        add_executable(${_target_name}_Header ${_src})
        target_link_libraries(${_target_name}_Header PRIVATE PlushieCStl)
        target_compile_definitions(${_target_name}_Header PRIVATE 
            PLUSHIE_CSTL_EXAMPLES_PROVIDE_IMPL)
        list(APPEND _target_list ${_target_name}_Header)
    endif()

    if(TARGET PlushieCStl_Static)
        add_executable(${_target_name}_Static ${_src})
        target_link_libraries(${_target_name}_Static PRIVATE PlushieCStl_Static)
        list(APPEND _target_list ${_target_name}_Static)
    endif()

    if(TARGET PlushieCStl_Shared)
        add_executable(${_target_name}_Shared ${_src})
        target_link_libraries(${_target_name}_Shared PRIVATE PlushieCStl_Shared)
        list(APPEND _target_list ${_target_name}_Shared)
    endif()

    # Now set the target properties for each one
    foreach(_target_entry IN LISTS _target_list)
        set_target_properties(${_target_entry} PROPERTIES
            FOLDER "plushie/plushie_cstl/examples")

        if(TARGET Plushie::StartupExe)
            target_link_libraries(${_target_entry} PRIVATE Plushie::StartupExe)
            plushie_startup_prepare_target(${_target_entry})
        endif()

    endforeach()

endforeach()