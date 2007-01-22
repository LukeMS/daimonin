#!/bin/sh
# Shell script for fixing the properties of files in arch/ and maps/.

mapsIgnore=".dedit"
archIgnore="Thumbs.db"
echo=

find arch -name "*.png" -print0 | xargs -0 $echo svn propset svn:mime-type image/png
find arch -name "*.arc" -print0 | xargs -0 $echo svn propset svn:mime-type text/plain
find arch -name "*.arc" -print0 | xargs -0 $echo svn propset svn:eol-style LF
find arch -name "*.anim" -print0 | xargs -0 $echo svn propset svn:mime-type text/plain
find arch -name "*.anim" -print0 | xargs -0 $echo svn propset svn:eol-style LF
find arch -type d -not -path "*/.svn/*" -not -name ".svn" -print0 | xargs -0 $echo svn propset svn:ignore "$mapsIgnore"

find maps -type f -not -path "*/.svn/*" -print0 | xargs -0 $echo svn propset svn:mime-type text/plain
find maps -type f -not -path "*/.svn/*" -print0 | xargs -0 $echo svn propset svn:eol-style LF
find maps -type d -not -path "*/.svn/*" -not -name ".svn" -print0 | xargs -0 $echo svn propset svn:ignore "$mapsIgnore"

find maps -type f -name "*.txt" -not -path "*/.svn/*" -print0 | xargs -0 $echo svn propset svn:mime-type text/plain
find maps -type f -name "*.txt" -not -path "*/.svn/*" -print0 | xargs -0 $echo svn propset svn:eol-style native
#find arch -type f -name "*.txt" -not -path "*/.svn/*" -print0 | xargs -0 $echo svn propset svn:mime-type text/plain
#find arch -type f -name "*.txt" -not -path "*/.svn/*" -print0 | xargs -0 $echo svn propset svn:eol-style native

svn commit -m "Fixed svn:eol-style, svn:ignore and svn:mime-type properties."
