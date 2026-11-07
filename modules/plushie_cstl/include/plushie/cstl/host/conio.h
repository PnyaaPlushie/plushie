/******************************************************************************
* @file include/plushie/cstl/host/conio.h
* @brief Console input and output
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_HOST_CONIO_H
#define PLUSHIE_CSTL_HOST_CONIO_H

#include "plushie/cstl/core/platform.h"
#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/containers/string.h"

// Implementation guard macro
#ifndef PLUSHIE_CSTL_HOST_CONIO_PROVIDE_IMPL
#  define PLUSHIE_CSTL_HOST_CONIO_PROVIDE_IMPL 0
#endif

// Define different possible backends for writing and reading to consoles
#define PLUSHIE_CSTL_CONIO_ID_NONE  0
#define PLUSHIE_CSTL_CONIO_ID_LIBC  1
#define PLUSHIE_CSTL_CONIO_ID_WIN32 2

// Determine console backend to use
#ifndef PLUSHIE_CSTL_CONIO_ID

#  if PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDIO && PLUSHIE_CSTL_HAS_LIBC_RUNTIME
#    define PLUSHIE_CSTL_CONIO_ID PLUSHIE_CSTL_CONIO_ID_LIBC

#  elif PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
#    define PLUSHIE_CSTL_CONIO_ID PLUSHIE_CSTL_CONIO_ID_WIN32

#  else
#    define PLUSHIE_CSTL_CONIO_ID PLUSHIE_CSTL_CONIO_ID_NONE
#  endif
#endif // !PLUSHIE_CSTL_CONIO_ID

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Public Structs declarations
*******************************************************************************/

typedef struct PlushieConIoHost PlushieConIoHost;

/******************************************************************************
* Public API
*******************************************************************************/

void plushie_conio_write(
	PlushieConIoHost* io,
	PlushieStr        msg
);


/******************************************************************************
* Declarations
*******************************************************************************/
#if PLUSHIE_CSTL_CONIO_ID == PLUSHIE_CSTL_CONIO_ID_LIBC
struct PlushieConIoHost
{
	PlushieUInt08 padding;
};

// TODO: Other implementations
#else


#endif

/******************************************************************************
* Implementation
*******************************************************************************/
#if PLUSHIE_CSTL_HOST_CONIO_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_HOST_CONIO_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_HOST_CONIO_PROVIDE_IMPL_GUARD

//
// LIBC implementation
//
#if PLUSHIE_CSTL_CONIO_ID == PLUSHIE_CSTL_CONIO_ID_LIBC
#include <stdio.h>

void plushie_conio_write(
	PlushieConIoHost* io,
	PlushieStr        msg
)
{
	(void)io;
	if (msg.data.ptr == NULL || msg.len == 0) return;

	if (msg.flags & kPlushieStrFlagEncodingAscii ||
		msg.flags & kPlushieStrFlagEncodingUtf8)
	{
		printf("%s", msg.data.str);
		return;
	}

#if PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_MSVC
	if (msg.flags & kPlushieStrFlagEncodingWide16)
	{
		wprintf_s(L"%ls", msg.data.wstr);
		return;
	}

#endif

}

#endif

/******************************************************************************
* End of implementation block
*******************************************************************************/
#endif // !PLUSHIE_CSTL_HOST_CONIO_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_HOST_CONIO_PROVIDE_IMPL

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_HOST_CONIO_H
