bindir = ../..
srcdir = ../../src

prgname = daimonin

SDL_LIBS = $(shell sdl-config --libs) -lSDL_image -lSDL_mixer

# Use one of this lines when you don't want sound support
# You must change in config.h the macro INSTALL_SOUND too
# Note, that the client will run fine on system without sound cards
# also with linked SDL_mixer - sound will disable automatically
# SDL_LIBS = $(shell sdl-config --libs) -lSDL_image

# Set this to your SDL include when not using sdl-config
# SDLINC = /usr/local/include
# SDLLIB = /usr/local/lib

SRCS =	$(srcdir)/client.c\
        $(srcdir)/inventory.c\
        $(srcdir)/map.c\
        $(srcdir)/player.c\
        $(srcdir)/sprite.c\
        $(srcdir)/commands.c\
        $(srcdir)/item.c\
        $(srcdir)/menu.c\
        $(srcdir)/textwin.c\
        $(srcdir)/event.c\
        $(srcdir)/misc.c\
        $(srcdir)/sound.c\
        $(srcdir)/wrapper.c\
        $(srcdir)/main.c\
        $(srcdir)/socket.c

ALLFILES = $(SRCS)

DEPEND = /usr/X11R6/bin/makedepend

SED = /bin/sed

OBJS = $(SRCS:.c=.o)

CFLAGS = -g -O2 $(EXTRA_CFLAGS)  -D__LINUX -ansi -Wall $(shell sdl-config --cflags)

INCLUDES= $(EXTRA_INCLUDES) -I$(srcdir)/include -I/usr/include 

LDFLAGS =

LIBS = -L/usr/lib -L/usr/X11R6/lib -lm -lX11 -lXext $(SDL_LIBS)

SHELL = /bin/sh

prefix = /usr
exec_prefix = ${prefix}

INSTALL = /usr/bin/install -c

MKDIR = /bin/mkdir
CC = gcc
RM = /bin/rm

$(prgname): $(OBJS) $(LOCAL_LIBRARIES)
	$(RM) -f $@
	$(CC) $(LDFLAGS) -o $(srcdir)/$@ $(OBJS) $(LOCAL_LIBRARIES) $(LIBS)

install:
	cp $(srcdir)/$(prgname) $(bindir)/$(prgname)

depend:
	$(DEPEND) $(DEPENDFLAGS) -- $(CFLAGS) $(INCLUDES) -I../../include -- $(srcdir)/$(SRCS)

clean::
	$(RM) -f $(srcdir)/$(prgname) $(bindir)/$(prgname) $(OBJS) Makefile.bak 

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $*.c

all:$(prgname)
