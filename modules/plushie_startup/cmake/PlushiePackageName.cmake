# Guard against very few supported presets
set(_plushie_startup_presets windows-x64-msvc)
if(NOT DEFINED PLUSHIE_PRESET_NAME)
    return()
endif()
if(NOT PLUSHIE_PRESET_NAME IN_LIST _plushie_startup_presets)
    message(STATUS "PlushieStartup: skipped for preset '${PLUSHIE_PRESET_NAME}'")
    return()
endif()

set(PLUSHIE_PACKAGE_NAME "PlushieStartup")



if(${PLUSHIE_ENABLE_FULL_BUILD_OPTIONS})

endif()