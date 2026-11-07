// Tests the span generator macro (new / is_empty / size).
// The header is pure macros + static inline, so no PROVIDE_IMPL is needed.
#include "unity.h"

#include "plushie/cstl/containers/span.h"

PLUSHIE_SPAN_DEFINE(TestU8Span, test_u8_span, PlushieUInt08)
PLUSHIE_SPAN_DEFINE(TestU32Span, test_u32_span, PlushieUInt32)

void setUp(void) {}
void tearDown(void) {}

void test_span_new_and_accessors(void)
{
	PlushieUInt08 data[4] = { 1, 2, 3, 4 };
	TestU8Span s = test_u8_span_new(data, 4);

	TEST_ASSERT_EQUAL_UINT(4u, (unsigned)s.len);
	TEST_ASSERT_FALSE(test_u8_span_is_empty(&s));
	TEST_ASSERT_EQUAL_UINT(4u, (unsigned)test_u8_span_size(&s)); // 4 * sizeof(u8)
	TEST_ASSERT_EQUAL_PTR(data, s.ptr);
}

void test_span_size_scales_with_element(void)
{
	PlushieUInt32 data[3] = { 10, 20, 30 };
	TestU32Span s = test_u32_span_new(data, 3);
	TEST_ASSERT_EQUAL_UINT(12u, (unsigned)test_u32_span_size(&s)); // 3 * sizeof(u32)
}

void test_span_empty(void)
{
	TestU8Span e = test_u8_span_new(NULL, 0);
	TEST_ASSERT_TRUE(test_u8_span_is_empty(&e));
	TEST_ASSERT_EQUAL_UINT(0u, (unsigned)test_u8_span_size(&e));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_span_new_and_accessors);
	RUN_TEST(test_span_size_scales_with_element);
	RUN_TEST(test_span_empty);
	return UNITY_END();
}
