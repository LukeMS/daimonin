#!/bin/bash
# Start a gameserver.

########
# Set global variables.
source ./dai_setvars.sh

########
# Set some script variables.
# Server exit statuses (see server/src/include/global.h).
server_exit[0]="NORMAL"
server_exit[1]="RESTART"
server_exit[2]="SHUTDOWN"
server_exit[3]="ARTIFACT"
server_exit[4]="TREASURE"
server_exit[5]="FATAL"
server_exit[6]="FLOOD"
server_exit[7]="SIGSEGV"
server_exit[8]="SIGINT"
server_exit[9]="SIGQUIT"
server_exit[10]="SIGPIPE"
server_exit[11]="SIGBUS"
server_exit[12]="SIGTERM"
server_exit[13]="SIGHUP"
server_exit[255]="MANUAL_REBOOT"

# When the reboot started and when the server died.
fromtime=`date +%Y%m%d_%H%M%S%Z --utc`
totime="UNKNOWN"

# Default reason code for the reboot.
reason=255

########
# Parse the command line for options.
while [ $# -gt 0 ]
do
    case ${1} in
        "(-h|--help)")
            echo "Usage: ${dai_reboot_sh} [OPTION ...]"
            echo "Build and run the Daimonin server"
            echo ""
            echo "Options:"
            echo "  -h, --help		display this help and exit"
            echo "  -r, --reason=NUMBER	the reason why the server was rebooted"
            exit
            ;;
        "(-r|--reason)(=.+)")
            reason=`expr match ${1} '.*\([0-9]+\)$'`
            ;;
        *)
            echo "${dai_reboot_sh}: unrecognised option: '${1}'"
            echo "Try ${dai_reboot_sh} --help for more information."
            exit
            ;;
    esac
    shift
done

########
# Read values from ${dai_time_data} if it exists or use those given above if it
# doesn't then write them back to ${dai_time_data} (creating it if it does not
# exist).
if [ -f ${dai_home}/${dai_time_data} ]
then
    fromtime=`cut -d , -f 1 ${dai_home}/${dai_time_data}`
    totime=`cut -d , -f 2 ${dai_home}/${dai_time_data}`
    reason=`cut -d , -f 3 ${dai_home}/${dai_time_data}`
fi

echo "${fromtime},${totime},${reason}" > ${dai_home}/${dai_time_data}

########
# Check for pidfile, exiting if it exists or creating it if it doesn't.
#ps ax | grep '${dai_reboot_sh}' | grep -v grep > /dev/null
#
#if [ $? -eq 0 ]
#then
#    echo "Another instance of ${dai_reboot_sh} is running, which means that the server is either already up or currently rebuilding."
#    echo "You may kill it by running ${dai_refresh_sh} if you're sure this is what you want."
#    echo "Once that has finished, run ${dai_reboot_sh} again."
#    exit 1
#fi

########
# Infinite loop of archiving any existing logs, rebuilding the server, and
# running it.
(
    while true
    do
        echo
        echo "################################################################################"
        date
        echo "################################################################################"
        echo
        echo "####################"
        echo "### Archive logs ###"
        echo "####################"
        {
            if [ -f ${dai_home}/${dai_time_data} ]
            then
                fromtime=`cut -d , -f 1 ${dai_home}/${dai_time_data}`
                totime=`cut -d , -f 2 ${dai_home}/${dai_time_data}`
                reason=`cut -d , -f 3 ${dai_home}/${dai_time_data}`
                logfile="${dai_builddir}/server/data/log/${fromtime}"

                if [ -n "${dai_logsdir}" ]
                then
                    logfile="${dai_logsdir}/${fromtime}"
                fi

                tlogfile="${logfile}-tech.txt"
                clogfile="${logfile}-chat.txt"
                tar -czf ${logfile}-${totime}-${server_exit[${reason}]}.tar.gz ${tlogfile} ${clogfile}
                rm ${dai_home}/${dai_time_data} ${tlogfile} ${clogfile}
            fi
        }

        # If we had a shutdown exit now.
        if [ ${server_exit[${reason}]} == "SHUTDOWN" ]
        then
            exit ${reason}
        fi

        echo
        echo "################################################################################"
        echo
        echo "########################"
        echo "### Build gameserver ###"
        echo "########################"
        {
            if [ ! -d ${dai_builddir} ]
            then
                echo
                echo "### Make ${dai_builddir} structure."
                mkdir ${dai_builddir}
            fi

            if [ ! -d ${dai_svndir} ]
            then
                echo
                echo "### Make ${dai_svndir} structure."
                mkdir ${dai_svndir}
                mkdir ${dai_svndir}/daimonin
                mkdir ${dai_svndir}/daimonin/${dai_gameserver}

                if [ -n "${dai_gridarta_repo}" ]
                then
                    mkdir ${dai_svndir}/gridarta
                fi

                if [ -n "${dai_use_newarch}" ]
                then
                    mkdir ${dai_svndir}/daimonin/newarch
                fi

                if [ -n "${dai_daiserv_repo}" ]
                then
                    mkdir ${dai_svndir}/daiserv
                fi

                if [ -n "${dai_mapserv_repo}" ]
                then
                    mkdir ${dai_svndir}/mapserv
                fi
            fi

            echo
            echo "### Remove previous build."
            {
                rm -rf ${dai_builddir}/arch
                rm -rf ${dai_builddir}/maps
                rm -rf ${dai_builddir}/gridarta
                cd ${dai_builddir}/server/make/linux
                make distclean
            }

            echo
            echo "### Checkout/update and export server"

            if [ ${dai_gameserver} == "test" ]
            then
                echo "  # from ${dai_daimonin_repo}/main"
                svn co ${dai_daimonin_repo}/main/server ${dai_svndir}/daimonin/${dai_gameserver}/server
            else
                echo "  # from ${dai_daimonin_repo}/${dai_gameserver}"
                svn co ${dai_daimonin_repo}/${dai_gameserver}/server ${dai_svndir}/daimonin/${dai_gameserver}/server
            fi

            svn export --force ${dai_svndir}/daimonin/${dai_gameserver}/server ${dai_builddir}/server

            echo
            echo "### Checkout/update and export arches"
            if [ ${dai_gameserver} == "test" ]
            then
                echo "  # from ${dai_daimonin_repo}/main"
                svn co ${dai_daimonin_repo}/main/arch ${dai_svndir}/daimonin/${dai_gameserver}/arch
            else
                echo "  # from ${dai_daimonin_repo}/${dai_gameserver}"
                svn co ${dai_daimonin_repo}/${dai_gameserver}/arch ${dai_svndir}/daimonin/${dai_gameserver}/arch
            fi

            if [ -n "${dai_daiserv_repo}" ]
            then
                echo "  # from ${dai_daiserv_repo}"
                svn co ${dai_daiserv_repo}/arch ${dai_svndir}/daiserv/arch
            fi

            if [ -n "${dai_mapserv_repo}" ]
            then
                echo "  # from ${dai_mapserv_repo}"
                svn co ${dai_mapserv_repo}/arch ${dai_svndir}/mapserv/arch
            fi

            if [ -n "${dai_use_newarch}" ]
            then
                echo "  # from ${dai_daimonin_repo}/streams/newarch"
                svn co ${dai_daimonin_repo}/streams/newarch/arch ${dai_svndir}/daimonin/newarch/arch
            fi

            svn export --force ${dai_svndir}/daimonin/${dai_gameserver}/arch ${dai_builddir}/arch

            if [ -n "${dai_daiserv_repo}" ]
            then
                svn export --force ${dai_svndir}/daiserv/arch ${dai_builddir}/arch
            fi

            if [ -n "${dai_mapserv_repo}" ]
            then
                svn export --force ${dai_svndir}/mapserv/arch ${dai_builddir}/arch
            fi

            if [ -n "${dai_use_newarch}" ]
            then
                svn export --force ${dai_svndir}/daimonin/newarch/arch ${dai_builddir}/arch
            fi

            echo
            echo "### Checkout/update and export maps"
            if [ ${dai_gameserver} == "test" ]
            then
                echo "  # from ${dai_daimonin_repo}/main"
                svn co ${dai_daimonin_repo}/main/maps ${dai_svndir}/daimonin/${dai_gameserver}/maps
            else
                echo "  # from ${dai_daimonin_repo}/${dai_gameserver}"
                svn co ${dai_daimonin_repo}/${dai_gameserver}/maps ${dai_svndir}/daimonin/${dai_gameserver}/maps
            fi

            if [ -n "${dai_daiserv_repo}" ]
            then
                echo "  # from ${dai_daiserv_repo}"
                svn co ${dai_daiserv_repo}/maps ${dai_svndir}/daiserv/maps
            fi

            if [ -n "${dai_mapserv_repo}" ]
            then
                echo "  # from ${dai_mapserv_repo}"
                svn co ${dai_mapserv_repo}/maps ${dai_svndir}/mapserv/maps
            fi

            if [ -n "${dai_use_newarch}" ]
            then
                echo "  # from ${dai_daimonin_repo}/streams/newarch"
                svn co ${dai_daimonin_repo}/streams/newarch/maps ${dai_svndir}/daimonin/newarch/maps
            fi

            svn export --force ${dai_svndir}/daimonin/${dai_gameserver}/maps ${dai_builddir}/maps

            if [ -n "${dai_daiserv_repo}" ]
            then
                svn export --force ${dai_svndir}/daiserv/maps ${dai_builddir}/maps
            fi

            if [ -n "${dai_mapserv_repo}" ]
            then
                svn export --force ${dai_svndir}/mapserv/maps ${dai_builddir}/maps
            fi

            if [ -n "${dai_use_newarch}" ]
            then
                svn export --force ${dai_svndir}/daimonin/newarch/maps ${dai_builddir}/maps
            fi

            echo
            echo "### Run tileset_updater.pl"
            {
                ${dai_home}/tileset_updater.pl ${dai_builddir}/maps
            }

            if [ -n "${dai_gridarta_repo}" ]
            then
                echo
                echo "### Checkout/update, export, build, and run Gridarta to collect arches"
                echo "  # from ${dai_gridarta_repo}/${dai_gridarta_ttbs}"
                svn co ${dai_gridarta_repo}/${dai_gridarta_ttbs} ${dai_svndir}/gridarta
                svn export ${dai_svndir}/gridarta ${dai_builddir}/gridarta
                {
                    cd ${dai_builddir}/gridarta/daimonin
                    ant
                    mv DaimoninEditor.jar ..
                    cd ..
                    java -Xmx256M -jar DaimoninEditor.jar -c
                }
            fi

            echo
            echo "### Build server."
            {
                cd ${dai_builddir}/server/make/linux
                chmod +x configure
                ./configure
                make EXTRA_CFLAGS="${dai_cflags}" install
            }

            if [ -z "${dai_gridarta_repo}" ]
            then
                ${dai_home}/${dai_recollect_sh} ${dai_builddir}/arch ${dai_builddir}/server/lib
            fi
        }
        echo
        echo "################################################################################"
        echo
        echo "######################"
        echo "### Run gameserver ###"
        echo "######################"
        {
            fromtime=`date +%Y%m%d_%H%M%S%Z --utc`
            echo "${fromtime},UNKNOWN,255" > ${dai_home}/${dai_time_data}
            logfile="${dai_builddir}/server/data/log/${fromtime}"

            if [ -n "${dai_logsdir}" ]
            then
                logfile="${dai_logsdir}/${fromtime}"
            fi

            tlogfile="${logfile}-tech.txt"
            clogfile="${logfile}-chat.txt"
            cd ${dai_builddir}/server

            if [ -z "${dai_use_valgrind}" ]
            then
                ./${dai_server_exe} ${dai_server_options} -log ${tlogfile},${clogfile}
            else
                valgrind ./${dai_server_exe} ${dai_server_options} -log ,${clogfile} 2>${tlogfile}
            fi

            reason=$?
            echo
            echo "### Reboot reason: ${server_exit[${reason}]}."
            totime=`date +%Y%m%d_%H%M%S%Z --utc`
            echo "${fromtime},${totime},${reason}" > ${dai_home}/${dai_time_data}
        }
    done
) &
