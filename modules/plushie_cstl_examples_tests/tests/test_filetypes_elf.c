// Tests the elf ident deserializer and its PlushieResult error mapping.
// Regression guard: this used to return magic negative ints (-1..-4); it now
// returns typed PlushieResult codes.
#include "tests_shared.h"

#if PLUSHIE_CSTL_TESTS_PROVIDE_IMPL
#  define PLUSHIE_CSTL_FILETYPES_ELF_PROVIDE_IMPL 1
#endif
#include "plushie/cstl/filetypes/elf.h"

void test_elf_ident_valid(void)
{
	PlushieUInt08 bytes[16] = { 0x7f, 'E', 'L', 'F', 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	PlushieElf_StructuredIdent id;
	TEST_ASSERT_EQUAL(kPlushieResultSuccess,
		plushie_elf_structured_ident_deserialize(&id, bytes, 16));
	TEST_ASSERT_EQUAL(kPlushieElf_FileClass_64bit, id.file_class);
	TEST_ASSERT_EQUAL(kPlushieElf_DataEncoding_LittleEndian, id.encoding);
}

void test_elf_ident_bad_magic(void)
{
	PlushieUInt08 bytes[16] = { 0 };
	bytes[0] = 0x7e; // wrong magic
	PlushieElf_StructuredIdent id;
	TEST_ASSERT_EQUAL(kPlushieResultErrParse,
		plushie_elf_structured_ident_deserialize(&id, bytes, 16));
}

void test_elf_ident_truncated(void)
{
	PlushieUInt08 bytes[4] = { 0x7f, 'E', 'L', 'F' };
	PlushieElf_StructuredIdent id;
	TEST_ASSERT_EQUAL(kPlushieResultErrEof,
		plushie_elf_structured_ident_deserialize(&id, bytes, 4));
}

void test_elf_ident_null(void)
{
	TEST_ASSERT_EQUAL(kPlushieResultErrNullptr,
		plushie_elf_structured_ident_deserialize(NULL, NULL, 0));
}

void filetypes_elf_main(void)
{
	RUN_TEST(test_elf_ident_valid);
	RUN_TEST(test_elf_ident_bad_magic);
	RUN_TEST(test_elf_ident_truncated);
	RUN_TEST(test_elf_ident_null);
}

TestFunctions gTestFuncsFileTypeElf =
{
	.mainPtr = filetypes_elf_main,
};

