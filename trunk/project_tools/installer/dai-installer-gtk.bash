#!/bin/bash
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
#       Name: Daimonin Installer GTK
#       Version: 0.5
#       Webpage: http://daimonin.elwin013.com/

source instfunctions.bash

# Variables
WINDOWTITLE="Daimonin Installer v0.5"
ACTION="";
MODE="";
DISTRO="";
INSTALLDIR="";

# Base functions
test_zenity() {
 # Is zenity installed?
 if [ -e /usr/bin/zenity ]; then echo "OK"
 else echo "You don't have zenity! Use command line version or install zenity!" && exit
 fi
}
choose() {
 # Choose options - action (install, update, remove or help), mode
 # (single-user or multi-user mode) and distribution (needed to install dependencies
 # and for install game on Ubunu, because it's need gksudo, not gksu)
 ACTION=$(zenity --list --text="What would you like to do?" \
        --title="${WINDOWTITLE}" --radiolist --column "Choice" \
        --column "Action" TRUE "install" FALSE "update" FALSE "remove" \
        FALSE "help" --width 400 --height 200);
 if [ "$?" = 1 ]; then exit 
 fi
 if [ "$ACTION" = "help" ]; then show_help;
 fi
 MODE=$(zenity --list --text="Choose mode:" \
      --title="${WINDOWTITLE}" --radiolist --column "Choice" \
      --column "Mode" TRUE "single" FALSE "multi" \
      --width 400 --height 200);
 if [ "$?" = 1 ]; then exit 
 fi
 case $ACTION in
 install|update)
  if [ "$MODE" = "single" ]; then
   zenity --question --title="${WINDOWTITLE}" --text="Do you want to specify installation directory?\nIf you choose no game will be installed in ~/daimonin or in ~/Games/daimonin if ~/daimonin exist."
    if [ "$?" = 0 ]; then 
     INSTALLDIR=$(zenity --entry --title="${WINDOWTITLE}" --text="Enter installation folder:" --entry-text "${HOME}/" --width 400);
     if [ "$?" = 1 ]; then exit 
     fi
     INSTALLDIR=${INSTALLDIR/#~/$HOME}
    fi
  fi
  # Load distro if distro file exist
  if [ -e ~/.daimonin/installer/distro ]; then
   DISTRO=`cat ~/.daimonin/installer/distro`;
  # If not ask for it
  else
   DISTRO=$(zenity --list --text="Choose your distribution (<b>if you use (K)Ubuntu</b> or want to <b>install dependencies</b>) or choose none" \
         --title="${WINDOWTITLE}" --radiolist --column "Choice" --column "Distribution" \
         TRUE "none" FALSE "Debian" FALSE "Ubuntu" FALSE "ArchLinux" FALSE "Fedora" FALSE "openSuse" \
         --width 400 --height 200);
   if [ "$?" = 1 ]; then exit 
   else 
   # If distro="none" then do nothing, else write distro to file
    if [ "$DISTRO" == "none" ]; then echo "Distribution not specified"
    else echo "$DISTRO" >> ~/.daimonin/installer/distro
    fi
   fi
  fi
  ;;
  remove)
   # If distro file exist then load distro
   if [ -e ~/.daimonin/installer/distro ]; then
    DISTRO=`cat ~/.daimonin/installer/distro`;
   else
   # Or ask for it
    if [ "$MODE" = "multi" ]; then
     DISTRO=$(zenity --list --text="Choose your distribution (it's needed to remove multi-user installation):" \
           --title="${WINDOWTITLE}" --radiolist --column "Choice" --column "Distribution" \
           TRUE "none" FALSE "Debian" FALSE "Ubuntu" FALSE "ArchLinux" FALSE "Fedora" FALSE "openSuse" \
           --width 400 --height 200);
    fi
   fi
   if [ "$?" = 1 ]; then exit 
   fi
  ;;
 esac
}
show_help() {
 zenity --text-info --title="${WINDOWTITLE} - Help" \
        --filename="dai-installer-help-gui" --width=400 --height=400
 exit
}
install_dependencies() {
 # Install dependencies (physfs, libsdl, libsdl-image, libsdl-mixer, autoconf, automake and libcurl)
 case $DISTRO in
  Debian)
  gksu -u root "apt-get update && apt-get install libsdl1.2-dev libsdl-mixer1.2-dev libsdl-image1.2-dev libcurl4-gnutls-dev libphysfs-dev autoconf automake" -m "Script needs your super user (root) password to install dependencies" -g
  ;;
  Ubuntu)
  gksudo "apt-get update && apt-get install libsdl1.2-dev libsdl-mixer1.2-dev libsdl-image1.2-dev libcurl4-gnutls-dev libphysfs-dev autoconf automake" -m "Please type your password - it's needed to install dependencies" -g
  ;;
  ArchLinux)
  gksu -u root "pacman -Sy sdl_mixer sdl_image physfs curl automake autoconf" -m "Script needs your super user (root) password to install dependencies" -g
  ;;
  Fedora)
  gksu -u root "yum install physfs sdl curl autoconf automake" -m "Script needs your super user (root) password to install dependencies" -g 
  ;;
  openSuse)
  gksu -u root "zypper install curl sdl sdl_image sdl_mixer physfs autoconf automake" -m "Script needs your super user (root) password to install dependencies" -g
  ;;
  none)
  return 0
  ;;
 esac
}
# Install functions
install_prepare() {
 echo "# Creating directiories"
 create_dirs;
 install_testdir;
 echo "# Downloading and unpacking source";
 prepare_source;
 echo "50"
 case $MODE in
  single) install_single; ;;
  multi) install_multi; ;;
 esac
}
install_single() {
 echo "# Compiling and installing game"
 ./bootstrap &&
 if [ "$INSTALLDIR" = "" ]; then 
  ./configure
  INSTALLDIR="${HOME}"
 else ./configure --prefix=${INSTALLDIR}
 fi &&
 echo "70" &&
 make
 echo "80" &&
 make install
 echo "90" &&
 create_shortcut_single &&
 create_starter_single &&
 remove_uninstaller &&
 echo "100" &&
 zenity --info --title="${WINDOWTITLE}" --text "Daimonin was succesfully installed\!\nEnter ${INSTALLDIR}/daimonin folder and run Daimonin\!\nOr run shortcut placed on desktop."
 clean;
}
install_multi() {
 echo "# Creating shortcuts" && echo "50" &&
 create_shortcut_multi &&
 create_starter_multi &&
 echo "# Compiling and installing game" &&
 ./bootstrap &&
 ./configure --prefix=/usr/share &&
 echo "70" &&
 make
 echo "90" &&
 DIRECTORY="`cd ~/ && pwd`" &&
 case $DISTRO in
 Ubuntu)
  gksudo -g "make install" -m "Type your password - it's needed to place game in /usr/" &&
  gksudo -g "mv daimonin.sh /usr/bin/daimonin" &&
  gksudo -g "mv daimonin.desktop /usr/share/applications"
 ;;
 *) 
  gksu -u root "make install && mv daimonin.sh /usr/bin/daimonin && mv daimonin.desktop /usr/share/applications" -m "Type super user (root) password - it's needed to place game in /usr/" -g
 ;;
 esac
 echo "100" &&
 zenity --info --title="${WINDOWTITLE}" --text "Daimonin was succesfully installed\!\nShortcut was placed in menu."
 clean;
}
# Update functions
update_prepare() {
 clean;
 echo "# Creating directiories"
 create_dirs;
 echo "# Downloading and unpacking source";
 prepare_source;
 echo "50"
 case $MODE in
  single) remove_single; install_single; ;;
  multi) remove_multi; install_multi; ;;
 esac
}
# Remove functions
remove_single() {
 # If installdir file exist just remove game. If not shout "i don't know where is it!"
 if [ -e ~/.daimonin/installer/main_single_installdir ]; then
  rm -rf `cat ~/.daimonin/installer/main_single_installdir`/daimonin &&
  rm -f ~/Desktop/daimonin.desktop &&
  rm -f ~/.local/share/applications/daimonin.desktop &&
  zenity --info --title="${WINDOWTITLE}" --text "Game succesfully removed\! \nThank you for using Daimonin Installer\!"
 else zenity --info --title="${WINDOWTITLE}" --text "Script can't find installed Daimonin\!"
      exit
 fi
 }
remove_multi() {
 if [ "$DISTRO" = "Ubuntu" ]; then
  gksudo -g "rm -rf /usr/share/daimonin &&
  rm -rf /usr/share/daimonin-0.10.0 &&
  rm -f /usr/share/applications/daimonin.desktop &&
  rm -f /usr/bin/daimonin" -m "Please write your password - it's needed to remove game files" &&
  zenity --info --title="${WINDOWTITLE}" --text "Game succesfully removed\! \nThank you for using Daimonin Installer\!"
 else
  gksu -u root -g "rm -rf /usr/share/daimonin /usr/share/applications/daimonin.desktop /usr/bin/daimonin" -m "Please write you super user (root) password - it's needed to remove game files" &&
  zenity --info --title="${WINDOWTITLE}" --text "Game succesfully removed\! \nThank you for using Daimonin Installer\!"
 fi
}
remove_uninstaller() {
 # Nice thing - remove current directory (game folder). User should run it from game folder.
 echo "#!/bin/sh
 zenity --question --title='${WINDOWTITLE}' --text='Do you really want to remove Daimonin?'
 if [ \$? = 0 ]; then
  if [ -e daimonin ] && [ -e daimonin-updater ] && [ -e daimonin.p0 ]; then
   rm -rf \$PWD &&
   rm -f ~/Desktop/daimonin.desktop &&
   rm -f ~/.local/share/applications/daimonin.desktop &&
   rm -f ~/.daimonin/installer/main_single_installdir &&
   rm -r ~/.daimonin/installer/client.tar.gz &&
   zenity --info --text='GAME UNINSTALLED\!' --title='${WINDOWTITLE}'
  else zenity --error --title='${WINDOWTITLE}' --text='There is no daimonin, daimonin-updater and daimonin.p0 file! Aborting...'
  fi
 fi" > ${INSTALLDIR}/daimonin/uninstall.sh
 chmod +x ${INSTALLDIR}/daimonin/uninstall.sh
 echo "${INSTALLDIR}" > ~/.daimonin/installer/main_single_installdir
}

test_zenity;
choose;
case $ACTION in
 install) install_dependencies; install_prepare | zenity --progress --title="${WINDOWTITLE}" \
         --text="Installing game" --percentage=0 --auto-close --width 400 ;;
 update) update_prepare | zenity --progress --title="${WINDOWTITLE}" \
         --text="Installing game" --percentage=0 --auto-close --width 400 ;;
 remove) remove_prepare; ;;
esac
