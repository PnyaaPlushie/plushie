# MinGW-w64 targeting Windows x64 (x86_64-w64-mingw32).
#
#   Windows x64 host : native build (MSYS2, WinLibs, ...). Not a cross build.
#   Linux / macOS    : cross build with x86_64-w64-mingw32-* (apt/dnf mingw-w64, Homebrew).
#
# Options (pass with -D or a preset's cacheVariables; all optional):
#   PLUSHIE_MINGW_ROOT            Install root containing bin/. If set, ONLY that is searched.
#   PLUSHIE_MINGW_SEARCH_PATHS    bin/ folders tried in order when no root is given
#                                 (default: common install locations below), then PATH.
#   PLUSHIE_MINGW_SYSROOT         Target sysroot for find_* in cross builds
#                                 (default: first existing common location below).
#   PLUSHIE_MINGW_SUFFIX          Appended to gcc/g++, e.g. "-posix" for the Debian/Ubuntu
#                                 posix-threads build (needed for std::thread).
#   PLUSHIE_MINGW_SHARED_RUNTIME  ON = link libgcc/libstdc++/winpthread as DLLs.
#
# Found tools are cached: after changing any of these, configure with --fresh.

set(_triple x86_64-w64-mingw32)

# try_compile projects re-read this file without the parent's cache: pass options through.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
  PLUSHIE_MINGW_ROOT PLUSHIE_MINGW_SEARCH_PATHS PLUSHIE_MINGW_SYSROOT
  PLUSHIE_MINGW_SUFFIX PLUSHIE_MINGW_SHARED_RUNTIME
  PLUSHIE_MINGW_CC PLUSHIE_MINGW_CXX PLUSHIE_MINGW_RC)
list(REMOVE_DUPLICATES CMAKE_TRY_COMPILE_PLATFORM_VARIABLES)

# -------------------------------------------------------------------------------------- 
# Where to look when no root is given, in order 
if(NOT DEFINED PLUSHIE_MINGW_SEARCH_PATHS)
  if(CMAKE_HOST_WIN32)
    file(TO_CMAKE_PATH "$ENV{USERPROFILE}" _home)
    set(PLUSHIE_MINGW_SEARCH_PATHS
      "C:/msys64/ucrt64/bin"                                          # MSYS2 UCRT64
      "C:/msys64/mingw64/bin"                                         # MSYS2 MINGW64
      "${_home}/scoop/apps/mingw/current/bin"                         # Scoop
      "C:/ProgramData/chocolatey/lib/mingw/tools/install/mingw64/bin" # Chocolatey
      "C:/ProgramData/mingw64/mingw64/bin"                            # Choco
      "C:/mingw64/bin")                                               # WinLibs / manual
  else()
    set(PLUSHIE_MINGW_SEARCH_PATHS
      "/usr/bin"                                                    # apt / dnf
      "/opt/homebrew/bin"                                           # Homebrew (Apple silicon)
      "/usr/local/bin")                                             # Homebrew (Intel) / manual
  endif()
endif()

# -------------------------------------------------------------------------------------- 
# --- Find the compiler: prefixed name only, so a plain `gcc` for another target never
# gets  used (or Strawberry Perl's gcc on PATH) is never picked by accident.
set(_cc_name  "${_triple}-gcc${PLUSHIE_MINGW_SUFFIX}")
set(_cxx_name "${_triple}-g++${PLUSHIE_MINGW_SUFFIX}")

if(PLUSHIE_MINGW_ROOT)
  file(TO_CMAKE_PATH "${PLUSHIE_MINGW_ROOT}" _root)
  find_program(PLUSHIE_MINGW_CC NAMES ${_cc_name} PATHS "${_root}/bin" NO_DEFAULT_PATH)
  set(_where "in ${_root}/bin")
else()
  find_program(PLUSHIE_MINGW_CC NAMES ${_cc_name} PATHS ${PLUSHIE_MINGW_SEARCH_PATHS} NO_DEFAULT_PATH)
  find_program(PLUSHIE_MINGW_CC NAMES ${_cc_name})   # PATH, only if the list found nothing
  set(_where "in PLUSHIE_MINGW_SEARCH_PATHS or on PATH")
endif()

if(NOT PLUSHIE_MINGW_CC)
  message(FATAL_ERROR
    "windows-x64-mingw: could not find '${_cc_name}' ${_where}.\n"
    "Install MinGW-w64, or pass -DPLUSHIE_MINGW_ROOT=<install root containing bin/>.")
endif()

# g++ and windres from the same folder, so two different installs never get mixed.
cmake_path(GET PLUSHIE_MINGW_CC PARENT_PATH _bin)
find_program(PLUSHIE_MINGW_CXX NAMES ${_cxx_name}       PATHS "${_bin}" NO_DEFAULT_PATH)
find_program(PLUSHIE_MINGW_RC  NAMES ${_triple}-windres PATHS "${_bin}" NO_DEFAULT_PATH)

# Check it really targets x64 MinGW (once, not in every try_compile).
get_property(_in_try_compile GLOBAL PROPERTY IN_TRY_COMPILE)
if(NOT _in_try_compile)
  execute_process(COMMAND "${PLUSHIE_MINGW_CC}" -dumpmachine
    OUTPUT_VARIABLE _machine OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  if(NOT _machine STREQUAL _triple)
    message(FATAL_ERROR
      "windows-x64-mingw: ${PLUSHIE_MINGW_CC} targets '${_machine}', expected ${_triple}.")
  endif()
endif()

set(CMAKE_C_COMPILER "${PLUSHIE_MINGW_CC}")
if(PLUSHIE_MINGW_CXX)
  set(CMAKE_CXX_COMPILER "${PLUSHIE_MINGW_CXX}")
endif()
if(PLUSHIE_MINGW_RC)
  set(CMAKE_RC_COMPILER "${PLUSHIE_MINGW_RC}")
endif()

# --- Native on Windows x64, cross everywhere else -----------------------------------------
if(NOT (CMAKE_HOST_WIN32 AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(AMD64|x86_64)$"))
  set(CMAKE_SYSTEM_NAME Windows)
  set(CMAKE_SYSTEM_PROCESSOR x86_64)

  # Sysroot for find_*: explicit, else the first common location that exists.
  if(PLUSHIE_MINGW_SYSROOT)
    file(TO_CMAKE_PATH "${PLUSHIE_MINGW_SYSROOT}" _sysroot)
  else()
    set(_sysroot "")
    foreach(_candidate
        "/usr/${_triple}/sys-root/mingw"                                  # Fedora
        "/usr/${_triple}"                                                 # Debian / Ubuntu
        "/opt/homebrew/opt/mingw-w64/toolchain-x86_64/${_triple}"         # Homebrew (Apple silicon)
        "/usr/local/opt/mingw-w64/toolchain-x86_64/${_triple}")           # Homebrew (Intel)
      if(EXISTS "${_candidate}")
        set(_sysroot "${_candidate}")
        break()
      endif()
    endforeach()
  endif()
  if(_sysroot)
    set(CMAKE_FIND_ROOT_PATH "${_sysroot}")
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
  endif()

  # Running the .exe files (ctest, try_run): WSL runs them directly; elsewhere use Wine.
  if(NOT CMAKE_HOST_WIN32 AND "$ENV{WSL_DISTRO_NAME}" STREQUAL "")
    find_program(PLUSHIE_WINE NAMES wine64 wine)
    if(PLUSHIE_WINE)
      set(CMAKE_CROSSCOMPILING_EMULATOR "${PLUSHIE_WINE}")
    endif()
  endif()
endif()

# --- Static GCC runtime by default: executables start without the runtime DLLs ------------
if(NOT PLUSHIE_MINGW_SHARED_RUNTIME)
  set(CMAKE_EXE_LINKER_FLAGS_INIT    "-static-libgcc -static-libstdc++ -static")
  set(CMAKE_SHARED_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")
  set(CMAKE_MODULE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")
endif()