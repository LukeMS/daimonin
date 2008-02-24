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

    The author can be reached via e-mail to info@daimonin.net
*/

#if !defined(__COMMANDS_H)
#define __COMMANDS_H

typedef enum client_cmd {
    CLIENT_CMD_GENERIC,
    CLIENT_CMD_STOP,

    CLIENT_CMD_MAX_NROF
} _client_cmd;

#define DATA_PACKED_CMD 0x80
enum
{
    DATA_CMD_NO,
    DATA_CMD_SKILL_LIST,
    DATA_CMD_SPELL_LIST,
    DATA_CMD_SETTINGS_LIST,
    DATA_CMD_ANIM_LIST,
    DATA_CMD_BMAP_LIST
};

/* spell list commands for client spell list */
#define SPLIST_MODE_ADD    0
#define SPLIST_MODE_REMOVE 1
#define SPLIST_MODE_UPDATE 2

extern void SoundCmd(unsigned char *data, int len);
extern void SetupCmd(char *buf, int len);
extern void FaceCmd(unsigned char *data, int len);
extern void Face1Cmd(unsigned char *data, int len);
extern void AddMeFail(char *data, int len);
extern void AddMeSuccess(char *data, int len);
extern void GoodbyeCmd(char *data, int len);
extern void NewAnimCmd(unsigned char *data, int len);
extern void ImageCmd(unsigned char *data, int len);
extern void DrawInfoCmd(char *data, int len);
extern void DrawInfoCmd2(char *data, int len);
extern void StatsCmd(unsigned char *data, int len);
extern void PreParseInfoStat(char *cmd);
extern void handle_query(char *data, int len);
extern void send_reply(char *text);
extern void PlayerCmd(unsigned char *data, int len);
extern void Item1Cmd(unsigned char *data, int len);
extern void UpdateItemCmd(unsigned char *data, int len);

extern void DeleteItem(unsigned char *data, int len);
extern void DeleteInventory(unsigned char *data, int len);
extern void Map2Cmd(unsigned char *data, int len);
extern void MagicMapCmd(unsigned char *data, int len);
extern void VersionCmd(char *data, int len);

extern void SendVersion(ClientSocket csock);
extern void SendAddMe(ClientSocket csock);
extern void RequestFile(ClientSocket csock, int index);
extern void SpelllistCmd(unsigned char *data, int len);
extern void SkilllistCmd(unsigned char *data, int len);

extern void SkillRdyCmd(unsigned char *data, int len);
extern void GolemCmd(unsigned char *data, int len);
extern void ItemXYCmd(unsigned char *data, int len, int bflag);
extern void ItemXCmd(unsigned char *data, int len);
extern void ItemYCmd(unsigned char *data, int len);
extern void GroupCmd(unsigned char *data, int len);
extern void GroupInviteCmd(unsigned char *data, int len);
extern void GroupUpdateCmd(unsigned char *data, int len);
extern void MarkCmd(unsigned char *data, int len);
extern void BookCmd(unsigned char *data, int len);
extern void InterfaceCmd(unsigned char *data, int len);
extern void TargetObject(unsigned char *data, int len);
extern void DataCmd(unsigned char *data, int len);
extern void NewCharCmd(char *data, int len);

#ifdef USE_CHANNELS
extern void ChannelMsgCmd(unsigned char *data, int len);
void break_string(char *text, char *prefix, Boolean one_prefix, char *result);
#endif

#endif
