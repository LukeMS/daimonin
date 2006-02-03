#!/bin/sh

aclocal
automake --add-missing --foreign
autoconf
sh configure
cp ./make/linux/plugins.cfg ./
