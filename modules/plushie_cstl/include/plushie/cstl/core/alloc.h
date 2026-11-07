/******************************************************************************
* @file include/plushie/cstl/core/alloc.h
* @brief Allocator interface and generic alignment helpers
* @license MIT
* @authors L Green
*
* @note Memory allocations in plushie should be rare, and should be grouped
* with predefined memory amounts as possible. Therefore the allocation flags
* and usage bits are very explicit.
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_ALLOC_H
#define PLUSHIE_CSTL_CORE_ALLOC_H

#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/mem.h"

// Default alignment used by the convenience wrappers below when the caller
// doesn't care. Safe for any of the plushie base types.
#ifndef PLUSHIE_ALLOC_DEFAULT_ALIGNMENT
#  define PLUSHIE_ALLOC_DEFAULT_ALIGNMENT 16
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Base allocator
*******************************************************************************/

/**
* @brief Function pointer type for an allocate function. Called when new memory
* is requested.
* @note name is an additional parameter used for debugging. It can be ignored
* with no problem
*/
typedef void* (*PlushieAllocatorPfnAlloc)(
	struct PlushieAllocator* allocator,
	PlushieUSize             size,
	PlushieUSize             alignment,
	PlushieUInt32            flags,
	const char* const        name
);

/**
* @brief Function pointer type for a reallocation function. Called when a
* memory block needs to be resized.
* @note Similar to Alloc, the name parameter can be ignored
* @note We need the old size of the allocation for compatibility
*/
typedef void* (*PlushieAllocatorPfnRealloc)(
	struct PlushieAllocator* allocator,
	void*                    ptr,
	PlushieUSize             old_size,
	PlushieUSize             new_size,
	PlushieUSize             alignment,
	PlushieUInt32            flags,
	const char* const        name
);

/**
* @brief Function pointer type for a free function. Called when memory is going
* to be released
*/
typedef void (*PlushieAllocatorPfnFree)(
	struct PlushieAllocator* allocator,
	void*                    ptr
);

/**
* @brief Default allocator structure
*/
typedef struct PlushieAllocator
{
	PlushieAllocatorPfnAlloc   alloc;
	PlushieAllocatorPfnRealloc realloc;
	PlushieAllocatorPfnFree    free;

	struct PlushieAllocator*   parent;
	void*                      user_data;
}
PlushieAllocator;

/**
* @brief Allocation request flags and hints
*/
typedef enum PlushieAllocatorAllocFlags
{
	kPlushieAllocatorAllocFlagsNone            = 0,

	// Initial hints
	kPlushieAllocatorAllocFlagsZeroInit        = 1 << 0,
	kPlushieAllocatorAllocFlagsNoFallback      = 1 << 1,

	// Page Protection
	kPlushieAllocatorAllocFlagsProtectRead     = 1 << 2,
	kPlushieAllocatorAllocFlagsProtectWrite    = 1 << 3,
	kPlushieAllocatorAllocFlagsProtectExecute  = 1 << 4,
	kPlushieAllocatorAllocFlagsProtectNoCommit = 1 << 5,

	// Visibility
	kPlushieAllocatorAllocFlagsThreadLocal     = 1 << 6,
	kPlushieAllocatorAllocFlagsProcShared      = 1 << 7,

	// Hardware and topology
	kPlushieAllocatorAllocFlagsNumaLocal       = 1 << 8,
	kPlushieAllocatorAllocFlagsHugePages       = 1 << 9,
	kPlushieAllocatorAllocFlagsCacheAligned    = 1 << 10,
	kPlushieAllocatorAllocFlagsPinned          = 1 << 11,
}
PlushieAllocatorAllocFlags;

// Combined masks. These OR together the individual single-bit flags so callers
// can test a whole category at once.
#define kPlushieAllocatorAllocFlagsReqOnly      (\
  kPlushieAllocatorAllocFlagsZeroInit           |\
  kPlushieAllocatorAllocFlagsNoFallback         |\
  kPlushieAllocatorAllocFlagsProtectRead        |\
  kPlushieAllocatorAllocFlagsProtectWrite       |\
  kPlushieAllocatorAllocFlagsProtectExecute     |\
  kPlushieAllocatorAllocFlagsProtectNoCommit    |\
  kPlushieAllocatorAllocFlagsProcShared         |\
  kPlushieAllocatorAllocFlagsPinned             )

#define kPlushieAllocatorAllocFlagsHintsOnly    (\
  kPlushieAllocatorAllocFlagsThreadLocal        |\
  kPlushieAllocatorAllocFlagsNumaLocal          |\
  kPlushieAllocatorAllocFlagsHugePages          |\
  kPlushieAllocatorAllocFlagsCacheAligned       )

#define kPlushieAllocatorAllocFlagsAll          (\
  kPlushieAllocatorAllocFlagsReqOnly            |\
  kPlushieAllocatorAllocFlagsHintsOnly          )

// Combined allocation flags which emulate a normal calloc
#define kPlushieAllocatorAllocFlagsCalloc       (\
  kPlushieAllocatorAllocFlagsZeroInit           |\
  kPlushieAllocatorAllocFlagsProtectRead        |\
  kPlushieAllocatorAllocFlagsProtectWrite       )
                                                

/******************************************************************************
* Functions that allocators can use to fake alignment
*******************************************************************************/

// Helper to calculate how much to allocate if the allocator can't perform
// aligned allocations natively. Shared by the allocator backends, so it stays
// defined for consumers of this header (not #undef'd).
#define PLUSHIE_ALLOCATOR_FAKE_ALIGNED_SIZE(SIZE, ALIGN) \
	((SIZE) + (ALIGN) + sizeof(void*))

// Helper used for allocators which cannot perform aligned allocs natively. It
// places a "header" of the actual address just before the aligned pointer.
static inline void* plushie_allocator_place_header_for_alignment(
	void*        ptr,
	PlushieUSize alignment
)
{
	// Advance to leave enough space for the header
	PlushieUPtr addr = ((PlushieUPtr)ptr) + sizeof(void*);

	// Align up
	PlushieUPtr aligned_addr = (addr + (alignment - 1)) & ~(alignment - 1);

	// Store the original address
	((void**)aligned_addr)[-1] = ptr;

	// return the aligned address
	return (void*)aligned_addr;
}

// Helper to fetch the actual address of the aligned allocation for allocators
// which don't have native allocation alignment specifiers
static inline void* plushie_allocator_fetch_header_for_alignment(
	void* ptr
)
{
	return ((void**)ptr)[-1];
}

/******************************************************************************
* Convenience wrappers over the allocator function pointers
*******************************************************************************/

// Allocate `size` bytes with the default alignment. Returns NULL on failure or
// if the allocator cannot allocate.
static inline void* plushie_alloc(
	PlushieAllocator* alloc,
	PlushieUSize      size
)
{
	if (alloc == NULL || alloc->alloc == NULL) return NULL;
	return alloc->alloc(alloc, size, PLUSHIE_ALLOC_DEFAULT_ALIGNMENT,
		kPlushieAllocatorAllocFlagsNone, NULL);
}

// Free memory previously returned by plushie_alloc/plushie_realloc.
static inline void plushie_free(
	PlushieAllocator* alloc,
	void*             ptr
)
{
	if (alloc == NULL || alloc->free == NULL || ptr == NULL) return;
	alloc->free(alloc, ptr);
}

// Resize an allocation. Uses the allocator's realloc if present, otherwise
// falls back to alloc + copy + free (needed for backends like libc/system that
// leave realloc NULL). Returns NULL on failure; the original block is left
// intact on a failed fallback.
static inline void* plushie_realloc(
	PlushieAllocator* alloc,
	void*             ptr,
	PlushieUSize      old_size,
	PlushieUSize      new_size
)
{
	if (alloc == NULL) return NULL;

	if (alloc->realloc != NULL)
	{
		return alloc->realloc(alloc, ptr, old_size, new_size,
			PLUSHIE_ALLOC_DEFAULT_ALIGNMENT,
			kPlushieAllocatorAllocFlagsNone, NULL);
	}

	if (alloc->alloc == NULL) return NULL;

	void* new_ptr = alloc->alloc(alloc, new_size,
		PLUSHIE_ALLOC_DEFAULT_ALIGNMENT,
		kPlushieAllocatorAllocFlagsNone, NULL);
	if (new_ptr == NULL) return NULL;

	if (ptr != NULL)
	{
		PlushieUSize copy_size = old_size < new_size ? old_size : new_size;
		plushie_mem_copy(new_ptr, ptr, copy_size);
		if (alloc->free != NULL) alloc->free(alloc, ptr);
	}

	return new_ptr;
}

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CORE_ALLOC_H
