/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2006 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to info@daimonin.org
*/

/* benchmark_main.c
 *
 * We use the check framework for benchmarks too, to take advantage
 * of the "checked fxitures"
 *
 * Copyright (C) 2005-2006 Björn Axelsson
 */

#include <global.h>

#if defined HAVE_CHECK && defined BUILD_BENCHMARKS
#include <check.h>
/* See http://check.sourceforge.net/doc/ for a check tutorial */

int benchmark_repetitions = 1000;

void print_delta_time(struct timeval *start, struct timeval *stop, int nops)
{
    struct timeval delta;
    start->tv_sec = -start->tv_sec;
    start->tv_usec = -start->tv_usec;
    add_time(&delta, stop, start);
    printf("  %d ops in %ld ms (%0.6f ms/op)\n", nops, delta.tv_sec * 1000 + delta.tv_usec / 1000, ((double)delta.tv_sec * 1000.0 + ((double)delta.tv_usec / 1000.0)) / (double)nops);
}

Suite *shstr_benchmark_suite(void);
Suite *hashtable_benchmark_suite(void);

void run_benchmarks(void)
{
    /* See http://check.sourceforge.net/doc/ for a check tutorial */
    int failed = 0;
    SRunner *sr = srunner_create(shstr_t_benchmark_suite());
    srunner_add_suite(sr, hashtable_benchmark_suite());

    fprintf(stderr, "Running Daimonin Benchmarks\n");

    srunner_run_all(sr, CK_NORMAL);
    failed += srunner_ntests_failed(sr);
    srunner_free(sr);

    exit(failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

#else
void run_benchmarks(void)
{
    fprintf(stderr, "Benchmarks not available. Please reconfigure and recompile\n");
    exit(EXIT_FAILURE);
}
#endif
