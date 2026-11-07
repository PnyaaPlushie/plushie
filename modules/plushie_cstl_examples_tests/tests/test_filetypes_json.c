// End-to-end test of the json parser after the rewrite (broken includes fixed,
// PlushieJson_ReturnCode replaced with PlushieResult, allocator wrappers used).
#include "unity.h"
#include <string.h>

#if PLUSHIE_CSTL_TESTS_PROVIDE_IMPL
#  define PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL 1
#  define PLUSHIE_CSTL_FILETYPES_JSON_PROVIDE_IMPL  1
#endif
#include "plushie/cstl/allocators/libc.h"
#include "plushie/cstl/filetypes/json.h"

void setUp(void) {}
void tearDown(void) {}

static PlushieU8StrSpan span_from_cstr(const char* s)
{
	return PlushieU8StrSpan_new((const PlushieUInt08*)s, (PlushieUSize)strlen(s));
}

void test_json_parse_object(void)
{
	PlushieAllocator* a = plushie_allocator_get_libc();
	PlushieJson_ParserCtx ctx;
	TEST_ASSERT_EQUAL(kPlushieResultSuccess,
		plushie_json_parser_ctx_new(&ctx, a, span_from_cstr("{ \"a\": 1, \"b\": true, \"c\": \"hi\" }")));

	PlushieJson_Value val;
	TEST_ASSERT_EQUAL(kPlushieResultSuccess, plushie_json_parse(&val, &ctx));
	TEST_ASSERT_EQUAL(kPlushieJson_ValueType_Object, val.type);
	TEST_ASSERT_EQUAL_UINT(3u, (unsigned)val.as.object.count);

	if (val.as.object.members) plushie_free(a, val.as.object.members);
}

void test_json_parse_number(void)
{
	PlushieAllocator* a = plushie_allocator_get_libc();
	PlushieJson_ParserCtx ctx;
	plushie_json_parser_ctx_new(&ctx, a, span_from_cstr("42.5"));

	PlushieJson_Value val;
	TEST_ASSERT_EQUAL(kPlushieResultSuccess, plushie_json_parse(&val, &ctx));
	TEST_ASSERT_EQUAL(kPlushieJson_ValueType_Number, val.type);
	TEST_ASSERT_TRUE(val.as.number > 42.0 && val.as.number < 43.0);
}

void test_json_parse_error(void)
{
	PlushieAllocator* a = plushie_allocator_get_libc();
	PlushieJson_ParserCtx ctx;
	plushie_json_parser_ctx_new(&ctx, a, span_from_cstr("{ bad }"));

	PlushieJson_Value val;
	TEST_ASSERT_EQUAL(kPlushieResultErrParse, plushie_json_parse(&val, &ctx));
}

void test_json_null_args(void)
{
	TEST_ASSERT_EQUAL(kPlushieResultErrNullptr, plushie_json_parse(NULL, NULL));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_json_parse_object);
	RUN_TEST(test_json_parse_number);
	RUN_TEST(test_json_parse_error);
	RUN_TEST(test_json_null_args);
	return UNITY_END();
}
