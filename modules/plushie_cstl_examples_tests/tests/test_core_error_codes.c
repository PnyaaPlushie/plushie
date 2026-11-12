// Tests the PlushieResult X-macro schema and the decode lookups.
// Regression guard: the decode functions used to fall off their non-void
// default path without a return.
#include "tests_shared.h"

#if PLUSHIE_CSTL_TESTS_PROVIDE_IMPL
#  define PLUSHIE_CSTL_CORE_ERROR_CODES_PROVIDE_IMPL 1
#endif
#include "plushie/cstl/core/error_codes.h"

void test_decode_name_nonempty_for_all(void)
{
	for (int i = 0; i < kPlushieResultCount; i++)
	{
		const char* name = plushie_result_decode_name((PlushieResult)i);
		TEST_ASSERT_NOT_NULL(name);
		TEST_ASSERT_TRUE(name[0] != '\0');
	}
}

void test_decode_desc_nonempty_for_all(void)
{
	for (int i = 0; i < kPlushieResultCount; i++)
	{
		const char* desc = plushie_result_decode_desc((PlushieResult)i);
		TEST_ASSERT_NOT_NULL(desc);
		TEST_ASSERT_TRUE(desc[0] != '\0');
	}
}

void test_decode_specific_names(void)
{
	TEST_ASSERT_EQUAL_STRING("Success", plushie_result_decode_name(kPlushieResultSuccess));
	TEST_ASSERT_EQUAL_STRING("ErrNullptr", plushie_result_decode_name(kPlushieResultErrNullptr));
	TEST_ASSERT_EQUAL_STRING("ErrAllocOom", plushie_result_decode_name(kPlushieResultErrAllocOom));
}

void test_decode_default_out_of_range(void)
{
	TEST_ASSERT_EQUAL_STRING("Error not found", plushie_result_decode_name((PlushieResult)9999));
	TEST_ASSERT_EQUAL_STRING("Error not found", plushie_result_decode_desc((PlushieResult)9999));
}

void core_error_codes_main(void)
{
	RUN_TEST(test_decode_name_nonempty_for_all);
	RUN_TEST(test_decode_desc_nonempty_for_all);
	RUN_TEST(test_decode_specific_names);
	RUN_TEST(test_decode_default_out_of_range);
}

TestFunctions gTestFuncsCoreErrorCodes =
{
	.mainPtr = core_error_codes_main,
};

