/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2008 Michael Toennies

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

#include <global.h>

#ifndef WIN32 /* ---win32 exclude unix header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif /* end win32 */

static int                  metafd  = -1;
static struct sockaddr_in   sock;

/* metaserver_init sets up the connection.  Its only called once.  If we are not
 * trying to contact the metaserver of the connection attempt fails, metafd will be
 * set to -1.  We use this instead of messing with the settings.meta_on so that
 * that can be examined to at least see what the user was trying to do.
 */
void metaserver_init()
{
#ifdef WIN32 /* ***win32 metaserver_init(): init win32 socket */
    struct hostent *hostbn;
    u_long             temp    = 1;
#endif

    if (!settings.meta_on)
    {
        metafd = -1;
        return;
    }

    if (isdigit(settings.meta_server[0]))
        sock.sin_addr.s_addr = inet_addr(settings.meta_server);
    else
    {
        struct hostent *hostbn  = gethostbyname(settings.meta_server);
        if (hostbn == NULL)
        {
            LOG(llevDebug, "metaserver_init: Unable to resolve hostname %s\n", settings.meta_server);
            return;
        }
        memcpy(&sock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }
#ifdef WIN32 /* ***win32 metaserver_init(): init win32 socket */
    ioctlsocket(metafd, FIONBIO, &temp);
#else
    fcntl(metafd, F_SETFL, O_NONBLOCK);
#endif
    if ((metafd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        LOG(llevDebug, "metaserver_init: Unable to create socket, err %d (%s)\n", errno, strerror_local(errno));
        return;
    }
    sock.sin_family = AF_INET;
    sock.sin_port = htons(settings.meta_port);
#if 0
    /* Freebsd seems to have a problem in that the sendto in metaserver_update will
     * fail on a connected socket.  So don't connect the socket.
     * Solaris has the same problem, so for now just disable this.
     */
    if (connect(metafd, &sock, sizeof(sock))<0) {
    LOG(llevDebug,"metaserver_init: Unable to connect to metaserver, err %d (%s)\n", errno, strerror_local(errno));
    }
#endif
    /* No hostname specified, so lets try to figure one out */
    if (settings.meta_host[0] == 0)
    {
        char    hostname[MEDIUM_BUF], domain[MEDIUM_BUF];
        if (gethostname(hostname, MEDIUM_BUF - 1))
        {
            LOG(llevDebug, "metaserver_init: gethostname failed - will not report hostname\n");
            return;
        }

#ifdef WIN32 /* ***win32 metaserver_init(): gethostbyname! */
        hostbn = gethostbyname(hostname);
        if (hostbn != (struct hostent *) NULL) /* quick hack */
            memcpy(domain, hostbn->h_addr, hostbn->h_length);

        if (hostbn == (struct hostent *) NULL)
        {
#else
        if (getdomainname(domain, MEDIUM_BUF - 1))
        {
#endif /* win32 */
            LOG(llevDebug, "metaserver_init: getdomainname failed - will not report hostname\n");
                return;
        }
        /* Potential overrun here but unlikely to occur */
        sprintf(settings.meta_host, "%s.%s", hostname, domain);
    }

    if (metafd != -1)
        LOG(llevInfo, "metaserver_init: Connected to %s. Done.\n", settings.meta_server);
    metaserver_update();
}

void metaserver_update(void)
{
    static char  sbuf[SMALL_BUF] = "";
    char         nbuf[MEDIUM_BUF];

    /* No valid connection. */
    if (metafd == -1)
    {
        return;
    }

    if (sbuf[0] == '\0')
    {
        char *cp;

        sprintf(sbuf, "%s|%s|%d|%d.%d.%d",
                settings.meta_name, settings.meta_host, settings.csport,
                DAI_VERSION_RELEASE, DAI_VERSION_MAJOR, DAI_VERSION_MINOR);
        cp = strchr(sbuf, '\0');

        if (DAI_VERSION_INTERIM != "")
        {
#if 0 // 0.10.z clients do not have enough space to show the interim desc
            sprintf(cp, "-%s", DAI_VERSION_INTERIM);

            while (*cp != '\0')
            {
                if (isspace(*cp))
                {
                    *cp = '_';
                }

                cp++;
            }
#else
            char  interim[TINY_BUF] = DAI_VERSION_INTERIM;
            uint8 i;

            sprintf(cp++, "%s", "-");

            for (i = 0; i < strlen(interim) && !isspace(interim[i]); i++)
            {
                sprintf(cp++, "%c", interim[i]);
            }
#endif
        }
    }

    sprintf(nbuf, " %s|%d|%s", sbuf, player_active_meta, settings.meta_comment);
    player_active_meta = player_active;

    if (sendto(metafd, nbuf, strlen(nbuf), 0, (struct sockaddr *)&sock, sizeof(sock)) < 0)
    {
        LOG(llevInfo, "INFO:: %s:metaserver_update(): sendto failed, err = %d (%s)!\n",
            __FILE__, errno, strerror_local(errno));
    }
}
