/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

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
#include <include.h>

void FreeMemory(void **p)
{
        if(p==NULL)
                return;
        if(*p != NULL)
                free(*p);
        *p=NULL;
}

char *show_input_string(char *text, struct _Font *font, int wlen)
{
        register int i, len;

        static char buf[MAX_INPUT_STR];
        strcpy(buf, text);
        strcat(buf,"_");
        for(len = 0,i=strlen(buf);i>=0;i--)
        {
                if(!buf[i])
                        continue;
                if(len+font->c[(int)(buf[i])].w+font->char_offset >=wlen )
                {
                        i--;
                        break;
                }
                len += font->c[(int)(buf[i])].w+font->char_offset;
        }
        if(GameTicksSec >500)
                buf[strlen(buf)-1]=0;
        return(&buf[++i]);
}

int read_substr_char(char *srcstr, char *desstr, int *sz, char ct)
{
        register unsigned char c;
        register int s=0;

        desstr[0]=0;
        for(;s<1023;)
        {
                c = *(desstr+s++) =*(srcstr+*sz); /* get character*/
                if(c==0x0d)
                        continue;
                if(c==0)
                        return(-1);
                if (c == 0x0a || c == ct)  /* if it END or WHITESPACE..*/
                        break; /* have a single word! (or not...)*/
                        (*sz)++; /* point to next source char */
        }
        *(desstr+(--s)) = 0; /* terminate all times with 0, */
                                        /* s: string length*/
        (*sz)++; /*point to next source charakter return(s);*/
        return s;
}

void * _my_malloc(size_t blen, char *info)
{
    LOG(LOG_DEBUG, "Malloc(): size %d info: %s\n", blen, info);
    return malloc(blen);
}

/*
 * Based on (n+1)^2 = n^2 + 2n + 1
 * given that	1^2 = 1, then
 *		2^2 = 1 + (2 + 1) = 1 + 3 = 4
 * 		3^2 = 4 + (4 + 1) = 4 + 5 = 1 + 3 + 5 = 9
 * 		4^2 = 9 + (6 + 1) = 9 + 7 = 1 + 3 + 5 + 7 = 16
 *		...
 * In other words, a square number can be express as the sum of the
 * series n^2 = 1 + 3 + ... + (2n-1)
 */
int isqrt(int n)
{
	int result, sum, prev;
	result = 0;
	prev = sum = 1;
	while (sum <= n) {
		prev += 2;
		sum += prev;
		++result;
	}
	return result;
}
