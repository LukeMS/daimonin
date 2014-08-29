#!/bin/bash
#       dai-installer-cli.bash
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
#       Name: Daimonin Installer CLI
#       Version: 0.5
#       Webpage: http://daimonin.elwin013.com/

source instfunctions.bash

# Variables
ACTION=$1
MODE=$2
DISTRO=$3
INSTALLDIR=$4

# Base functions
show_help() {
 echo " Daimonin Installer v0.5 for main version.
 Install Daimonin MMORPG (http://www.daimonin.org) with one command!
 Script webpage: http://daimonin.elwin013.com
 Usage: 
  bash dai-installer-cli.bash action mode distribution installdir
  
  action 
    what you want to do - install, remove, or update to latest version
  mode 
    single user (install in home directory) or multi user 
    (install in /usr) mode
  distribution (optional)
    distribution which you\'re using - Debian, Ubuntu, Fedora, 
    ArchLinux, openSuse - needed for install dependencies and install game
    on (K)Ubuntu in multi-user mode
  installdir (optional)
    where you want to put daimonin folder with game (absolute path), 
    e.g. ~/Games will put game in ~/Games/daimonin. If not specified 
    script will install game in ~/daimonin or if it exist in ~/Games/daimonin
  
  Examples:
   - multi user install mode on Ubuntu:
      bash dai-installer.bash install multi ubuntu
   - single user install mode in ~/Games directory:
      bash dai-installer.bash install single ~/Games
   - multi user update:
      bash dai-installer update multi"
 exit
}
install_dependencies() {
 echo "Script need password to install dependencies (root or user if sudo (e.g. in Ubuntu) used):
libsdl, libsdl-mixer, libsdl-image, physfs and curl."
 case $DISTRO in
  Debian|debian)
  su -c "apt-get update && apt-get install libsdl1.2-dev libsdl-mixer1.2-dev libsdl-image1.2-dev libcurl4-gnutls-dev libphysfs-dev autoconf automake"
  ;;
  Ubuntu|ubuntu|Kubuntu|kubuntu|xubuntu|Xubuntu)
  $DISTRO=Ubuntu
  sudo apt-get update
  sudo apt-get install libsdl1.2-dev libsdl-mixer1.2-dev libsdl-image1.2-dev libcurl4-gnutls-dev libphysfs-dev autoconf automake
  ;;
  ArchLinux|arch|archlinux)
  su -c "pacman -Sy sdl_mixer sdl_image physfs curl automake autoconf"
  ;;
  Fedora|fedora)
  su -c "yum install physfs sdl curl autoconf automake"
  ;;
  openSuse|opensuse|suse)
  su -c "zypper install curl sdl sdl_image sdl_mixer physfs autoconf automake"
  ;;
  *)
  echo "Not found or not supported at this time"
  ;;
 esac
}
# Install functions
install_prepare() {
 echo "Creating directiories"
 create_dirs;
 echo "Downloading and unpacking source";
 prepare_source;
 case $MODE in
  single) install_single; ;;
  multi) install_multi; ;;
 esac
}
install_single() {
 install_testdir;
 echo "Compiling and installing game"
 ./bootstrap &&
 if [ "$INSTALLDIR" = "" ]; then 
  ./configure
  INSTALLDIR="${HOME}"
 else ./configure --prefix=${INSTALLDIR}
 fi &&
 make
 make install
 create_shortcut_single
 create_starter_single
 remove_uninstaller &&
 echo "Daimonin was succesfully installed!
Enter ${INSTALLDIR}/daimonin folder and run Daimonin. Or run shortcut placed on desktop."
 clean;
}
install_multi() {
 echo "Creating shortcuts" &&
 create_shortcut_multi &&
 create_starter_multi &&
 echo "Compiling and installing game" &&
 ./bootstrap &&
 ./configure --prefix=/usr/share &&
 make
 case $DISTRO in
 Ubuntu)
  sudo mv daimonin.sh /usr/bin/daimonin &&
  sudo mv daimonin.desktop /usr/share/applications &&
  sudo make install
 ;;
 *)
  su -c "mv daimonin.sh /usr/bin/daimonin && mv daimonin.desktop /usr/share/applications && make install"
 ;;
 esac &&
 echo "Daimonin was succesfully installed!
Shortcut was placed in menu."
 clean;
}
# Upgrade functions
update_prepare() {
 clean;
 echo "Creating directiories"
 create_dirs;
 echo "Downloading and unpacking source";
 prepare_source;
 case $MODE in
  single) remove_single; install_single; ;;
  multi) remove_multi; install_multi; ;;
 esac
}
# Remove functions
remove_single() {
 if [ -e ~/.daimonin/installer/main_single_installdir ]; then
  rm -rf `cat ~/.daimonin/installer/main_single_installdir`/daimonin &&
  rm -f ~/Desktop/daimonin.desktop &&
  rm -f ~/.local/share/applications/daimonin.desktop &&
  echo "Game succesfully removed!
Thank you for using Daimonin Installer!"
 else echo "Can't found game installation folder."
      exit
 fi
}
remove_multi() {
 case $DISTRO in
 Ubuntu)
  sudo "rm -rf /usr/share/daimonin &&
  rm -rf /usr/share/daimonin-0.10.0 &&
  rm -f /usr/share/applications/daimonin.desktop &&
  rm -f /usr/bin/daimonin"
  echo "Game succesfully removed!
Thank you for using Daimonin Installer!"
 ;;
 *)
  su -c "rm -rf /usr/share/daimonin && rm -rf /usr/share/daimonin-0.10.0 && rm /usr/share/applications/daimonin.desktop && rm /usr/bin/daimonin" &&
  echo "Game succesfully removed!
Thank you for using Daimonin Installer!"
 ;;
 esac
}
remove_uninstaller() {
 # Create uninstaller which removes working directory.
 echo "#!/bin/sh
 if [ -e daimonin ] && [ -e daimonin-updater ] && [ -e daimonin.p0 ]; then
  rm -rf \$PWD &&
  rm -f ~/Desktop/daimonin.desktop &&
  rm -f ~/.local/share/applications/daimonin.desktop &&
  rm -f ~/.daimonin/installer/main_single_installdir &&
  rm -r ~/.daimonin/installer/client.tar.gz &&
  echo 'GAME UNINSTALLED!'
 else
  echo 'There is no daimonin, daimonin-updater and daimonin.p0 file! Aborting...'
 fi" > ${INSTALLDIR}/daimonin/uninstall.sh
 chmod +x ${INSTALLDIR}/daimonin/uninstall.sh
 echo "${INSTALLDIR}" > ~/.daimonin/installer/main_single_installdir
}

case $MODE in 
 single|multi)
   case $ACTION in
    install) distro_or_installdir; load_or_set_distro; install_dependencies; install_prepare; ;;
    update) update_prepare; ;;
    remove) load_or_set_distro; remove_prepare; ;;
    *) show_help;
   esac
   ;;
 *) show_help; ;;
esac
