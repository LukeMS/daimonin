#!/bin/sh
#
# This is the Daimonin installation script.
# Use it after compiling or when using a binary package

prgname="daimonin_server"
pythonplug="plugin_python.so.0.1"

basedir="./../.."
datadir="/data"

echo "Copy binaries"
cp ./../../src/server/$prgname ./../../$prgname
cp ./../../src/plugin_python/$pythonplug ./../../plugins/$pythonplug
cp $basedir/src/utils/dmonloop $basedir

echo "Create data folders"
mkdir $basedir/$datadir
mkdir $basedir/$datadir/tmp
mkdir $basedir/$datadir/log
mkdir $basedir/$datadir/players
mkdir $basedir/$datadir/unique-items

echo "Copy server data"
cp $basedir/install/* $basedir/$datadir

mkdir $basedir/lib
echo "Copy arch and lib files"
cp ./../../../arch/* $basedir/lib
echo "done."
