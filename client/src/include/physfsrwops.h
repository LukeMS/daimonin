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
/*
 * This code provides a glue layer between PhysicsFS and Simple Directmedia
 *  Layer's (SDL) RWops i/o abstraction.
 *
 * License: this code is public domain. I make no warranty that it is useful,
 *  correct, harmless, or environmentally safe.
 *
 * This particular file may be used however you like, including copying it
 *  verbatim into a closed-source project, exploiting it commercially, and
 *  removing any trace of my name from the source (although I hope you won't
 *  do that). I welcome enhancements and corrections to this file, but I do
 *  not require you to send me patches if you make changes. This code has
 *  NO WARRANTY.
 *
 * Unless otherwise stated, the rest of PhysicsFS falls under the zlib license.
 *  Please see LICENSE.txt in the root of the source tree.
 *
 * SDL falls under the LGPL license. You can get SDL at http://www.libsdl.org/
 *
 *  This file was written by Ryan C. Gordon. (icculus@icculus.org).
 */

#ifndef __PHYSFSRWOPS_H
#define __PHYSFSRWOPS_H

#include "physfs.h"
#include "SDL.h"

/* some older physfs libs need that */
#ifndef PHYSFS_File
#define PHYSFS_File PHYSFS_file
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Open a platform-independent filename for reading, and make it accessible
 *  via an SDL_RWops structure. The file will be closed in PhysicsFS when the
 *  RWops is closed. PhysicsFS should be configured to your liking before
 *  opening files through this method.
 *
 *   @param filename File to open in platform-independent notation.
 *  @return A valid SDL_RWops structure on success, NULL on error. Specifics
 *           of the error can be gleaned from PHYSFS_getLastError().
 */
__EXPORT__ SDL_RWops *PHYSFSRWOPS_openRead(const char *fname);

/**
 * Open a platform-independent filename for writing, and make it accessible
 *  via an SDL_RWops structure. The file will be closed in PhysicsFS when the
 *  RWops is closed. PhysicsFS should be configured to your liking before
 *  opening files through this method.
 *
 *   @param filename File to open in platform-independent notation.
 *  @return A valid SDL_RWops structure on success, NULL on error. Specifics
 *           of the error can be gleaned from PHYSFS_getLastError().
 */
__EXPORT__ SDL_RWops *PHYSFSRWOPS_openWrite(const char *fname);

/**
 * Open a platform-independent filename for appending, and make it accessible
 *  via an SDL_RWops structure. The file will be closed in PhysicsFS when the
 *  RWops is closed. PhysicsFS should be configured to your liking before
 *  opening files through this method.
 *
 *   @param filename File to open in platform-independent notation.
 *  @return A valid SDL_RWops structure on success, NULL on error. Specifics
 *           of the error can be gleaned from PHYSFS_getLastError().
 */
__EXPORT__ SDL_RWops *PHYSFSRWOPS_openAppend(const char *fname);

/**
 * Make a SDL_RWops from an existing PhysicsFS file handle. You should
 *  dispose of any references to the handle after successful creation of
 *  the RWops. The actual PhysicsFS handle will be destroyed when the
 *  RWops is closed.
 *
 *   @param handle a valid PhysicsFS file handle.
 *  @return A valid SDL_RWops structure on success, NULL on error. Specifics
 *           of the error can be gleaned from PHYSFS_getLastError().
 */
__EXPORT__ SDL_RWops *PHYSFSRWOPS_makeRWops(PHYSFS_File *handle);

#ifdef __cplusplus
}
#endif

#endif /* ifndef __PHSYFSRWOPS_H */
