#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

/* Just a few definitions that we can use for benchmarking */

#define timer_start(x) { struct timeval start_time, stop_time; gettimeofday(&start_time, NULL);
#define timer_stop(nops) gettimeofday(&stop_time, NULL); print_delta_time(&start_time, &stop_time, nops); }

extern void print_delta_time(struct timeval *start, struct timeval *stop, int nops);
extern int benchmark_repetitions;

#endif
