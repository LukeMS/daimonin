SUBDIRS = include

INCLUDES = $(FMOD_CFLAGS) $(OGRE_CFLAGS) -I include
          
bin_PROGRAMS = daimonin3d
daimonin3d_SOURCES = \
	adler32.c   dialog.cpp  inftrees.c       network.cpp      uncompr.c \
	client.cpp  logfile.cpp      option.cpp       xyz.cpp \
	crc32.c     inffast.c   main.cpp         option_init.cpp  zutil.c \
	inflate.c   network_cmd.cpp  textwindow.cpp

daimonin3d_LDADD = $(OGRE_LIBS) $(FMOD_LIBS)
