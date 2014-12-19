#!/bin/bash
# uninstall.bash
# Copyright (C) 2014 Julian Arnold
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
# Initialise some variables.
########
GUI_HOWTO="To uninstall Daimonin, run the launcher in uninstall mode."
WILLDO="willdo.html"
GUI_CONFIRM="Are you sure you want to proceed?"
GUI_GOODBYE="Daimonin has been uninstalled. Thank you for playing."

########
# Pull in some code (essentially function definitions) contained in external
# files.
########
source ./general.bash
source ./gui.bash

########
# Assign script parameters.
########
if [ $# -ne 3 ]; then
  gui_showtext "1" "$GUI_HOWTO"
fi
LAUNCHER="$1"
INSTALLDIR="$2"
DESKTOPENTRY="$3"

########
#
########
#
replace_text "$WILLDO" "%LAUNCHER%" "$LAUNCHER" "%INSTALLDIR%" "$INSTALLDIR" "%DESKTOPENTRY%" "$DESKTOPENTRY"
gui_showtext "!" "$WILLDO" "$GUI_CONFIRM" || exit 0
#
call_if_exists "gui_progress_start" "Uninstalling Daimonin..."
# 
{
  P=0; echo "$P"
  echo "# Uninstalling game files..."
  echo "# Uninstalling launcher '$LAUNCHER'..."
  rm -f  "$LAUNCHER"
  P=1; echo "$P"
  T=99
  shopt -s globstar
  FILES="$(ls -d $INSTALLDIR/**)"
  NFILES=$(echo "$FILES" | wc -w)
  if [ $NFILES -lt $T ]; then NCYCLES=$NFILES; else NCYCLES=$T; fi
  ((FILESPERCYCLE=$NFILES/$NCYCLES))
  ((PROGPERCYCLE=$T/$NCYCLES))
  NCYCLES=0
  for FILE in $FILES; do
    echo "# Uninstalling '$FILE'..."
    [ ! -d "$FILE" ] && rm -f "$FILE"
    ((NCYCLES++))
    [ $NCYCLES -eq $FILESPERCYCLE ] && { NCYCLES=0; ((P+=$PROGPERCYCLE)); [ $P -gt $T ] && P=$T; echo "$P"; }
  done
  rm -rf "$INSTALLDIR"
  P=$T; echo "$P"
  echo "# Uninstalling menu entry '$DESKTOPENTRY'..."
  rm -f "$DESKTOPENTRY"
  P=100;  echo "$P"
  echo "# Finished uninstallation!"
} | gui_progress "Uninstalling Daimonin..."
# 
call_if_exists "gui_progress_end" "Daimonin client uninstalled"
# 
gui_showtext "0" "$GUI_GOODBYE"
