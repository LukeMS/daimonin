#       dai-installer-gtk.bash
#       
#       Copyright 2010 Kamil "elwin013" Banach <kontakt AT elwin013.com>
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
#       (at your option) any later version.
#       
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#       
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#       MA 02110-1301, USA.
#
#       Name: Installer Functions
#       Version: 0.5
#       Webpage: http://daimonin.elwin013.com/

create_dirs() {
 # Create directiories for installer
 mkdir -p ~/.daimonin/installer
}

prepare_source() {
 # Download source code and put it into ~/.daimonin/installer/main
 cd ~/.daimonin/installer
 wget -c http://daimonin.svn.sourceforge.net/viewvc/daimonin/main/client.tar.gz -O client.tar.gz
 tar xzf client.tar.gz
 mv -f client/ main/
 cd main/make/linux
}

load_or_set_distro() {
 # Load and save distro to file
 if [ -e ~/.daimonin/installer/distro ]; then
  DISTRO=`cat ~/.daimonin/installer/distro`;
 elif [ "$DISTRO" = "" ]; then echo "Distribution not specified" 
 else echo "$DISTRO" >> ~/.daimonin/installer/distro
 fi
}

clean() {
 # Install - remove unpacked source, but not tarball
 # Remove/update - remove unpacked source and tarball
 case $ACTION in
  install) rm -rf ~/.daimonin/installer/main ;;
  update|remove) 
   rm -rf ~/.daimonin/installer/main 
   rm -r ~/.daimonin/installer/client.tar.gz
   ;;
 esac
}

create_shortcut_single() {
 # Create shortcut on desktop and in user menu (for single-user mode)
 touch daimonin.desktop &&
 echo "#!/usr/bin/env xdg-open
[Desktop Entry]
Type=Application
Version=1.0
Name=Daimonin
GenericName=MMORPG game
Comment=Free, open source, Massively Multiplayer Online RPG game
Icon=${INSTALLDIR}/daimonin/bitmaps/icon.png
Categories=Game
TryExec=${INSTALLDIR}/daimonin/dai-starter
Exec=${INSTALLDIR}/daimonin/dai-starter" > daimonin.desktop
chmod +x daimonin.desktop
cp daimonin.desktop ~/.local/share/applications/daimonin.desktop
mkdir -p ~/Desktop
cp daimonin.desktop ~/Desktop/daimonin.desktop
}

create_starter_single() {
 # Create starter for single-user mode
 touch daistarer.sh
 echo "#!/bin/sh
 cd $INSTALLDIR/daimonin
./daimonin $@" > daistarter.sh
 chmod +x daistarter.sh
 mv daistarter.sh $INSTALLDIR/daimonin/dai-starter
}

create_shortcut_multi() {
 # Create shortcut for multi-user mode
 touch daimonin.desktop &&
 echo "[Desktop Entry]
Type=Application
Version=1.0
Name=Daimonin
GenericName=MMORPG game
Comment=Free, open source, Massively Multiplayer Online RPG game
Icon=/usr/share/daimonin/bitmaps/icon.png
Categories=Game
Exec=daimonin" > daimonin.desktop
}

create_starter_multi() {
 # Create starter for multi-user mode
 touch daimonin.sh
 echo "#!/bin/sh
cd /usr/share/daimonin
./daimonin $@" > daimonin.sh
 chmod +x daimonin.sh
}

distro_or_installdir() {
 # Tests arguments - which is DISTRO and which is INSTALLDIR
 if [ "${DISTRO:0:1}" = "/" ] || [ "${DISTRO:0:1}" = "~" ]; then
  if [ "$INSTALLDIR" = "" ]; then
   INSTALLDIR=$DISTRO;
   DISTRO="";
  else
   TEMP=$DISTRO
   DISTRO=$INSTALLDIR
   INSTALLDIR=$TEMP
  fi
  INSTALLDIR=${INSTALLDIR/#~/$HOME}
 fi
}

install_testdir(){
 # Test install directory - if ~/daimonin exist set $INSTALLDIR as
 # ~/Games/ and create that dir. If $INSTALLDIR set just create that dir.
 if [ "$INSTALLDIR" = "" ]; then
  if [ -d ~/daimonin ]; then 
   INSTALLDIR=${HOME}/Games
   mkdir -p ${HOME}/Games
  fi
 else mkdir -p ${INSTALLDIR}
 fi
}

remove_prepare() {
 case $MODE in
  single) remove_single; ;;
  multi) remove_multi; ;;
 esac
 clean;
}


