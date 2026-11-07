/******************************************************************************
* @file include/plushie/cstl/containers/string.h
* @brief Plushie string type, tries to handle a bunch of encoding and stuff.
* The type is a little wider than a regular ptr, len combo, but tries to keep
* the type size reasonably small.
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_CONTAINERS_STRING_H
#define PLUSHIE_CSTL_CONTAINERS_STRING_H

#include "plushie/cstl/core/platform.h"
#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/calling_conv.h"
#include "plushie/cstl/core/error_codes.h"

// Macro to force the header to provide the implementation for the functions
#ifndef PLUSHIE_CSTL_CONTAINERS_STRING_PROVIDE_IMPL
#  define PLUSHIE_CSTL_CONTAINERS_STRING_PROVIDE_IMPL 0
#endif

// By default we assume you want a vector type, but it can be disabled
#ifndef PLUSHIE_CSTL_CONTAINERS_STRING_INCLUDE_VEC_TYPE
#  define PLUSHIE_CSTL_CONTAINERS_STRING_INCLUDE_VEC_TYPE 1
#endif
#if PLUSHIE_CSTL_CONTAINERS_STRING_INCLUDE_VEC_TYPE
#include "plushie/cstl/containers/vector.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Plushie base string type, ensuring that we capture the length,
* encoding and other details about the string. Ideally we didn't want to have
* a bunch of different string types.
*
* @note If you're creating an auto expandable string, we place the allocator
* as a header in front of the pointer.
* @note Flags entry requires 16 bits minimum
*/
typedef struct PlushieStr
{
    union
    {
        const void*          ptr;    // !< Typeless memory address
        const char*          str;    // !< Encoded as ascii or multibyte
        const wchar_t*       wstr;   // !< 2 byte wide char, careful how wide char is interpreted on linux
        const PlushieUInt32* utf32;  // !< unicode codepoints encoded as u32
    }
    data;                            //
    PlushieUSize len;                // !< Length encodes the byte count of the valid chars in string
    PlushieUSize capacity;           //
    PlushieUSize flags;              //
}
PlushieStr;

/**
* @brief Different flags attached to a string
*/
typedef enum PlushieStrFlag
{
    kPlushieStrFlagNone               = 0,      // !< Book keeping flag

    // Ownership and memory status
    kPlushieStrFlagOwnershipIsOwned   = 1 << 0, // !< This pointer is the actual allocation
    kPlushieStrFlagOwnershipIsView    = 1 << 1, // !< Pointer is a slice from another memory region
    kPlushieStrFlagOwnershipIsConst   = 1 << 2, // !< Read only memory
    kPlushieStrFlagOwnershipIsResize  = 1 << 3, // !< Can be resized

    // Encoding
    kPlushieStrFlagEncodingAscii      = 1 << 4, // !< Encoded as ascii
    kPlushieStrFlagEncodingWide16     = 1 << 5, // !< Encoded as win32 wide string
    kPlushieStrFlagEncodingUtf8       = 1 << 6, // !< Encoded as utf8
    kPlushieStrFlagEncodingUtf32      = 1 << 7, // !< Encoded as utf32

    // Additional flags
    kPlushieStrFlagStatusHasNullTerm  = 1 << 8, // !< Memory has final null terminator
    kPlushieStrFlagStatusHasAllocator = 1 << 9, // !< Pointer has an allocator header

    kPlushieStrFlagMax                = 0xffff, // !< Book keeping flag
}
PlushieStrFlag;


#if PLUSHIE_CSTL_CONTAINERS_STRING_INCLUDE_VEC_TYPE
PlushieVectorDecl(PlushieVecStr, PlushieStr, str);
#endif

/******************************************************************************
* Function declarations
*******************************************************************************/
/**
* @brief Create a plushie string from a standard null terminated CString. There
* is no copy taken, so the valid lifetime of this string is limited by that of
* the input pointer.
* @note This is perfect for wrapping static lifetime strings
* @note Since this is a cstring we're assuming utf8 encoding by default
*/
PlushieResult plushie_str_new_view_from_cstr(
    PlushieStr*  out_str,
    const void*  ptr,
    PlushieUSize size
);

/**
* @brief Take a win32 formatted wstring (ie 16bit chars) and transmute it into a
* string-view which has its lifetime determined by the input ptr. No copy is
* taken.
*/
PlushieResult plushie_str_new_view_from_win32_wstr(
    PlushieStr*    out_str,
    const wchar_t* ptr
);

/**
* @brief Converts the string from src type encoding into the dst encoding type.
* @note Allocator is an optional flag to pass for a new copy
*/
PlushieResult plushie_str_convert(
    PlushieStr*              src_str,
    PlushieStr*              dst_str,
    struct PlushieAllocator* alloc
);

/**
* @brief Calls the embedded allocator for a memory backed string in order to
* release that memory
*/
void plushie_str_release(
    PlushieStr* src_str
);

// -- Raw helpers for working with unwrapped buffers

/**
* @brief String raw convert for win wide cstring preallocated. A raw helper
* function which converts a win32 wide string to utf8, this is when the wstring
* has a null terminator. The output string will be null terminated.
*
* @note Both inputs and outputs are null terminated, sizes include the nullterm
* @note See `plushie_str_rawcvrt_wwcstr_size` to precalculate the size of
* buffer required
*/
PLUSHIE_API_CSTL PlushieResult plushie_str_rawcvrt_wwcstr(
    char*            out_str,
    PlushieUSize*    out_size,
    PlushieUSize     buff_size,
    PlushieWinWchar* pwstr
);

/**
* @brief String raw convert for win wide cstring get size. A raw helper
* function which calculates the size of an allocation required to convert a
* win32 wide string to utf8, when that wstring is explicitly null terminated.
*/
PLUSHIE_API_CSTL PlushieResult plushie_str_rawcvrt_wwcstr_size(
    PlushieUSize*    out_size,
    PlushieWinWchar* pwstr
);

/******************************************************************************
* Function definition
*******************************************************************************/

#if PLUSHIE_CSTL_CONTAINERS_STRING_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_CONTAINERS_STRING_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_CONTAINERS_STRING_PROVIDE_IMPL_GUARD

PlushieResult plushie_str_new_view_from_cstr(
    PlushieStr*  out_str,
    const void*  ptr,
    PlushieUSize size
)
{
    if (out_str == NULL || ptr == NULL) return kPlushieResultErrNullptr;
    if (size == 0) return kPlushieResultErrInvalidParam;

    *out_str = (PlushieStr)
    {
        .data.ptr = ptr,
        .len = size - 1,
        .capacity = size,
        .flags = kPlushieStrFlagOwnershipIsView
               | kPlushieStrFlagEncodingUtf8
               | kPlushieStrFlagStatusHasNullTerm
    };

    return kPlushieResultSuccess;
}

PlushieResult plushie_str_new_view_from_win32_wstr(
    PlushieStr*    out_str,
    const wchar_t* ptr
)
{
    if (out_str == NULL || ptr == NULL) return kPlushieResultErrNullptr;

    out_str->data.wstr = ptr;

    return kPlushieResultSuccess;
}

// --
// -- Raw helpers for working with unwrapped buffers
// --

#if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
#include <Windows.h>

PLUSHIE_API_CSTL PlushieResult plushie_str_rawcvrt_wwcstr(
    char*            out_str,
    PlushieUSize*    out_size,
    PlushieUSize     buff_size,
    PlushieWinWchar* pwstr
)
{
    if (out_str == NULL || out_size == NULL || pwstr == NULL)
        return kPlushieResultErrNullptr;

    // Requires windows 2000 and Kernel32.dll (Fine limitation)
    // TODO [#64] Provide alternative
    int result = WideCharToMultiByte(CP_UTF8, 0, pwstr,
        -1, // Means input is nullterminated like a CStr
        out_str, (int)buff_size, NULL, NULL);

    // Check for success?
    if (result == 0)
    {
        return kPlushieResultErrOs;
    }

    // Success status
    *out_size = (PlushieUSize)result;
    return kPlushieResultSuccess;
}

PLUSHIE_API_CSTL PlushieResult plushie_str_rawcvrt_wwcstr_size(
    PlushieUSize*    out_size,
    PlushieWinWchar* pwstr
)
{
    if (out_size == NULL || pwstr == NULL) return kPlushieResultErrNullptr;

    // Requires windows 2000 and Kernel32.dll (Fine limitation)
    // TODO [#64] Provide alternative
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, pwstr,
        -1, // Means input is nullterminated like a CStr
        NULL, 0, NULL, NULL);
    if (size_needed <= 0) return kPlushieResultErrOs;

    // Cast output
    *out_size = (PlushieUSize)size_needed;
    return kPlushieResultSuccess;
}
#endif // PLUSHIE_TARGET_OS_ID_WINDOWS

#if PLUSHIE_CSTL_CONTAINERS_STRING_INCLUDE_VEC_TYPE
PlushieVectorDefine(PlushieVecStr, PlushieStr, str);
#endif

#endif // PLUSHIE_CSTL_CONTAINERS_STRING_PROVIDE_IMPL_GUARD
#endif // PLUSHIE_CSTL_CONTAINERS_STRING_PROVIDE_IMPL

/******************************************************************************
* End of implementation and guards
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // !PLUSHIE_CSTL_CONTAINERS_STRING_H
