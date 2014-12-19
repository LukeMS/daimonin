#!/bin/bash
# install.bash
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
# Define some generally useful functions.
########
# command_exists returns and echoes to stdout whether or not a command list
# exists.
# $1: the command(s)
# $?: zero if all commands in $1 exist, non-zero if any do not
command_exists() {
  for C in $1; do
    $(hash "$C" 2>/dev/null); R=$?; [ $R -ne 0 ] && break;
  done;
  echo "$R"
  return $R
}
# call_if_exists calls and returns whether or not a function exists.
# $1: the function
# $2 ...: args for $1
# $?: non-127 if the function exists (ie, it is called and its exit status is
#     returned), 127 if it does not
call_if_exists() {
  F="$1"; shift
  if [ "$(type -t "$F")" = "function" ]; then "$F" "$@"; R=$?; else R=127; fi
  return $R
}
# replace_text edits the text of a file.
# $1: filename
# $2: string to replace (all instances are changed)
# $3: replacement string
# $4 $5 ...: as $2 $3 when you want to replace lots of text
# NOTE: Where $2 ends with a $ and $3 is "" that whole line is deleted.
replace_text() {
  F="$1"; shift
  while [ "$#" -gt 1 ]; do
    if [ -n "$(expr "$1" : '.*\(\$\)$')" -a -z "$2" ]; then sed -i -e "/$1/d" "$F"
    else sed -i -e "s|$1|$2|g" "$F"; fi
    shift 2
  done
}

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
# Set $GUI to indicate which GUI to use. Possible values are cli, gtk and qt.
GUI="cli"
[ $(command_exists "zenity") -eq 0 ] && GUI="gtk"
[ $(command_exists "kdialog qdbus") -eq 0 -o $(command_exists "kdialog dcop") -eq 0 ] && GUI="qt"
# $WIDTH and $HEIGHT are the dimensions of the GUI windows in pixels when
# showing a textfile (only if $GUI != "cli").
WIDTH=800
HEIGHT=600
# $GUI_TITLE is the text in the title bar of each GUI window (only if $GUI !=
# "cli").
GUI_TITLE="Daimonin Client Installer"
# $GUI_* are miscellaneous strings shown in GUI windows.
GUI_ACCEPT="I have definitely read and accept the terms of this license"
GUI_INSTALLDIR="In which directory would you like to install Daimonin?"
GUI_NOTDIR="The requested location is not a directory!"
GUI_NOWRITE="You do not have write access to the requested location!"
GUI_EXISTS="'$INSTALLSUFFIX' already exists in the requested location!"
GUI_INTERNAL="The installer has encountered an internal problem -- your installation may not work properly!"

########
# Define the various gui_* functions to display info in a manner appropriate to
# $GUI.
#
# gui_showtext: show text in GUI.
# $1: mode (for numeric modes, respect the reserved exit codes -- see
#     http://tldp.org/LDP/abs/html/exitcodes.html#AEN23549):
#     0: show $2 as info (return 0)
#     1 to 255: show $2 as error (exit shell with $1 [does not return])
#     -1 to -255: show $2 as warning (return absolute $1)
#     ?: show $2 as question (return 0 if yes, 1 if no)
#     !: show $2 as textfile (return depends on $3)
# $2: explanatory text to show (according to $1)
# $3: ONLY IF $1 IS !: acknowledgement text to show (return 0 if yes, 1 if no)
# $?: see $1 (mode error returns 255)
#
# gui_choosedir: show default directory in GUI and allow it or another
# to be selected.
# $1: explanatory text to show.
# $2: default.
# $?: 0 if OK selected; 1 if CANCEL selected.
# The selected directory is echoed to stdout.
#
# gui_progress: show the ongoing progress of the chosen action in the GUI.
# The function is used at the end of a pipeline (eg,
# { ...code... } | gui_progress). Pipes beginning with a # are interpreted as
# labels informing the user of the current situation and numbers only indicate
# the total percentage complete..
########
if [ "$GUI" = "cli" ]; then
  gui_showtext() {
    case "$1" in
      0)
      echo -e "$2" && return 0 ;;
      [1-9] | [1-9][0-9] | 1[0-9][0-9] | 2[0-4][0-9] | 25[0-5])
      echo -e "ERROR! $2" && exit $1 ;;
      -[1-9] | -[1-9][0-9] | -1[0-9][0-9] | -2[0-4][0-9] | -25[0-5])
      R=${1/#-/}; echo -e "WARNING! $2" && return $R ;;
      "?")
      echo -e "$2 [Y/n] "
      read
      [ -z "$REPLY" -o "${REPLY:0:1}" = "y" -o "${REPLY:0:1}" = "Y" ] && return 0 || return 1 ;;
      "!")
      if [ -e "$2" ]; then
        # If $2 is a .html, uncomment the <!-- GUI=... --> markup for $GUI and
        # strip all HTML.
        if [ -n "$(expr "$2" : '.*\(\.html\)')" ]; then
          replace_text "$2" "<!-- GUI=[a-z,]*$GUI[a-z,]* *\(.\+\) -->" "\1"
          replace_text "$2" "<!-- .\+ -->$" "" "<hr />" "-- -- -- -- --" "<.\+>" ""
        fi
        cat "$2"
        if [ -n "$3" ]; then
          echo -e "$3 [y/N] "
          read
          [ "${REPLY:0:1}" = "y" -o "${REPLY:0:1}" = "Y" ] && return 0 || return 1
        else
          echo -e "Continue? [Y/n] "
          read
          [ -z "$REPLY" -o "${REPLY:0:1}" = "y" -o "${REPLY:0:1}" = "Y" ] && return 0 || return 1
        fi
      fi ;;
    esac
    return 255
  }
  gui_choosedir() {
    echo -e "$1\n\n(Default: $2)" >&2
    read
    if [ -n "$REPLY" ]; then echo "$REPLY"; else echo "$2"; fi
  }
  gui_progress() {
    while read IN; do
      if [ "$(expr "$IN" : '\([0-9]\+$\)')" = "$IN" ]; then PROGRESS=$IN
      elif [ "${IN:0:1}" = "#" ]; then echo -e "[$PROGRESS%] ${IN:1}"; fi
    done
  }
elif [ "$GUI" = "gtk" ]; then
  gui_showtext() {
    case "$1" in
      0)
      $(zenity --info --text "$2" --title "$GUI_TITLE") && return 0 ;;
      [1-9] | [1-9][0-9] | 1[0-9][0-9] | 2[0-4][0-9] | 25[0-5])
      $(zenity --error --text "$2" --title "$GUI_TITLE") && exit $1 ;;
      -[1-9] | -[1-9][0-9] | -1[0-9][0-9] | -2[0-4][0-9] | -25[0-5])
      R=${1/#-/}; $(zenity --warning --text "$2" --title "$GUI_TITLE") && return $R ;;
      "?")
      $(zenity --question --text "$2" --title "$GUI_TITLE") && return ;;
      "!")
      if [ -e "$2" ]; then
        # If $2 is a .html, uncomment the <!-- GUI=... --> markup for $GUI.
        if [ -n "$(expr "$2" : '.*\(\.html\)')" ]; then
          replace_text "$2" "<!-- GUI=[a-z,]*$GUI[a-z,]* *\(.\+\) -->" "\1"
          S="--html"
        else
          S=""
        fi
        [ -n "$(expr "$2" : '.*\(\.html\)')" ] && S="--html"
        if [ -z "$3" ]; then $(zenity --text-info --filename "$2"  $S --title "$GUI_TITLE" --width $WIDTH --height $HEIGHT) && return
        else $(zenity --text-info --filename "$2" --checkbox "$3"  $S --title "$GUI_TITLE" --width $WIDTH --height $HEIGHT); return; fi
      fi ;;
    esac
    return 255
  }
  gui_choosedir() {
    V=$(zenity --file-selection --directory --filename "$2" --title "$GUI_TITLE" --text "$1")
    R=$?
    echo "$V"
    return $R
  }
  gui_progress() {
    zenity --progress --percentage 0 --auto-close --title "$GUI_TITLE" --text "$1"
  }
# FIXME: Unfortunately zenity --notification is bugged -- see
# https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=716717
# Not mentioned is the fact that this freezes the script.
#  gui_progress_end() {
#    zenity --notification --title "$GUI_TITLE" --text "$1"
#  }
elif [ "$GUI" = "qt" ]; then
  gui_showtext() {
    case "$1" in
      0)
      kdialog --title "$GUI_TITLE" --msgbox "$2" && return 0 ;;
      [1-9] | [1-9][0-9] | 1[0-9][0-9] | 2[0-4][0-9] | 25[0-5])
      kdialog --title "$GUI_TITLE" --error "$2" && exit $1 ;;
      -[1-9] | -[1-9][0-9] | -1[0-9][0-9] | -2[0-4][0-9] | -25[0-5])
      R=${1/#-/}; kdialog --title "$GUI_TITLE" --sorry "$2" && return $R ;;
      "?")
      kdialog --title "$GUI_TITLE" --yesno "$2"; return ;;
      "!")
      if [ -e "$2" ]; then
        # If $2 is a .html, uncomment the <!-- GUI=... --> markup for $GUI and
        # strip HTML comments, </pre>, and </p> (because kdialog doesn't seem
        # to recognise them and displays them in the dialog).
        if [ -n "$(expr "$2" : '.*\(\.html\)')" ]; then
          replace_text "$2" "<!-- GUI=[a-z,]*$GUI[a-z,]* *\(.\+\) -->" "\1"
          replace_text "$2" "<!-- .\+ -->$" "" "</pre>" "" "</p>" ""
        fi
        if [ -n "$3" ]; then
          kdialog --title "$GUI_TITLE" --textbox "$2" $WIDTH $HEIGHT
          [ "$?" -eq 0 ] && kdialog --title "$GUI_TITLE" --yesno "$3"
        else
          kdialog --title "$GUI_TITLE" --textbox "$2" $WIDTH $HEIGHT
        fi
        return
      fi ;;
    esac
    return 255
  }
  gui_choosedir() {
   V=$(kdialog --title "$GUI_TITLE" --getexistingdirectory "$2")
    R=$?
    echo "$V"
    return $R
  }
  gui_progress_start() {
    QTPROGRESS=$(kdialog --title "$GUI_TITLE" --progressbar "$1" 100)
  }
  # Although gui_progress_start is the same in all cases (it uses kdialog
  # directly), gui_progress and gui_progress_end use either qdbus or dcop.
  if [ $(command_exists "qdbus") -eq 0 ]; then
    gui_progress() {
      while read IN; do
        if [ "$(expr "$IN" : '\([0-9]\+$\)')" = "$IN" ]; then qdbus $QTPROGRESS Set "" value "$IN"
        elif [ "${IN:0:1}" = "#" ]; then qdbus $QTPROGRESS setLabelText "${IN:1}"; fi
      done
    }
    gui_progress_end() {
      qdbus $QTPROGRESS close
      kdialog --title "$GUI_TITLE" --passivepopup "$1" 4
    }
  else
    gui_progress() {
      while read IN; do
        if [ "$(expr "$IN" : '\([0-9]\+$\)')" = "$IN" ]; then dcop $QTPROGRESS setProgress "$IN"
        elif [ "${IN:0:1}" = "#" ]; then dcop $QTPROGRESS setLabel "${IN:1}"; fi
      done
    }
    gui_progress_end() {
      dcop $QTPROGRESS close
      kdialog --title "$GUI_TITLE" --passivepopup "$1" 4
    }
  fi
fi

########
#
########
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
    INSTALLDIR=$(gui_choosedir "$GUI_INSTALLDIR" "$HOME/") || exit 0
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
  replace_text "$LAUNCHER" "%INSTALLDIR%" "$INSTALLDIR" "%INSTALLSUFFIX%" "$INSTALLSUFFIX" "%MENUDIR%" "$MENUDIR" "%DESKTOPENTRY%" "$DESKTOPENTRY"
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
