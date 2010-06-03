#!/bin/sh
# Kill, rebuild, and restart a gameserver.

svndir="$HOME/svn"
builddir="$HOME/build"

echo "### Kill any svn, build, server, or dmonloopx processes." 1>&2
ps ax|grep 'svn'|grep -v grep|awk '{print $1}'|xargs kill
ps ax|grep 'build.sh'|grep -v grep|awk '{print $1}'|xargs kill
ps ax|grep 'daimonin_server'|grep -v grep|awk '{print $1}'|xargs kill
ps ax|grep 'memcheck'|grep -v grep|awk '{print $1}'|xargs kill
ps ax|grep 'dmonloopx'|grep -v grep|awk '{print $1}'|xargs kill

echo "### Checkout/update and export project_tools" 1>&2
svn co https://daimonin.svn.sourceforge.net/svnroot/daimonin/trunk/project_tools $svndir/trunk/project_tools
svn export --force $svndir/trunk/project_tools $builddir/project_tools
cp -f $builddir/project_tools/bash-scripts/compile.sh $HOME

echo "### Build gameserver." 1>&2
if [ -n "$1" ]; then
    echo "$1" >$builddir/server/data/stream
fi
rm -rf $builddir/server/data/logs/*
$builddir/project_tools/bash-scripts/build.sh

echo "### Replace dmonloopx." 1>&2
cp $builddir/project_tools/bash-scripts/dmonloopx $builddir/server/dmonloopx

echo "### Run dmonloopx." 1>&2
cd $builddir/server
./dmonloopx &

exit 0
