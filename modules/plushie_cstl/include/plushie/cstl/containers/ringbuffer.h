/******************************************************************************
* @file include/plushie/cstl/containers/ringbuffer.h
* @brief Ring buffer type generator (placeholder)
* @license MIT
* @authors L Green
*
* @note Similar to the vector type, this is intended to provide macros for
* generating a per-element-type ring buffer. It is not implemented yet; the
* header exists so the include path is stable and so downstream code can be
* wired against it. Guarded so it is safe to include standalone.
*
* TODO: PlushieRingBufferDecl / PlushieRingBufferDefine generator macros,
* mirroring containers/vector.h (init / push / pop / free returning
* PlushieResult).
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CONTAINERS_RINGBUFFER_H
#define PLUSHIE_CSTL_CONTAINERS_RINGBUFFER_H

#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Intentionally empty until the generator is implemented. */

#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CONTAINERS_RINGBUFFER_H
