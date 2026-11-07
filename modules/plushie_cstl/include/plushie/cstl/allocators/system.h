/******************************************************************************
* @file include/plushie/cstl/allocators/system.h
* @brief Heap allocator backed by an OS primitive (Win32 process heap)
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_ALLOCATORS_SYSTEM_H
#define PLUSHIE_CSTL_ALLOCATORS_SYSTEM_H

#include "plushie/cstl/core/alloc.h"

// Macro to force the header to provide the implementation for the functions
#ifndef PLUSHIE_CSTL_ALLOCATORS_SYSTEM_PROVIDE_IMPL
#  define PLUSHIE_CSTL_ALLOCATORS_SYSTEM_PROVIDE_IMPL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Get the system allocator which uses platform dependent code
*/
struct PlushieAllocator* plushie_allocator_get_system(
);

/******************************************************************************
* Implementation of system allocator.
*******************************************************************************/
#if PLUSHIE_CSTL_ALLOCATORS_SYSTEM_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_ALLOCATORS_SYSTEM_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_ALLOCATORS_SYSTEM_PROVIDE_IMPL_GUARD

/******************************************************************************
* WIN32 Implementation
*******************************************************************************/
#if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS

#include "plushie/cstl/core/atomic.h"
#include <Windows.h>

static PlushieAtomicUInt16 s_plushie_heap_init = 0;
static HANDLE s_plushie_process_heap_handle = NULL;


static void* s_local_plushie_allocator_win32_alloc(
	struct PlushieAllocator* allocator,
	PlushieUSize             size,
	PlushieUSize             alignment,
	PlushieUInt32            flags,
	struct PlushieStr*       name
)
{
	(void)allocator;
	(void)name;

	DWORD alloc_flags =
		flags & kPlushieAllocatorAllocFlagsZeroInit ? HEAP_ZERO_MEMORY : 0;

	void* raw_ptr = HeapAlloc(
		s_plushie_process_heap_handle,
		alloc_flags,
		PLUSHIE_ALLOCATOR_FAKE_ALIGNED_SIZE(size, alignment)
	);
	if (raw_ptr == NULL) return NULL;
	void* ptr = plushie_allocator_place_header_for_alignment(raw_ptr, alignment);

	return ptr;
}

static void s_local_plushie_allocator_win32_free(
	struct PlushieAllocator* allocator,
	void*                    ptr
)
{
	(void)allocator;
	if (ptr == NULL) return;
	HeapFree(
		s_plushie_process_heap_handle,
		0,
		plushie_allocator_fetch_header_for_alignment(ptr));
}


static PlushieAllocator sLocalPlushieAllocatorWin32 =
{
	s_local_plushie_allocator_win32_alloc,
	NULL,
	s_local_plushie_allocator_win32_free,
	NULL
};

PlushieAllocator* plushie_allocator_get_system(
)
{
	// Get the init status of the atomic, only set to 1 if current val is 0
	PlushieUInt16 prev = plushie_atomic_u16_compare_exchange(&s_plushie_heap_init, 1, 0);
	if (prev == 0)
	{
		// This thread won the init atomic check
		// TODO: We could use one new heap for zero init
		// but then we'd have to destroy the heap too
		s_plushie_process_heap_handle = GetProcessHeap();

		// We've finished the init process
		plushie_atomic_u16_exchange(&s_plushie_heap_init, 2);
	}
	else if (prev == 1)
	{
		// This thread lost the init check, while the other thread is
		// still performing the init
		plushie_yield_processor();
	}

	// It is possible for this to fail
	if (s_plushie_process_heap_handle == NULL
		|| s_plushie_process_heap_handle == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	return &sLocalPlushieAllocatorWin32;
}

#endif // PLUSHIE_TARGET_OS_ID_WINDOWS

/******************************************************************************
* End of implementation
*******************************************************************************/
#endif // !PLUSHIE_CSTL_ALLOCATORS_SYSTEM_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_ALLOCATORS_SYSTEM_PROVIDE_IMPL

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_ALLOCATORS_SYSTEM_H
