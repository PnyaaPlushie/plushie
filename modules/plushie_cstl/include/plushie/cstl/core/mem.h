/******************************************************************************
* @file include/plushie/cstl/core/mem.h
* @brief Small memory helpers (zero init) with and without a libc runtime
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_MEM_H
#define PLUSHIE_CSTL_CORE_MEM_H

#include "plushie/cstl/core/platform.h"
#include "plushie/cstl/core/fixed_width.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Zero initialization
*******************************************************************************/

// We supply different routes for zero initialization because a lot of
// compilers can perform a lot of optimization if they detect you're doing a 0
// init, and the best hint for that is "memset". However it might not be
// available.

#if PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDSTRING && PLUSHIE_CSTL_HAS_LIBC_RUNTIME
#include <string.h>
static inline void plushie_zero_set(
	void*        ptr,
	PlushieUSize size
)
{
	memset(ptr, 0, size);
}

static inline void plushie_mem_copy(
	void*        dst,
	const void*  src,
	PlushieUSize size
)
{
	memcpy(dst, src, size);
}

#else

// TODO [#32]: Speedup for no memset plushie_zero_set
static inline void plushie_zero_set(
	void*        ptr,
	PlushieUSize size
)
{
	for (PlushieUSize i = 0; i < size; i++)
	{
		((PlushieUInt08*)ptr)[i] = 0;
	}
}

static inline void plushie_mem_copy(
	void*        dst,
	const void*  src,
	PlushieUSize size
)
{
	for (PlushieUSize i = 0; i < size; i++)
	{
		((PlushieUInt08*)dst)[i] = ((const PlushieUInt08*)src)[i];
	}
}

#endif

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CORE_MEM_H
