/******************************************************************************
* @file include/plushie/cstl/core/fixed_width.h
* @brief Fixed-width integer and size base types for the plushie cstl
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_FIXED_WIDTH_H
#define PLUSHIE_CSTL_CORE_FIXED_WIDTH_H

#include "plushie/cstl/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Fixed width base types
*******************************************************************************/

#if PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDINT
#include <stdint.h>
typedef uint8_t  PlushieUInt08;
typedef uint16_t PlushieUInt16;
typedef uint32_t PlushieUInt32;
typedef uint64_t PlushieUInt64;

typedef int8_t   PlushieSInt08;
typedef int16_t  PlushieSInt16;
typedef int32_t  PlushieSInt32;
typedef int64_t  PlushieSInt64;

typedef uintptr_t PlushieUPtr;
#else
#  error Fixed widths not implemented without stdint
#endif

#if PLUSHIE_CSTL_HAS_LIBC_HEADERS_STDDEF
#include <stddef.h>
typedef size_t PlushieUSize;
#else
#  error Size type not implemented without stddef
#endif

// Wide char
#if PLUSHIE_COMPILER_ID == PLUSHIE_COMPILER_ID_MSVC
typedef wchar_t PlushieWinWchar;
#else
typedef PlushieUInt16 PlushieWinWchar;
#endif

/******************************************************************************
* Multi Component Fixed width types 
*******************************************************************************/

// Force padding for float32x3 type up to bytes 
#ifndef PLUSHIE_CSTL_FIXED_WIDTH_GPU_ALIGNMENT
#define PLUSHIE_CSTL_FIXED_WIDTH_GPU_ALIGNMENT 0
#endif

/*
* Multi-component floating point type 
*/

// 4 component floating point, 32 bit per component
typedef struct PlushieFloat32x4
{
	PlushieUInt32 x, y, z, w;
}
PlushieFloat32x4;

// 3 component floating point, 32 bit per component
typedef struct PlushieFloat32x3
{
	PlushieUInt32 x, y, z;
}
PlushieFloat32x3;

// 2 component floating point, 32 bit per component
typedef struct PlushieFloat32x2
{
	PlushieUInt32 x, y;
}
PlushieFloat32x2;

/*
* Multi-component signed integer type
*/

// 4 component unsigned int, 32 bit per component
typedef struct PlushieSInt32x4 
{
	PlushieSInt32 x, y, z, w;
}
PlushieSInt32x4;

// 3 component unsigned int, 32 bit per component
typedef struct PlushieSInt32x3
{
	PlushieSInt32 x, y, z;
}
PlushieSInt32x3;

// 2 component unsigned int, 32 bit per component
typedef struct PlushieSInt32x2
{
	PlushieUInt32 x, y;
}
PlushieSInt32x2;

/*
* Multi-component unsigned integer type
*/

// 4 component unsigned int, 32 bit per component
typedef struct PlushieUInt32x4
{
	PlushieUInt32 x, y, z, w;
}
PlushieUInt32x4;

// 3 component unsigned int, 32 bit per component
typedef struct PlushieUInt32x3
{
	PlushieUInt32 x, y, z;
}
PlushieUInt32x3;

// 2 component unsigned int, 32 bit per component
typedef struct PlushieUInt32x2
{
	PlushieUInt32 x, y;
}
PlushieUInt32x2;

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CORE_FIXED_WIDTH_H
