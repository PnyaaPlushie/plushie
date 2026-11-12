/******************************************************************************
* @file include/plushie/cstl/core/platform.h
* @brief Detects the target OS, compiler, C standard, architecture and libc
*  availability. Pure preprocessor detection with no declarations.
* @license MIT
* @authors L Green
*
* @note You'll notice that we don't rely on a macro being defined-or-not;
* instead the preprocessor var is always defined, but it can have a "truthy"
* or "falsy" value. Header guards are the sole exception to this.
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_PLATFORM_H
#define PLUSHIE_CSTL_CORE_PLATFORM_H

/******************************************************************************
* CSTL Runtime Availability
* Plushie CSTL is built around using it with or without libc runtime and
* headers. Most of the time a user has libc, and if not they at least have the
* headers for typedecl.
*******************************************************************************/

// By default we assume you have libc
#ifndef PLUSHIE_CSTL_HAS_LIBC_RUNTIME
#  define PLUSHIE_CSTL_HAS_LIBC_RUNTIME 1
#endif

// Default availability of headers for libc.
#ifndef PLUSHIE_CSTL_HAS_LIBC_HEADERS
#  define PLUSHIE_CSTL_HAS_LIBC_HEADERS 1
#endif

// Stdint can be used without a runtime
#ifndef PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDINT
#  define PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDINT PLUSHIE_CSTL_HAS_LIBC_HEADERS
#endif

// Stddef can be used for size_t without a runtime
#ifndef PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDDEF
#  define PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDDEF PLUSHIE_CSTL_HAS_LIBC_HEADERS
#endif

// StdString contains useful implementations like memset
#ifndef PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDSTRING
#  define PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDSTRING PLUSHIE_CSTL_HAS_LIBC_HEADERS
#endif

// Stdio contains printf and friends
#ifndef PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDIO
#  define PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDIO PLUSHIE_CSTL_HAS_LIBC_HEADERS
#endif

/******************************************************************************
* Target OS detection
*******************************************************************************/

// list of platform ids
#define PLUSHIE_TARGET_OS_ID_NONE     0
#define PLUSHIE_TARGET_OS_ID_WINDOWS  1
#define PLUSHIE_TARGET_OS_ID_LINUX    2
#define PLUSHIE_TARGET_OS_ID_FREEBSD  3
#define PLUSHIE_TARGET_OS_ID_ANDROID  4

#ifndef PLUSHIE_TARGET_OS_ID

#  if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#    define PLUSHIE_TARGET_OS_ID PLUSHIE_TARGET_OS_ID_WINDOWS

#  elif defined(__ANDROID__)
#    define PLUSHIE_TARGET_OS_ID PLUSHIE_TARGET_OS_ID_ANDROID

#  elif defined(__linux__)
#    define PLUSHIE_TARGET_OS_ID PLUSHIE_TARGET_OS_ID_LINUX

#  else
#    define PLUSHIE_TARGET_OS_ID PLUSHIE_TARGET_OS_ID_NONE
#  endif

#endif

/******************************************************************************
* Compiler detection
*******************************************************************************/

// List of compilers
#define PLUSHIE_COMPILER_ID_NONE     0
#define PLUSHIE_COMPILER_ID_MSVC     1
#define PLUSHIE_COMPILER_ID_CLANG    2
#define PLUSHIE_COMPILER_ID_GCC      3
#define PLUSHIE_COMPILER_ID_INTEL    4

#ifndef PLUSHIE_COMPILER_ID

#  if defined(_MSC_VER)
#    define PLUSHIE_COMPILER_ID PLUSHIE_COMPILER_ID_MSVC

#  elif defined(__clang__)
#    define PLUSHIE_COMPILER_ID PLUSHIE_COMPILER_ID_CLANG

#  elif defined(__GNUC__)
#    define PLUSHIE_COMPILER_ID PLUSHIE_COMPILER_ID_GCC

#  elif defined(__INTEL_COMPILER) || defined(__ICC)
#    define PLUSHIE_COMPILER_ID PLUSHIE_COMPILER_ID_INTEL

#  else
#    define PLUSHIE_COMPILER_ID PLUSHIE_COMPILER_ID_NONE
#  endif

#endif

/******************************************************************************
* Symbol visibility
*******************************************************************************/
#ifndef PLUSHIE_SYMBOL_EXPORT
#  if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
#    define PLUSHIE_SYMBOL_EXPORT __declspec(dllexport)
#    define PLUSHIE_SYMBOL_IMPORT __declspec(dllimport)
#    define PLUSHIE_SYMBOL_HIDDEN
#  elif PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_GCC || PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_CLANG
#    define PLUSHIE_SYMBOL_EXPORT __attribute__((visibility("default")))
#    define PLUSHIE_SYMBOL_IMPORT __attribute__((visibility("default")))
#    define PLUSHIE_SYMBOL_HIDDEN __attribute__((visibility("hidden")))
#  else
#    define PLUSHIE_SYMBOL_EXPORT
#    define PLUSHIE_SYMBOL_IMPORT
#    define PLUSHIE_SYMBOL_HIDDEN
#  endif
#endif

// Each library needs to have it's own api signal
#define PLUSHIE_CSTL_USAGE_MODE_HEADER_ONLY   0  // header-only
#define PLUSHIE_CSTL_USAGE_MODE_STATIC        1  // static library
#define PLUSHIE_CSTL_USAGE_MODE_SHARED        2  // using the DLL / .so (import)
#define PLUSHIE_CSTL_USAGE_MODE_SHARED_BUILD  3  // building the DLL / .so (export)

// By default we assume the user is using the library in header only mode
#ifndef PLUSHIE_CSTL_USAGE_MODE
#  define PLUSHIE_CSTL_USAGE_MODE PLUSHIE_CSTL_USAGE_MODE_HEADER_ONLY
#endif

#ifndef PLUSHIE_CSTL_API
#  if PLUSHIE_CSTL_USAGE_MODE == PLUSHIE_CSTL_USAGE_MODE_HEADER_ONLY
#    define PLUSHIE_CSTL_API static inline
#  elif PLUSHIE_CSTL_USAGE_MODE == PLUSHIE_CSTL_USAGE_MODE_SHARED
#    define PLUSHIE_CSTL_API PLUSHIE_SYMBOL_IMPORT
#  elif PLUSHIE_CSTL_USAGE_MODE == PLUSHIE_CSTL_USAGE_MODE_SHARED_BUILD
#    define PLUSHIE_CSTL_API PLUSHIE_SYMBOL_EXPORT
#  elif PLUSHIE_CSTL_USAGE_MODE == PLUSHIE_CSTL_USAGE_MODE_STATIC
#    define PLUSHIE_CSTL_API
#  else
#    error "PLUSHIE_CSTL_USAGE_MODE has an unknown value"
#  endif 
#endif

/******************************************************************************
* C Standard version detection
*******************************************************************************/

// Normalize based on the stdc macro. We use the year.

// Normalize msvc when user hasn't used conformance flag
#if PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_MSVC && !defined(PLUSHIE_C_STD)

#  ifndef __STDC_VERSION__
#    pragma message(__FILE__ " append /Zc:__STDC_VERSION__ for better conformance")
#    if _MSC_VER >= 1928         /* VS 2019 v 16.8+ supports C11/C17 */
#      define PLUSHIE_C_STD 2017
#    elif _MSC_VER >= 1900       /* VS 2015+ has C99 support*/
#      define PLUSHIE_C_STD 1999
#    else                        /* Else we guess */
#      define PLUSHIE_C_STD 1990
#    endif
#  endif

#endif

// Use __STDC_VERSION__ macro to determine standard
#if !defined(PLUSHIE_C_STD) && defined __STDC_VERSION__

#  if __STDC_VERSION__ >= 202311L
#    define PLUSHIE_C_STD 2023
#  elif __STDC_VERSION__ >= 201710L
#    define PLUSHIE_C_STD 2017
#  elif __STDC_VERSION__ >= 201112L
#    define PLUSHIE_C_STD 2011
#  elif __STDC_VERSION__ >= 199901L
#    define PLUSHIE_C_STD 1999
#  elif __STDC_VERSION__ == 199409L /* MSVC legacy value C95 ~= C90 */
#    define PLUSHIE_C_STD 1990
#  else
#    define PLUSHIE_C_STD 1990
#  endif

#elif !defined(PLUSHIE_C_STD)
#  define PLUSHIE_C_STD 1990  /* Final guess */
#endif

/******************************************************************************
* Architecture detection
*******************************************************************************/

// List of architectures
#define PLUSHIE_ARCH_ID_NONE    0
#define PLUSHIE_ARCH_ID_X86_32  1
#define PLUSHIE_ARCH_ID_X86_64  2
#define PLUSHIE_ARCH_ID_ARM_32  3
#define PLUSHIE_ARCH_ID_ARM_64  4

/******************************************************************************
* Alignment macro
*******************************************************************************/

// alignof becomes a keyword in 2023
#if PLUSHIE_C_STD >= 2023
#  define PLUSHIE_ALIGNOF(type) alignof(type)

// Msvc has it's own version for older standards
#elif PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_MSVC
#  define PLUSHIE_ALIGNOF(type) __alignof(type)

// C11 - C17 has keyword _Alignof. Deprecated in C2023
#elif PLUSHIE_C_STD >= 2011
#  define PLUSHIE_ALIGNOF(type) _Alignof(type)

// Gcc/Clang: Use a GNU extension for pre-C11
#elif (PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_GCC) || (PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_CLANG)
#  define PLUSHIE_ALIGNOF(type) __alignof__(type)

// Final guess which is very bad, but if you take the size of the element
// it will usually work
#else
#  define PLUSHIE_ALIGNOF(type) sizeof(type)
#endif

/******************************************************************************
* End of file content
*******************************************************************************/
#endif // !PLUSHIE_CSTL_CORE_PLATFORM_H
