# --- Linuix using clang
set(PLUSHIE_TARGET_TRIPLE_LLVM "aarch64-unknown-linux-gnu" )
set(PLUSHIE_TARGET_ARCH        "arm64"                     )
set(PLUSHIE_TARGET_OS          "Linux"                     )

# --- cross compile only when the host isn't already linux arm64
if(NOT (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$"))
    set(CMAKE_SYSTEM_NAME      Linux  )
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(CMAKE_LINKER_TYPE      LLD    )

    # run target binaries (tests, try_run) under qemu if available
    find_program(QEMU_AARCH64 NAMES qemu-aarch64 qemu-aarch64-static QUIET)
    if(QEMU_AARCH64)
        if(CMAKE_SYSROOT)
            set(CMAKE_CROSSCOMPILING_EMULATOR ${QEMU_AARCH64} -L ${CMAKE_SYSROOT})
        else()
            set(CMAKE_CROSSCOMPILING_EMULATOR ${QEMU_AARCH64} -L /usr/aarch64-linux-gnu)
        endif()
    endif()
endif()


# --- find clang --------------------------------------------
find_program(CLANG_CC  NAMES clang   QUIET REQUIRED)
find_program(CLANG_CXX NAMES clang++ QUIET         )


set(CMAKE_C_COMPILER_TARGET   ${PLUSHIE_TARGET_TRIPLE_LLVM})
set(CMAKE_CXX_COMPILER_TARGET ${PLUSHIE_TARGET_TRIPLE_LLVM})
set(CMAKE_ASM_COMPILER_TARGET ${PLUSHIE_TARGET_TRIPLE_LLVM})
set(CMAKE_C_COMPILER ${CLANG_CC})
if(CLANG_CXX)
    set(CMAKE_CXX_COMPILER ${CLANG_CXX})
endif()