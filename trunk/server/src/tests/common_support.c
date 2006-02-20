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

    The author can be reached via e-mail to daimonin@nord-com.net
*/

/* common_support.c 
 * Copyright (C) 2006 Björn Axelsson
 */

#include <global.h>

#if defined BUILD_BENCHMARKS || defined BUILD_UNIT_TESTS
#include "common_support.h"

#define MAX_WORDS 10000
char *words[MAX_WORDS];
int num_words;
int word_lengths[MAX_WORDS];

void read_words(void)
{
    FILE *f = fopen(WORDS_FILE, "r");
    char buf[1024], *ptr;
    num_words = 0;
    if(f) {
        while(num_words < MAX_WORDS && fgets(buf, 1023, f)) {            
            ptr = malloc(strlen(buf)+1);
            strcpy(ptr, buf);
            word_lengths[num_words] = strlen(ptr);
            words[num_words++] = ptr;
        }

        /*assert(strcmp(words[0], words[1]) != 0);
        assert(strlen(words[0]) > 0);
        assert(strlen(words[1]) > 0); */

        fclose(f);
    } else {
        fprintf(stderr, "!  No \"%s\" words file found. Generating %d random words\n", WORDS_FILE, MAX_WORDS);
        fprintf(stderr, "   You can use for example http://homepages.gold.ac.uk/rachel/words.txt\n");

        int i,j;
        for(i=0; i<MAX_WORDS; i++)
        {
            word_lengths[i] = rand()%20+5;
            words[i] = malloc(word_lengths[i]+1);
            words[i][word_lengths[i]] = '\0';
            for(j=0; j<word_lengths[i]; j++)
                words[i][j] = rand()%26 + 'a';
        }
        num_words = MAX_WORDS;
    }
}

#endif
