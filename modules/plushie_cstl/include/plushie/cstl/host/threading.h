/******************************************************************************
* @file include/plushie/cstl/host/threading.h
* @brief Multi-threading and synchronization objects
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_HOST_THREADING_H
#define PLUSHIE_CSTL_HOST_THREADING_H

#include "plushie/cstl/core/platform.h"
#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/error_codes.h"

// Macro to force the header to provide the implementation for the functions
#ifndef PLUSHIE_CSTL_HOST_THREADING_PROVIDE_IMPL
#  define PLUSHIE_CSTL_HOST_THREADING_PROVIDE_IMPL 0
#endif

// Different threading backends by ID
#define PLUSHIE_CSTL_THREADING_BACKEND_ID_NONE     0
#define PLUSHIE_CSTL_THREADING_BACKEND_ID_WIN32    1
#define PLUSHIE_CSTL_THREADING_BACKEND_ID_PTHREADS 2
#define PLUSHIE_CSTL_THREADING_BACKEND_ID_COUNT    3

// Select the backend if not specified
#ifndef PLUSHIE_CSTL_THREADING_BACKEND_ID
#  if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
#    define PLUSHIE_CSTL_THREADING_BACKEND_ID PLUSHIE_CSTL_THREADING_BACKEND_ID_WIN32
#  else
#    define PLUSHIE_CSTL_THREADING_BACKEND_ID PLUSHIE_CSTL_THREADING_BACKEND_ID_NONE
#  endif // !PLUSHIE_TARGET_OS_ID
#endif // !PLUSHIE_CSTL_THREADING_BACKEND_ID

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Structs and API Declaration
*******************************************************************************/

struct PlushieThread;
struct PlushieMutex;

/**
* @brief Different kinds of threading backend
*/
typedef enum PlushieThreadingBackendKind
{
	kPlushieThreadingBackendKindWin32,
	kPlushieThreadingBackendKindPthread,
}
PlushieThreadingBackendKind;

/**
* @brief Function pointer type used to start plushie threads.
*/
typedef int (*PlushieThreadingStartPfn)(
	void* param
);

/**
* @brief Info used to initialize a thread
*/
typedef struct PlushieThreadInitInfo
{
	PlushieThreadingStartPfn function;
}
PlushieThreadInitInfo;


PLUSHIE_CSTL_API PlushieResult plushie_thread_init(
	struct PlushieThread*  out_thread,
	PlushieThreadInitInfo* init
);

PLUSHIE_CSTL_API PlushieResult plushie_thread_wait(
	const struct PlushieThread* thread
);

/******************************************************************************
* Per Platform struct definitions
*******************************************************************************/

// -------------- WIN32 ------------------

#if PLUSHIE_CSTL_THREADING_BACKEND_ID == PLUSHIE_CSTL_THREADING_BACKEND_ID_WIN32
#include <Windows.h>

typedef struct PlushieThread
{
	HANDLE thread_handle;
	DWORD thread_id;
	PlushieThreadingStartPfn function;
}
PlushieThread;

typedef struct PlushieMutex
{
	LPCRITICAL_SECTION handle;
}
PlushieMutex;

#endif


/******************************************************************************
* Implementation
*******************************************************************************/
#if PLUSHIE_CSTL_HOST_THREADING_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_HOST_THREADING_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_HOST_THREADING_PROVIDE_IMPL_GUARD

PLUSHIE_CSTL_API PlushieResult plushie_thread_init(
	struct PlushieThread*  out_thread,
	PlushieThreadInitInfo* init
)
{
	if (out_thread == NULL || init == NULL) return kPlushieResultErrNullptr;

#if PLUSHIE_CSTL_THREADING_BACKEND_ID == PLUSHIE_CSTL_THREADING_BACKEND_ID_WIN32
	// TODO: Pull in the options to configure instead of using default values
	out_thread->function = init->function;
	out_thread->thread_handle = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)init->function,
		NULL,
		0,
		&out_thread->thread_id
	);
	return out_thread->thread_handle == INVALID_HANDLE_VALUE ? kPlushieResultErrOs : kPlushieResultSuccess;
	
#else
	// TODO: spawn the OS thread via the selected backend.
	return kPlushieResultErrUnsupported;
#endif
}


PLUSHIE_CSTL_API PlushieResult plushie_thread_wait(
	const struct PlushieThread* thread
)
{
	if (thread == NULL) return kPlushieResultErrNullptr;

#if PLUSHIE_CSTL_THREADING_BACKEND_ID == PLUSHIE_CSTL_THREADING_BACKEND_ID_WIN32
	// TODO: Pull in the options to configure instead of using default values
	WaitForSingleObject(thread->thread_handle, INFINITE);
	return kPlushieResultSuccess;
#else
	// TODO:
	return kPlushieResultErrUnsupported;
#endif
}


#endif // !PLUSHIE_CSTL_HOST_THREADING_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_HOST_THREADING_PROVIDE_IMPL
/******************************************************************************
* End of file
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_HOST_THREADING_H
