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

    The author can be reached via e-mail to info@daimonin.org
*/

#ifndef __SRVFILE_H
#define __SRVFILE_H

enum
{
    SRVFILE_STATUS_UPDATE,
    SRVFILE_STATUS_OK
};

typedef struct srvfile_t
{
    uint8  status;
    int    len;
    uint32 crc;
    int    server_len;
    uint32 server_crc;
}
srvfile_t;

extern srvfile_t srvfile[SRV_CLIENT_FILES];

extern void  srvfile_check(void);
extern void  srvfile_set_status(uint8 num, uint8 status, int len, uint32 crc);
extern void  srvfile_save(const char *fname, uint8 num, unsigned char *data,
                          int len);
extern uint8 srvfile_get_status(uint8 num);
extern void  srvfile_load(void);

#endif /* ifndef __SRVFILE_H */
