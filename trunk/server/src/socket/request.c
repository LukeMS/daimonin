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

    The author can be reached via e-mail to info@daimonin.org
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

/* send_target_command() updates the client with the current target data. The
 * function does not change or verify this data, just sends it. So it is the
 * caller's responsibility to do any necessary checks first. */
/* TODO: Split this to SERVER_CMD_COMBAT (just the first byte, in fact a bit
 * will do) and SERVER_CMD_TARGET (everything else). Requires Y update.
 *
 * -- Smacky 20160929 */
void send_target_command(player_t *pl)
{
    char buf[MEDIUM_BUF];

    /* First char is the combat mode (0 or 1). */
    buf[0] = (char)pl->combat_mode;

    /* Second char is the target colour. */
    buf[1] = (char)pl->target_colr;

    /* Third char is the target mode (LOS_TARGET_SELF, LOS_TARGET_ENEMY, or
     * LOS_TARGET_FRIEND). */
    buf[2] = (char)pl->target_mode;

    /* The rest is a string consisting of the target's name. And SA/MMs also
     * the actual level. */
    sprintf(buf + 3, "%s", STRING_OBJ_NAME(pl->target_ob));

#ifdef DAI_DEVELOPMENT_CONTENT
    if ((pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_MM | GMASTER_MODE_MW)))
#else
    if ((pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_MM)))
#endif
    {
        sprintf(strchr(buf + 3, '\0'), " (lvl %d)", pl->target_level);
    }

    Write_String_To_Socket(&pl->socket, SERVER_CMD_TARGET, buf, strlen(buf + 3) + 3);
}

void send_spelllist_cmd(object_t *op, char *spellname, int mode)
{
    player_t *pl;
    char    tmp[1024 * 10]; /* we should careful set a big enough buffer here */

    /* Sanity check. */
    if (!(pl = CONTR(op)))
    {
        return;
    }

    sprintf(tmp, "%d ", mode);
    if (spellname) /* send single name */
    {
        strcat(tmp, "/");
        strcat(tmp, spellname);
    }
    else
    {
        int i, n = ((pl->gmaster_mode & GMASTER_MODE_SA)) ? NROFREALSPELLS : pl->nrofknownspells;

        for (i = 0; i < n; i++)
        {
            int spnum = ((pl->gmaster_mode & GMASTER_MODE_SA)) ? i : pl->known_spells[i];

            sprintf(strchr(tmp, '\0'), "/%s", spells[spnum].name);
        }
    }
    Write_String_To_Socket(&pl->socket, SERVER_CMD_SPELL_LIST, tmp, strlen(tmp));
}

static char *MakeListString(object_t *skill)
{
    static char buf[SMALL_BUF];
    int         exp;

    if (skill->last_eat == INDIRECT)
    {
        exp = skill->stats.exp;
    }
    else if (skill->last_eat == DIRECT)
    {
        exp = -2;
    }
    else // if (skill->last_eat == NONLEVELING)
    {
        exp = -1;
    }

    sprintf(buf, "/%s|%d|%d", skill->name, skill->level, exp);

    return buf;
}

void send_skilllist_cmd(player_t *pl, int snr, int mode)
{
    int  i = MAX(0, MIN(snr, NROFSKILLS - 1));
    char buf[LARGE_BUF] = "";

    while (i < NROFSKILLS)
    {
        object_t *skill;

        switch (mode)
        {
            case SPLIST_MODE_ADD:
            case SPLIST_MODE_UPDATE:
            skill = pl->skill_ptr[i];
            break;

            case SPLIST_MODE_REMOVE:
            skill = &skills[i]->clone;
            break;

            default:
            LOG(llevBug, "BUG:: send_skilllist_cmd(): Unknown mode (%d)!\n",
                mode);
            return;
        }

        if (skill &&
            (mode != SPLIST_MODE_UPDATE ||
             skill->stats.exp != pl->skill_exp[i] ||
             skill->level != pl->skill_level[i]))
        {
            /* Got one so start the update string. */
            if (buf[0] == '\0')
            {
                sprintf(buf, "%d ", mode);
            }

            sprintf(strchr(buf, '\0'), "%s", MakeListString(skill));
            pl->skill_exp[i] = skill->stats.exp;
            pl->skill_level[i] = skill->level;
        }

        if (snr >= 0 &&
            snr < NROFSKILLS)
        {
            i = NROFSKILLS;
        }
        else
        {
            i++;
        }
    }

    if (buf[0] != '\0')
    {
        Write_String_To_Socket(&pl->socket, SERVER_CMD_SKILL_LIST, buf,
            strlen(buf));
    }
}

/* all this functions are not really bulletproof. filling tmp[] can be easily produce
 * a stack overflow. Doing here some more intelligent is needed. I do this here
 * with sprintf() only for fast beta implementation */

void send_ready_skill(player_t *pl, shstr_t *name)
{
    Write_String_To_Socket(&pl->socket, SERVER_CMD_SKILLRDY, name, strlen(name));
}

/* send to the client the golem face & name. Note, that this is only cosmetical
 * information to fill the range menu in the client.
 */
void send_golem_control(object_t *golem, int mode)
{
    char    tmp[MEDIUM_BUF]; /* we should careful set a big enough buffer here */

    if (mode == GOLEM_CTR_RELEASE)
        sprintf(tmp, "%d %d %s", mode, 0, golem->name);
    else
        sprintf(tmp, "%d %d %s", mode, golem->face->number, golem->name);
    Write_String_To_Socket(&CONTR(golem->owner)->socket, SERVER_CMD_GOLEMCMD, tmp, strlen(tmp));
}

/*
 * esrv_update_stats sends a statistics update.  We look at the old values,
 * and only send what has changed.  Stat mapping values are in newclient.h
 * Since this gets sent a lot, this is actually one of the few binary
 * commands for now.
 */
void esrv_update_stats(player_t *pl)
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
    if (pl->target_ob != pl->ob) /* never send our own status - client will sort this out */
    {
        /* we don't care about count - target function will readjust itself */
        if (pl->target_ob && pl->target_ob->stats.hp != pl->target_hp) /* just for secure...*/
        {
            /* well, i would like to avoid this calc here but we won't give the player the true hp value of target */
            char hp = (char) (((float) pl->target_ob->stats.hp / (float) pl->target_ob->stats.maxhp) * 100.0f);
            pl->target_hp = pl->target_ob->stats.hp;
            AddIfChar(pl->target_hp_p, hp, CS_STAT_TARGET_HP);
        }
    }

    AddIfShort(pl->last_gen_hp, pl->gen_hp, CS_STAT_REG_HP);
    AddIfShort(pl->last_gen_sp, pl->gen_sp, CS_STAT_REG_MANA);
    AddIfShort(pl->last_gen_grace, pl->gen_grace, CS_STAT_REG_GRACE);
    AddIfInt(pl->last_weight_limit, pl->weight_limit, CS_STAT_WEIGHT_LIM);
    AddIfInt(pl->last_weapon_sp, pl->weapon_sp, CS_STAT_WEAP_SP);
    AddIfInt(pl->last_speed_enc, pl->speed_enc, CS_STAT_SPEED);
    AddIfInt(pl->last_spell_fumble, pl->spell_fumble, CS_STAT_SPELL_FUMBLE);

    if (pl->ob != NULL)
    {
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
        
        AddIfInt(pl->last_action_timer, pl->action_timer, CS_STAT_ACTION_TIME);

        /* these will update too when we are in a group */
        AddIfIntFlag(pl->last_stats.hp, pl->ob->stats.hp, group_update, GROUP_UPDATE_HP, CS_STAT_HP);
        AddIfIntFlag(pl->last_stats.maxhp, pl->ob->stats.maxhp, group_update, GROUP_UPDATE_MAXHP, CS_STAT_MAXHP);
        AddIfShortFlag(pl->last_stats.sp, pl->ob->stats.sp, group_update, GROUP_UPDATE_SP, CS_STAT_SP);
        AddIfShortFlag(pl->last_stats.maxsp, pl->ob->stats.maxsp, group_update, GROUP_UPDATE_MAXSP, CS_STAT_MAXSP);
        AddIfShortFlag(pl->last_stats.grace, pl->ob->stats.grace, group_update, GROUP_UPDATE_GRACE, CS_STAT_GRACE);
        AddIfShortFlag(pl->last_stats.maxgrace, pl->ob->stats.maxgrace, group_update, GROUP_UPDATE_MAXGRACE,CS_STAT_MAXGRACE);
        AddIfCharFlag(pl->last_level, pl->ob->level, group_update, GROUP_UPDATE_LEVEL, CS_STAT_LEVEL);
    }

    /* TODO: (a) Might this make more sense as part of esrv_update_skills()
     * above; (b) is it necessary at all?The only way a skillgroup can change
     * level/exp is when a skill does. The skill change is sent to the client
     * so can't the client then work out the skillgroup level/exp (which is
     * just the highest of that group's skills)?
     * -- Smacky    20130923 */
    for (i = 0; i < NROFSKILLGROUPS_ACTIVE; i++)
    {
        AddIfInt(pl->last_skillgroup_exp[i], pl->skillgroup_ptr[i]->stats.exp, cs_stat_skillexp[i]);
        AddIfChar(pl->last_skillgroup_level[i], pl->skillgroup_ptr[i]->level, cs_stat_skillexp[i]+1);
    }

    flags = 0;
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
        SOCKBUF_REQUEST_FINISH(&pl->socket, SERVER_CMD_STATS, SOCKBUF_DYNAMIC);
    else
        SOCKBUF_REQUEST_RESET(&pl->socket);

    if(group_update && pl->group_status & GROUP_STATUS_GROUP && pl->update_ticker != ROUND_TAG)
        party_client_group_update(pl->ob, group_update);
    pl->update_ticker = ROUND_TAG;
}


void esrv_new_player(player_t *pl, uint32 weight)
{
    int len;
    sockbuf_struct *sptr;

    SOCKBUF_REQUEST_BUFFER(&pl->socket, 128);
    sptr = ACTIVE_SOCKBUF(&pl->socket);

    SockBuf_AddInt(sptr, pl->ob->count);
    SockBuf_AddInt(sptr, weight);
    SockBuf_AddInt(sptr, pl->ob->face->number);
    SockBuf_AddChar(sptr, (len = strlen(pl->ob->name)) + 1);
    SockBuf_AddString(sptr, pl->ob->name, len);

    SOCKBUF_REQUEST_FINISH(&pl->socket, SERVER_CMD_PLAYER, SOCKBUF_DYNAMIC);
}
