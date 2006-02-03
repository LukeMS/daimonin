## Process this file with automake to produce Makefile.in

INCLUDES = \
	-Iinclude \
	-I3rdParty/tinyxml \
	-I3rdParty/zlib \
	-I3rdParty/fmod

AM_CXXFLAGS =\
	${OGRE_CFLAGS} \
	-Wall \
	-g

bin_PROGRAMS = \
	client3d

client3d_SOURCES = \
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
	3rdParty/zlib/zutil.c \
	TileChunk.cpp \
	TileInterface.cpp \
	TileManager.cpp \
	animate.cpp \
	events.cpp \
	events_user.cpp \
	gui_animation.cpp \
	gui_container.cpp \
	gui_cursor.cpp \
	gui_element.cpp \
	gui_gadget.cpp \
	gui_graphic.cpp \
	gui_imageset.cpp \
	gui_listbox.cpp \
	gui_manager.cpp \
	gui_statusbar.cpp \
	gui_textout.cpp \
	gui_window.cpp \
	light_manager.cpp \
	logger.cpp \
	main.cpp \
	network.cpp \
	network_cmd.cpp \
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
	spell_range.cpp

client3d_LDFLAGS = \
	-L../make/linux \
	${OGRE_LIBS}

client3d_LDADD = \
	-lfmodex64

AUTOMAKE_OPTIONS = \
	subdir-objects

install-data-hook:
	$(CP) ./../make/linux/plugins.cfg  ../plugins.cfg
	$(CP) ./../make/linux/run_client3d ../run_client3d
