/******************************************************************************
* @file include/plushie/cstl/filetypes/elf.h
* @brief Very small micro parser for elf files, which I'm using for my
* riscv emulator.
* @license MIT
* @authors L Green
*
* @note Heavily references https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.eheader.html
* @note Heavily references https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
*
* @note A lot of elf file parsers provide different structs per bit width. That
* makes sense for process loading. Our microparser is for loading into a riscv
* emulator from a 64 bit host, so we provide just the u64 width types but use
* the correct (de)serialization per file type.
*******************************************************************************/
#ifndef PLUSHIE_CSTL_FILETYPES_ELF_H
#define PLUSHIE_CSTL_FILETYPES_ELF_H

#include "plushie/cstl/core/fixed_width.h"
#include "plushie/cstl/core/error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief By default we assume that the including file doesn't want to provide
* the implementation; the user should place that in one source file by defining
* PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL=1 before the include.
*/
#ifndef PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL
#  define PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL 0
#endif

/******************************************************************************
* Identification. Focuses on the first 16 bytes of the elf file.
* This includes the target platform and deriving the endianness of the file
* and even whether it is an elf file.
*******************************************************************************/

/**
* @brief The first 16 bytes of the elf file are used for the identification of
* target and interpretation of the data.
*/
typedef enum PlushieElf_Ident
{
	kPlushieElf_Ident_Magic0       = 0,  // 0x7f
	kPlushieElf_Ident_Magic1       = 1,  // 'E'
	kPlushieElf_Ident_Magic2       = 2,  // 'L'
	kPlushieElf_Ident_Magic3       = 3,  // 'F'
	kPlushieElf_Ident_FileClass    = 4,  // Index of byte storing the bit width of this elf
	kPlushieElf_Ident_DataEncoding = 5,  // Index of byte storing the endianness of this elf
	kPlushieElf_Ident_FileVersion  = 6,  // Index of byte storing file version of this elf
	kPlushieElf_Ident_OsAbi        = 7,  // Index of byte storing target os and/or abi
	kPlushieElf_Ident_AbiVersion   = 8,  // Index of byte storing abi ver
	kPlushieElf_Ident_PadStart     = 9,  // Index of first pad byte
	kPlushieElf_Ident_PadEnd       = 15, // Index of last pad byte
	kPlushieElf_Ident_Count        = 16, // Number of bytes you need to read for an ident
}
PlushieElf_Ident;

typedef enum PlushieElf_FileClass
{
	kPlushieElf_FileClass_Invalid = 0,
	kPlushieElf_FileClass_32bit   = 1,
	kPlushieElf_FileClass_64bit   = 2,
}
PlushieElf_FileClass;

typedef enum PlushieElf_DataEncoding
{
	kPlushieElf_DataEncoding_Invalid      = 0,
	kPlushieElf_DataEncoding_LittleEndian = 1,
	kPlushieElf_DataEncoding_BigEndian    = 2,
}
PlushieElf_DataEncoding;

typedef enum PlushieElf_OsAbi
{
	kPlushieElf_OsAbi_None = 0
}
PlushieElf_OsAbi;

typedef struct PlushieElf_StructuredIdent
{
	PlushieElf_FileClass    file_class;
	PlushieElf_DataEncoding encoding;
	PlushieUInt08           file_version;
	PlushieElf_OsAbi        abi;
	PlushieUInt08           abi_version;
}
PlushieElf_StructuredIdent;

/**
* @brief Attempts to validate and deserialize the identity section of the elf
* file
*/
PLUSHIE_CSTL_API PlushieResult plushie_elf_structured_ident_deserialize(
	PlushieElf_StructuredIdent* out_ident,
	PlushieUInt08*              bytes_ptr,
	PlushieUSize                bytes_len
);

/******************************************************************************
* Header section.
* In order to be read correctly, the data encoding of the source elf file needs
* establishing via the identity section.
*******************************************************************************/

typedef enum PlushieElf_ObjectType
{
	kPlushieElf_ObjectType_Unknown     = 0,
	kPlushieElf_ObjectType_Relocatable = 1,
	kPlushieElf_ObjectType_Executable  = 2,
	kPlushieElf_ObjectType_Dynamic     = 3,
	kPlushieElf_ObjectType_Core        = 4,
	kPlushieElf_ObjectType_Max         = 0xffff,
}
PlushieElf_ObjectType;

typedef enum PlushieElf_Machine
{
	kPlushieElf_Machine_None          = 0x0000,
	kPlushieElf_Machine_X86           = 0x0003,
	kPlushieElf_Machine_Mips          = 0x0008,
	kPlushieElf_Machine_PowerPc32     = 0x0014,
	kPlushieElf_Machine_PowerPc64     = 0x0015,
	kPlushieElf_Machine_Amdx8664      = 0x003e,
	kPlushieElf_Machine_Armv7_AArch32 = 0x0028,
	kPlushieElf_Machine_Armv8_AArch64 = 0x00b7,
	kPlushieElf_Machine_RiscV         = 0x00f3,
	kPlushieElf_Machine_Max           = 0xffff,
}
PlushieElf_Machine;

typedef struct PlushieElf_StructuredHeader
{
	PlushieUInt64 entry_addr;
	PlushieUInt64 program_header_table_offset;
	PlushieUInt64 section_header_table_offset;
	PlushieUInt32 flags;
	PlushieUInt16 header_size;

	PlushieUInt16 program_header_table_entry_size;
	PlushieUInt16 program_header_table_entry_count;
	PlushieUInt16 section_header_table_entry_size;
	PlushieUInt16 section_header_table_entry_count;

	PlushieUInt16 section_names_index;
}
PlushieElf_StructuredHeader;

/******************************************************************************
* Program Header
* Contains the segments which are responsible for memory allocations and flags
* inside the elf into virtual memory
*******************************************************************************/

typedef struct PlushieElf_ProgramHeader
{
	PlushieUInt08 _todo;
}
PlushieElf_ProgramHeader;

/******************************************************************************
* Section Header
* Responsible for the organized sections of memory in the elf, not all of these
* are mapped into memory
*******************************************************************************/

typedef struct PlushieElf_SectionHeader
{
	PlushieUInt08 _todo;
}
PlushieElf_SectionHeader;

/******************************************************************************
* Implementation section
*******************************************************************************/
#if PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL
#ifndef PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL_GUARD
#define PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL_GUARD

PLUSHIE_CSTL_API PlushieResult plushie_elf_structured_ident_deserialize(
	PlushieElf_StructuredIdent* out_ident,
	PlushieUInt08*              bytes_ptr,
	PlushieUSize                bytes_len
)
{
	if (out_ident == NULL || bytes_ptr == NULL)
	{
		return kPlushieResultErrNullptr;
	}
	if (bytes_len < kPlushieElf_Ident_Count)
	{
		return kPlushieResultErrEof;
	}

	// Check the magic number
	if (bytes_ptr[kPlushieElf_Ident_Magic0] != 0x7f ||
		bytes_ptr[kPlushieElf_Ident_Magic1] != 'E' ||
		bytes_ptr[kPlushieElf_Ident_Magic2] != 'L' ||
		bytes_ptr[kPlushieElf_Ident_Magic3] != 'F')
	{
		return kPlushieResultErrParse;
	}

	// The file class and the data encodings must both be non-zero, as zero means
	// invalid.
	if (bytes_ptr[kPlushieElf_Ident_FileClass] == 0) return kPlushieResultErrParse;
	if (bytes_ptr[kPlushieElf_Ident_DataEncoding] == 0) return kPlushieResultErrParse;

	// Output
	out_ident->file_class   = (PlushieElf_FileClass)bytes_ptr[kPlushieElf_Ident_FileClass];
	out_ident->encoding     = (PlushieElf_DataEncoding)bytes_ptr[kPlushieElf_Ident_DataEncoding];
	out_ident->file_version = bytes_ptr[kPlushieElf_Ident_FileVersion];
	out_ident->abi          = (PlushieElf_OsAbi)bytes_ptr[kPlushieElf_Ident_OsAbi];
	out_ident->abi_version  = bytes_ptr[kPlushieElf_Ident_AbiVersion];
	return kPlushieResultSuccess;
}

#endif // !PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL_GUARD
#endif // !PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL

/******************************************************************************
* End of File
*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // !PLUSHIE_CSTL_FILETYPES_ELF_H
