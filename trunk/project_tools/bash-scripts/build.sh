#!/bin/sh
# Builds a gamesever.

ulimit -c unlimited

svndir="$HOME/svn"
builddir="$HOME/build"
STARTTIME=`/bin/date +date/time:\ %Y%m%d\ %H:%M:%S\ %Z`

echo "### Backup some persistent server files." 1>&2
cp $builddir/server/data/ban_file $HOME/ban_file
cp $builddir/server/data/clockdata $HOME/clockdata
cp $builddir/server/data/gmaster_file $HOME/gmaster_file
cp $builddir/server/data/motd $HOME/motd
cp $builddir/server/data/settings $HOME/settings
cp $builddir/server/data/stream $HOME/stream

echo "### Remove previous build." 1>&2
rm -rf $builddir/arch
rm -rf $builddir/maps
rm -rf $builddir/gridarta
(
    cd $builddir/server/make/linux
    make distclean
)
wait

echo "### Checkout/update and export arches." 1>&2
(
    svn co https://daimonin.svn.sourceforge.net/svnroot/daimonin/trunk/arch $svndir/trunk/arch
    svn co https://daimonin.svn.sourceforge.net/svnroot/daimonin/streams/newarch/arch $svndir/newarch/arch
    svn co svn+ssh://svn@88.198.46.59/daiserv/trunk/arch-B5 $svndir/daiserv/arch
    svn co svn+ssh://svn@88.198.46.59/mapwiz/trunk/arch-B5 $svndir/mapserv/arch
)
wait
svn export $svndir/trunk/arch $builddir/arch
svn export --force $svndir/newarch/arch $builddir/arch
svn export --force $svndir/daiserv/arch $builddir/arch
svn export --force $svndir/mapserv/arch $builddir/arch

echo "### Checkout/update and export maps." 1>&2
(
    svn co https://daimonin.svn.sourceforge.net/svnroot/daimonin/trunk/maps $svndir/trunk/maps
    svn co https://daimonin.svn.sourceforge.net/svnroot/daimonin/streams/newarch/maps $svndir/newarch/maps
    svn co svn+ssh://svn@88.198.46.59/daiserv/trunk/maps-B5 $svndir/daiserv/maps
    svn co svn+ssh://svn@88.198.46.59/mapwiz/trunk/maps-B5 $svndir/mapserv/maps
)
wait
svn export $svndir/trunk/maps $builddir/maps
svn export --force $svndir/newarch/maps $builddir/maps
svn export --force $svndir/daiserv/maps $builddir/maps
svn export --force $svndir/mapserv/maps $builddir/maps

echo "### Checkout/update and export server." 1>&2
(
    svn co https://daimonin.svn.sourceforge.net/svnroot/daimonin/trunk/server $svndir/trunk/server
)
wait
svn export --force $svndir/trunk/server $builddir/server

(
    read stream <$HOME/stream
    if [[ $stream != "vanilla" ]]; then
        echo "### Direct export streams/$stream arch." 1>&2
        svn export --force https://daimonin.svn.sourceforge.net/svnroot/daimonin/streams/$stream/arch $builddir/arch
        echo "### Direct export streams/$stream maps." 1>&2
        svn export --force https://daimonin.svn.sourceforge.net/svnroot/daimonin/streams/$stream/maps $builddir/maps
        echo "### Direct export streams/$stream server." 1>&2
        success=`svn export --force https://daimonin.svn.sourceforge.net/svnroot/daimonin/streams/$stream/server $builddir/server`
        if [ -n "$success" ]; then
            echo "$success"
            echo "$stream" >$HOME/stream
            success=`svn export https://daimonin.svn.sourceforge.net/svnroot/daimonin/streams/$stream/DESCRIPTION.txt $HOME/DESCRIPTION.txt`
            if [ -n "$success" ]; then
                cat $HOME/DESCRIPTION.txt >>$HOME/stream
            fi
        fi
    fi
)
wait

echo "### Run tileset_updater.pl" 1>&2
(
    $builddir/project_tools/perl-scripts/tileset_updater.pl $builddir/maps
)
wait

echo "### Checkout/update, export, build, and run Gridarta to collect arches." 1>&2
svn co https://gridarta.svn.sourceforge.net/svnroot/gridarta/trunk/ $svndir/gridarta/trunk
svn export $svndir/gridarta/trunk $builddir/gridarta
(
    cd $builddir/gridarta
    ant jar-daimonin
    java -Xmx256M -jar DaimoninEditor.jar -c
)
wait

echo "### Build server." 1>&2
(
    cd $builddir/server/make/linux
    chmod +x configure
    ./configure
    make CFLAGS="-O2 -march=opteron -D_TESTSERVER="" -ffast-math -fPIC -funroll-loops -g" install
)
wait

echo "### Restore the persistent files." 1>&2
cp $HOME/ban_file $builddir/server/data/ban_file
cp $HOME/clockdata $builddir/server/data/clockdata
cp $HOME/gmaster_file $builddir/server/data/gmaster_file
cp $HOME/motd $builddir/server/data/motd
cp $HOME/settings $builddir/server/data/settings
cp $HOME/stream $builddir/server/data/stream

echo "=== Start $STARTTIME." 1>&2
echo "=== Stop  `/bin/date +date/time:\ %Y%m%d\ %H:%M:%S\ %Z`." 1>&2

exit 0
