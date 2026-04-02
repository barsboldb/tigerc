#ifndef TEST_H
#define TEST_H

#include <stdio.h>

#define GREEN "\033[32m"
#define RED   "\033[31m"
#define RESET "\033[0m"

#define ASSERT(cond) \
  if (!(cond)) { \
    printf(RED "FAIL" RESET ": %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    return 0; \
  }

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)

typedef int (*test_fn)();
typedef struct {
  const char *name;
  test_fn     fn;
} test_entry_t;

#define MAX_TESTS 256
extern test_entry_t test_registry[];
extern int          test_count;

#define REGISTER_TEST(fn) \
  __attribute__((constructor)) \
  static void register_##fn() { \
    test_registry[test_count++] = (test_entry_t){ #fn, fn }; \
  }

#endif
