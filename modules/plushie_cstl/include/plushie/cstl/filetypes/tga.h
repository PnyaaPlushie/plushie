/******************************************************************************
* @file include/plushie/cstl/filetypes/tga.h
* @brief Simplest image format I could find which is widely supported for
* viewing
* @license MIT
* @authors L Green
*
* @ref https://en.wikipedia.org/wiki/Truevision_TGA
* @note For quick implementation, just does RGB8Linear uncompressed
*******************************************************************************/
#ifndef PLUSHIE_CSTL_FILETYPES_TGA_H
#define PLUSHIE_CSTL_FILETYPES_TGA_H

#include "plushie/cstl/core/fixed_width.h"

#ifndef PLUSHIE_CSTL_FILETYPES_TGA_PROVIDE_IMPL
#  define PLUSHIE_CSTL_FILETYPES_TGA_PROVIDE_IMPL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Declarations
*******************************************************************************/

/**
* @brief Calculates the buffer size the user needs to allocate for a 24bit per
* pixel true color image inside a tga file
*/
static inline PlushieUSize plushie_tgafile_buffer_size(
    PlushieUInt32 width,
    PlushieUInt32 height
)
{
    return 0x12 + width * height * 3; // 18 byte header, uncompressed data
}

/**
* @brief Initializes the header of the tga file and returns a pointer to the
* image data that the user can start writing to
*
* @todo Use proper serialization of fields
*/
static inline void* plushie_tgafile_buffer_init(
    void*         start_ptr,
    PlushieUInt32 width,
    PlushieUInt32 height
)
{
    PlushieUInt08* ptr = (PlushieUInt08*)start_ptr;

    // Todo: Use the zero init function
    for (PlushieUInt08 i = 0; i < 0x12; i++) { ptr[i] = 0; }

    ptr[2] = 2;                     // True color no compression
    ptr[12] = 0xff & width;         // little endian width
    ptr[13] = 0xff & (width >> 8);
    ptr[14] = 0xff & height;        // little endian height
    ptr[15] = 0xff & (height >> 8);
    ptr[16] = 24;                   // bits per pixel


    // Image descriptor
    // [7-6] interleave mode    : None: (00)
    // [5]   top to bottom?     : Yes : (1)
    // [4]   right-to-left?     : No  : (0)
    // [3-0] alpha channel depth: None: (0000)
    // 00100000 = 32
    ptr[17] = 32;

    // Return out
    return ptr + 0x12;
}

/******************************************************************************
* Implementation
*******************************************************************************/
#if PLUSHIE_CSTL_FILETYPES_TGA_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_FILETYPES_TGA_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_FILETYPES_TGA_PROVIDE_IMPL_GUARD

// (No out-of-line implementation yet; all functions above are static inline.)

#endif // !PLUSHIE_CSTL_FILETYPES_TGA_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_FILETYPES_TGA_PROVIDE_IMPL

/******************************************************************************
* End of file
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // !PLUSHIE_CSTL_FILETYPES_TGA_H
