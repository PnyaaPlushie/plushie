/******************************************************************************
* @file include/plushie/cstl/filetypes/bmp.h
* @brief Minimal BMP (bitmap) file helpers
* @license MIT
* @authors L Green
*
* @ref https://en.wikipedia.org/wiki/BMP_file_format
*******************************************************************************/
#ifndef PLUSHIE_CSTL_FILETYPES_BMP_H
#define PLUSHIE_CSTL_FILETYPES_BMP_H

#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/error_codes.h"
#include "plushie/cstl/core/serde.h"

// Macro to force the header to provide the implementation for the functions
#ifndef PLUSHIE_CSTL_FILETYPES_BMP_PROVIDE_IMPL
#  define PLUSHIE_CSTL_FILETYPES_BMP_PROVIDE_IMPL 0
#endif

// Fixed sizes of the on-disk headers this module understands
#define PLUSHIE_BMP_FILE_HEADER_SIZE 14
#define PLUSHIE_BMP_INFO_HEADER_SIZE 40

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Enum declarations
*******************************************************************************/

/**
* @brief Different kinds of bitmap dib information which is located directly
* after the file header, this describes the details
*/
typedef enum PlushieBmpDibHeaderKind
{
	kPlushieBmpDibHeaderKindCore,   //
	kPlushieBmpDibHeaderKindOs21x,  //
	kPlushieBmpDibHeaderKindOs22x,  //
	kPlushieBmpDibHeaderKindInfo,   // !< The main supported profile, for win32
	kPlushieBmpDibHeaderKindInfoV2, //
	kPlushieBmpDibHeaderKindInfoV3, //
	kPlushieBmpDibHeaderKindInfoV4, //
	kPlushieBmpDibHeaderKindInfoV5, //
}
PlushieBmpDibHeaderKind;

/**
* @brief Identify the number of bits per pixel being used with an enum
*/
typedef enum PlushieBmpBppKind
{
	kPlushieBmpBppKind1bpp,  // !< Less than a byte - Unsupported rn
	kPlushieBmpBppKind4bpp,  // !< Less than a byte - Unsupported rn
	kPlushieBmpBppKind8bpp,  //
	kPlushieBmpBppKind16bpp, //
	kPlushieBmpBppKind24bpp, // !< BGR8 no alpha channel main kind
	kPlushieBmpBppKind32bpp, //
}
PlushieBmpBppKind;

/******************************************************************************
* User facing struct declarations
*******************************************************************************/

typedef struct PlushieBmpDescriptor
{
	PlushieUInt32 width;
	PlushieUInt32 height;
	PlushieUInt08 bpp;
}
PlushieBmpDescriptor;

//
// BmpFile::FileHeader
//

#define PLUSHIE_BMP_FILEHEADER_SCHEMA(PLUSHIE_XFIELD, PLUSHIE_XARRAY, PLUSHIE_XBLOB) \
    PLUSHIE_XBLOB(magic             , 2)                      \
    PLUSHIE_XFIELD(file_size        , PlushieUInt32, ui32_le) \
    PLUSHIE_XBLOB(app_specific      , 4)                      \
    PLUSHIE_XFIELD(pixel_data_offset, PlushieUInt32, ui32_le)

typedef struct PlushieBmpFileHeader
{
	PLUSHIE_BMP_FILEHEADER_SCHEMA(
		PLUSHIE_SERDE_DECL_FIELD,
		PLUSHIE_SERDE_DECL_ARRAY,
		PLUSHIE_SERDE_DECL_BLOB
	)
}
PlushieBmpFileHeader;

//
// BmpFile::DIB::BITMAPINFOHEADER
//

#define PLUSHIE_BMP_DIPBITMAPINFOHEADER_SCHEMA(PLUSHIE_X)     \
    PLUSHIE_X(PlushieUInt32, sizeof_this,           int32_le) \
	PLUSHIE_X(PlushieSInt32, pxwidth,               int32_le) \
	PLUSHIE_X(PlushieSInt32, pxheight,              int32_le) \
	PLUSHIE_X(PlushieUInt16, color_planes_count,    int16_le) \
	PLUSHIE_X(PlushieUInt16, bpp,                   int16_le) \
	PLUSHIE_X(PlushieUInt32, compression_id,        int32_le) \
	PLUSHIE_X(PlushieUInt32, image_size,            int32_le) \
	PLUSHIE_X(PlushieSInt32, res_x,                 int32_le) \
	PLUSHIE_X(PlushieSInt32, res_y,                 int32_le) \
	PLUSHIE_X(PlushieUInt32, color_palette_count,   int32_le) \
	PLUSHIE_X(PlushieUInt32, color_important_count, int32_le)

/**
* @brief The most common dib header for bmp files. `BITMAPINFOHEADER`, which
* seems to be compatible with windows NT and later.
*/
typedef struct PlushieBmpDipBitmapInfoHeader
{
#define PLUSHIE_STRUCT_FIELD_DECL(field_type, field_name, serde) \
    field_type field_name;

	PLUSHIE_BMP_DIPBITMAPINFOHEADER_SCHEMA(
		PLUSHIE_STRUCT_FIELD_DECL
	)

#undef PLUSHIE_STRUCT_FIELD_DECL
}
PlushieBmpDipBitmapInfoHeader;

/**
* @brief Calculates the number of bytes in a single scan line of a bmp file.
* Rows are padded up to a 4 byte boundary.
*/
static inline PlushieUSize plushie_bmp_line_stride(
	PlushieUInt32 px_width,
	PlushieUInt08 bpp)
{
	return ((31 + ((PlushieUSize)px_width) * ((PlushieUSize)bpp)) / 32) * 4;
}

/**
* @brief Calculates the total on-disk byte size of a bmp file described by desc
* (file header + BITMAPINFOHEADER + padded pixel rows).
*/
static inline PlushieResult plushie_bmp_file_buffer_size(
	PlushieUSize*         out_size,
	PlushieBmpDescriptor* desc
)
{
	if (out_size == NULL || desc == NULL) return kPlushieResultErrNullptr;

	PlushieUSize stride = plushie_bmp_line_stride(desc->width, desc->bpp);
	*out_size = (PlushieUSize)PLUSHIE_BMP_FILE_HEADER_SIZE
	          + (PlushieUSize)PLUSHIE_BMP_INFO_HEADER_SIZE
	          + stride * (PlushieUSize)desc->height;
	return kPlushieResultSuccess;
}

/**
* @brief Byte offset of pixel (x, y) within the pixel-data region for a given
* row stride, top-down. Callers writing a real BMP (bottom-up) should flip y.
*/
static inline PlushieUSize plushie_bmp_offset_to_pixel(
	PlushieUInt32 x,
	PlushieUInt32 y,
	PlushieUInt32 px_width,
	PlushieUInt08 bpp
)
{
	PlushieUSize stride = plushie_bmp_line_stride(px_width, bpp);
	return (PlushieUSize)y * stride + (PlushieUSize)x * ((PlushieUSize)bpp / 8);
}

/******************************************************************************
* Implementation
*******************************************************************************/
#if PLUSHIE_CSTL_FILETYPES_BMP_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_FILETYPES_BMP_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_FILETYPES_BMP_PROVIDE_IMPL_GUARD

// (No out-of-line implementation yet; all functions above are static inline.)

#endif // !PLUSHIE_CSTL_FILETYPES_BMP_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_FILETYPES_BMP_PROVIDE_IMPL

/******************************************************************************
* End of file content
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_FILETYPES_BMP_H
