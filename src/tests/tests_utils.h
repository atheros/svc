#ifndef __TESTS_UTILS_H
#define __TESTS_UTILS_H

void test_start(const char* name);
void __test_assert(int result, const char* condition, const char* file, int line);
void test_end();

#define test_assert(condition) __test_assert(condition?1:0, __STRING(condition), __FILE__, __LINE__)

#endif /* !__TESTS_UTILS_H */
