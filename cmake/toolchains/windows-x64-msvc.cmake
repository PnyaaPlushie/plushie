
# cmake/toolchains/windows-x64-msvc.cmake
# Target: Windows x64, MSVC ABI.
# 	Windows + Visual Studio generator -> VS picks cl/link itself
# 	Windows + Ninja (or other)        -> cl from an x64 developer prompt
# 	Linux / macOS                     -> clang-cl + xwin
if(CMAKE_GENERATOR MATCHES "^Visual Studio")

	# Equivalent of -A x64. Only applied if the user didn't pass -A themselves.
  if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64)
  endif()

endif()