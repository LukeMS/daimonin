daimonindir=$(d_bindir)
daimonin_PROGRAMS = daimonin
daimonin_SOURCES =  \
	../../src/adler32.c \
	../../src/book.c \
	../../src/client.c \
	../../src/commands.c \
	../../src/crc32.c \
	../../src/dialog.c \
	../../src/event.c \
	../../src/filewrap.c \
	../../src/group.c \
	../../src/inffast.c \
	../../src/inflate.c \
	../../src/inftrees.c \
	../../src/interface.c \
	../../src/inventory.c \
	../../src/item.c \
	../../src/main.c \
	../../src/map.c \
	../../src/menu.c \
	../../src/misc.c \
	../../src/player.c \
	../../src/socket.c \
	../../src/sound.c \
	../../src/sprite.c \
	../../src/textwin.c \
	../../src/uncompr.c \
	../../src/wrapper.c \
	../../src/zutil.c

AUTOMAKE_OPTIONS = subdir-objects
daimonin_OBJDIR = objs
daimonin_LDADD = -lSDL_mixer -lSDL_image @SDL_LIBS@ @EXTRA_LIBS@
AM_CFLAGS = @SDL_CFLAGS@ -Wall
AM_LDFLAGS = @LDFLAGS@ 

install-data-hook:
#	@if [ x$$UID = 'x0' -o x$$USER = 'xroot' ]; then \
#		echo '***'; \
#		echo '*** ROOT INSTALL NOT SUPPORTED YET! Please install as a normal user.'; \
#		echo '***'; \
#		exit 42; \
#	fi;
	$(MKDIR) -p $(d_datadir)/sfx
	$(MKDIR) -p $(d_datadir)/media
	$(MKDIR) -p $(d_datadir)/cache
	$(MKDIR) -p $(d_datadir)/bitmaps
	$(MKDIR) -p $(d_datadir)/gfx_user
	$(MKDIR) -p $(d_datadir)/srv_files
	$(MKDIR) -p $(d_datadir)/icons
	$(CP) ../../sfx/*.* $(d_datadir)/sfx
	$(CP) ../../media/*.* $(d_datadir)/media
	$(CP) ../../cache/*.* $(d_datadir)/cache
	$(CP) ../../icons/*.* $(d_datadir)/icons
	$(CP) ../../srv_files/*.* $(d_datadir)/srv_files
	$(CP) ../../gfx_user/*.* $(d_datadir)/gfx_user
	$(CP) ../../bitmaps/*.* $(d_datadir)/bitmaps
	$(CP) ../../options.dat $(d_datadir)
	$(CP) ../../keys.dat $(d_datadir)
	$(CP) ../../daimonin.p0 $(d_datadir)
	$(CP) ../../License $(d_datadir)
	$(CP) ../../archdef.dat $(d_datadir)
	@echo "*** Daimonin client successful installed in $(daimonindir)!"
	@echo "*** Enter your install folder and type ./daimonin"
	@echo "*** to start the game!"
