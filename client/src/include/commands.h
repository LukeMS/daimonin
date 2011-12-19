/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to info@daimonin.org
*/

#ifndef __COMMANDS_H
#define __COMMANDS_H

/* TODO: In 0.11.0 the server will send 24bit colour info so the NDI_* defines
 * should be moved to protocol.h then and NDI_MASK_* will not be needed.
 * Currently it sends both in one uint16, the low bit being colour and the high
 * bit being flags (meaning we only have space for 8 flags [UNIQUE and ALL are
 * unused client-side] and 256 colours. */
#define NDI_MASK_COLRS 0xFF
#define NDI_MASK_FLAGS 0xFFFF

/* These 17 named colours (the CSS 2 standards) are just for convenience
 * really. Any of the 255^3 0xRRGGBB combinations can be used. */
#define NDI_COLR_AQUA    0x00ffff
#define NDI_COLR_BLACK   0x000000
#define NDI_COLR_BLUE    0x0000ff
#define NDI_COLR_FUSCHIA 0xff00ff
#define NDI_COLR_GREY    0x808080
#define NDI_COLR_GREEN   0x008000
#define NDI_COLR_LIME    0x00ff00
#define NDI_COLR_MAROON  0x800000
#define NDI_COLR_NAVY    0x000080
#define NDI_COLR_OLIVE   0x808000
#define NDI_COLR_ORANGE  0xffa500
#define NDI_COLR_PURPLE  0x800080
#define NDI_COLR_RED     0xff0000
#define NDI_COLR_SILVER  0xc0c0c0
#define NDI_COLR_TEAL    0x008080
#define NDI_COLR_WHITE   0xffffff
#define NDI_COLR_YELLOW  0xffff00

#define NDI_FLAG_SAY       (1 << 8)
#define NDI_FLAG_SHOUT     (1 << 9)
#define NDI_FLAG_TELL      (1 << 10)
#define NDI_FLAG_GSAY      (1 << 11)
#define NDI_FLAG_EMOTE     (1 << 12)
#define NDI_FLAG_ADMIN     (1 << 13) // from VOL/GM/SA
#define NDI_FLAG_PLAYER    (1 << 14) // from a player
#define NDI_FLAG_VIM       (1 << 15) // Very Important Message (eg, quest completion)
#define NDI_FLAG_UNIQUE    (1 << 16) // print immediately, don't buffer
#define NDI_FLAG_ALL       (1 << 17) // to all players
#define NDI_FLAG_EAVESDROP (1 << 18) // GM/SA listening in
#define NDI_FLAG_BUDDY     (1 << 19) // from buddy

extern void DoClient(void);

extern void PingCmd(char *data, int len);
extern void SoundCmd(char *data, int len);
extern void SetupCmd(char *buf, int len);
extern void Face1Cmd(char *data, int len);
extern void AddMeFail(char *data, int len);
extern void AccNameSuccess(char *data, int len);
extern void ImageCmd(char *data, int len);
extern void DrawInfoCmd(char *data, int len);
extern void DrawInfoCmd2(char *data, int len);
extern void StatsCmd(char *data, int len);
extern void PlayerCmd(char *data, int len);
extern void Item1Cmd(char *data, int len);
extern void UpdateItemCmd(char *data, int len);
extern void DeleteItem(char *data, int len);
extern void DeleteInventory(char *data, int len);
extern void Map2Cmd(char *data, int len);
extern void SpelllistCmd(char *data, int len);
extern void SkilllistCmd(char *data, int len);
extern void SkillRdyCmd(char *data, int len);
extern void GolemCmd(char *data, int len);
extern void ItemXYCmd(char *data, int len, int bflag);
extern void ItemXCmd(char *data, int len);
extern void ItemYCmd(char *data, int len);
extern void GroupCmd(char *data, int len);
extern void GroupInviteCmd(char *data, int len);
extern void GroupUpdateCmd(char *data, int len);
extern void MarkCmd(char *data, int len);
extern void BookCmd(char *data, int len);
extern void InterfaceCmd(char *data, int len);
extern void TargetObject(char *data, int len);
extern void DataCmd(char *data, int len);
extern void AccountCmd(char *data, int len);

#ifdef USE_CHANNELS
extern void ChannelMsgCmd(char *data, int len);
#endif

#endif /* ifndef __COMMANDS_H */
