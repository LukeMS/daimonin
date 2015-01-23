#ifndef _COMMON_SUPPORT_H_
#define _COMMON_SUPPORT_H_

#define WORDS_FILE "words"

extern char *words[];
extern int num_words;
extern int word_lengths[];
extern void read_words(void); /* Can be used as a text fixture */

/* Debug helpers */
void dump_inventory(object_t *op);
void dump_objects();
void dump_objlinks();
void dump_treasurelist_tweaks();

static void dummy_teardown() /* Usable as a fixture teardown */
{
}

void prepare_memleak_detection();
int memleak_detected();

#endif
