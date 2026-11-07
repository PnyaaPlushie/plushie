/******************************************************************************
* @file include/plushie/cstl/core/fixed_point.h
* @brief Fixed-point intrinsics for when floating point isn't available or for 
*  software rasterizers where a specific subpixel covering is required
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_FIXED_POINT
#define PLUSHIE_CSTL_CORE_FIXED_POINT

#include "plushie/cstl/core/fixed_width.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* 32 bit Fixed point. 24.8 
*******************************************************************************/

#define PLUSHIE_FIXED_32Q8_ONE (1 << 8)
#define PLUSHIE_FIXED_32Q8_HALF (PLUSHIE_FIXED_32Q8_ONE >> 1)
#define PLUSHIE_FIXED_32Q8_MASK (PLUSHIE_FIXED_32Q8_ONE - 1)

/**
* @brief Fixed point precision 32 bit value, with 8 bits dedicated to the 
* fractional component. 
*/
typedef PlushieSInt32 PlushieFixed32Q8;

/**
* @brief Converts a floating point number into a fixed point 
*/
static inline PlushieFixed32Q8 plushie_fixed32Q8_to_fixed(float f)
{
	float s = f * PLUSHIE_FIXED_32Q8_ONE; // Set base
	s += (s >= 0.0f ? 0.5f : -0.5f);      // Add bias (Half away from zero)
	return (PlushieFixed32Q8)s;           // Cast result
}

static inline float plushie_fixed32Q8_to_float(PlushieFixed32Q8 s)
{
	float f = (float)s;
	return f /= PLUSHIE_FIXED_32Q8_ONE;
}



/******************************************************************************
* End of file
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // PLUSHIE_CSTL_CORE_FIXED_POINT
