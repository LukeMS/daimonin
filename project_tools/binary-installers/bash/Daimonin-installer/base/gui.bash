# gui.bash
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
# Set $GUI to indicate which GUI to use. Possible values are cli, gtk and qt.
GUI="cli"
[ $(command_exists "zenity") -eq 0 ] && GUI="gtk"
[ $(command_exists "kdialog qdbus") -eq 0 -o $(command_exists "kdialog dcop") -eq 0 ] && GUI="qt"
# $WIDTH and $HEIGHT are the dimensions of the GUI windows in pixels when
# showing a textfile (when $GUI != "cli").
WIDTH=800
HEIGHT=600
# $TITLE is the text in the title bar of each GUI window (when $GUI != "cli").
TITLE="Daimonin Client Installer"

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
          replace_text "$2" "<!-- .\+ -->$" "" "<hr />" "-- -- -- -- --"  "<[^>]\+>" ""
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
      $(zenity --info --text "$2" --title "$TITLE") && return 0 ;;
      [1-9] | [1-9][0-9] | 1[0-9][0-9] | 2[0-4][0-9] | 25[0-5])
      $(zenity --error --text "$2" --title "$TITLE") && exit $1 ;;
      -[1-9] | -[1-9][0-9] | -1[0-9][0-9] | -2[0-4][0-9] | -25[0-5])
      R=${1/#-/}; $(zenity --warning --text "$2" --title "$TITLE") && return $R ;;
      "?")
      $(zenity --question --text "$2" --title "$TITLE") && return ;;
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
        if [ -z "$3" ]; then $(zenity --text-info --filename "$2"  $S --title "$TITLE" --width $WIDTH --height $HEIGHT) && return
        else $(zenity --text-info --filename "$2" --checkbox "$3"  $S --title "$TITLE" --width $WIDTH --height $HEIGHT); return; fi
      fi ;;
    esac
    return 255
  }
  gui_choosedir() {
    V=$(zenity --file-selection --directory --filename "$2" --title "$TITLE" --text "$1")
    R=$?
    echo "$V"
    return $R
  }
  gui_progress() {
    zenity --progress --percentage 0 --auto-close --title "$TITLE" --text "$1"
  }
# FIXME: Unfortunately zenity --notification is bugged -- see
# https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=716717
# Not mentioned is the fact that this freezes the script.
#  gui_progress_end() {
#    zenity --notification --title "$TITLE" --text "$1"
#  }
elif [ "$GUI" = "qt" ]; then
  gui_showtext() {
    case "$1" in
      0)
      kdialog --title "$TITLE" --msgbox "$2" && return 0 ;;
      [1-9] | [1-9][0-9] | 1[0-9][0-9] | 2[0-4][0-9] | 25[0-5])
      kdialog --title "$TITLE" --error "$2" && exit $1 ;;
      -[1-9] | -[1-9][0-9] | -1[0-9][0-9] | -2[0-4][0-9] | -25[0-5])
      R=${1/#-/}; kdialog --title "$TITLE" --sorry "$2" && return $R ;;
      "?")
      kdialog --title "$TITLE" --yesno "$2"; return ;;
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
          kdialog --title "$TITLE" --textbox "$2" $WIDTH $HEIGHT
          [ "$?" -eq 0 ] && kdialog --title "$TITLE" --yesno "$3"
        else
          kdialog --title "$TITLE" --textbox "$2" $WIDTH $HEIGHT
        fi
        return
      fi ;;
    esac
    return 255
  }
  gui_choosedir() {
   V=$(kdialog --title "$TITLE" --getexistingdirectory "$2")
    R=$?
    echo "$V"
    return $R
  }
  gui_progress_start() {
    PROGRESS=$(kdialog --title "$TITLE" --progressbar "$1" 100)
  }
  # Although gui_progress_start is the same in all cases (it uses kdialog
  # directly), gui_progress and gui_progress_end use either qdbus or dcop.
  if [ $(command_exists "qdbus") -eq 0 ]; then
    gui_progress() {
      while read IN; do
        if [ "$(expr "$IN" : '\([0-9]\+$\)')" = "$IN" ]; then qdbus $PROGRESS Set "" value "$IN"
        elif [ "${IN:0:1}" = "#" ]; then qdbus $PROGRESS setLabelText "${IN:1}"; fi
      done
    }
    gui_progress_end() {
      qdbus $PROGRESS close
      kdialog --title "$TITLE" --passivepopup "$1" 4
    }
  else
    gui_progress() {
      while read IN; do
        if [ "$(expr "$IN" : '\([0-9]\+$\)')" = "$IN" ]; then dcop $PROGRESS setProgress "$IN"
        elif [ "${IN:0:1}" = "#" ]; then dcop $PROGRESS setLabel "${IN:1}"; fi
      done
    }
    gui_progress_end() {
      dcop $PROGRESS close
      kdialog --title "$TITLE" --passivepopup "$1" 4
    }
  fi
fi
