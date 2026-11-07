/******************************************************************************
* @file include/plushie/cstl/core/calling_conv.h
* @brief Symbol visibility and the cstl usage kind (header-only / shared /
*  static) that drives it.
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CORE_CALLING_CONV_H
#define PLUSHIE_CSTL_CORE_CALLING_CONV_H

#include "plushie/cstl/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* CSTL Function Definition Kind
* Different methods of using plushie libraries including the cstl
*******************************************************************************/

#define PLUSHIE_USAGE_ID_NONE        0
#define PLUSHIE_USAGE_ID_HEADER_LIB  1
#define PLUSHIE_USAGE_ID_SHARED_LIB  2
#define PLUSHIE_USAGE_ID_STATIC_LIB  3

#ifndef PLUSHIE_CSTL_USAGE_ID
#  define PLUSHIE_CSTL_USAGE_ID PLUSHIE_USAGE_ID_HEADER_LIB
#endif

/******************************************************************************
* Symbol Visibility
*******************************************************************************/

#ifndef PLUSHIE_EXPORT
#  if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
#    define PLUSHIE_EXPORT __declspec(dllexport)
#  else
#    define PLUSHIE_EXPORT
#  endif
#endif

#ifndef PLUSHIE_IMPORT
#  if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
#    define PLUSHIE_IMPORT __declspec(dllimport)
#  else
#    define PLUSHIE_IMPORT
#  endif
#endif

#ifndef PLUSHIE_API_CSTL
#  if PLUSHIE_CSTL_USAGE_ID == PLUSHIE_USAGE_ID_SHARED_LIB
#    define PLUSHIE_API_CSTL PLUSHIE_IMPORT
#  else
#    define PLUSHIE_API_CSTL
#  endif
#endif

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_CORE_CALLING_CONV_H
