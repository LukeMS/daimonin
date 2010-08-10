#!/bin/bash
# Set and export variables needed by other scripts.

########
# Local directories

# The home directory under which everything else takes place. You need rw
# access here.
export dai_home="${HOME}"

# Checkouts of the various repos go here.
export dai_svndir="${dai_home}/svn"

# The server, arches, maps, and Gridarta go here.
export dai_builddir="${dai_home}/build"

# If not null, server logs go here. If null, they go in
# ${builddir}/server/data/log.
export dai_logsdir=""


########
# File/process names

# Probably you have no reason to change these names, but if you do, you can.
export dai_setvars_sh="dai_setvars.sh"
export dai_refresh_sh="dai_refresh.sh"
export dai_reboot_sh="dai_reboot.sh"
export dai_recollect_sh="dai_recollect.sh"
export dai_time_data="dai_time.data"
export dai_server_exe="daimonin_server"


########
# SVN repository URLs

# These are public repositories so anyone can access them. The scripts won't
# work without ${dai_daimonin_repo}. If ${dai_gridarta_repo} is not null,
# Gridarta will be downloaded, built, and run to collect the arches. If it is
# null, this will be done by the ${dai_recollect_sh} script. The main
# differences are size and complexity (Gridarta is a full featured map editor,
# though we only use it for this simple task from the command line here and
# requires you to install a few hundred MB of a Java SDK and it's dependencies,
# while ${dai_recollect_sh} is a purpose-written BASH scriptt of about 4KB) and
# speed (Gridarta does the job in seconds -- but you do need to fetch and build
# it first -- whereas ${dai_recollect_sh} takes a few minutes).
export dai_daimonin_repo="https://daimonin.svn.sourceforge.net/svnroot/daimonin"
export dai_gridarta_repo="https://gridarta.svn.sourceforge.net/svnroot/gridarta"

# If not null, these point to repositories containg maps and arches which are
# added to the public ones. If null, they are ignored.
export dai_daiserv_repo=""
export dai_mapserv_repo=""


########
# Miscellaneous

# The precise part of the Daimonin SVN you want to checkout and build. This
# determines the basic type of gameserver to build. It should be one of
# 'trunk', 'main', or 'test' (for historical reasons, the latest development
# area is called trunk). You probably won't need to change this often, if at
# all. If in doubt, leave it as this default.
export dai_gameserver="trunk"

# The precise part of the Gridarta SVN you want to checkout and build. You
# probably won't need to change this often. If in doubt, leave it as this
# default.
export dai_gridarta_ttbs="tags/0.8.1"

# If this is not null, the arches and maps from
# ${dai_daimonin_repo}/streams/newarch will also be added. If null, they won't
# be.
export dai_use_newarch=""

# If this is null ${dai_recollect_sh} will show a running collection count.
# This is useful on screen but not in logs, so set it to not null if the reboot
# is to be logged.
export dai_suppress_progress=""

# Any CFLAGS you want to pass to make. Foe example, use -DDAI_DEVELOPMENT_CODE
# and/or -DDAI_DEVELOPMENT_CONTENT to compile a development server (note that
# if ${dai_gameserver} == "trunk" both these flags will already be defined by
# default).
export dai_cflags=""

# If not null, the server will be run through valgrind, with the valgrind output
# being logged in the server techlog. If null, valgrind will not be used.
export dai_use_valgrind=""

# Any options you want to pass to the server. Do not include -log as this must
# be handled via the reboot script.
export dai_server_options=""
