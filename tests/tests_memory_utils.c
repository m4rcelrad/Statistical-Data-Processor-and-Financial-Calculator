#include "memory_utils.h"
#include "unity/unity.h"

void test_AlignedCalloc_Standard(void)
{
    size_t *ptr = (size_t *)aligned_calloc(10, sizeof(size_t), CACHE_LINE_SIZE);

    TEST_ASSERT_NOT_NULL(ptr);

    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_UINT(0, ptr[i]);
    }

    aligned_free(ptr);
}

void test_AlignedCalloc_ZeroSize(void)
{
    void *ptr1 = aligned_calloc(0, 10, CACHE_LINE_SIZE);
    TEST_ASSERT_NULL(ptr1);

    void *ptr2 = aligned_calloc(10, 0, CACHE_LINE_SIZE);
    TEST_ASSERT_NULL(ptr2);
}

void test_AlignedCalloc_InvalidAlignment(void)
{
    void *ptr_not_power_of_two = aligned_calloc(10, 10, 63);
    TEST_ASSERT_NULL(ptr_not_power_of_two);

    if (sizeof(void *) > 2) {
        void *ptr_too_small = aligned_calloc(10, 10, 2);
        TEST_ASSERT_NULL(ptr_too_small);
    }
}

void test_AlignedCalloc_OverflowMultiplication(void)
{
    void *ptr = aligned_calloc(SIZE_MAX, 2, CACHE_LINE_SIZE);
    TEST_ASSERT_NULL(ptr);
}

void run_memory_utils_tests(void)
{
    RUN_TEST(test_AlignedCalloc_Standard);
    RUN_TEST(test_AlignedCalloc_ZeroSize);
    RUN_TEST(test_AlignedCalloc_InvalidAlignment);
    RUN_TEST(test_AlignedCalloc_OverflowMultiplication);
}
