#define __TEST_C__
#include "test.h"
#undef __TEST_C__

#define _PRINT_TEST_MODULE(module) fprintf(stdout, "\x1b[1;37m[%s]\x1b[0m\n", module);
#define PRINT_TEST                 fprintf(stdout, "Running test \x1b[1;33m%s\x1b[0m ----> ", test);
#define PRINT_SUCCESS              fprintf(stdout, "\x1b[1;32msucceeded\x1b[0m [%d]\n", msec)
#define PRINT_FAILURE              fprintf(stdout, "\x1b[1;31mfailed\x1b[0m [%d]\n", msec)
#define PRINT_INVALID              fprintf(stdout, "\x1b[1;91mtest invalid\x1b[0m [%d]\n", msec)
#define PRINT_DESC(desc)           fprintf(stdout, "\x1b[1;33mError desc:\x1b[0m %s\n", desc);

char*
atype_to_str(assert_t atype)
{
    switch(atype) {
        case ASSERT:
            return "ASSERT";
        case ASSERT_EQ:
            return "ASSERT_EQ";
        case ASSERT_NEQ:
            return "ASSERT_NEQ";
    }
}

void
assert_failed(assert_t atype, char* desc, int line)
{
    char* ds = (desc == NULL) ? "No description" : desc;
    char* t  = atype_to_str(atype);

    sprintf(errors[errc++], "%s failed at line %d: %s\n", t, line, ds);
}

void
start_test(char* test)
{
    __test_start = 1;
    msec         = 0;
    PRINT_TEST;
    start_time = clock();
}

void
end_test(void)
{
    if(__test_start == 0) {
        PRINT_INVALID;
    } else {
        clock_t difference = clock() - start_time;
        msec               = difference * 1000 / CLOCKS_PER_SEC;
        __test_end         = 1;

        if(errc == 0)
            PRINT_SUCCESS;
        else {
            PRINT_FAILURE;
            for(int i = 0; i < errc; i++)
                PRINT_DESC(errors[errc])
        }
    }
}
