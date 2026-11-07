// If we're using the header only implementation, then we need to explicitly request them
// inside this translation unit. This define is compile definition is added in cmake
#if PLUSHIE_CSTL_EXAMPLES_PROVIDE_IMPL
# define PLUSHIE_CSTL_ALLOCATORS_SYSTEM_PROVIDE_IMPL 1
# define PLUSHIE_CSTL_HOST_FILEHANDLE_PROVIDE_IMPL   1
# define PLUSHIE_CSTL_CONTAINERS_STRING_PROVIDE_IMPL 1
#endif


#include "plushie/cstl/core/alloc.h"
#include "plushie/cstl/allocators/libc.h"
#include "plushie/cstl/allocators/system.h"
#include "plushie/cstl/host/filehandle.h"
#include "plushie/cstl/filetypes/tga.h"
#include "plushie/cstl/containers/string.h"

#include <stdio.h>

int main() 
{
	// Output file name
	const char* file_name_raw = "tga_01_from_buffer.tga";


	// Forward declare resources that require cleanup
	PlushieAllocator* alloc = plushie_allocator_get_system(); // Allocator
	PlushieFileHandle file_handle = { 0 };                  // File handle
	uint8_t* file_buffer = NULL;                            // Dynamic sized buffer
	PlushieStr file_name;

	// Rolling return code
	PlushieResult res = kPlushieResultSuccess;

	
	// Failable section begins ------------------------------------------------
	do
	{

	// Convert the file name to be consumed by file handle library, 
	// realistically this should never fail
	res = plushie_str_new_view_from_cstr(&file_name, file_name_raw, sizeof(file_name_raw));
	if (res != kPlushieResultSuccess) { break; }

	// Getting the allocator CAN fail, i.e window fails to get process heap
	if (alloc == NULL) { res = kPlushieResultErrAllocOom; break; }

	// Allocate enough for the image
	PlushieUSize alloc_size = plushie_tgafile_buffer_size(256, 256);
	file_buffer = alloc->alloc(alloc, alloc_size, PLUSHIE_ALIGNOF(void*),
		kPlushieAllocatorAllocFlagsProtectRead
		  | kPlushieAllocatorAllocFlagsProtectWrite
		  | kPlushieAllocatorAllocFlagsZeroInit, 
		NULL);
	if (file_buffer == NULL) { res = kPlushieResultErrAllocOom; break; }


	// Since it's uncompressed image data we can write directly into 
	// the same memory the file buffer is using
	PlushieUInt08* img = plushie_tgafile_buffer_init(file_buffer, 256, 256);
	if (img == NULL) { res = kPlushieResultErrUnclassified; break; }


	// Write the image data 
	// Pixel data is BGR left to right, top to bottom 3bpp
	for (PlushieUInt32 y = 0; y < 256; y++)
	{
		for (PlushieUInt32 x = 0; x < 256; x++)
		{
			PlushieUInt32 offset = (x * 3) + (y * 256 * 3);
			
			// Make blue component brighter on the right 
			// side of the image
			img[offset] = (PlushieUInt08)(x);

			// Keep the green component constant
			img[offset + 1] = 128;

			// Make red component brightest at the top
			// of the image
			img[offset + 2] = 255 - y;
		}
	}

	// Output the file to a handle to inspect the results 
	// Now we should output the tga file for inspection
	res = plushie_file_handle_open(&file_handle,
		&(PlushieFileHandleCreateInfo)
		{
			.path = file_name,
		    .access_flags = kPlushieFileHandleAccessFlagWrite,
			.disposition = kPlushieFileHandleDispositionTruncateOrCreate,
		}
	);
	if (res != kPlushieResultSuccess) { break; }


	// Write the file out
	res = plushie_file_handle_write(&file_handle, file_buffer, alloc_size);

	// Failable section ends --------------------------------------------------
	} 
	while (0);

	// Resource cleanup

	// Deallocate
	if (file_buffer != NULL)
	{
		alloc->free(alloc, file_buffer);
		file_buffer = NULL;
	}

	// File close 
	if (file_handle.status_flags
		& (kPlushieFileHandleStatusFlagIsOwned | kPlushieFileHandleStatusFlagIsOpen))
	{
		plushie_file_handle_close(&file_handle);
	}


}