/******************************************************************************
* @file include/plushie/cstl/allocators/libc.h
* @brief Heap allocator backed by libc malloc/free
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_ALLOCATORS_LIBC_H
#define PLUSHIE_CSTL_ALLOCATORS_LIBC_H

#include "plushie/cstl/core/alloc.h"

// Macro to force the header to provide the implementation for the functions
#ifndef PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL
#  define PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Get the allocator based on libc, that uses malloc, free etc
*/
PLUSHIE_CSTL_API struct PlushieAllocator* plushie_allocator_get_libc(
);

/******************************************************************************
* Implementation of libc allocator.
*******************************************************************************/
#if PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL_GUARD

#include "plushie/cstl/core/mem.h"
#include <stdlib.h>

// Onto actually define the functions
static void* s_local_plushie_allocator_libc_alloc(
	struct PlushieAllocator* allocator,
	PlushieUSize             size,
	PlushieUSize             alignment,
	PlushieUInt32            flags,
	const char* const        name
)
{
	(void)allocator;
	(void)name;

	void* raw_ptr = malloc(PLUSHIE_ALLOCATOR_FAKE_ALIGNED_SIZE(size, alignment));
	if (raw_ptr == NULL) return NULL;
	void* ptr = plushie_allocator_place_header_for_alignment(raw_ptr, alignment);

	// handle 0 init
	if (flags & kPlushieAllocatorAllocFlagsZeroInit)
	{
		plushie_zero_set(ptr, size);
	}

	return ptr;
}

static void s_local_plushie_allocator_libc_free(
	struct PlushieAllocator* allocator,
	void*                    ptr
)
{
	(void)allocator;
	if (ptr == NULL) return;
	free(plushie_allocator_fetch_header_for_alignment(ptr));
}


static PlushieAllocator sLocalPlushieAllocatorLibc =
{
	s_local_plushie_allocator_libc_alloc,
	NULL,
	s_local_plushie_allocator_libc_free,
	NULL
};

PLUSHIE_CSTL_API PlushieAllocator* plushie_allocator_get_libc(
)
{
	return &sLocalPlushieAllocatorLibc;
}

/******************************************************************************
* End of implementation
*******************************************************************************/
#endif // !PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_ALLOCATORS_LIBC_H
