SUBDIRS = include

INCLUDES = $(FMOD_CFLAGS) $(OGRE_CFLAGS) -I include
          
bin_PROGRAMS = daimonin3d
daimonin3d_SOURCES = \
	adler32.c   inffast.c    map.cpp          option_init.cpp  tile_gfx.cpp \
	client.cpp  inflate.c    network_cmd.cpp  player.cpp       uncompr.c \
	crc32.c     inftrees.c   network.cpp      serverfile.cpp   zutil.c \
	dialog.cpp  logfile.cpp  npc.cpp          sound.cpp \
	event.cpp   main.cpp     option.cpp       textwindow.cpp 

daimonin3d_LDADD = $(OGRE_LIBS) $(FMOD_LIBS)
