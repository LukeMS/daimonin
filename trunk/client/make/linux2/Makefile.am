daimonindir = $(prefix)/client-@VERSION@
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
daimonin_LDADD = @SDL_LIBS@ -lSDL_mixer -lSDL_image
AM_CPPFLAGS = @SDL_CFLAGS@ 

install-data-hook:
	@if [ x$$UID = 'x0' -o x$$USER = 'xroot' ]; then \
		echo '***'; \
		echo '*** ROOT INSTALL NOT SUPPORTED YET! Please install as a normal user.'; \
		echo '***'; \
		exit 42; \
	fi;
	$(MKDIR) -p $(daimonindir)/sfx
	$(MKDIR) -p $(daimonindir)/media
	$(MKDIR) -p $(daimonindir)/cache
	$(MKDIR) -p $(daimonindir)/bitmaps
	$(MKDIR) -p $(daimonindir)/gfx_user
	$(MKDIR) -p $(daimonindir)/srv_files
	$(MKDIR) -p $(daimonindir)/icons
	$(CP) ../../sfx/*.* $(daimonindir)/sfx
	$(CP) ../../media/*.* $(daimonindir)/media
	$(CP) ../../cache/*.* $(daimonindir)/cache
	$(CP) ../../icons/*.* $(daimonindir)/icons
	$(CP) ../../srv_files/*.* $(daimonindir)/srv_files
	$(CP) ../../gfx_user/*.* $(daimonindir)/gfx_user
	$(CP) ../../bitmaps/*.* $(daimonindir)/bitmaps
	$(CP) ../../options.dat $(daimonindir)
	$(CP) ../../keys.dat $(daimonindir)
	$(CP) ../../daimonin.p0 $(daimonindir)
	$(CP) ../../License $(daimonindir)
	$(CP) ../../archdef.dat $(daimonindir)
	@echo "*** Daimonin client successful installed in $(daimonindir)!"
	@echo "*** Enter your install folder and type daimonin"
	@echo "*** to start the game!"
