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
/* This is a config file for the client.
 */

#define PROG_NAME "Daimonin"
#define VERSION_INFO "Daimonin SDL Client"
#define VERSION_NR "0.95b"
#define VERSION_CS 991013
#define VERSION_SC 991013

#define KEYBIND_FILE "./keys.dat"
#define OPTION_FILE  "./options.dat"
#define ARCHDEF_FILE "./archdef.dat"

#define LOG_FILE     "./client.log"

#define CLIENT_ICON_NAME "icon.png"
 
/* COMPILER OPTIONS */

/* Note: OS defines like  __WIN_32 or __LINUX are defined in the IDEs or makefiles */

/* Use OpenGL to do the render/blits
 * WARNING: the OpenGL interface is not finished.
 * This is for developing only. Remove this text when interface is usable
 */
/* #define INSTALL_OPENGL        */

/* Use the SDL_mixer sound system. Remove when you have no sound card or slow computer */
#define INSTALL_SOUND 

/* socket timeout value */
#define MAX_TIME 100


/* Default Screen
 * TODO: allowing different screen sizes.
 * Because i want a fixed map size (gaming issue), bigger screens will give
 * only more space for menus. Some work to do - i used some fixed positions.
 */
#define SCREEN_XLEN 800
#define SCREEN_YLEN 600

/* Increase when we got MANY new face... Hopefully,we need to increase this
 * in the future...
 */
#define MAX_FACE_TILES 10000

#define MAXANIM 2000

#define MAP_MAX_SIZE	17

/* Careful when changing this, should be no need */
#define MAX_INPUT_STRING 256				/* max. string len in input string*/


/* Maximum size of any packet we expect.  Using this makes it so we don't need to
 * allocated and deallocated teh same buffer over and over again and the price
 * of using a bit of extra memory.  IT also makes the code simpler.
 */
#define MAXSOCKBUF 20480

/* The numbers of our dark levels */
/* for each level-1 we store a own bitmap copy, so be careful */
#define DARK_LEVELS 4

#define HAVE_STRING_H
#ifndef __WIN_32
#define HAVE_UNISTD_H
#endif
#define HAVE_FCNTL_H
