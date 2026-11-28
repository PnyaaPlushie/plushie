include_guard(GLOBAL)

function(plushie_startup_apply_target_options target_name)

	# Link to our library 
	target_link_libraries(${target_name} PRIVATE PlushieStartup)

	# Remove libc and specify our custom startup entry point name
	if(MSVC)
		target_link_options(${target_name} PRIVATE 
			/ENTRY:plushie_startup
			/NODEFAULTLIB)
	else()
		target_link_options(${target_name} PRIVATE 
			-nostdlib 
			-Wl,-e,plushie_startup)
	endif()

endfunction()