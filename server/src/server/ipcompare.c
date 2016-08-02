/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Application

    IP-COMPARE component written by: Brian Angeletti (gramlath)

    Copyright (C) 2001 Michael Toennies

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

#include "global.h"

#define MASK_ALL      4
#define MASK_HUNDREDS 3
#define MASK_TENS     2
#define MASK_ONES     1
#define MASK_NONE     0

/* #define DEBUG_IPCOMPARE */

static objectlink_t *add_ip_list(player_t *pl, objectlink_t *ip_list);
static void        free_iplist_node(objectlink_t *ol);

/* Parse a single IP address
 * Updated by Torchwood, Aug 2012 */
int parse_ip(const char * ip, unsigned char ip_terms[], int mask_pos[])
{
    char buffer[SMALL_BUF];
    int  term_start = 0, term_end = 0, term_numb = 0;
    int  pos = 0;

    if (!ip)
    {
        LOG(llevDebug, "parse_ip: IP address is null.\n");
        return 0;
    }

    mask_pos[0] = -1; /* Term containing a mask */
    mask_pos[1] = MASK_NONE;  /* Mask in hundreds, tens, ones, or all */

    memset(ip_terms, 0, sizeof(char) * 16); /* set the IP to all zeros */

    /* predefined special case */
    if (!strcmp(ip, "::1"))
    {
        ip_terms[10] = 255;
        ip_terms[11] = 255;
        ip_terms[12] = 127;
        ip_terms[15] = 1;
        return 1;
    }

    strcpy(buffer,ip);

    /* Strip out wild-cards from our temporary copy of the IP string */
    while (buffer[pos] != '\0')
    {
        if (buffer[pos] == '*')
        {
#ifdef DEBUG_IPCOMPARE
            LOG(llevDebug,"parse_ip: replaced * at %d with 1.\n",pos);
#endif

            /* We can't 'parse' and address containing a '*' so temporarily
             * replace it.  Note - don't replace with '0' as per previous code
             * else *12 will turn into 12
             */
            buffer[pos] = '1';
        }
        pos++;
    }

#ifdef DEBUG_IPCOMPARE
    LOG(llevDebug,"parse_ip: buffer = %s.\n",buffer);
#endif

    /* Split the IP string into separate terms ... */
    if (strchr(buffer, ':'))
    {
        if (!inet_pton(AF_INET6, buffer, ip_terms))
            return 0;
    }
    else
    {
        if (!inet_pton(AF_INET, buffer, ip_terms))
            return 0;

        /* IPv4 only uses 4 terms ... */
        ip_terms[10] = 255;
        ip_terms[11] = 255;
        ip_terms[12] = ip_terms[0];
        ip_terms[13] = ip_terms[1];
        ip_terms[14] = ip_terms[2];
        ip_terms[15] = ip_terms[3];
        ip_terms[0] = ip_terms[1] = ip_terms[2] = ip_terms[3] = 0;
    }

    /* Did original IP string contain any wild-cards? */
    if (strchr(ip, '*'))
    {
        pos = 0;

        /* Find the position of the first '*' character.  Note: any later '*' are ignored */
        while (ip[pos] != '\0' && ip[pos] != '*')
            pos++;

        term_end = term_start = pos;

        /* Find the end of this term */
        while (ip[term_end] != ':' && ip[term_end] != '.' &&  ip[term_end] != '\0')
            term_end++;

        /* Find the start of this term */
        while (ip[term_start] != ':' && ip[term_start] != '.' && term_start != -1)
            term_start--;

#ifdef DEBUG_IPCOMPARE
        LOG(llevDebug, "pos=%d; term_start=%d; term_end=%d\n",pos, term_start, term_end);
#endif

        switch (term_end - term_start)
        {
            case 2: /* Term is 1 character long */
                mask_pos[1] = MASK_ALL;
                break;

            case 3: /* Term is 2 characters long */
                if (pos - term_start == 2)
                    mask_pos[1] = MASK_ONES;
                else
                    mask_pos[1] = MASK_TENS;
                break;

            case 4: /* Term is 3 characters long */
                if (pos - term_start == 3)
                    mask_pos[1] = MASK_ONES;
                else if (pos - term_start == 2)
                    mask_pos[1] = MASK_TENS;
                else
                    mask_pos[1] = MASK_HUNDREDS;
                break;

            default: /* TW: Not sure this is possible? */
                mask_pos[1] = MASK_ALL;
                break;
        }

        /* Now find which particular term contains the first wildcard */

        /* For IPv4 data is only contained in terms 12 onwards */
        if (strchr(buffer, '.'))
            term_numb = 12;

        pos = 0;

        /* Search through the IP string again, looking for '*' */
        while (ip[pos] != '*' && ip[pos] != '\0')
        {
            /* Check if we've reached the next term yet */
            if (ip[pos] == '.' || ip[pos] == ':')
                term_numb++;

            pos++;
        }

#ifdef DEBUG_IPCOMPARE
        LOG(llevDebug, "term_finder: term_numb=%d; term_start=%d; pos=%d\n",term_numb, term_start, pos);
#endif

        mask_pos[0] = term_numb;

    }

    return 1;
}

/* Compare IP address strings
 * returns 1 if a match is found
 * Updated by Torchwood, Aug 2012 */
int ip_compare(const char * ip1, const char * ip2)
{
    unsigned char ip1_terms[16], ip2_terms[16];
    int mask_pos1[2], mask_pos2[2];
    int i = 0, mask_power = -1, mask_pos = -1;

    LOG(llevDebug, "ip_compare: Comparing %s to %s.\n", ip1, ip2);

    if (!ip1 || !ip2)
        return 0;

    /* Check we have valid IP address strings, and find the position of the FIRST wildcard */
    if (!parse_ip(ip1, ip1_terms, mask_pos1))
        return 0;
    if (!parse_ip(ip2, ip2_terms, mask_pos2))
        return 0;

#ifdef DEBUG_IPCOMPARE
    for (i=0; i<16; i++)
        LOG(llevDebug, "%d,", ip1_terms[i]);
    LOG(llevDebug, "\n");

    for (i=0; i<16; i++)
        LOG(llevDebug, "%d,", ip2_terms[i]);
    LOG(llevDebug, "\n");
#endif

    /* If one or both IP strings contains a mask, work out which mask to 'use' */

    /* IP1 has mask, IP2 does not ... */
    if (mask_pos1[0] != -1 && mask_pos2[0] == -1)
    {
        mask_pos = mask_pos1[0];
        mask_power = mask_pos1[1];
    }

    /* IP1 does not, IP2 has mask ... */
    else if (mask_pos1[0] == -1 && mask_pos2[0] != -1)
    {
        mask_pos = mask_pos2[0];
        mask_power = mask_pos2[1];
    }

    /* Both terms have a mask */
    else if (mask_pos1[0] != -1 && mask_pos2[0] != -1)
    {
        /* Now figure out which mask is more significant
         * Note:  A higher term number is LESS significant - i.e. term 0 is the
         * first, most important term!
         */

        /* IP1 term number < IP2 term number */
        if (mask_pos1[0] < mask_pos2[0])
        {
            mask_pos = mask_pos1[0];
            mask_power = mask_pos1[1];
        }

        /* IP1 term number > IP2 term number */
        else if (mask_pos1[0] > mask_pos2[0])
        {
            mask_pos = mask_pos2[0];
            mask_power = mask_pos2[1];
        }

        /* Term numbers are equal, so look at the position of the mask within the term */

        /* Higher value of [1] means a more significant 'bit' */
        else if (mask_pos1[1] > mask_pos2[1])
        {
            mask_pos = mask_pos1[0];
            mask_power = mask_pos1[1];
        }
        else
        {
            mask_pos = mask_pos2[0];
            mask_power = mask_pos2[1];
        }
    }

    /* Compare each term of the IP strings in turn */
    /* TW: How does an IPv4 address get compared to an IPv6 address ??? */
    for (i = 0; i < 16; i++)
    {
#ifdef DEBUG_IPCOMPARE
        LOG(llevDebug, "comparing term %d: %d vs. %d\n",i, ip1_terms[i], ip2_terms[i]);
#endif

        /* Are terms different, and is there no mask? */
        if (ip1_terms[i] != ip2_terms[i] && i != mask_pos)
            return 0;

        /* If we do have a mask, then we only compare this term, and not anything beyond */
        else if (i == mask_pos)
        {
#ifdef DEBUG_IPCOMPARE
            LOG(llevDebug, "mask test: pos=%d power=%d\n", mask_pos, mask_power);
#endif

            switch (mask_power)
            {
                case MASK_ALL:
#ifdef DEBUG_IPCOMPARE
                    LOG(llevDebug, "(* any match)\n");
#endif
                    /* Tests: NONE! */
                    break;

                case MASK_HUNDREDS:
#ifdef DEBUG_IPCOMPARE
                    LOG(llevDebug, "(* in hundreds)\n");
#endif
                    /* Tests: 100s term is non-zero in both, match in tens term, match in ones term */
                    if (!(ip1_terms[i] / 100) ||
                        !(ip2_terms[i] / 100) ||
                        ip1_terms[i] % 10 != ip2_terms[i] % 10 ||
                        (ip1_terms[i] / 10) % 10 != (ip2_terms[i] / 10) % 10)
                        return 0;
                    break;

                case MASK_TENS:
#ifdef DEBUG_IPCOMPARE
                    LOG(llevDebug, "%d %d; (* in tens)\n",ip1_terms[i]/100, ip2_terms[i]/100);
#endif
                    /* Tests: 10s term is non-zero in both, match in hundreds term, match in ones term */
                    if (!(ip1_terms[i] / 10) ||
                        !(ip2_terms[i] / 10) ||
                        ip1_terms[i] / 100 != ip2_terms[i] / 100 ||
                        ip1_terms[i] % 10 != ip2_terms[i] % 10)
                        return 0;
                    break;

                case MASK_ONES:
#ifdef DEBUG_IPCOMPARE
                    LOG(llevDebug, "%d %d; (* in ones)\n",ip1_terms[i]/10, ip2_terms[i]/10);
#endif
                    /* Tests: 100s terms match and tens terms match */
                    if (ip1_terms[i] / 100 != ip2_terms[i] / 100 ||
                        ip1_terms[i] / 10 != ip2_terms[i] / 10)
                        return 0;
                    break;
            }

            /* We don't compare any more terms past the first mask,
             * so 123.*.123.123 will match 123.9.9.9
             */
            return 1;
        }
    }

    /* No masks found, and all individual IP terms matched */
    return 1;
}

objectlink_t *find_players_on_ip(char *ipmask)
{
    player_t     *pl;
    objectlink_t *ip_list = NULL;

    if(!ipmask)
        return NULL;

    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (pl->ob && !QUERY_FLAG(pl->ob, FLAG_REMOVED) && ip_compare(pl->socket.ip_host, ipmask))
            ip_list = add_ip_list(pl, ip_list);
    }

    return ip_list;
}

static objectlink_t *add_ip_list(player_t *pl, objectlink_t *ip_list)
{
    objectlink_t *ol;

    if(!pl)
        return NULL;

    ol = objectlink_get(OBJLNK_FLAG_OB);
    ol->objlink.ob = pl->ob;

    objectlink_link(&ip_list, NULL, NULL, ip_list, ol);

    return ol;
}

/* Free the whole ip_list list
 */
void free_iplist(objectlink_t *ip_list)
{
    objectlink_t *ol = ip_list;

    LOG(llevDebug, "Freeing all ip-list entries\n");

    for(; ol; ol = ol->next)
    {
        free_iplist_node(ol);
    }
}

/* free the the used objectlink
 */
static void free_iplist_node(objectlink_t *ol)
{
    return_poolchunk(ol, pool_objectlink);
}

/* Some versions of libc don't do this right, so we need to replicate inet_pton for the victims */
#ifdef NEED_INET_PTON
/*static int errno = 0;*/

 /*
  * Copyright (C) 1996-2001  Internet Software Consortium.
  *
  * Permission to use, copy, modify, and distribute this software for any
  * purpose with or without fee is hereby granted, provided that the above
  * copyright notice and this permission notice appear in all copies.
  *
  * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM
  * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
  * INTERNET SOFTWARE CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT,
  * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
  * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
  * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
  * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
  */

/*#include "rsync.h"*/

#define NS_INT16SZ       2
#define NS_INADDRSZ      4
#define NS_IN6ADDRSZ    16

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static int inet_pton4(const char *src, unsigned char *dst);
static int inet_pton6(const char *src, unsigned char *dst);

/* int
 * isc_net_pton(af, src, dst)
 *      convert from presentation format (which usually means ASCII printable)
 *      to network format (which is usually some kind of binary format).
 * return:
 *      1 if the address was valid for the specified address family
 *      0 if the address wasn't valid (`dst' is untouched in this case)
 *      -1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *      Paul Vixie, 1996.
 */
int
inet_pton(int af,
          const char *src,
          void *dst)
{
        switch (af) {
        case AF_INET:
                return (inet_pton4(src, dst));
#ifdef INET6
        case AF_INET6:
                return (inet_pton6(src, dst));
#endif
        default:
                errno = EAFNOSUPPORT;
                return (-1);
        }
        /* NOTREACHED */
}

/* int
 * inet_pton4(src, dst)
 *      like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *      1 if `src' is a valid dotted quad, else 0.
 * notice:
 *      does not touch `dst' unless it's returning 1.
 * author:
 *      Paul Vixie, 1996.
 */
static int
inet_pton4(src, dst)
        const char *src;
        unsigned char *dst;
{
        static const char digits[] = "0123456789";
        int saw_digit, octets, ch;
        unsigned char  tmp[NS_INADDRSZ] = "",
                      *tp = tmp;

        saw_digit = 0;
        octets = 0;
        while ((ch = *src++) != '\0') {
                 const char *pch;

                if ((pch = strchr(digits, ch)) != NULL) {
                        unsigned int new = *tp * 10 + (pch - digits);

                        if (new > 255)
                                return (0);
                        *tp = new;
                        if (! saw_digit) {
                                if (++octets > 4)
                                        return (0);
                                saw_digit = 1;
                        }
                } else if (ch == '.' && saw_digit) {
                        if (octets == 4)
                                return (0);
                        *++tp = 0;
                        saw_digit = 0;
                } else
                        return (0);
        }
        if (octets < 4)
                return (0);
        memcpy(dst, tmp, NS_INADDRSZ);
        return (1);
}

/* int
 * inet_pton6(src, dst)
 *      convert presentation level address to network order binary form.
 * return:
 *      1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *      (1) does not touch `dst' unless it's returning 1.
 *      (2) :: in a full address is silently ignored.
 * credit:
 *      inspired by Mark Andrews.
 * author:
 *      Paul Vixie, 1996.
 */
#ifdef INET6
static int
inet_pton6(src, dst)
        const char *src;
        unsigned char *dst;
{
        static const char xdigits_l[] = "0123456789abcdef",
                          xdigits_u[] = "0123456789ABCDEF";
        unsigned char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
        const char *xdigits, *curtok;
        int ch, saw_xdigit;
        unsigned int val;

        memset((tp = tmp), '\0', NS_IN6ADDRSZ);
        endp = tp + NS_IN6ADDRSZ;
        colonp = NULL;
        /* Leading :: requires some special handling. */
        if (*src == ':')
                if (*++src != ':')
                        return (0);
        curtok = src;
        saw_xdigit = 0;
        val = 0;
        while ((ch = *src++) != '\0') {
                const char *pch;

                if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
                        pch = strchr((xdigits = xdigits_u), ch);
                if (pch != NULL) {
                        val <<= 4;
                        val |= (pch - xdigits);
                        if (val > 0xffff)
                                return (0);
                        saw_xdigit = 1;
                        continue;
                }
                if (ch == ':') {
                        curtok = src;
                        if (!saw_xdigit) {
                                if (colonp)
                                        return (0);
                                colonp = tp;
                                continue;
                        }
                        if (tp + NS_INT16SZ > endp)
                                 return (0);
                         *tp++ = (unsigned char) (val >> 8) & 0xff;
                         *tp++ = (unsigned char) val & 0xff;
                         saw_xdigit = 0;
                         val = 0;
                         continue;
                 }
                 if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
                    inet_pton4(curtok, tp) > 0) {
                         tp += NS_INADDRSZ;
                         saw_xdigit = 0;
                         break;  /* '\0' was seen by inet_pton4(). */
                 }
                 return (0);
         }
         if (saw_xdigit) {
                 if (tp + NS_INT16SZ > endp)
                         return (0);
                 *tp++ = (unsigned char) (val >> 8) & 0xff;
                 *tp++ = (unsigned char) val & 0xff;
         }
         if (colonp != NULL) {
                 /*
                  * Since some memmove()'s erroneously fail to handle
                  * overlapping regions, we'll do the shift by hand.
                  */
                 const int n = tp - colonp;
                 int i;

                 for (i = 1; i <= n; i++) {
                         endp[- i] = colonp[n - i];
                         colonp[n - i] = 0;
                 }
                 tp = endp;
         }
         if (tp != endp)
                 return (0);
         memcpy(dst, tmp, NS_IN6ADDRSZ);
         return (1);
}
#endif
#endif /* NEED_INET_PTON */
