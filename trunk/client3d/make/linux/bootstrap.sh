#!/bin/sh

aclocal
automake --add-missing --foreign
autoconf
sh configure
