/******************************************************************************
* @file include/plushie/cstl/allocators/alloca.h
* @brief Memory allocation on the stack using alloca
* @license MIT
* @authors L Green
*
* @warning This allocator returns memory whose lifetime is bound to the
* *calling frame* of the alloc callback. Because the generic PlushieAllocator
* indirection calls through a function pointer, the storage is released when
* that call returns, so pointers handed back here MUST NOT be used after the
* alloc call site's scope. It is only safe for very short-lived, same-scope
* scratch space. Prefer libc/system allocators for anything that escapes.
*******************************************************************************/
#ifndef PLUSHIE_CSTL_ALLOCATORS_ALLOCA_H
#define PLUSHIE_CSTL_ALLOCATORS_ALLOCA_H

#include "plushie/cstl/core/alloc.h"

/* Macro to force the header to provide the implementation for the functions */
#ifndef PLUSHIE_CSTL_ALLOCATORS_ALLOCA_PROVIDE_IMPL
#  define PLUSHIE_CSTL_ALLOCATORS_ALLOCA_PROVIDE_IMPL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Provides the allocator which uses alloca, the most unsafe backing
* allocator, but also the fastest. See the file-level warning about lifetime.
*/
PLUSHIE_CSTL_API struct PlushieAllocator* plushie_allocator_get_alloca(
);

/******************************************************************************
* Implementation of alloca allocator.
*******************************************************************************/
#if PLUSHIE_CSTL_ALLOCATORS_ALLOCA_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_ALLOCATORS_ALLOCA_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_ALLOCATORS_ALLOCA_PROVIDE_IMPL_GUARD

#include "plushie/cstl/core/mem.h"

/* Define the stack allocation primitives PLUSHIE_ALLOCATOR_LOCAL_ALLOCA / */
/* PLUSHIE_ALLOCATOR_LOCAL_FREEA. If one isn't placed for your platform here, */
/* you can define them before including the header. */
#if !((defined PLUSHIE_ALLOCATOR_LOCAL_ALLOCA) && (defined PLUSHIE_ALLOCATOR_LOCAL_FREEA))

   /* MSVC has a special header */
#  if PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_MSVC
#    include <malloc.h>
#    define PLUSHIE_ALLOCATOR_LOCAL_ALLOCA(X) _malloca(X)
#    define PLUSHIE_ALLOCATOR_LOCAL_FREEA(X)  _freea(X)

   /* GCC always has the built in */
#  elif PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_GCC
#    define PLUSHIE_ALLOCATOR_LOCAL_ALLOCA(X) __builtin_alloca(X)
#    define PLUSHIE_ALLOCATOR_LOCAL_FREEA(X)  (void)(X)

   /* Clang provides via a built in */
#  elif PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_CLANG
#    ifndef __has_builtin         /* Optional of course. */
#      define __has_builtin(x) 0  /* Compatibility with non-clang compilers. */
#    endif
#    if __has_builtin(__builtin_alloca)
#      define PLUSHIE_ALLOCATOR_LOCAL_ALLOCA(X) __builtin_alloca(X)
#      define PLUSHIE_ALLOCATOR_LOCAL_FREEA(X)  (void)(X)
#    else
#      error No clang built in primative for alloca
#    endif 

#  else
#    error No stack alloc primitive
#  endif

#endif

/* Onto actually define the functions */
static void* s_local_plushie_allocator_alloca_alloc(
	struct PlushieAllocator* allocator,
	PlushieUSize             size,
	PlushieUSize             alignment,
	PlushieUInt32            flags,
	const char* const        name
)
{
	(void)allocator;
	(void)name;

	void* raw_ptr = PLUSHIE_ALLOCATOR_LOCAL_ALLOCA(
		PLUSHIE_ALLOCATOR_FAKE_ALIGNED_SIZE(size, alignment));
	if (raw_ptr == PLUSHIE_NULL) return PLUSHIE_NULL;
	void* ptr = plushie_allocator_place_header_for_alignment(raw_ptr, alignment);

	/* handle 0 init */
	if (flags & kPlushieAllocatorAllocFlagsZeroInit)
	{
		plushie_zero_set(ptr, size);
	}

	return ptr;
}

static void s_local_plushie_allocator_alloca_free(
	struct PlushieAllocator* allocator,
	void*                    ptr
)
{
	(void)allocator;
	if (ptr == PLUSHIE_NULL) return;
	PLUSHIE_ALLOCATOR_LOCAL_FREEA(plushie_allocator_fetch_header_for_alignment(ptr));
}

static PlushieAllocator sLocalPlushieAllocatorAlloca =
{
	s_local_plushie_allocator_alloca_alloc,
	PLUSHIE_NULL,
	s_local_plushie_allocator_alloca_free,
	PLUSHIE_NULL
};

PLUSHIE_CSTL_API PlushieAllocator* plushie_allocator_get_alloca(
)
{
	return &sLocalPlushieAllocatorAlloca;
}

/* Internal helper macros do not leak out of the implementation block */
#undef PLUSHIE_ALLOCATOR_LOCAL_ALLOCA
#undef PLUSHIE_ALLOCATOR_LOCAL_FREEA

/******************************************************************************
* End of implementation
*******************************************************************************/
#endif /* !PLUSHIE_CSTL_ALLOCATORS_ALLOCA_PROVIDE_IMPL_GUARD */
#endif /* !PLUSHIE_CSTL_ALLOCATORS_ALLOCA_PROVIDE_IMPL */

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif /* !PLUSHIE_CSTL_ALLOCATORS_ALLOCA_H */
