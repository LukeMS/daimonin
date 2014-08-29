#!/bin/bash
#       dai-installer-kde.bash
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
#       Name: Daimonin Installer KDE
#       Version: 0.5
#       Webpage: http://daimonin.elwin013.com/

source instfunctions.bash

# Variables
WINDOWTITLE="Daimonin Installer v0.4.1"
ACTION="";
MODE="";
DISTRO="";
INSTALLDIR="";
progress="";

# Base functions
test_dialog() {
 # Is kdialog installed?
 if [ -e /usr/bin/kdialog ]; then echo "OK"
 else echo "You don't have kdialog! Use command line version or install kdialog!" && exit
 fi
}
choose() {
 # Choose options - action (install, update, remove or help), mode
 # (single-user or multi-user mode) and distribution (needed to install dependencies
 # and for install game on Ubunu, because it's need kdesudo, not kdesu)
 ACTION=$(kdialog --title="${WINDOWTITLE}" \
        --radiolist "What would you like to do?" \
        install "install" on update "update" off remove "remove" off help "help" off);
 if [ "$?" = 1 ]; then exit 
 fi
 if [ "$ACTION" = "help" ]; then show_help;
 fi
 MODE=$(kdialog --title="${WINDOWTITLE}" \
      --radiolist "Choose mode:" \
      single "single" on multi "multi" off);
 if [ "$?" = 1 ]; then exit 
 fi
 case $ACTION in
 install|update)
  if [ "$MODE" = "single" ]; then
   kdialog --title="${WINDOWTITLE}" --yesno "Do you want to specify installation directory?\nIf you choose no game will be installed in ~/daimonin or in ~/Games/daimonin if ~/daimonin exist."
    if [ "$?" = 0 ]; then 
     INSTALLDIR=$(kdialog --title="${WINDOWTITLE}" --inputbox "Enter installation folder:" "${HOME}/");
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
   DISTRO=$(kdialog --title="${WINDOWTITLE}" \
          --radiolist "Choose your distribution (<b>if you use (K)Ubuntu</b> or want to <b>install dependencies</b>) or choose none" \
          none "None" on Debian "Debian" off Ubunut "Ubuntu" off ArchLinux "ArchLinux" off Fedora "Fedora" off openSuse "openSuse" off);
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
     DISTRO=$(kdialog --title="${WINDOWTITLE}" \
          --radiolist "Choose your distribution (it's needed to remove multi-user installation):" \
          none "None" on Debian "Debian" off Ubunut "Ubuntu" off ArchLinux "ArchLinux" off Fedora "Fedora" off openSuse "openSuse" off);
    fi
   fi
   if [ "$?" = 1 ]; then exit 
   fi
  ;;
 esac
}
show_help() {
 kdialog --title="${WINDOWTITLE} - Help" --textbox dai-installer-help-gui 400 400
 exit
}
install_dependencies() {
 # Install dependencies (physfs, libsdl, libsdl-image, libsdl-mixer, autoconf, automake and libcurl)
 if  [ "$DISTRO" == "none" ]; then
  return 0
 else
  kdialog --title="${WINDOWTITLE} " --msgbox "Now we should install dependencies.\nType your password for (K)Ubuntu or root password for other distributions"
  case $DISTRO in
   Debian)
   kdesu -u root -c "apt-get update && apt-get install libsdl1.2-dev libsdl-mixer1.2-dev libsdl-image1.2-dev libcurl4-gnutls-dev libphysfs-dev autoconf automake" 
   ;;
   Ubuntu)
   kdesudo -c "apt-get update && apt-get install libsdl1.2-dev libsdl-mixer1.2-dev libsdl-image1.2-dev libcurl4-gnutls-dev libphysfs-dev autoconf automake" 
   ;;
   ArchLinux)
   kdesu -u root -c "pacman -Sy sdl_mixer sdl_image physfs curl automake autoconf"
   ;;
   Fedora)
   kdesu -u root -c "yum install physfs sdl curl autoconf automake"
   ;;
   openSuse)
   kdesu -u root -c "zypper install curl sdl sdl_image sdl_mixer physfs autoconf automake"
   ;;
  esac
 fi
}
install_prepare() {
 qdbus $progress setLabelText "Creating directiories"
 create_dirs;
 install_testdir;
 qdbus $progress setLabelText "Downloading and unpacking source"
 prepare_source;
 qdbus $progress Set "" value 2 &&
 case $MODE in
  single) install_single; ;;
  multi) install_multi; ;;
 esac
}
install_single() {
 qdbus $progress setLabelText "Compiling and installing game"
 ./bootstrap &&
 if [ "$INSTALLDIR" = "" ]; then 
  ./configure
  INSTALLDIR="${HOME}"
 else ./configure --prefix=${INSTALLDIR}
 fi &&
 qdbus $progress Set "" value 3 &&
 make
 make install
 qdbus $progress Set "" value 4 &&
 case $DISTRO in
  Ubuntu) create_shortcut_single_ubuntu; ;;
  *) create_shortcut_single; ;;
 esac &&
 remove_uninstaller &&
 qdbus $progress Set "" value 5 &&
 qdbus $progress close &&
 kdialog --title="${WINDOWTITLE}" --msgbox "Daimonin was succesfully installed\!\nEnter ${INSTALLDIR}/daimonin folder and run Daimonin\!\nOr run shortcut placed on desktop."
 clean;
}
install_multi() {
 qdbus $progress setLabelText "Creating shortcuts" && 
 create_shortcut_multi &&
 create_starter_multi &&
 qdbus $progress setLabelText "Compiling and installing game" &&
 ./bootstrap &&
 ./configure --prefix=/usr/share &&
 qdbus $progress Set "" value 3 &&
 make
 qdbus $progress Set "" value 4 &&
 DIRECTORY="`cd ~/ && pwd`" &&
 kdialog --title="${WINDOWTITLE}" --msgbox "Type super user (root) password - it's needed to place game in /usr/" &&
 case $DISTRO in
 Ubuntu)
  kdesudo -c "make install" &&
  kdesudo -c "mv daimonin.sh /usr/bin/daimonin" &&
  kdesudo -c "mv daimonin.desktop /usr/share/applications"
 ;;
 *) 
  kdesu -u root -c "make install && mv daimonin.sh /usr/bin/daimonin && mv daimonin.desktop /usr/share/applications" 
 ;;
 esac
 qdbus $progress Set "" value 5
 qdbus $progress close &&
 kdialog --title="${WINDOWTITLE}" --msgbox "Daimonin was succesfully installed\!\nShortcut was placed in menu."
 clean;
}
# Update functions
update_prepare() {
 clean;
 qdbus $progress setLabelText "Creating directiories"
 create_dirs;
 qdbus $progress setLabelText "Downloading and unpacking source"
 prepare_source;
 qdbus $progress Set "" value 2 &&
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
  kdialog --title="${WINDOWTITLE}" --msgbox "Game succesfully removed\! \nThank you for using Daimonin Installer\!"
 else kdialog --title="${WINDOWTITLE}" --msgbox "Script can't find installed Daimonin\!"
      exit
 fi
 }
remove_multi() {
 kdialog --title="${WINDOWTITLE}" --msgbox "Please write your password - it's needed to remove game files" 
 if [ "$DISTRO" = "Ubuntu" ]; then
  kdesudo -c "rm -rf /usr/share/daimonin &&
  rm -rf /usr/share/daimonin-0.10.0 &&
  rm -f /usr/share/applications/daimonin.desktop &&
  rm -f /usr/bin/daimonin" &&
  kdialog --title="${WINDOWTITLE}" --msgbox "Game succesfully removed\! \nThank you for using Daimonin Installer\!"
 else
  kdesu -u root -c "rm -rf /usr/share/daimonin /usr/share/applications/daimonin.desktop /usr/bin/daimonin" &&
  kdialog --title="${WINDOWTITLE}" --msgbox "Game succesfully removed\! \nThank you for using Daimonin Installer\!"
 fi
}
remove_uninstaller() {
 # Nice thing - remove current directory (game folder). User should run it from game folder.
 echo "#!/bin/sh
 kdialog --title='${WINDOWTITLE}' --yesno 'Do you really want to remove Daimonin?'
 if [ \$? = 0 ]; then
  if [ -e daimonin ] && [ -e daimonin-updater ] && [ -e daimonin.p0 ]; then
   rm -rf \$PWD &&
   rm -f ~/Desktop/daimonin.desktop &&
   rm -f ~/.local/share/applications/daimonin.desktop &&
   rm -f ~/.daimonin/installer/main_single_installdir &&
   rm -r ~/.daimonin/installer/client.tar.gz &&
   kdialog --title='${WINDOWTITLE}' --msgbox 'GAME UNINSTALLED\!'
  else kdialog --title='${WINDOWTITLE}' --msgbox 'There is no daimonin, daimonin-updater and daimonin.p0 file! Aborting...'
  fi
 fi" > ${INSTALLDIR}/daimonin/uninstall.sh
 chmod +x ${INSTALLDIR}/daimonin/uninstall.sh
 echo "${INSTALLDIR}" > ~/.daimonin/installer/main_single_installdir
}

test_dialog;
choose;
case $ACTION in
 install) install_dependencies;  progress=`kdialog --title="${WINDOWTITLE}" --progressbar "Installing game" 5` install_prepare; ;;
 update)  progress=`kdialog --title="${WINDOWTITLE}" --progressbar "Installing game" 5` update_prepare; ;;
 remove) prepare_remove; ;;
esac
