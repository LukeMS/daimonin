## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES =\
	 -Iinclude \
	 -Iinclude/fmod

AM_CXXFLAGS =\
	${OGRE_CFLAGS}\
	 -Wall\
	 -g

bin_PROGRAMS = client3d

client3d_SOURCES = \
	adler32.c\
	client.cpp\
	crc32.c\
	dialog.cpp\
	event.cpp\
	inffast.c\
	inflate.c\
	inftrees.c\
	logfile.cpp\
	main.cpp\
	network.cpp\
	network_cmd.cpp\
	option.cpp\
	option_init.cpp\
	serverfile.cpp\
	textwindow.cpp\
	uncompr.c\
	zutil.c\
	sound.cpp\
	player.cpp\
	tile_gfx.cpp\
	npc.cpp\
	tile_map.cpp

client3d_LDFLAGS = \
	-L../make/linux \
	${OGRE_LIBS}

client3d_LDADD = \
	 -lfmod-3.74

