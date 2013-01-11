daimonindir=$(d_bindir)
daimonin_PROGRAMS = daimonin daimonin-updater

daimonin_SOURCES =  \
	../../src/anim.c \
	../../src/book.c \
	../../src/buddy.c \
	../../src/chatfilter.c \
	../../src/client_cmd.c \
	../../src/dialog.c \
	../../src/event.c \
	../../src/face.c \
	../../src/font.c \
	../../src/gameserver.c \
	../../src/group.c \
	../../src/ignore.c \
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
	../../src/wdh.c \
	../../src/widget.c \
	../../src/wrapper.c \
	../../src/3rdparty/sprig/SPG_extended.c \
	../../src/3rdparty/sprig/SPG_misc.c \
	../../src/3rdparty/sprig/SPG_polygon.c \
	../../src/3rdparty/sprig/SPG_primitives.c \
	../../src/3rdparty/sprig/SPG_rotation.c \
	../../src/3rdparty/sprig/SPG_surface.c \
	../../src/3rdparty/zlib/adler32.c \
	../../src/3rdparty/zlib/crc32.c \
	../../src/3rdparty/zlib/inffast.c \
	../../src/3rdparty/zlib/inflate.c \
	../../src/3rdparty/zlib/inftrees.c \
	../../src/3rdparty/zlib/uncompr.c \
	../../src/3rdparty/zlib/zutil.c
daimonin_LDADD = -lSDL_mixer -lSDL_image @SDL_LIBS@ @EXTRA_LIBS@ @PHYSFS_LIBS@ @CURL_LIBS@ -lm
daimonin_OBJDIR = objs

daimonin_updater_SOURCES = \
	../../src/updater/linux.c \
	../../src/updater/main.c \
	../../src/updater/xdelta3.c \
	../../src/updater/physfsunpack.c \
	../../src/updater/md5.c

daimonin_updater_LDADD = @PHYSFS_LIBS@ @CURL_LIBS@ -lm
daimonin_updater_OBJDIR = updater_objs

# NOTE: if building xdelta, run its configure with "--disable-shared"

AUTOMAKE_OPTIONS = subdir-objects
AM_CFLAGS = @SDL_CFLAGS@ -Wextra -funsigned-char -DDAI_DEVELOPMENT
AM_CPPFLAGS = -I../../src/3rdparty/sprig -I../../src/3rdparty/zlib
AM_LDFLAGS = @LDFLAGS@

# ./configure adds CFLAGS = "-g -O2" if the compiler is GCC (which it probably
# is). Unfortuunately it seems that some(?) current Linux installations (ie my
# Ubuntu 12.04 64bit and my Ubuntu 11.10 32bit) are broken *somewhere* so that
# compiling the client with -O2 will crash at runtime every time (although it
# compiles fine and the code is good). Other users of recent Ubuntus (although
# I cannot definitely say it is a Ubuntu-only issue) have similar crashes. But
# still others have no problem. See
# http://www.daimonin.org/forums/daimonin-project/trunk-client-r7147-broken/
# Removing -O2 'solves' the problem in that it still crashes (for me 30-90% of
# the time) but not 100% of the time.
CFLAGS =

install-data-hook:
	sh ../../install/linux/install.sh $(d_datadir) $(d_bindir)
