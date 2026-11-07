/******************************************************************************
* @file include/plushie/cstl/containers/span.h
* @brief Generic span type generator macro which aims to produce structs which
* behave similar to Rust's slices or C++20 std::span
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CONTAINERS_SPAN_H
#define PLUSHIE_CSTL_CONTAINERS_SPAN_H

#include "plushie/cstl/core/fixed_width.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Macro to generate a generic span struct for any element type
*
* Example usage:
*   PLUSHIE_SPAN_DEFINE(PlushieU8Span,   PlushieU8Span,   unsigned char)
*   PLUSHIE_SPAN_DEFINE(PlushieWideSpan, PlushieWideSpan, wchar_t)
*   PLUSHIE_SPAN_DEFINE(PlushieU32Span,  PlushieU32Span,  PlushieUInt32)
*
* @param SpanStructName  Name of the generated struct type
* @param SpanFuncName    Prefix used for the generated functions
* @param ElementType     Element type the span points at
*
* Generated struct members:
*   - const ElementType* ptr // Pointer to memory
*   - PlushieUSize       len // Number of elements
*
* Generated struct functions (inline):
*   - *SpanFuncName*_new
*   - *SpanFuncName*_is_empty
*   - *SpanFuncName*_size
*/
#define PLUSHIE_SPAN_DEFINE(SpanStructName, SpanFuncName, ElementType)    \
	typedef struct SpanStructName                                        \
	{                                                                    \
		const ElementType* ptr;                                          \
		PlushieUSize       len;                                          \
	} SpanStructName;                                                    \
                                                                         \
	static inline SpanStructName SpanFuncName##_new(                     \
		const ElementType* ptr,                                          \
		PlushieUSize       len)                                          \
	{                                                                    \
		SpanStructName span = { 0 };                                     \
		span.ptr = ptr;                                                  \
		span.len = len;                                                  \
		return span;                                                     \
	}                                                                    \
                                                                         \
	static inline int SpanFuncName##_is_empty(                           \
		const SpanStructName* span                                       \
	)                                                                    \
	{                                                                    \
		return span->len == 0;                                           \
	}                                                                    \
                                                                         \
	static inline PlushieUSize SpanFuncName##_size(                      \
		const SpanStructName* span                                       \
	)                                                                    \
	{                                                                    \
		return span->len * sizeof(ElementType);                          \
	}

#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CONTAINERS_SPAN_H
