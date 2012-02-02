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

datadir="$1"
bindir="$2"

cd ../..

echo "### Create zip of all defaults in $datadir"
zip -9qr $datadir/defaults.zip bitmaps gfx_user icons
zip -9qr $datadir/defaults.zip man
zip -9qr $datadir/defaults.zip media sfx
zip -9qr $datadir/defaults.zip settings
zip -9qr $datadir/defaults.zip update

echo "### Copy license to $datadir"
cp License $datadir

echo "### Copy multiarch info & facepack to $datadir"
cp archdef.dat $datadir
cp facepack.* $datadir

echo "*** Daimonin client successful installed in $bindir!"
echo "*** Enter your install folder and type ./daimonin"
echo "*** to start the game!"
