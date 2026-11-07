



function(add_example_targets ExampleSuffix PlushieLibName LangExt)

	add_executable("plushie_bmp_example_01_into_buffer_${ExampleSuffix}"
		${CMAKE_CURRENT_LIST_DIR}/plushie_bmp_example_01_into_buffer.${LangExt})
	target_link_libraries("plushie_bmp_example_01_into_buffer_${ExampleSuffix}"
		PRIVATE ${PlushieLibName})

endfunction()



if(TARGET Plushie::CStl_Static)

	add_example_targets("static" Plushie::CStl_Static "c")

endif()


