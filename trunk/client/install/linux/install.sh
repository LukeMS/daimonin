#!/bin/sh
# Daimonin SDL client, a client program for the Daimonin MMORPG.
#
# Copyright (C) 2012 Michael Toennies
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# The author can be reached via e-mail to info@daimonin.org

builddir="./../.."
datadir="$1"
bindir="$2"

mmkdir() {
    [ -d "$1" ] || mkdir "$1"
}

echo "### Copy default skin to $datadir"
mmkdir $datadir/bitmaps
mmkdir $datadir/bitmaps/locator
mmkdir $datadir/gfx_user
mmkdir $datadir/icons
cp $builddir/bitmaps/*.* $datadir/bitmaps
cp $builddir/bitmaps/locator/*.* $datadir/bitmaps/locator
cp $builddir/gfx_user/*.* $datadir/gfx_user
cp $builddir/icons/*.* $datadir/icons

echo "### Copy help to $datadir"
mmkdir $datadir/man
mmkdir $datadir/man/commands
#cp $builddir/man/*.* $datadir/man
cp $builddir/man/commands/* $datadir/man/commands

echo "### Copy music & sfx to $datadir"
mmkdir $datadir/media
mmkdir $datadir/sfx
cp $builddir/media/*.* $datadir/media
cp $builddir/sfx/*.* $datadir/sfx

echo "### Copy default settings to $datadir"
mmkdir $datadir/settings
cp $builddir/settings/*.* $datadir/settings

echo "### Copy update info to $datadir"
mmkdir $datadir/update
cp $builddir/update/version $datadir/update

echo "### Copy license to $datadir"
cp $builddir/License $datadir

echo "### Copy multiarch info & default images to $datadir"
cp $builddir/archdef.dat $datadir
cp $builddir/facepack.* $datadir

echo "*** Daimonin client successful installed in $bindir!"
echo "*** Enter your install folder and type ./daimonin"
echo "*** to start the game!"
