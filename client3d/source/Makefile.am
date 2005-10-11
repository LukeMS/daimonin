## Process this file with automake to produce Makefile.in

INCLUDES =\
	 -Iinclude \
	 -I3rdParty/tinyxml \
	 -I3rdParty/zlib \
	 -I3rdParty/fmod

AM_CXXFLAGS =\
	${OGRE_CFLAGS}\
	 -Wall\
	 -g

bin_PROGRAMS = client3d

client3d_SOURCES = \
    animate.cpp \
	client.cpp \
	event.cpp \
	gui_cursor.cpp \
	gui_gadget.cpp \
	gui_graphic.cpp \
	gui_listbox.cpp \
	gui_manager.cpp \
	gui_textout.cpp \
	gui_window.cpp \
	light_manager.cpp \
	logger.cpp \
	logger_html.cpp \
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
	TileChunk.cpp \
	TileEnvironment.cpp \
	TileManager.cpp \
	3rdParty/tinyxml/tinystr.cpp \
	3rdParty/tinyxml/tinyxml.cpp \
	3rdParty/tinyxml/tinyxmlerror.cpp \
	3rdParty/tinyxml/tinyxmlparser.cpp \
	3rdParty/zlib/adler32.c \
	3rdParty/zlib/crc32.c \
	3rdParty/zlib/inffast.c \
	3rdParty/zlib/inflate.c \
	3rdParty/zlib/inftrees.c \
	3rdParty/zlib/uncompr.c \
	3rdParty/zlib/zutil.c

client3d_LDFLAGS = \
	-L../make/linux \
	${OGRE_LIBS}

client3d_LDADD = \
	 -lfmod-3.74

AUTOMAKE_OPTIONS = subdir-objects
