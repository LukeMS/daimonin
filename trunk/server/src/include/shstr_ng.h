/* enable module statistic.
 * This will add some inc/dec counter to the hash table.
 * small & very small cpu use
 */
#define SS_STATISTICS

/* The offsetof macro is part of ANSI C, but many compilers lack it, for
 * example "gcc -ansi"
 */
#if !defined (offsetof)
#define offsetof(type, member) (int)&(((type *)0)->member)
#endif

/* SS(string) will return the address of the shared_string struct which
 * contains "string".
 */
#define SS(x) ((struct shared_string *) ((x) - offsetof(struct shared_string, string)))

#ifndef SS_DUMP_TOTALS
#define SS_DUMP_TOTALS  1
#endif

/* This should be used to differentiate shared strings from normal strings */
typedef const char shstr;
