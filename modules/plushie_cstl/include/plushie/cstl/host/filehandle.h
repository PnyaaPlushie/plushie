/******************************************************************************
* @file include/plushie/cstl/host/filehandle.h
* @brief Os specific file handle abstraction
* @license MIT
* @authors L Green
*******************************************************************************/
#ifndef PLUSHIE_CSTL_HOST_FILEHANDLE_H
#define PLUSHIE_CSTL_HOST_FILEHANDLE_H

#ifndef PLUSHIE_CSTL_HOST_FILEHANDLE_PROVIDE_IMPL
#  define PLUSHIE_CSTL_HOST_FILEHANDLE_PROVIDE_IMPL 0
#endif

#include "plushie/cstl/core/platform.h"
#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/error_codes.h"
#include "plushie/cstl/containers/string.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Enums
*******************************************************************************/

typedef enum PlushieFileHandleStatusFlag
{
	kPlushieFileHandleStatusFlagNone       = 0,
	kPlushieFileHandleStatusFlagIsOwned    = (1 << 0),
	kPlushieFileHandleStatusFlagIsOpen     = (1 << 1),
	kPlushieFileHandleStatusFlagIsEof      = (1 << 2),
	kPlushieFileHandleStatusFlagIsErr      = (1 << 3),
	kPlushieFileHandleStatusFlagIsReadable = (1 << 4),
	kPlushieFileHandleStatusFlagIsWritable = (1 << 5),
	kPlushieFileHandleStatusFlagIsSeekable = (1 << 6),
}
PlushieFileHandleStatusFlag;

typedef enum PlushieFileHandleAccessFlag
{
	kPlushieFileHandleAccessFlagNone  = 0,
	kPlushieFileHandleAccessFlagRead  = (1 << 0),
	kPlushieFileHandleAccessFlagWrite = (1 << 1),
}
PlushieFileHandleAccessFlag;

typedef enum PlushieFileHandleDisposition
{
	kPlushieFileHandleDispositionOpenExisting,      // Exists: Open      | Not Exists: Fail
	kPlushieFileHandleDispositionOpenOrCreate,      // Exists: Open      | Not Exists: Create
	kPlushieFileHandleDispositionTruncateExisting,  // Exists: Truncate  | Not Exists: Fail
	kPlushieFileHandleDispositionTruncateOrCreate,  // Exists: Truncate  | Not Exists: Create
	kPlushieFileHandleDispositionCreateNew          // Exists: Fail      | Not Exists: Create
}
PlushieFileHandleDisposition;

typedef enum PlushieFileHandleSharingFlag
{
	kPlushieFileHandleSharingFlagNone   = 0,
	kPlushieFileHandleSharingFlagRead   = (1 << 0),
	kPlushieFileHandleSharingFlagWrite  = (1 << 1),
	kPlushieFileHandleSharingFileDelete = (1 << 2),
}
PlushieFileHandleSharingFlag;

/******************************************************************************
* Declarations
*******************************************************************************/

typedef struct PlushieFileHandle
{
	union
	{
		void* pvoid;
		int   int_descriptor;
	}
	u;

	PlushieUInt32 status_flags;
}
PlushieFileHandle;

typedef struct PlushieFileHandleCreateInfo
{
	PlushieStr                   path;
	PlushieUInt32                access_flags;
	PlushieFileHandleDisposition disposition;
	PlushieUInt08                sharing;
}
PlushieFileHandleCreateInfo;

PLUSHIE_CSTL_API PlushieResult plushie_file_handle_open(
	PlushieFileHandle*           out,
	PlushieFileHandleCreateInfo* info
);

PLUSHIE_CSTL_API PlushieResult plushie_file_handle_close(
	PlushieFileHandle* handle
);

PLUSHIE_CSTL_API PlushieResult plushie_file_handle_write(
	PlushieFileHandle* handle,
	const void*        ptr,
	PlushieUSize       len
);

/******************************************************************************
* Implementation
*******************************************************************************/

#if PLUSHIE_CSTL_HOST_FILEHANDLE_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_HOST_FILEHANDLE_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_HOST_FILEHANDLE_PROVIDE_IMPL_GUARD

#if PLUSHIE_TARGET_OS_ID == PLUSHIE_TARGET_OS_ID_WINDOWS

#include <Windows.h>

PLUSHIE_CSTL_API PlushieResult plushie_file_handle_open(
	PlushieFileHandle*           out,
	PlushieFileHandleCreateInfo* info
)
{
	if (out == NULL || info == NULL) return kPlushieResultErrNullptr;
	if ((info->path.flags & kPlushieStrFlagStatusHasNullTerm) == 0)
	{
		// Paths on windows have to be null terminated
		return kPlushieResultErrInvalidParam;
	}
	DWORD access_flags = 0, share_mode = 0, disposition = 0;
	HANDLE out_handle = INVALID_HANDLE_VALUE;

	// Setup the access flags
	if (info->access_flags & kPlushieFileHandleAccessFlagWrite)
	{
		access_flags |= GENERIC_WRITE;
	}

	if (info->access_flags & kPlushieFileHandleAccessFlagRead)
	{
		access_flags |= GENERIC_READ;
	}

	// Setup the disposition flags
	switch (info->disposition)
	{
	case kPlushieFileHandleDispositionOpenExisting:
		disposition = OPEN_EXISTING; break;
	case kPlushieFileHandleDispositionOpenOrCreate:
		disposition = OPEN_ALWAYS; break;
	case kPlushieFileHandleDispositionTruncateExisting:
		disposition = TRUNCATE_EXISTING; break;
	case kPlushieFileHandleDispositionTruncateOrCreate:
		disposition = CREATE_ALWAYS; break;
	case kPlushieFileHandleDispositionCreateNew:
		disposition = CREATE_NEW; break;
	default:
		return kPlushieResultErrInvalidParam;
	}

	// OS call changes based on path encoding
	if (info->path.flags & kPlushieStrFlagEncodingWide16)
	{
		out_handle = CreateFileW(
			info->path.data.wstr,
			access_flags, share_mode, NULL, disposition, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	else
	{
		out_handle = CreateFileA(
			info->path.data.str,
			access_flags, share_mode, NULL, disposition, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	if (out_handle == INVALID_HANDLE_VALUE) return kPlushieResultErrOs;

	// Output to the caller
	out->u.pvoid = out_handle;
	out->status_flags = kPlushieFileHandleStatusFlagIsOpen
		| kPlushieFileHandleStatusFlagIsOwned;

	return kPlushieResultSuccess;
}

PLUSHIE_CSTL_API PlushieResult plushie_file_handle_close(
	PlushieFileHandle* handle
)
{
	if (handle == NULL) return kPlushieResultErrNullptr;
	if (handle->u.pvoid == NULL || handle->u.pvoid == INVALID_HANDLE_VALUE)
	{
		return kPlushieResultErrInvalidParam;
	}

	CloseHandle((HANDLE)handle->u.pvoid);
	handle->u.pvoid = INVALID_HANDLE_VALUE;

	return kPlushieResultSuccess;
}

PLUSHIE_CSTL_API PlushieResult plushie_file_handle_write(
	PlushieFileHandle* handle,
	const void*        ptr,
	PlushieUSize       len
)
{
	if (handle == NULL || ptr == NULL) return kPlushieResultErrNullptr;

	DWORD written_amount = 0;
	BOOL res = WriteFile((HANDLE)handle->u.pvoid, ptr,
		(DWORD)(len & 0xffffffff), &written_amount, NULL);

	if (res == FALSE) return kPlushieResultErrOs;
	return kPlushieResultSuccess;
}


#endif // PLUSHIE_TARGET_OS_ID_WINDOWS

#endif // !PLUSHIE_CSTL_HOST_FILEHANDLE_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_HOST_FILEHANDLE_PROVIDE_IMPL

/******************************************************************************
* End of file
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // !PLUSHIE_CSTL_HOST_FILEHANDLE_H
