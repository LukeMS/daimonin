## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I../../include \
	-I../../include/gui \
	-I../../3rdParty/tinyxml \
	-I../../3rdParty/zlib \
	-I../../3rdParty/fmod

AM_CXXFLAGS =\
	@SDL_CFLAGS@ \
	${OGRE_CFLAGS} \
	${OIS_CFLAGS} \
	-Wall \
	-Wno-deprecated\
	-g

bin_PROGRAMS = \
	client3d

client3d_SOURCES = \
	../../3rdParty/tinyxml/tinystr.cpp \
	../../3rdParty/tinyxml/tinyxml.cpp \
	../../3rdParty/tinyxml/tinyxmlerror.cpp \
	../../3rdParty/tinyxml/tinyxmlparser.cpp \
	../../3rdParty/zlib/adler32.c \
	../../3rdParty/zlib/crc32.c \
	../../3rdParty/zlib/inffast.c \
	../../3rdParty/zlib/inflate.c \
	../../3rdParty/zlib/inftrees.c \
	../../3rdParty/zlib/uncompr.c \
	../../3rdParty/zlib/zutil.c \
	../../source/gui/gui_animation.cpp \
	../../source/gui/gui_cursor.cpp \
	../../source/gui/gui_element.cpp \
	../../source/gui/gui_gadget_slot.cpp \
	../../source/gui/gui_gadget_combobox.cpp \
	../../source/gui/gui_gadget_button.cpp \
	../../source/gui/gui_gadget_scrollbar.cpp \
	../../source/gui/gui_graphic.cpp \
	../../source/gui/gui_imageset.cpp \
	../../source/gui/gui_table.cpp \
	../../source/gui/gui_listbox.cpp \
	../../source/gui/gui_manager.cpp \
	../../source/gui/gui_statusbar.cpp \
	../../source/gui/gui_textout.cpp \
	../../source/gui/gui_window.cpp \
	../../source/gui/gui_window_dialog.cpp \
	../../source/tile_chunk.cpp \
	../../source/tile_manager.cpp \
	../../source/tile_map.cpp \
	../../source/events.cpp \
	../../source/events_user.cpp \
	../../source/light_manager.cpp \
	../../source/logger.cpp \
	../../source/resourceloader.cpp \
	../../source/main.cpp \
	../../source/network.cpp \
	../../source/network_cmd.cpp \
	../../source/network_serverfile.cpp \
	../../source/object_equipment.cpp \
	../../source/object_animate.cpp \
	../../source/object_visuals.cpp \
	../../source/object_manager.cpp \
	../../source/object_npc.cpp \
	../../source/object_hero.cpp \
	../../source/object_missile.cpp \
	../../source/object_static.cpp \
	../../source/option.cpp \
	../../source/option_init.cpp \
	../../source/particle_manager.cpp \
	../../source/sound.cpp \
	../../source/spell_manager.cpp \
	../../source/spell_range.cpp \
	../../source/item.cpp 

client3d_LDFLAGS = \
	-L. \
	${OGRE_LIBS}

client3d_LDADD = \
	${FMOD_LIBS} \
	${OIS_LIBS} \
	@SDL_LIBS@

AUTOMAKE_OPTIONS = \
	subdir-objects

install-data-hook:
#	$(CP) ./make/linux/plugins.cfg  ./plugins.cfg
#	$(CP) ./make/linux/run_client3d ./run_client3d

all-local: 
	@echo ""
	@echo "Daimonin 3d client techdemo built. Use the following command to play: "
	@echo "  ./run_client3d"
