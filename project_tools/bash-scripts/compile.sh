#!/bin/sh
# Kill, rebuild, and restart a gameserver.

serverdir="$HOME/build/server"

echo "### Kill any svn, build, server, or dmonloopx processes." 1>&2
ps ax|grep 'svn'|grep -v grep|awk '{print $1}'|xargs kill
ps ax|grep 'build.sh'|grep -v grep|awk '{print $1}'|xargs kill
ps ax|grep 'daimonin_server'|grep -v grep|awk '{print $1}'|xargs kill
ps ax|grep 'memcheck'|grep -v grep|awk '{print $1}'|xargs kill
ps ax|grep 'dmonloopx'|grep -v grep|awk '{print $1}'|xargs kill

echo "### Build gameserver." 1>&2
rm -rf $serverdir/data/logs/*
if [ -e $serverdir/data/stream ]; then
    rm $serverdir/data/stream
fi
$HOME/build.sh

echo "### Replace dmonloopx." 1>&2
cp $HOME/dmonloopx $serverdir/dmonloopx

echo "### Run dmonloopx." 1>&2
cd $serverdir
./dmonloopx &

exit 0
