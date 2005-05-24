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
	animate.cpp \
	client.cpp \
	dialog.cpp \
	event.cpp \
	light_manager.cpp \
	logfile.cpp \
	main.cpp \
	network_cmd.cpp \
	network.cpp \
	object_manager.cpp \
	object_npc.cpp \
	object_static.cpp \
	option.cpp \
	option_init.cpp \
	particle.cpp \
	particle_manager.cpp \
	serverfile.cpp \
	sound.cpp \
	spell_manager.cpp \
	spell_range.cpp \
	textwindow.cpp \
	tile_gfx.cpp \
	tile_map.cpp

client3d_LDFLAGS = \
	-L../make/linux \
	${OGRE_LIBS}

client3d_LDADD = \
	 -lfmod-3.74

