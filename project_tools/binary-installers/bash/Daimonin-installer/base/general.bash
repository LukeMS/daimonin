# general.bash
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
