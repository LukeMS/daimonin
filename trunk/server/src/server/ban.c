/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

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

/*
 * Ban.c
 * Code was grabbed from the netrek source and modified to work with 
 * crossfire. This function checks a file in the lib directory for any
 * banned players. If it finds one it returns a 1. Wildcards can be used.
 */ 

#include <global.h>
#include <sproto.h>
#ifndef WIN32 /* ---win32 : remove unix headers */
#include <sys/ioctl.h>
#endif /* win32 */
#ifdef hpux
#include <sys/ptyio.h>
#endif

#ifndef WIN32 /* ---win32 : remove unix headers */
#ifdef NO_ERRNO_H
    extern int errno;
#else
#   include <errno.h>
#endif
#include <stdio.h>
#include <sys/file.h>
#endif /* win32 */

/* reloading every time this list? i don't think so! */
int checkbanned(char *login, char *host)
{
  FILE  *bannedfile;
  char  buf[MAX_BUF];
  char  log_buf[64], host_buf[64], line_buf[160];
  char  *indexpos;
  int           num1;
  int   Hits=0;                 /* Hits==2 means we're banned */

  if(Hits==0)
	  return 0;

  sprintf (buf, "%s/%s", settings.localdir, BANFILE);
  if ((bannedfile = fopen(buf, "r")) == NULL) {
    LOG(llevDebug, "Could not find file Banned file.\n");
    return(0);
  }
  while(fgets(line_buf, 160, bannedfile) != NULL) {
    /* Split line up */
    if((*line_buf=='#')||(*line_buf=='\n'))
      continue;
    if ((indexpos = (char *) strrchr(line_buf, '@')) == 0) {
      LOG(llevDebug, "Bad line in banned file\n");
      continue;
    }
    num1 = indexpos - line_buf;
    strncpy(log_buf, line_buf, num1); /* copy login name into log_buf */
    log_buf[num1] = '\0';
    strncpy(host_buf, indexpos + 1, 64); /* copy host name into host_buf */
    /* Cut off any extra spaces on the host buffer */
    indexpos = host_buf;
    while (!isspace(*indexpos))
      indexpos++;
    *indexpos = '\0';

    /*
      LOG(llevDebug, "Login: <%s>; host: <%s>\n", login, host);
      LOG(llevDebug, "    Checking Banned <%s> and <%s>.\n",log_buf,host_buf);
    */
    if(*log_buf=='*')

      Hits=1;
    else if (!strcmp(login, log_buf))
      Hits=1;
    if(Hits==1)
      {
        if (*host_buf == '*'){  /* Lock out any host */
          Hits++;
          break;                /* break out now. otherwise Hits will get reset
                                   to one */
        }
        else if(strstr(host,host_buf)!=NULL){ /* Lock out subdomains (eg, "*@usc.edu" */
          Hits++;
          break;                /* break out now. otherwise Hits will get reset
                                   to one */
        }
        else if (!strcmp(host, host_buf)){ /* Lock out specific host */
          Hits++;
          break;                /* break out now. otherwise Hits will get reset
                                   to one */
        }
      }
  }
  fclose(bannedfile);
  if(Hits>=2)
    return(1);
  else
    return(0);
}
