// Tests the allocator flag masks and the alloc/free/realloc wrappers.
// Regression guard: the combined flag masks used to be built with bitwise-AND
// (evaluating to 0); they must be bitwise-OR now.
#include "unity.h"

#if PLUSHIE_CSTL_TESTS_PROVIDE_IMPL
#  define PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL 1
#endif
#include "plushie/cstl/core/alloc.h"
#include "plushie/cstl/allocators/libc.h"

void setUp(void) {}
void tearDown(void) {}

void test_flag_masks_are_nonzero(void)
{
	TEST_ASSERT_NOT_EQUAL(0, kPlushieAllocatorAllocFlagsReqOnly);
	TEST_ASSERT_NOT_EQUAL(0, kPlushieAllocatorAllocFlagsHintsOnly);
	TEST_ASSERT_NOT_EQUAL(0, kPlushieAllocatorAllocFlagsAll);
}

void test_flag_masks_contain_expected_bits(void)
{
	TEST_ASSERT_TRUE(kPlushieAllocatorAllocFlagsReqOnly & kPlushieAllocatorAllocFlagsZeroInit);
	TEST_ASSERT_TRUE(kPlushieAllocatorAllocFlagsReqOnly & kPlushieAllocatorAllocFlagsPinned);
	TEST_ASSERT_TRUE(kPlushieAllocatorAllocFlagsHintsOnly & kPlushieAllocatorAllocFlagsThreadLocal);
	TEST_ASSERT_TRUE(kPlushieAllocatorAllocFlagsAll & kPlushieAllocatorAllocFlagsZeroInit);
	TEST_ASSERT_TRUE(kPlushieAllocatorAllocFlagsAll & kPlushieAllocatorAllocFlagsThreadLocal);
}

void test_libc_alloc_roundtrip(void)
{
	PlushieAllocator* a = plushie_allocator_get_libc();
	TEST_ASSERT_NOT_NULL(a);

	void* p = plushie_alloc(a, 64);
	TEST_ASSERT_NOT_NULL(p);
	plushie_free(a, p);
}

void test_realloc_fallback_preserves_bytes(void)
{
	PlushieAllocator* a = plushie_allocator_get_libc();

	unsigned char* p = (unsigned char*)plushie_alloc(a, 4);
	TEST_ASSERT_NOT_NULL(p);
	p[0] = 1; p[1] = 2; p[2] = 3; p[3] = 4;

	// libc allocator has no realloc pfn, so this exercises the alloc+copy+free
	// fallback in plushie_realloc.
	unsigned char* q = (unsigned char*)plushie_realloc(a, p, 4, 8);
	TEST_ASSERT_NOT_NULL(q);
	TEST_ASSERT_EQUAL_UINT8(1, q[0]);
	TEST_ASSERT_EQUAL_UINT8(2, q[1]);
	TEST_ASSERT_EQUAL_UINT8(3, q[2]);
	TEST_ASSERT_EQUAL_UINT8(4, q[3]);
	plushie_free(a, q);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_flag_masks_are_nonzero);
	RUN_TEST(test_flag_masks_contain_expected_bits);
	RUN_TEST(test_libc_alloc_roundtrip);
	RUN_TEST(test_realloc_fallback_preserves_bytes);
	return UNITY_END();
}
