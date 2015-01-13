# gui.bash
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
# gui_showlist: show list of options in GUI.
# $1: mode:
#     +: check list (multiple options may be selected)
#     *: radio list (single option only may be selected)
#     -: normal list (single option only may be selected)
# $2: explanatory text to show
# $3: list of single-word column names
# $4 ...: text of options, per column
# $?: 0 if OK selected; 1 if CANCEL selected
# The selected option is echoed to stdout (multiple selections are separated
# with '|')
#
# gui_choosedir: show default directory in GUI and allow it or another
# to be selected.
# $1: explanatory text to show
# $2: default
# $?: 0 if OK selected; 1 if CANCEL selected
# The selected directory is echoed to stdout
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
  gui_showlist() {
    # $M is the mode.
    M="$1"
    # $S indicates a selected option and $U an unselected option.
    case "$M" in
    "+") S="[X]" U="[ ]" ;;
    "*") S="<X>" U="< >" ;;
    "-") S="" U="" ;;
    esac
    # $T is the explanatory text.
    T="$2"
    # $N is a count of the number of columns.
    N=0
    for E in $3; do ((N++)); done
    shift 3
    # Copy the remaining parameters (the list data) to a local array ($P[]).
    # Each index ($I) is padded by an empty one before it (space for our
    # mode-dependent column).
    local P
    I=1
    while [ $# -ne 0 ]; do [ -n "$1" ] && { P[I]="$1"; ((I+=2)); }; shift; done
    # For check/radio lists the first column of data are $U/$S values. For user
    # simplicity we force all options to $U except the first which is forced to
    # $S.
    I=0
    while [ -n "${P[I+1]}" ]; do P[I]="$U"; ((I=$I+$N*2)); done
    P[0]="$S"
    echo -e "$T" >&2
    # A continuous loop of showing the list of options then getting and acting
    # on user input (which is how we may break out of the loop).
    while [ 0 ]; do
      # Show each row of data (incrementally numbered ($J)).
      I=0 J=1
      while [ -n "${P[I+1]}" ]; do
        echo -en "$J) ${P[I]}" >&2
        for X in $(seq 1 2 $(($N*2-1))); do echo -en "  ${P[I+X]}" >&2; done
        echo >&2
        ((I=$I+$N*2))
        ((J++))
      done
      ((J--))
      # Check/radio lists also have CANCEL and OK buttons.
      if [ "$M" != "-" ]; then
        echo -en "c) cancel  o) ok\n(1-$J/c/O)? " >&2
        read
        if [ "$REPLY" = "c" -o "$REPLY" = "C" ]; then R=1 V=""; break
        elif [ "$REPLY" = "o" -o "$REPLY" = "O" -o "$REPLY" = "" ]; then R=0 V="${V:1}"; break; fi
      else
        echo -n "(1-$J)? " >&2
        read
      fi
      # When the user inputs a number in the range 1-$J then depending on mode
      # do: (for normal lists) select this option and break out of the loop;
      # (for check lists) select or deselect the option depending on its
      # previous status; (for radio lists) if the option is currently
      # unselected, select it and deselect all others.
      if [ -n "$(expr "$REPLY" : '^\([0-9]\+\)$')" -a $REPLY -ge 1 -a $REPLY -le $J ]; then
        if [ "$M" = "-" ]; then
          ((I=($REPLY-1)*$N*2+1))
          R=0 V="${P[I]}"
          break
        else
          V="" I=0 J=1
          while [ -n "${P[I+1]}" ]; do
            if [ "$M" = "+" ]; then
              [ $J -eq $REPLY ] && if [ "${P[I]}" = "$U" ]; then P[I]="$S"; else P[I]="$U"; fi
              [ "${P[I]}" = "$S" ] && V="$V|${P[I+1]}"
            else
              if [ $J -eq $REPLY ]; then V="|${P[I+1]}" P[I]="$S"; else P[I]="$U"; fi
            fi
            ((I=$I+$N*2))
            ((J++))
          done
        fi
      fi
    done
    echo "$V"
    return $R
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
  gui_showlist() {
    # $M is the mode.
    # $S indicates a selected option and $U an unselected option.
    case "$1" in
    "+") M="--checklist" S="TRUE" U="FALSE" ;;
    "*") M="--radiolist" S="TRUE" U="FALSE"  ;;
    "-") M="" S="" U=""  ;;
    esac
    # $T is the explanatory text.
    T="$2"
    # $N is a count of the number of columns.
    if [ -z "$M" ]; then L="" N=0; else L="--column Sel" N=1; fi
    for E in $3; do L="$L --column $E"; ((N++)); done
    shift 3
    # Copy the remaining parameters (the list data) to a local array ($P[]).
    # For normal lists,the indices ($I) are simply incremental. For check/radio
    # lists, each index ($I) is padded by an empty one before it. The first
    # column of data are $U/$S values. For user simplicity we force all options
    # to $U except the first which is forced to $S.
    local P
    if [ -z "$M" ]; then
      I=0
      while [ $# -ne 0 ]; do [ -n "$1" ] && { P[I]="$1"; ((I+=1)); }; shift; done
    else
      I=1
      while [ $# -ne 0 ]; do [ -n "$1" ] && { P[I]="$1"; ((I+=2)); }; shift; done
      I=0
      while [ -n "${P[I+1]}" ]; do P[I]="$U"; ((I+=($N*2-2))); done
      P[0]="$S"
    fi
    V=$(zenity --list $M --title "$TITLE" --text "$T" $L "${P[@]}")
    R=$?
    # A bug in zenity seems to double up the output when choosing with the
    # keyboard (| is the default separator).
    [ -z "$M" ] && V="$(expr "$V" : '\([^|]*\)')"
    echo "$V"
    return $R
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
  gui_showlist() {
    # $M is the mode.
    # $S indicates a selected option and $U an unselected option.
    case "$1" in
    "+") M="--checklist" S="on" U="off" ;;
    "*") M="--radiolist" S="on" U="off" ;;
    "-") M="--menu" S="" U="" ;;
    esac
    # $T is the explanatory text.
    T="$2"
    # $N is a count of the number of columns.
    if [ "$M" = "--menu" ]; then N=0; else N=1; fi
    for E in $3; do ((N++)); done
    shift 3
    # Copy the remaining parameters (the list data) to a local array ($P[]).
    # For normal lists,the indices ($I) are simply incremental. For check/radio
    # lists, each index ($I) is padded by an empty one before it. The last
    # column of data are $U/$S values. For user simplicity we force all options
    # to $U except the first which is forced to $S.
    local P
    if  [ "$M" = "--menu" ]; then
      I=0
      while [ $# -ne 0 ]; do [ -n "$1" ] && { P[I]="$1"; ((I+=1)); }; shift; done
    else
      I=0
      while [ $# -ne 0 ]; do [ -n "$1" ] && { P[I]="$1"; ((I+=2)); }; shift; done
      I=0
      while [ -n "${P[I]}" ]; do ((I=$I+$N*2-1)); P[I]="$U"; ((I++)); done
      P[$N]*2-1="$S"
    fi
    V=$(kdialog --title "$TITLE" $M "$T" "${P[@]}")
    R=$?
    echo "$V"
    return $R
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
