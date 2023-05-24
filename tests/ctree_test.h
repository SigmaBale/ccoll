#include <stddef.h>

#ifndef _CTREE_TEST_H_
#define _CTREE_TEST_H_

#ifndef PRINT_TEST_FILE
#define PRINT_TEST_FILE(file) printf("\x1b[1;31m%s\x1b[0m\n", file);
#endif

#ifndef PRINT_TEST
#define PRINT_TEST(test)                                                       \
  printf("Running test \x1b[1;33m%s\x1b[0m ----> ", test);
#endif

#ifndef PRINT_SUCCESS
#define PRINT_SUCCESS printf("\x1b[1;32msucceeded\x1b[0m\n");
#endif

/* Debug */
void print_tree(struct AVLTree *tree);

#endif
