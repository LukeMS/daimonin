#include <global.h>

#if defined BUILD_BENCHMARKS

void run_benchmarks(void)
{
    fprintf(stderr, "Running Daimonin Benchmarks\n");
    fprintf(stderr, "  (Well, not really. But Real Soon Now(tm).\n");
    exit(EXIT_SUCCESS);
}

#else
void run_benchmarks(void)
{
    fprintf(stderr, "Benchmarks not available. Please reconfigure and recompile\n");
    exit(EXIT_FAILURE);
}
#endif
