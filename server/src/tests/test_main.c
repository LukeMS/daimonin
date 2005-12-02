#include <global.h>

#if defined HAVE_CHECK && defined BUILD_UNIT_TESTS
#include <check.h>

void run_unit_tests(void)
{
    fprintf(stderr, "Running Daimonin Test Suites\n");
    fprintf(stderr, "  (Well, not really. But Real Soon Now(tm).\n");
    exit(EXIT_SUCCESS);
}

#else
void run_unit_tests(void)
{
    fprintf(stderr, "Unit tests not available. Please reconfigure and recompile\n");
    exit(EXIT_FAILURE);
}
#endif
