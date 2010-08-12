#!/bin/bash
# Kill gameserver, and update scripts.

########
# Set global variables.
source ./dai_setvars.sh

if [ -f ./nohup.out ]
then
    mv ./nohup.out logs/`date +%Y%m%d_%H%M%S%Z --utc`.log
fi

echo
echo "#######################################################"
echo "### Permanently backup some persistent server files ###"
echo "#######################################################"
cp ${dai_builddir}/server/data/ban_file ${dai_home}/ban_file.bak
cp ${dai_builddir}/server/data/clockdata ${dai_home}/clockdata.bak
cp ${dai_builddir}/server/data/gmaster_file ${dai_home}/gmaster_file.bak
cp ${dai_builddir}/server/data/motd ${dai_home}/motd.bak
cp ${dai_builddir}/server/data/settings ${dai_home}/settings.bak
cp ${dai_builddir}/server/data/stream ${dai_home}/stream.bak

echo
echo "#######################"
echo "### Kill gameserver ###"
echo "#######################"

ps ax | grep "${dai_server_exe}" | grep -v grep | awk '{print $1}' | xargs kill > /dev/null
ps ax | grep "${dai_reboot_sh}" | grep -v grep | awk '{print $1}' | xargs kill > /dev/null
sleep 15

echo
echo "#############################"
echo "### Direct export scripts ###"
echo "#############################"
svn export --force ${dai_daimonin_repo}/trunk/project_tools/bash-scripts/dai_refresh.sh ${dai_home}/${dai_refresh_sh}
svn export --force ${dai_daimonin_repo}/trunk/project_tools/bash-scripts/dai_reboot.sh ${dai_home}/${dai_reboot_sh}

if [ -z "${dai_gridarta_repo}" ]
then
    svn export --force ${dai_daimonin_repo}/trunk/project_tools/bash-scripts/dai_recollect.sh ${dai_home}/${dai_recollect_sh}
fi

svn export --force ${dai_daimonin_repo}/trunk/project_tools/perl-scripts/tileset_updater.pl ${dai_home}/tileset_updater.pl

echo
echo "##################################"
echo "### Cleanup SVN working copies ###"
echo "##################################"

if [ -d ${dai_svndir} ]
then
    svn cleanup ${dai_svndir}/daimonin/trunk/arch
    svn cleanup ${dai_svndir}/daimonin/trunk/maps

    if [ ${dai_gameserver} != "trunk" ]
    then
        svn cleanup ${dai_svndir}/daimonin/${dai_gameserver}/arch
        svn cleanup ${dai_svndir}/daimonin/${dai_gameserver}/maps
    fi

    svn cleanup ${dai_svndir}/daimonin/${dai_gameserver}/server

    if [ -n "${dai_gridarta_repo}" ]
    then
        svn cleanup ${dai_svndir}/gridarta
    fi

    if [ -e ${dai_home}/stream ]
    then
        read stream < ${dai_home}/stream

        if [ ${stream} != "none" ]
        then
            rm -rf ${dai_svndir}/daimonin/stream/*
        fi
    fi

    if [ -n "${dai_use_newarch}" ]
    then
        svn cleanup ${dai_svndir}/daimonin/newarch/arch
        svn cleanup ${dai_svndir}/daimonin/newarch/maps
    fi

    if [ -n "${dai_daiserv_repo}" ]
    then
        svn cleanup ${dai_svndir}/daiserv/arch
        svn cleanup ${dai_svndir}/daiserv/maps
    fi

    if [ -n "${dai_mapserv_repo}" ]
    then
        svn cleanup ${dai_svndir}/mapserv/arch
        svn cleanup ${dai_svndir}/mapserv/maps
    fi
fi
