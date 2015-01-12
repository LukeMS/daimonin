#!/bin/bash
# install.bash
# Copyright (C) 2014-2015 Julian Arnold
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
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-13

########
# Test the hardware (well, kernel too) and bail out immediately if it is
# unsupported.
########
KERNEL="$(uname)"
ARCH="$(uname -m)"
BORKED=""
case "$KERNEL" in
  "Linux")
  [ "$ARCH" != "x86_64" -a -n $(expr "$ARCH" : '.*\(86\).*') ] && ARCH="x86"
  [ -z $(expr "$ARCH" : '\(x86\)') ] && BORKED="the $ARCH architecture" ;;
  *)
  BORKED="the $KERNEL kernel"
esac
if [ -n "$BORKED" ];  then
  echo "Although Daimonin can be built for $BORKED and in theory"
  echo "this installer will also work, there is currently no-one on he devteam with"
  echo "access to such a system. If you can help, please contact us at"
  echo "www.daimonin.org."
  exit 1
fi

########
# Initialise some variables.
########
# $UNPACKS is a space-separated list of groups of directories. Within groups
# | is the separator. The first dir of each group is the dir within the
# installer to unpack from. Subsequent dirs are dirs within $INSTALLDIR to
# unpack to.
UNPACKS="base|. $ARCH|. resources|main|dev"
# $MODE is the user mode which determines where/how Daimonin is installed.
MODE="normal"
[ $(id -u) -eq 0 ] && MODE="root"
# $INSTALLDIR, $LAUNCHDIR, and $MENUDIR are the paths under which Daimonin will
# be installed. Obviously in normal user mode the first two are just
# placeholders (the user selects the real path later on).
if [ "$MODE" = "normal" ]; then
  INSTALLDIR=".../path/of/your/choice"
  LAUNCHDIR=".../same/path"
  MENUDIR="$HOME/.local/share/applications"
else
  INSTALLDIR="/usr/share"
  LAUNCHDIR="/usr/bin"
  MENUDIR="/usr/share/applications"
fi
# $INSTALLSUFFIX is the directory which will be created within $INSTALLDIR and
# in which the client will ultimately be installed.
INSTALLSUFFIX="daimonin_client"
# $WELCOME, $LICENSE, $INSTRUCTIONS, and $GOODBYE are textfiles displayed by
# the installer. $LICENSE is important -- this is the license under which
# Daimonin exists and must be accepted to install.
WELCOME="welcome.html"
LICENSE="base/gpl-2.0.txt"
INSTRUCTIONS="instructions.html"
GOODBYE="goodbye.html"
# $DESKTOPENTRY is the desktop entry file which will be insitalled -- see
# http://standards.freedesktop.org/desktop-entry-spec/latest/
DESKTOPENTRY="org.daimonin.Client.desktop"
# $LAUNCHER is the script file to launch/uninstall Daimonin which will be
# installed.
LAUNCHER="daimonin"
# $GUI_* are miscellaneous strings shown in GUI windows.
GUI_ROOTEXISTS="There appears to be a root installation of Daimonin on this system. Are you sure you want to continue with a new installation?"
GUI_ACCEPT="I have definitely read and accept the terms of this license"
GUI_INSTALLDIR="In which directory would you like to install Daimonin?"
GUI_NOTDIR="The requested location is not a directory!"
GUI_NOWRITE="You do not have write access to the requested location!"
GUI_EXISTS="'$INSTALLSUFFIX' already exists in the requested location!"
GUI_INTERNAL="The installer has encountered an internal problem -- your installation may not work properly!"

########
# Pull in some code (essentially function definitions) contained in external
# files.
########
source ./base/general.bash
source ./base/gui.bash

########
#
########
# Check for existing root installation.
[ $(command_exists "$LAUNCHER") -ne 0 ] || gui_showtext "?" "$GUI_ROOTEXISTS" || exit 0
# Welcome the user, get him to accept the license, and give him instructions.
gui_showtext "!" "$WELCOME" || exit 0
gui_showtext "!" "$LICENSE" "$GUI_ACCEPT" || exit 0
replace_text "$INSTRUCTIONS" "%MODE%" "$MODE" "%INSTALLDIR%" "$INSTALLDIR" "%INSTALLSUFFIX%" "$INSTALLSUFFIX" "%LAUNCHER%" "$LAUNCHER" "%LAUNCHDIR%" "$LAUNCHDIR" "%MENUDIR%" "$MENUDIR"
gui_showtext "!" "$INSTRUCTIONS" || exit 0
# Establish $INSTALLDIR. In normal mode this means allowing the user to choose
# a dir. If not valid, give a warning and repeat. In root mode the onlt likely
# problem is an installation already exists. So if not valid, give an error and
# exit.
while [ 0 ]; do
  if [ "$MODE" = "normal" ]; then
    V=-1 # warning, loop until $INSTALLDIR is valid
    INSTALLDIR=$(gui_choosedir "$GUI_INSTALLDIR" "$HOME") || exit 0
    INSTALLDIR=${INSTALLDIR/#~/$HOME} # ~/directory -> /home/foo/directory
    LAUNCHDIR="$INSTALLDIR/$INSTALLSUFFIX"
  elif [ "$MODE" = "root" ]; then
    V=1 # error, try once, exit on failure
  fi
  [ -e "$INSTALLDIR" ] || mkdir -p "$INSTALLDIR" 2>/dev/null
  if [ ! -d "$INSTALLDIR" ]; then
    gui_showtext "$V" "$GUI_NOTDIR"
  elif [ ! -w "$INSTALLDIR" ]; then
    gui_showtext "$V" "$GUI_NOWRITE"
  else
    if [ -e "$INSTALLDIR/$INSTALLSUFFIX" ]; then
      gui_showtext "$V" "$GUI_EXISTS"
    else
      mkdir "$INSTALLDIR/$INSTALLSUFFIX"
      break
    fi
  fi
done
#
call_if_exists "gui_progress_start" "Installing Daimonin..."
# 
{
  P=0; echo "$P"
  echo "# Installing game files..."
  T=98
  shopt -s globstar
  PACKS=""
  for UNPACK in $UNPACKS; do
    FROM="$(expr "$UNPACK" : '\([^|]\+\)')"
    if [ -z "$FROM" -o ! -d "$FROM" ]; then gui_showtext "-1" "$GUI_INTERNAL"; else PACKS="$PACKS $(ls -d $FROM/**)"; fi
  done
  NPACKS=$(echo "$PACKS" | wc -w)
  if [ $NPACKS -lt $T ]; then NCYCLES=$NPACKS; else NCYCLES=$T; fi
  ((PACKSPERCYCLE=$NPACKS/$NCYCLES))
  ((PROGPERCYCLE=$T/$NCYCLES))
  NCYCLES=0
  for PACK in $PACKS; do
    FROM="$(expr "$PACK" : '\([^/]\+\)')"
    for TOS in $UNPACKS; do
      if [ "$(expr "$TOS" : '\([^|]\+\)')" != "$FROM" ]; then
        TOS=""
      else
        TOS="$(expr "$TOS" : '[^|]\+|\(.\+\)')"
        TOS=${TOS//|/ }
        break
      fi
    done
    [ -z "$TOS" ] && gui_showtext "1" "$GUI_INTERNAL"
    echo "# Unpacking '$PACK'..."
    for TO in $TOS; do
      UNPACK="$INSTALLDIR/$INSTALLSUFFIX/$TO"
      if [ "$PACK" = "$FROM/" ]; then
        mkdir -p "$UNPACK"
      else
        UNPACK="$UNPACK/${PACK#$FROM/}"
        if [ -d "$PACK" ]; then mkdir -p "$UNPACK"
        else cp "$PACK" "$UNPACK"; fi
      fi
      if [ -x "$UNPACK" ]; then chmod 755 "$UNPACK"; else chmod 644 "$UNPACK"; fi
    done
    ((NCYCLES++))
    [ $NCYCLES -eq $PACKSPERCYCLE ] && { NCYCLES=0; ((P+=$PROGPERCYCLE)); [ $P -gt $T ] && P=$T; echo "$P"; }
  done
  P=$T; echo "$P"
  echo "# Creating $MODE user mode launcher..."
  replace_text "$LAUNCHER" "%INSTALLDIR%" "$INSTALLDIR" "%INSTALLSUFFIX%" "$INSTALLSUFFIX" "%MENUDIR%" "$MENUDIR" "%DESKTOPENTRY%" "$DESKTOPENTRY" "%LAUNCHDIR%" "$LAUNCHDIR" "%LAUNCHER%" "$LAUNCHER"
  cp -t "$LAUNCHDIR" "$LAUNCHER"
  chmod 755 "$LAUNCHDIR/$LAUNCHER"
  P=99; echo "$P"
  echo "# Creating $MODE user mode menu entry..."
  replace_text "$DESKTOPENTRY" "%INSTALLDIR%" "$INSTALLDIR" "%INSTALLSUFFIX%" "$INSTALLSUFFIX" "%LAUNCHDIR%" "$LAUNCHDIR" "%LAUNCHER%" "$LAUNCHER"
  mkdir -p "$MENUDIR" # KDE seems to miss the $HOME one
  cp -t "$MENUDIR" "$DESKTOPENTRY"
  chmod 644 "$MENUDIR/$DESKTOPENTRY"
  P=100;  echo "$P"
  echo "# Finished installation!"
} | gui_progress "Installing Daimonin..."
# 
call_if_exists "gui_progress_end" "Daimonin client installed in '$INSTALLDIR'"
# 
gui_showtext "!" "$GOODBYE"
