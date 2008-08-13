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

    The author can be reached via e-mail to daimonin@nord-com.net
*/

#include "ipcompare.h"

#define BUF_SZ 100

/*#define DEBUG_IPCOMPARE*/

#define MASK_ALL 4
#define MASK_HUNDREDS 3
#define MASK_TENS 2
#define MASK_ONES 1

/* parse a single IP address */
int parse_ip(const char * ip, char ip_terms[], int mask_pos[])
{
    char buffer[BUF_SZ];
    int index = 0;
    int pos = 0;
    int len = 0;
    int temp = 0;

    if (!ip)
    {
        LOG(llevDebug, "parse_ip: IP address is null.\n");
        return 1; // NULL IP!
    }

    mask_pos[0] = -1;
    mask_pos[1] = 0;
    memset(ip_terms,0,sizeof(char)*16); /* set the IP to all zeros */

    /* predefined special case */
    if (!strcmp(ip, "::1"))
    {
        ip_terms[10] = 255;
        ip_terms[11] = 255;
        ip_terms[12] = 127;
        ip_terms[15] = 1;
        return 0;
    }

    strcpy(buffer,ip);
    while (buffer[pos] != '\0')
    {
        if (buffer[pos] == '*')
        {
#ifdef DEBUG_IPCOMPARE
            LOG(llevDebug,"parse_ip: replaced * at %d with 0.\n",pos);
#endif
            if (pos)
                buffer[pos] = '0';
            else
                buffer[pos] = '1';
        }
        pos++;
    }
#ifdef DEBUG_IPCOMPARE
    LOG(llevDebug,"parse_ip: buffer = %s.\n",buffer);
#endif
    len = pos;

    if (strchr(buffer, ':'))
    {
        if (!inet_pton(AF_INET6, buffer, ip_terms))
            return 0;
    }
    else
    {
        if (!inet_pton(AF_INET, buffer, ip_terms))
            return 0;
        ip_terms[10] = 255;
        ip_terms[11] = 255;
        ip_terms[12] = ip_terms[0];
        ip_terms[13] = ip_terms[1];
        ip_terms[14] = ip_terms[2];
        ip_terms[15] = ip_terms[3];
        ip_terms[0] = ip_terms[1] = ip_terms[2] = ip_terms[3] = 0;
    }

    if (strchr(ip, '*'))
    {
        pos = 0;
        while (ip[pos] != '\0' && ip[pos] != '*')
        {
            pos++;
        }
        temp = index = pos;
        /* find the end of this term */
        while (ip[pos] != '.' &&  ip[pos] != '\0')
            pos++;
        /* find the start of the term */
        while (ip[index] != ':' && ip[index] != '.' && index)
            index--;
#ifdef DEBUG_IPCOMPARE
        LOG(llevDebug, "temp=%d; index=%d; pos=%d\n",temp, index, pos);
#endif
        /* xx* = 1; x*x = 2; *xx = 3; x* = 1; *x = 2; * = 3; x = 0 */
        switch (pos-index)
        {
            case 1:
                mask_pos[1] = MASK_ALL;
                break;
            case 2:
                if (temp-index == 1)
                    mask_pos[1] = MASK_ONES;
                else
                    mask_pos[1] = MASK_TENS;
                break;
            case 3:
                if (temp-index == 2)
                    mask_pos[1] = MASK_ONES;
                else if (temp-index == 1)
                    mask_pos[1] = MASK_TENS;
                else
                    mask_pos[1] = MASK_HUNDREDS;
                break;
            case 4: /* "boundary conditions are annoying": last term includes \0 */
                if (temp-index == 3)
                    mask_pos[1] = MASK_ONES;
                else if (temp-index == 2)
                    mask_pos[1] = MASK_TENS;
                else if (temp-index == 1)
                    mask_pos[1] = MASK_HUNDREDS;
                break;
        }
        temp=12; /* start at the end, count the dots until the masked term */
        pos = 0;
        while (ip[pos] != '*' && ip[pos] != '\0')
        {
#ifdef DEBUG_IPCOMPARE
            LOG(llevDebug, "pos_finder: temp=%d; index=%d; pos=%d\n",temp, index, pos);
#endif
            if (ip[pos] == '.')
                temp++;
            pos++;
        }
#ifdef DEBUG_IPCOMPARE
        LOG(llevDebug, "pos_finder: temp=%d; index=%d; pos=%d\n",temp, index, pos);
        mask_pos[0] = temp;
#endif
    }
    return 0;
}

/* compare IP address strings */
int ip_compare(const char * ip1, const char * ip2)
{
    unsigned char ip1_terms[16], ip2_terms[16];
    int mask_pos1[2], mask_pos2[2];
    int i = 0, mask_power = -1, mask_pos = -1;

    LOG(llevDebug, "ip_compare: Comparing %s to %s.\n", ip1, ip2);

    if (!ip1 || !ip2)
        return 0;
    if (parse_ip(ip1, ip1_terms, mask_pos1))
        return 0;
    if (parse_ip(ip2, ip2_terms, mask_pos2))
        return 0;

#ifdef DEBUG_IPCOMPARE
    for (i=0;i<16;i++)
        LOG(llevDebug,"%d,",ip1_terms[i]);
    LOG(llevDebug,"\n");
    for (i=0;i<16;i++)
        LOG(llevDebug,"%d,",ip2_terms[i]);
    LOG(llevDebug,"\n");
#endif

    if (mask_pos1[0] != -1 && mask_pos2[0] == -1)
    {
        mask_pos = mask_pos1[0];
        mask_power = mask_pos1[1];
    }
    else if (mask_pos1[0] == -1 && mask_pos2[0] != -1)
    {
        mask_pos = mask_pos2[0];
        mask_power = mask_pos2[1];
    }
    else if (mask_pos1[0] != -1 && mask_pos2[0] != -1)
    {
        if (mask_pos1[0] > mask_pos2[0])
        {
            mask_pos = mask_pos1[0];
            mask_power = mask_pos1[1];
        }
        else if (mask_pos1[0] < mask_pos2[0])
        {
            mask_pos = mask_pos2[0];
            mask_power = mask_pos2[1];
        }
        else if (mask_pos1[1] < mask_pos2[1])
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

    for (i = 0; i < 16; i++)
    {
#ifdef DEBUG_IPCOMPARE
        LOG(llevDebug, "%d vs %d\n",ip1_terms[i], ip2_terms[i]);
#endif
        if (ip1_terms[i] != ip2_terms[i] && i != mask_pos)
        {
            return 0;
        }
        else if (i == mask_pos)
        {
#ifdef DEBUG_IPCOMPARE
            LOG(llevDebug, "mask test: %d %d\n", mask_pos, mask_power);
            LOG(llevDebug, "mask test: %d %d\n", ip1_terms[i], ip2_terms[i]);
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
                    if (!(ip1_terms[i] / 100) || !(ip2_terms[i] / 100) || ip1_terms[i] % 10 != ip2_terms[i] % 10 || (ip1_terms[i] / 10) % 10 != (ip2_terms[i] / 10) % 10)
                        return 0;
                    break;
                case MASK_TENS:
#ifdef DEBUG_IPCOMPARE
                    LOG(llevDebug, "%d %d; (* in tens)\n",ip1_terms[i]/100, ip2_terms[i]/100);
#endif
                    /* Tests: 10s term is non-zero in both, match in hundreds term, match in ones term */
                    if (!(ip1_terms[i] / 10) || !(ip2_terms[i] / 10) ||ip1_terms[i] / 100 != ip2_terms[i] / 100 || ip1_terms[i] % 10 != ip2_terms[i] % 10)
                        return 0;
                    break;
                case MASK_ONES:
#ifdef DEBUG_IPCOMPARE
                    LOG(llevDebug, "%d %d; (* in ones)\n",ip1_terms[i]/10, ip2_terms[i]/10);
#endif
                    /* Tests: 100s terms match and tens terms match */
                    if (ip1_terms[i] / 100 != ip2_terms[i] / 100 || ip1_terms[i] / 10 != ip2_terms[i] / 10)
                        return 0;
                    break;
            }
            return 1;
        }
    }

    return 1;
}

#ifdef DEBUG_IPCOMPARE_MAIN
int main(int argc, char **argv)
{
    char *a, *b;
    if (argc > 2)
    {
        a = argv[1];
        b = argv[2];
    }
    else
        return 0;

    if(ip_compare(a,b))
        printf("match: %s == %s\n",a,b);
    else
        printf("no match: %s != %s\n",a,b);

    return 0;
}

#endif

/* Some versions of libc don't do this right, so we need to replicate inet_pton for the victims */
#ifdef NEED_INET_PTON
static int errno = 0;

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
        unsigned char tmp[NS_INADDRSZ], *tp;

        saw_digit = 0;
        octets = 0;
        *(tp = tmp) = 0;
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
