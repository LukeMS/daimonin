daimonindir=$(d_bindir)
daimonin_PROGRAMS = daimonin daimonin-updater

daimonin_SOURCES =  \
	../../src/adler32.c \
	../../src/anim.c \
	../../src/book.c \
	../../src/buddy.c \
	../../src/chatfilter.c \
	../../src/client.c \
	../../src/commands.c \
	../../src/crc32.c \
	../../src/dialog.c \
	../../src/event.c \
	../../src/filewrap.c \
	../../src/group.c \
	../../src/ignore.c \
	../../src/inffast.c \
	../../src/inflate.c \
	../../src/inftrees.c \
	../../src/interface.c \
	../../src/inventory.c \
	../../src/item.c \
	../../src/kerbholz.c \
	../../src/main.c \
	../../src/map.c \
	../../src/menu.c \
	../../src/misc.c \
	../../src/physfsrwops.c \
	../../src/player.c \
	../../src/socket.c \
	../../src/sound.c \
	../../src/sprite.c \
	../../src/textwin.c \
	../../src/uncompr.c \
	../../src/wrapper.c \
	../../src/widget.c \
	../../src/zutil.c
daimonin_LDADD = -lSDL_mixer -lSDL_image @SDL_LIBS@ @EXTRA_LIBS@ @PHYSFS_LIBS@
daimonin_OBJDIR = objs

daimonin_updater_SOURCES = \
	../../src/adler32.c \
	../../src/crc32.c \
	../../src/inffast.c \
	../../src/inflate.c \
	../../src/inftrees.c \
	../../src/uncompr.c \
	../../src/zutil.c \
	../../src/updater/linux.c \
	../../src/updater/main.c \
	../../src/updater/xdelta3.c \
	../../src/updater/physfsunpack.c \
	../../src/updater/md5.c

daimonin_updater_LDADD = @PHYSFS_LIBS@ @CURL_LIBS@
daimonin_updater_OBJDIR = updater_objs

# NOTE: if building xdelta, run its configure with "--disable-shared"

AUTOMAKE_OPTIONS = subdir-objects
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
	$(MKDIR) -p $(d_datadir)/logs
	$(MKDIR) -p $(d_datadir)/gfx_user
	$(MKDIR) -p $(d_datadir)/srv_files
	$(MKDIR) -p $(d_datadir)/skins
	$(MKDIR) -p $(d_datadir)/settings
	$(MKDIR) -p $(d_datadir)/update
	$(CP) ../../sfx/*.* $(d_datadir)/sfx
	$(CP) ../../media/*.* $(d_datadir)/media
	$(CP) ../../cache/*.* $(d_datadir)/cache
	$(CP) -R ../../skins/* $(d_datadir)/skins
	$(CP) -R ../../settings/*.* $(d_datadir)/settings
	$(CP) ../../srv_files/*.* $(d_datadir)/srv_files
	$(CP) ../../update/version $(d_datadir)/update
	$(CP) ../../gfx_user/*.* $(d_datadir)/gfx_user
	$(CP) ../../daimonin.p0 $(d_datadir)
	$(CP) ../../License $(d_datadir) || $(CP) ../../../License $(d_datadir)
	$(CP) ../../archdef.dat $(d_datadir)
	@echo "*** Daimonin client successful installed in $(daimonindir)!"
	@echo "*** Enter your install folder and type ./daimonin"
	@echo "*** to start the game!"
