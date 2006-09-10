#!/bin/sh
#
# This is the Daimonin installation script.
# Use it after compiling or when using a binary package

prgname="daimonin_server"
luaplug="plugin_lua.so.0.1"

prgname_dst=$prgname
luaplug_dst=$luaplug
test ! -z "$1" && prgname_dst=$1
test ! -z "$2" && luaplug_dst=$2

basedir="./../.."
datadir="/data"

fail() {
	echo "***"
	echo "*** Installation failed"
	echo "***"
	exit 10
}

# instbin source dest
instbin() {
	# Since linux locks binaries that are running, we try to unlink old
	# version before replacing. This way installation works even on
	# a running server.
	[ -f $2 ] && rm -f $2
	cp $1 $2 || fail	
}

mmkdir() {
	[ -d $1 ] || mkdir $1
}

echo "Copy binaries"
instbin ./../../src/server/$prgname ./../../$prgname_dst
instbin ./../../src/plugin_lua/$luaplug ./../../plugins/$luaplug_dst
instbin $basedir/src/utils/dmonloop $basedir 

echo "Create data folders"
mmkdir $basedir/$datadir
mmkdir $basedir/$datadir/tmp
mmkdir $basedir/$datadir/log
mmkdir $basedir/$datadir/players
mmkdir $basedir/$datadir/unique-items
mmkdir $basedir/$datadir/global

echo "Copy server data"
cp $basedir/install/* $basedir/$datadir

mmkdir $basedir/lib
echo "Copy arch and lib files"
cp ./../../../arch/* $basedir/lib
echo "done."
