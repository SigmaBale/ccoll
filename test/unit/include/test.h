#ifndef __TEST_H__
#define __TEST_H__

#if !defined(__TEST__) && !defined(__TEST_C__)
#error "This header can only be included in test files"
#endif

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define MAXERROR 1000

typedef enum { ASSERT, ASSERT_EQ, ASSERT_NEQ } assert_t;

int __test_start = 0;
int __test_end = 0;
int msec = 0;
clock_t start_time;

int errc = 0;
char *errors[MAXERROR];

void start_test(char *);
void end_test(void);
void assert_failed(assert_t, char *, int line);

#ifndef PRINT_TEST_MODULE
#define PRINT_TEST_MODULE(module) _PRINT_TEST_MODULE(module)
#endif

#ifndef START_TEST
#define START_TEST(test) start_test(test)
#endif

#ifndef END_TEST
#define END_TEST end_test()
#endif

#ifndef ASSERT_EQ
#define ASSERT_EQ(src, dst, desc)                                              \
  if ((src) != (dst))                                                          \
  assert_failed(ASSERT_EQ, desc, __LINE__)
#endif

#ifndef ASSERT_NEQ
#define ASSERT_NEQ(src, dst, desc)                                             \
  if ((src) == (dst))                                                          \
  assert_failed(ASSERT_NEQ, desc, __LINE__)
#endif

#ifndef ASSERT
#define ASSERT(exp, desc)                                                      \
  if (!(exp))                                                                  \
  assert_failed(ASSERT, desc, __LINE__)
#endif

#endif
