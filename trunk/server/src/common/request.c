/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

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

/*
 * This file implements all of the goo on the server side for handling
 * clients.  It's got a bunch of global variables for keeping track of
 * each of the clients.
 *
 * Note:  All functions that are used to process data from the client
 * have the prototype of (char *data, int datalen, int client_num).  This
 * way, we can use one dispatch table.
 *
 */

#include <global.h>

/* This block is basically taken from socket.c - I assume if it works there,
 * it should work here.
 */
#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

static int  cs_stat_skillexp[]    =
{
    CS_STAT_SKILLEXP_AGILITY, CS_STAT_SKILLEXP_PERSONAL, CS_STAT_SKILLEXP_MENTAL, CS_STAT_SKILLEXP_PHYSIQUE,
    CS_STAT_SKILLEXP_MAGIC, CS_STAT_SKILLEXP_WISDOM
};





/******************************************************************************
 *
 * Start of commands the server sends to the client.
 *
 ******************************************************************************/

/*
 * send_query asks the client to query the user.  This way, the client knows
 * it needs to send something back (vs just printing out a message
 */
void send_query(NewSocket *ns, uint8 flags, char *text)
{
    char    buf[MAX_BUF];

    sprintf(buf, "%d %s", flags, text!= NULL ? text : "");
    Write_String_To_Socket(ns, BINARY_CMD_QUERY, buf, strlen(buf));
}

void esrv_update_skills(player *pl)
{
    object *tmp2;
    int     i;
    char    buf[256];
    char    tmp[2048]; /* we should careful set a big enough buffer here */

    sprintf(tmp, "%d ", SPLIST_MODE_UPDATE);

    for (i = 0; i < NROFSKILLS; i++)
    {
        /* update exp skill we have only */
        if (pl->skill_ptr[i] && pl->skill_ptr[i]->last_eat)
        {
            tmp2 = pl->skill_ptr[i];
            /* send only when really something has changed */
            if (tmp2->stats.exp != pl->skill_exp[i] || tmp2->level != pl->skill_level[i])
            {
                if (tmp2->last_eat == 1)
                    sprintf(buf, "/%s|%d|%d", tmp2->name, tmp2->level, tmp2->stats.exp);
                else if (tmp2->last_eat == 2)
                    sprintf(buf, "/%s|%d|-2", tmp2->name, tmp2->level);
                else
                    sprintf(buf, "/%s|%d|-1", tmp2->name, tmp2->level);
                strcat(tmp, buf);
                pl->skill_exp[i] = tmp2->stats.exp;
                pl->skill_level[i] = tmp2->level;
            }
        }
    }

    Write_String_To_Socket(&pl->socket, BINARY_CMD_SKILL_LIST, tmp, strlen(tmp));
}

/*
 * esrv_update_stats sends a statistics update.  We look at the old values,
 * and only send what has changed.  Stat mapping values are in newclient.h
 * Since this gets sent a lot, this is actually one of the few binary
 * commands for now.
 */
void esrv_update_stats(player *pl)
{
    int         i, group_update = 0; /* set to true when a group update stat has changed */
	sockbuf_struct * AddIf_SOCKBUF_PTR;
    uint16      flags;

	SOCKBUF_REQUEST_BUFFER(&pl->socket, 128);
	AddIf_SOCKBUF_PTR = ACTIVE_SOCKBUF(&pl->socket);

    /* small trick: we want send the hp bar of our target to the player.
     * We want send a char with x% the target has of full hp.
     * To avoid EVERY time the % calculation, we store the real HP
     * - if it has changed, we calc the % and use them normal.
     * this simple compare will not deal in speed but we safe
     * some unneeded calculations.
     */
    if (pl->target_object != pl->ob) /* never send our own status - client will sort this out */
    {
        /* we don't care about count - target function will readjust itself */
        if (pl->target_object && pl->target_object->stats.hp != pl->target_hp) /* just for secure...*/
        {
            /* well, i would like to avoid this calc here but we won't give the player the true hp value of target */
            char hp = (char) (((float) pl->target_object->stats.hp / (float) pl->target_object->stats.maxhp) * 100.0f);
            pl->target_hp = pl->target_object->stats.hp;
            AddIfChar(pl->target_hp_p, hp, CS_STAT_TARGET_HP);
        }
    }

    AddIfShort(pl->last_gen_hp, pl->gen_hp, CS_STAT_REG_HP);
    AddIfShort(pl->last_gen_sp, pl->gen_sp, CS_STAT_REG_MANA);
    AddIfShort(pl->last_gen_grace, pl->gen_grace, CS_STAT_REG_GRACE);
    AddIfCharFlag(pl->last_level, pl->ob->level, group_update, GROUP_UPDATE_LEVEL, CS_STAT_LEVEL);
    AddIfInt(pl->last_weight_limit, pl->weight_limit, CS_STAT_WEIGHT_LIM);
    AddIfInt(pl->last_weapon_sp, pl->weapon_sp, CS_STAT_WEAP_SP);
    AddIfInt(pl->last_speed_enc, pl->speed_enc, CS_STAT_SPEED);
    AddIfInt(pl->last_spell_fumble, pl->spell_fumble, CS_STAT_SPELL_FUMBLE);

    if (pl->ob != NULL)
    {
        /* these will update too when we are in a group */
        AddIfIntFlag(pl->last_stats.hp, pl->ob->stats.hp, group_update, GROUP_UPDATE_HP, CS_STAT_HP);
        AddIfIntFlag(pl->last_stats.maxhp, pl->ob->stats.maxhp, group_update, GROUP_UPDATE_MAXHP, CS_STAT_MAXHP);
        AddIfShortFlag(pl->last_stats.sp, pl->ob->stats.sp, group_update, GROUP_UPDATE_SP, CS_STAT_SP);
        AddIfShortFlag(pl->last_stats.maxsp, pl->ob->stats.maxsp, group_update, GROUP_UPDATE_MAXSP, CS_STAT_MAXSP);
        AddIfShortFlag(pl->last_stats.grace, pl->ob->stats.grace, group_update, GROUP_UPDATE_GRACE, CS_STAT_GRACE);
        AddIfShortFlag(pl->last_stats.maxgrace, pl->ob->stats.maxgrace, group_update, GROUP_UPDATE_MAXGRACE,CS_STAT_MAXGRACE);

        AddIfChar(pl->last_stats.Str, pl->ob->stats.Str, CS_STAT_STR);
        AddIfChar(pl->last_stats.Int, pl->ob->stats.Int, CS_STAT_INT);
        AddIfChar(pl->last_stats.Pow, pl->ob->stats.Pow, CS_STAT_POW);
        AddIfChar(pl->last_stats.Wis, pl->ob->stats.Wis, CS_STAT_WIS);
        AddIfChar(pl->last_stats.Dex, pl->ob->stats.Dex, CS_STAT_DEX);
        AddIfChar(pl->last_stats.Con, pl->ob->stats.Con, CS_STAT_CON);
        AddIfChar(pl->last_stats.Cha, pl->ob->stats.Cha, CS_STAT_CHA);

        AddIfInt(pl->last_stats.exp, pl->ob->stats.exp, CS_STAT_EXP);
        AddIfShort(pl->last_stats.wc, pl->ob->stats.wc, CS_STAT_WC);
        AddIfShort(pl->last_stats.ac, pl->ob->stats.ac, CS_STAT_AC);
        AddIfShort(pl->last_dps, pl->dps, CS_STAT_DAM);
        AddIfShort(pl->last_food_status, pl->food_status, CS_STAT_FOOD);

		AddIfShort(pl->dist_last_wc, pl->dist_wc, CS_STAT_DIST_WC);
		AddIfShort(pl->dist_last_dps, pl->dist_dps, CS_STAT_DIST_DPS);
		AddIfInt(pl->dist_last_action_time, pl->dist_action_time, CS_STAT_DIST_TIME);
    }

    for (i = 0; i < NROFSKILLGROUPS_ACTIVE; i++)
    {
        AddIfInt(pl->last_exp_obj_exp[i], pl->exp_obj_ptr[i]->stats.exp, cs_stat_skillexp[i]);
        AddIfChar(pl->last_exp_obj_level[i], pl->exp_obj_ptr[i]->level, cs_stat_skillexp[i]+1);
    }

    flags = 0;
    if (pl->run_on)
        flags |= SF_RUNON;
    /* we add additional player status flags - in old style, you got a msg
     * in the text windows when you get xray of get blineded - we will skip
     * this and add the info here, so the client can track it down and make
     * it the user visible in it own, server indepentend way.
     */

    if (QUERY_FLAG(pl->ob, FLAG_BLIND)) /* player is blind */
        flags |= SF_BLIND;
    if (QUERY_FLAG(pl->ob, FLAG_XRAYS)) /* player has xray */
        flags |= SF_XRAYS;
    if (QUERY_FLAG(pl->ob, FLAG_SEE_IN_DARK)) /* player has infravision */
        flags |= SF_INFRAVISION;
    AddIfShort(pl->last_flags, flags, CS_STAT_FLAGS);

    /* TODO: Add a fix_player marker here for all values who MUST be altered in fix_player */
    for (i = 0; i < NROFATTACKS; i++)
        AddIfChar(pl->last_resist[i], pl->ob->resist[i], CS_STAT_RES_START+i);

    if (pl->socket.ext_title_flag)
    {
        generate_ext_title(pl);
        SockBuf_AddChar( AddIf_SOCKBUF_PTR , CS_STAT_EXT_TITLE);
        i = (int) strlen(pl->ext_title);
        SockBuf_AddChar( AddIf_SOCKBUF_PTR , i+1);
		SockBuf_AddString( AddIf_SOCKBUF_PTR, pl->ext_title, i);
        pl->socket.ext_title_flag = 0;
    }
    /* Only send it away if we have some actual data */
    if (SOCKBUF_REQUEST_BUFSIZE( AddIf_SOCKBUF_PTR ))
		SOCKBUF_REQUEST_FINISH(&pl->socket, BINARY_CMD_STATS, SOCKBUF_DYNAMIC);
	else
		SOCKBUF_REQUEST_RESET(&pl->socket);

    if(group_update && pl->group_status & GROUP_STATUS_GROUP && pl->update_ticker != ROUND_TAG)
        party_client_group_update(pl->ob, group_update);
    pl->update_ticker = ROUND_TAG;
}


void esrv_new_player(player *pl, uint32 weight)
{
    int len;
	sockbuf_struct *sptr;

	SOCKBUF_REQUEST_BUFFER(&pl->socket, 128);
	sptr = ACTIVE_SOCKBUF(&pl->socket);

	SockBuf_AddInt(sptr, pl->ob->count);
    SockBuf_AddInt(sptr, weight);
    SockBuf_AddInt(sptr, pl->ob->face->number);
    SockBuf_AddChar(sptr,(len=strlen(pl->ob->name)));
	SockBuf_AddString(sptr, pl->ob->name, len);

	SOCKBUF_REQUEST_FINISH(&pl->socket, BINARY_CMD_PLAYER, SOCKBUF_DYNAMIC);
}
