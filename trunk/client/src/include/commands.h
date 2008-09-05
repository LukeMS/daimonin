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

extern void DoClient(ClientSocket *csocket);

extern void SoundCmd(char *data, int len);
extern void SetupCmd(char *buf, int len);
extern void Face1Cmd(char *data, int len);
extern void AddMeFail(char *data, int len);
extern void AddMeSuccess(char *data, int len);
extern void ImageCmd(char *data, int len);
extern void DrawInfoCmd(char *data, int len);
extern void DrawInfoCmd2(char *data, int len);
extern void StatsCmd(char *data, int len);
extern void PreParseInfoStat(char *cmd);
extern void handle_query(char *data, int len);
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
extern void NewCharCmd(char *data, int len);

#ifdef USE_CHANNELS
extern void ChannelMsgCmd(char *data, int len);
#endif

#endif
