# --- Linuix using clang
set(PLUSHIE_TARGET_TRIPLE_LLVM "x86_64-unknown-linux-gnu" )
set(PLUSHIE_TARGET_ARCH        "x64"                      )
set(PLUSHIE_TARGET_OS          "Linux"                    )


# --- find clang --------------------------------------------
find_program(CLANG_CC  NAMES clang   QUIET REQUIRED)
find_package(CLANG_CXX NAMES clang++ QUIET         )


set(CMAKE_C_COMPILER_TARGET   ${PLUSHIE_TARGET_TRIPLE_LLVM})
set(CMAKE_CXX_COMPILER_TARGET ${PLUSHIE_TARGET_TRIPLE_LLVM})
set(CMAKE_ASM_COMPILER_TARGET ${PLUSHIE_TARGET_TRIPLE_LLVM})
set(CMAKE_C_COMPILER ${CLANG_CC})
if(CLANG_CXX)
    set(CMAKE_CXX_COMPILER ${CLANG_CXX})
endif()