# --- Linux using gcc
set(PLUSHIE_TARGET_TRIPLE_GNU  "x86_64-linux-gnu" )
set(PLUSHIE_TARGET_ARCH        "x64"              )
set(PLUSHIE_TARGET_OS          "Linux"            )

# --- Native on linux x64, otherwise cross compile with the prefixed gcc
# (apt: gcc-x86-64-linux-gnu g++-x86-64-linux-gnu)
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
    set(_gcc_prefix "")
else()
    set(CMAKE_SYSTEM_NAME      Linux )
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
    set(_gcc_prefix "${PLUSHIE_TARGET_TRIPLE_GNU}-")

    # run target binaries (tests, try_run) under qemu if available
    find_program(QEMU_X86_64 NAMES qemu-x86_64 qemu-x86_64-static QUIET)
    if(QEMU_X86_64)
        if(CMAKE_SYSROOT)
            set(CMAKE_CROSSCOMPILING_EMULATOR ${QEMU_X86_64} -L ${CMAKE_SYSROOT})
        else()
            set(CMAKE_CROSSCOMPILING_EMULATOR ${QEMU_X86_64} -L /usr/${PLUSHIE_TARGET_TRIPLE_GNU})
        endif()
    endif()
endif()


# --- find gcc ----------------------------------------------
find_program(GCC_CC  NAMES ${_gcc_prefix}gcc QUIET REQUIRED)
find_program(GCC_CXX NAMES ${_gcc_prefix}g++ QUIET         )


set(CMAKE_C_COMPILER ${GCC_CC})
if(GCC_CXX)
    set(CMAKE_CXX_COMPILER ${GCC_CXX})
endif()
