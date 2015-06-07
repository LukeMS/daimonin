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

# Install datadir if it does not already exist (ie, a new installation). Once
# this has been done once, this dir is persistent between reboots. It is left
# to server admins to arrange backups.
[ ! -d $datadir ] && echo "Install new persistent data" && cp -r $installdir/data $datadir

echo "Copy arch and lib files"
mmkdir $libdir
cp $basedir/../arch/* $libdir
echo "done."
