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

    The author can be reached via e-mail to daimonin@nord-com.net
*/

#if !defined(__COMMANDS_H)
#define __COMMANDS_H

/* spell list commands for client spell list */
#define SPLIST_MODE_ADD    0
#define SPLIST_MODE_REMOVE 1
#define SPLIST_MODE_UPDATE 2

extern void SoundCmd ( unsigned char *data, int len );
extern void SetupCmd ( char *buf, int len );
extern void FaceCmd(unsigned char *data,  int len);
extern void Face1Cmd ( unsigned char *data, int len );
extern void AddMeFail ( char *data, int len );
extern void AddMeSuccess ( char *data, int len );
extern void GoodbyeCmd ( char *data, int len );
extern void AnimCmd ( unsigned char *data, int len );
extern void ImageCmd ( unsigned char *data, int len );
extern void DrawInfoCmd ( char *data, int len );
extern void StatsCmd ( unsigned char *data, int len );
extern void PreParseInfoStat(char *cmd);
extern void handle_query ( char *data, int len );
extern void send_reply ( char *text );
extern void PlayerCmd ( unsigned char *data, int len );
extern void Item1Cmd ( unsigned char *data, int len );
extern void UpdateItemCmd ( unsigned char *data, int len );

extern void DeleteItem ( unsigned char *data, int len );
extern void DeleteInventory ( unsigned char *data, int len );
extern void Map2Cmd ( unsigned char *data, int len );
extern void map_scrollCmd ( char *data, int len );
extern void MagicMapCmd ( unsigned char *data, int len );
extern void VersionCmd ( char *data, int len );

extern void SendVersion ( ClientSocket csock );
extern void SendAddMe ( ClientSocket csock );
extern void SendSetFaceMode ( ClientSocket csock, int mode );
extern void MapstatsCmd(unsigned char *data, int len);
extern void SpelllistCmd(unsigned char *data, int len);
extern void SkilllistCmd(unsigned char *data, int len);

extern void SkillRdyCmd(char *data, int len);
extern void GolemCmd(unsigned char *data, int len);
extern void ItemXCmd(unsigned char *data, int len);
extern void TargetObject(unsigned char *data, int len);
#endif
