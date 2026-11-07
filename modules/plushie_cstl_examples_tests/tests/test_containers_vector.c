// Tests the collapsed typed vector generator (init / push_back / grow / free).
#include "unity.h"

#if PLUSHIE_CSTL_TESTS_PROVIDE_IMPL
#  define PLUSHIE_CSTL_ALLOCATORS_LIBC_PROVIDE_IMPL 1
#endif
#include "plushie/cstl/containers/vector.h"
#include "plushie/cstl/allocators/libc.h"

// A test-local vector type. This is always defined in this TU (the library only
// ships PlushieVecStr), so it never collides with the linked library.
PlushieVectorDecl(PlushieVecInt, int, i32);
PlushieVectorDefine(PlushieVecInt, int, i32);

void setUp(void) {}
void tearDown(void) {}

void test_vector_push_grows_and_reads_back(void)
{
	PlushieAllocator* a = plushie_allocator_get_libc();
	PlushieVecInt v;
	TEST_ASSERT_EQUAL(kPlushieResultSuccess, plushie_vec_i32_init(&v, a));

	for (int i = 0; i < 100; i++)
	{
		TEST_ASSERT_EQUAL(kPlushieResultSuccess, plushie_vec_i32_push_back(&v, i * i));
	}

	TEST_ASSERT_EQUAL_UINT(100u, (unsigned)v.size);
	TEST_ASSERT_TRUE(v.cap >= 100);
	for (int i = 0; i < 100; i++)
	{
		TEST_ASSERT_EQUAL_INT(i * i, v.ptr[i]);
	}

	plushie_vec_i32_free(&v);
	TEST_ASSERT_NULL(v.ptr);
	TEST_ASSERT_EQUAL_UINT(0u, (unsigned)v.size);
	TEST_ASSERT_EQUAL_UINT(0u, (unsigned)v.cap);
}

void test_vector_reserve_is_monotonic(void)
{
	PlushieAllocator* a = plushie_allocator_get_libc();
	PlushieVecInt v;
	plushie_vec_i32_init(&v, a);

	TEST_ASSERT_EQUAL(kPlushieResultSuccess, plushie_vec_i32_reserve(&v, 32));
	TEST_ASSERT_TRUE(v.cap >= 32);
	// reserving fewer than current capacity is a no-op success
	TEST_ASSERT_EQUAL(kPlushieResultSuccess, plushie_vec_i32_reserve(&v, 4));
	TEST_ASSERT_TRUE(v.cap >= 32);

	plushie_vec_i32_free(&v);
}

void test_vector_null_args(void)
{
	TEST_ASSERT_EQUAL(kPlushieResultErrNullptr, plushie_vec_i32_init(NULL, NULL));
	TEST_ASSERT_EQUAL(kPlushieResultErrNullptr, plushie_vec_i32_push_back(NULL, 1));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_vector_push_grows_and_reads_back);
	RUN_TEST(test_vector_reserve_is_monotonic);
	RUN_TEST(test_vector_null_args);
	return UNITY_END();
}
