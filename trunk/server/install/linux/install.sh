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
installdir="$basedir/install"
srcdir="$basedir/src"
datadir="$basedir/data"
libdir="$basedir/lib"

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
instbin $srcdir/server/$prgname $basedir/$prgname_dst
instbin $srcdir/plugin_lua/$luaplug $basedir/plugins/$luaplug_dst

echo "Create data folders"
mmkdir $datadir
mmkdir $datadir/tmp
mmkdir $datadir/log
mmkdir $datadir/accounts
mmkdir $datadir/players
mmkdir $datadir/unique-items
mmkdir $datadir/global

echo "Copy server data"
cp $installdir/* $datadir

mmkdir $libdir
echo "Copy arch and lib files"
cp $basedir/../arch/* $libdir
echo "done."
