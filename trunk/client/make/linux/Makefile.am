daimonindir=$(d_bindir)
daimonin_PROGRAMS = daimonin daimonin-updater

daimonin_SOURCES =  \
	../../src/adler32.c \
	../../src/anim.c \
	../../src/book.c \
	../../src/buddy.c \
	../../src/chatfilter.c \
	../../src/client_cmd.c \
	../../src/crc32.c \
	../../src/dialog.c \
	../../src/event.c \
	../../src/face.c \
	../../src/filewrap.c \
	../../src/font.c \
	../../src/gameserver.c \
	../../src/group.c \
	../../src/ignore.c \
	../../src/inffast.c \
	../../src/inflate.c \
	../../src/inftrees.c \
	../../src/interface.c \
	../../src/inventory.c \
	../../src/item.c \
	../../src/kerbholz.c \
	../../src/locator.c \
	../../src/main.c \
	../../src/map.c \
	../../src/menu.c \
	../../src/misc.c \
	../../src/physfsrwops.c \
	../../src/player.c \
	../../src/protocol.c \
	../../src/server_cmd.c \
	../../src/skin.c \
	../../src/socket.c \
	../../src/sound.c \
	../../src/sprite.c \
	../../src/srvfile.c \
	../../src/strout.c \
	../../src/textwin.c \
	../../src/tile_stretcher.c \
	../../src/uncompr.c \
	../../src/wdh.c \
	../../src/widget.c \
	../../src/wrapper.c \
	../../src/zutil.c \
	../../src/3rdparty/sprig/SPG_extended.c \
	../../src/3rdparty/sprig/SPG_misc.c \
	../../src/3rdparty/sprig/SPG_polygon.c \
	../../src/3rdparty/sprig/SPG_primitives.c \
	../../src/3rdparty/sprig/SPG_rotation.c \
	../../src/3rdparty/sprig/SPG_surface.c
daimonin_LDADD = -lSDL_mixer -lSDL_image @SDL_LIBS@ @EXTRA_LIBS@ @PHYSFS_LIBS@ @CURL_LIBS@
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
AM_CFLAGS = @SDL_CFLAGS@ -Wextra -funsigned-char -DDAI_DEVELOPMENT
AM_CPPFLAGS = -I../../src/3rdparty/sprig
AM_LDFLAGS = @LDFLAGS@

install-data-hook:
	sh ../../install/linux/install.sh $(d_datadir) $(d_bindir)
