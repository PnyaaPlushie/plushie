set(PLUSHIE_TARGET_TRIPLE_LLVM "x86_64-pc-windows-msvc" )
set(PLUSHIE_TARGET_ARCH        "x64"                    )
set(PLUSHIE_TARGET_OS          "windows"                )
set(PLUSHIE_HOST_IS_TARGET     FALSE                    ) 

# Usually we'd confirm that are host doesn't match the target before setting the system name
# but in this instance, this toolchain is specifically for cross compiling, and we need to
# set it, or else CMake assumes the target is host (Most likely linux) and discards our choice
# of linker 
set(CMAKE_SYSTEM_NAME          Windows                  )
set(CMAKE_SYSTEM_VERSION       10.0                     )
set(CMAKE_SYSTEM_PROCESSOR     AMD64                    )

# --- xWin ---------------------------------------------------------------------------------------------
# We need to locate the user's xwin cache of the windows headers. 
# There is no predefined location for it though. We can list some 
# common ones, and let the user specify with an override
find_path(XWIN_DIR
    NAMES crt/include/vcruntime.h
    HINTS ENV XWIN_DIR
    PATHS
        $ENV{HOME}/.xwin-cache
        $ENV{HOME}/.xwin
        $ENV{HOME}/xwin-cache
        $ENV{HOME}/xwin
        $ENV{HOME}/.local/share/xwin-cache
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
    DOC "Where windows sdk and runtime are stored. xwin splat output dir.")

if(NOT XWIN_DIR)
    message(FATAL_ERROR "Failed to find xwin cache directory, either you need to rerun xwin, "
        "or you can specify to the output of xwin --splat via the environment variable "
        "\"XWIN_DIR\""
        "\n"
        "  export $XWIN_DIR=\"~/.local/share/xwin-cache\n" 
        "  xwin splat --output $XWIN_DIR\n" 
        "  xwin --cache-dir $XWIN_DIR splat\n")
endif()
set(CMAKE_FIND_ROOT_PATH ${XWIN_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# --- clang-cl ------------------------------------------------------------------------------------------
find_program(clang_cl_exe_path
    NAMES clang-cl
    NO_CMAKE_FIND_ROOT_PATH
    DOC "Clang with windows compatibility compiler"
)
if(NOT clang_cl_exe_path)
    message(FATAL_ERROR "Couldn't find clang-cl path")
endif()
set(CMAKE_C_COMPILER   ${clang_cl_exe_path})
set(CMAKE_CXX_COMPILER ${clang_cl_exe_path})
set(CMAKE_C_COMPILER_TARGET   ${PLUSHIE_TARGET_TRIPLE_LLVM})
set(CMAKE_CXX_COMPILER_TARGET ${PLUSHIE_TARGET_TRIPLE_LLVM})

# xwin headers. -imsvc marks them as system includes, like INCLUDE= on windows.
set(_xwin_compile_flags
    "-imsvc \"${XWIN_DIR}/crt/include\" -imsvc \"${XWIN_DIR}/sdk/include/ucrt\" -imsvc \"${XWIN_DIR}/sdk/include/um\" -imsvc \"${XWIN_DIR}/sdk/include/shared\"")
set(CMAKE_C_FLAGS_INIT   "${_xwin_compile_flags}")
set(CMAKE_CXX_FLAGS_INIT "${_xwin_compile_flags}")

# --- linker / tools --------------------------------------------------------------------------------
# With CMAKE_SYSTEM_NAME=Windows and an MSVC-like frontend, CMake invokes CMAKE_LINKER directly.
find_program(lld_link_exe_path NAMES lld-link   NO_CMAKE_FIND_ROOT_PATH DOC "lld (MSVC link.exe compatible)")
find_program(llvm_lib_exe_path NAMES llvm-lib   NO_CMAKE_FIND_ROOT_PATH DOC "llvm-lib (MSVC lib.exe compatible)")
find_program(llvm_rc_exe_path  NAMES llvm-rc    NO_CMAKE_FIND_ROOT_PATH DOC "llvm-rc (MSVC rc.exe compatible)")
find_program(llvm_mt_exe_path  NAMES llvm-mt    NO_CMAKE_FIND_ROOT_PATH DOC "llvm-mt (MSVC mt.exe compatible)")
foreach(_tool lld_link_exe_path llvm_lib_exe_path llvm_rc_exe_path llvm_mt_exe_path)
    if(NOT ${_tool})
        message(FATAL_ERROR "Couldn't find ${_tool}")
    endif()
endforeach()

set(CMAKE_LINKER      ${lld_link_exe_path} CACHE FILEPATH "" FORCE)
set(CMAKE_AR          ${llvm_lib_exe_path} CACHE FILEPATH "" FORCE)
set(CMAKE_RC_COMPILER ${llvm_rc_exe_path}  CACHE FILEPATH "" FORCE)
set(CMAKE_MT          ${llvm_mt_exe_path}  CACHE FILEPATH "" FORCE)

set(_xwin_link_flags
    "/libpath:\"${XWIN_DIR}/crt/lib/x86_64\" /libpath:\"${XWIN_DIR}/sdk/lib/ucrt/x86_64\" /libpath:\"${XWIN_DIR}/sdk/lib/um/x86_64\"")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${_xwin_link_flags}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_xwin_link_flags}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_xwin_link_flags}")

# --- runtime library ---------------------------------------------------------------------------------
# xwin only downloads the debug CRT (msvcrtd.lib, ucrtd.lib, ...) with --include-debug-libs.
# Without it Debug configs (/MDd) can't link, so fall back to the release DLL runtime.
if(NOT EXISTS "${XWIN_DIR}/crt/lib/x86_64/msvcrtd.lib")
    message(STATUS "xwin cache has no debug CRT, using /MD for all configs. "
        "For /MDd re-run: xwin --include-debug-libs splat --output \"${XWIN_DIR}\"")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()
