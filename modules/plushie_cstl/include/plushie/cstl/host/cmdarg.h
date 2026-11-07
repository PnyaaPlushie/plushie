/******************************************************************************
* @file include/plushie/cstl/host/cmdarg.h
* @brief Helper for converting raw cmdline into utf8 vectors
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_HOST_CMDARG_H
#define PLUSHIE_CSTL_HOST_CMDARG_H

#include "plushie/cstl/core/platform.h"
#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/error_codes.h"
#include "plushie/cstl/core/alloc.h"
#include "plushie/cstl/containers/string.h"

// Macro to force the header to provide the implementation for the functions
#ifndef PLUSHIE_CSTL_HOST_CMDARG_PROVIDE_IMPL
#  define PLUSHIE_CSTL_HOST_CMDARG_PROVIDE_IMPL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS

/**
* @brief Converts a wWinMain command line into an array of utf8 PlushieStr.
* @note Keeps the null terminator for each of the strings
* @note It is valid for this function to return null for buffers and strings.
* For example, if a user passes no arguments, we don't allocate an empty space.
*/
PlushieResult plushie_cmdarg_wwinmain_convert(
    PlushieStr**             out_strs,
    PlushieUInt32*           out_strs_count,
    void**                   out_buffer,
    struct PlushieAllocator* alloc,
    PlushieWinWchar*         cmdarg
);
#endif

/******************************************************************************
* Implementation
*******************************************************************************/
#if PLUSHIE_CSTL_HOST_CMDARG_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_HOST_CMDARG_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_HOST_CMDARG_PROVIDE_IMPL_GUARD

#if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS
#include <Windows.h>

PlushieResult plushie_cmdarg_wwinmain_convert(
    PlushieStr**             out_strs,
    PlushieUInt32*           out_strs_count,
    void**                   out_buffer,
    struct PlushieAllocator* alloc,
    PlushieWinWchar*         cmdarg
)
{
    // Allocator and out buffers are all required
    if (out_strs == NULL || out_buffer == NULL || alloc == NULL || cmdarg == NULL)
        return kPlushieResultErrNullptr;
    if (alloc->alloc == NULL)
        return kPlushieResultErrAllocPfnMissing;

    // We use `CommandLineToArgvW` for conversion, it has some odd behaviour.
    // If the argstring is empty L"" then it gives you the exe's current path.
    // This isn't an argument to parse, and a caller couldn't determine if a
    // user passed one exe path, or if they get back their own exe path without
    // an opaque check. So although this is different behaviour from the function
    // we mainly depend on, we add an early exit for empty args.
    if (*cmdarg == L'\0')
    {
        *out_strs = NULL;
        *out_strs_count = 0;
        *out_buffer = NULL;
        return kPlushieResultSuccess;
    }

    PlushieResult res = kPlushieResultSuccess;
    PlushieWinWchar** argv = NULL;
    void* buffer = NULL;
    int argc = 0;

    do
    {
        // Convert the arg from winmain to the classic version. This does
        // require shell32.dll (6.0+) but for now that's fine as a limit.
        // TODO [#64]: Provide alternative
        argv = CommandLineToArgvW(cmdarg, &argc);
        if (argv == NULL) { res = kPlushieResultErrOs; break; }

        // How much memory do we need from the allocator?
        // PlushieStr alignment > char, don't need to align up
        PlushieUSize alloc_size = argc * sizeof(PlushieStr);
        PlushieUSize str_buff_size = 0;
        for (PlushieUInt32 i = 0; i < (PlushieUInt32)argc; i++)
        {
            PlushieUSize req_size = 0;
            res = plushie_str_rawcvrt_wwcstr_size(
                &req_size, argv[i]);
            if (res != kPlushieResultSuccess) break;
            str_buff_size += req_size;
        }
        if (res != kPlushieResultSuccess) break;

        // Allocate the buffer for utf8 and the PlushieStrs
        buffer = alloc->alloc(
            alloc,
            alloc_size + str_buff_size,
            PLUSHIE_ALIGNOF(PlushieStr),
            kPlushieAllocatorAllocFlagsProtectRead
             | kPlushieAllocatorAllocFlagsProtectWrite
             | kPlushieAllocatorAllocFlagsZeroInit,
            NULL);
        if (buffer == NULL)
        {
            res = kPlushieResultErrAllocOom;
            break;
        }

        // We have a buffer for the string objects and then the array of memory
        PlushieStr* str_plush = (PlushieStr*)buffer;
        (void)str_plush;
        PlushieUPtr str_mem = ((PlushieUPtr)buffer) + alloc_size;
        for (PlushieUInt32 i = 0; i < (PlushieUInt32)argc; i++)
        {
            // Convert
            PlushieUSize str_size = 0;
            res = plushie_str_rawcvrt_wwcstr((char*)str_mem, &str_size,
                str_buff_size, argv[i]);
            if (res != kPlushieResultSuccess) break;

            res = plushie_str_new_view_from_cstr(&str_plush[i], (void*)str_mem,
                str_size);
            if (res != kPlushieResultSuccess) break;

            // Move the buffer and remaining allocations
            str_mem += str_size;
            str_buff_size -= str_size;
        }
    }
    while (0);

    // Cleanup os converted argv alloc
    if (argv != NULL) { LocalFree(argv); }

    // Cleanup the buffer if we failed
    if (res != kPlushieResultSuccess)
    {
        if (buffer != NULL)
        {
            alloc->free(alloc, buffer);
            buffer = NULL;
        }
    }
    // Else return resources to the caller
    else
    {
        *out_strs = (PlushieStr*)buffer;
        *out_strs_count = (PlushieUInt32)argc;
        *out_buffer = buffer;
    }

    return res;
}
#endif // PLUSHIE_TARGET_OS_ID_WINDOWS

#endif // PLUSHIE_CSTL_HOST_CMDARG_PROVIDE_IMPL_GUARD
#endif // PLUSHIE_CSTL_HOST_CMDARG_PROVIDE_IMPL
/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_HOST_CMDARG_H
