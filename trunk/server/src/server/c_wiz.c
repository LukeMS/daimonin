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

    The author can be reached via e-mail to daimonin@nord-com.net
*/

/*
 * End of non-DM commands.  DM-only commands below.
 * (This includes commands directly from socket)
 */

/* Some commands not accepted from socket */

#include <global.h>


/* Gecko: since we no longer maintain a complete list of all objects,
 * all functions using find_object are a lot less useful...
 */

/* This finds and returns the object which matches the name or
 * object nubmer (specified via num #whatever).
 */

static object * find_object_both(char *params)
{
    if (!params)
        return NULL;
    if (params[0] == '#')
        return find_object(atol(params + 1));
    else
        return find_object_name(params);
}

/* Sets the god for some objects.  params should contain two values -
 * first the object to change, followed by the god to change it to.
 */
int command_setgod(object *op, char *params)
{
    object *ob;
    char   *str;

    if (!params || !(str = strchr(params, ' ')))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Usage: setgod object god");
        return 0;
    }
    /* kill the space, and set string to the next param */
    *str++ = '\0';
    if (!(ob = find_object_both(params)))
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "Set whose god - can not find object %s?", params);
        return 1;
    }
    /* Perhaps this is overly restrictive?  Should we perhaps be able
     * to rebless altars and the like?
     */
    if (ob->type != PLAYER)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s is not a player - can not change its god", ob->name);
        return 1;
    }
    change_skill(ob, SK_PRAYING);
    if (!ob->chosen_skill || ob->chosen_skill->stats.sp != SK_PRAYING)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s doesn't have praying skill.", ob->name);
        return 1;
    }
    if (find_god(str) == NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "No such god %s.", str);
        return 1;
    }
    become_follower(ob, find_god(str));
    return 1;
}

/* command_kickcmd is called when a gmaster triggers the command.
 * command_kick is also used internal to force a player logout.
 */
int command_kickcmd(object *ob, char *params)
{
	int ticks;

	if(ob && CONTR(ob)->gmaster_mode < GMASTER_MODE_VOL)
		return 0;
	
	if(!command_kick(ob, params))
		return 0;

	/* we kicked player params succesfull. 
	 * Now we give him a 1min temp ban, so he can
	 * think about it.
	 * If its a "technical" kick, the 1min is a protection.
	 * Perhaps we want reset a map or whatever.
	 */
	ticks = (int) (pticks_second*10.0f);
	add_ban_entry(params, ticks, ticks, 'p');

	return 1;			
}
	
/* called command_kick(NULL,NULL) or command_kick(op,<player name>.
 * NULl,NULL will global kick *all* players, the 2nd format only <player name>.
 * op,NULL is invalid
 */
int command_kick(object *ob, char *params)
{
    struct pl_player   *pl;
    const char         *name_hash;
	int					ret=0;

	if(ob && CONTR(ob)->gmaster_mode < GMASTER_MODE_VOL)
		return 0;

    if (ob && params == NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "Use: /kick <name>");
        return 0;
    }

    if (ob)
    {
        transform_name_string(params);
        if (!(name_hash = find_string(params)))
        {
            new_draw_info(NDI_UNIQUE, 0, ob, "No such player.");
            return 0;
        }
    }

    if (ob && ob->name == name_hash)
    {
        new_draw_info_format(NDI_UNIQUE, 0, ob, "You can't /kick yourself!");
        return 0;
    }


    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (!ob || (pl->ob != ob && pl->ob->name && pl->ob->name == name_hash))
        {
            object *op;
            op = pl->ob;
			ret=1;
            remove_ob(op);
            check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
            op->direction = 0;
            if (params)
                new_draw_info_format(NDI_UNIQUE | NDI_ALL, 5, ob, "%s is kicked out of the game.", op->name);
            LOG(llevInfo, "%s is kicked out of the game.\n", op->name);
            CONTR(op)->killer[0] = '\0';
            check_score(op); /* Always check score */
            container_unlink(CONTR(op), NULL);
            save_player(op, 1);
            CONTR(op)->socket.status = Ns_Dead;
#if MAP_MAXTIMEOUT
            op->map->timeout = MAP_TIMEOUT(op->map);
#endif
        }
    }

    /* not reached for NULL, NULL calling */
    return ret;
}


int command_shutdown(object *op, char *params)
{
    if (op != NULL && !QUERY_FLAG(op, FLAG_WIZ))
    {
        /*new_draw_info(NDI_UNIQUE,0,op,"Sorry, you can't shutdown the server.");*/
        return 1;
    }
    LOG(llevSystem, "SERVER SHUTDOWN STARTED\n");
    command_kick(NULL, NULL);
    cleanup(EXIT_SHUTODWN);
    /* not reached - server will terminate itself before that line */
    return 1;
}

int command_goto(object *op, char *params)
{
    char   *name;
    object *dummy;

    if (!op)
        return 0;

    if (params == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Go to what level?");
        return 1;
    }
    name = params;
    dummy = get_object();
    dummy->map = op->map;
    FREE_AND_COPY_HASH(EXIT_PATH(dummy), name);
    FREE_AND_COPY_HASH(dummy->name, name);

    enter_exit(op, dummy);
    new_draw_info_format(NDI_UNIQUE, 0, op, "Difficulty: %d.", op->map->difficulty);
    return 1;
}

/* is this function called from somewhere ? -Tero */
int command_generate(object *op, char *params)
{
    object *tmp;
    int     nr = 1, i, retry;

    if (!op)
        return 0;

    if (params != NULL)
        sscanf(params, "%d", &nr);
    for (i = 0; i < nr; i++)
    {
        retry = 50;
        while ((tmp = generate_treasure(0, op->map->difficulty)) == NULL && --retry);
        if (tmp != NULL)
        {
            tmp = insert_ob_in_ob(tmp, op);
            if (op->type == PLAYER)
                esrv_send_item(op, tmp);
        }
    }
    return 1;
}

int command_summon(object *op, char *params)
{
    int     i;
    object *dummy;
    player *pl;

	/* allowed for GM and DM only */
	if(CONTR(op)->gmaster_mode < GMASTER_MODE_GM)
		return 0;

	if (!op)
        return 0;

    if (params == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Usage: summon <player>.");
        return 1;
    }

    pl = find_player(params);

    if (pl == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");
        return 1;
    }
    if (pl->ob == op)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't summon yourself next to yourself.");
        return 1;
    }
    if (pl->state != ST_PLAYING)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "That player can't be summoned right now.");
        return 1;
    }
    i = find_free_spot(op->arch, op->map, op->x, op->y, 1, 8);
    if (i == -1)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Can not find a free spot to place summoned player.");
        return 1;
    }
    dummy = get_object();
    FREE_AND_ADD_REF_HASH(EXIT_PATH(dummy), op->map->path);
    EXIT_X(dummy) = op->x + freearr_x[i];
    EXIT_Y(dummy) = op->y + freearr_y[i];
    enter_exit(pl->ob, dummy);
    new_draw_info(NDI_UNIQUE, 0, pl->ob, "You are summoned.");
    new_draw_info(NDI_UNIQUE, 0, op, "OK.");
    return 1;
}

/* Teleport next to target player */
/* mids 01/16/2002 */
int command_teleport(object *op, char *params)
{
    int     i;
    object *dummy;
    player *pl;

	/* allowed for GM and DM only */
	if(CONTR(op)->gmaster_mode < GMASTER_MODE_GM)
		return 0;

    if (!op)
        return 0;

    if (params == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Usage: teleport <player>.");
        return 1;
    }

    pl = find_player(params);

    if (pl == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");
        return 1;
    }
    if (pl->ob == op)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't teleport yourself next to yourself.");
        return 1;
    }
    if (pl->state != ST_PLAYING)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't teleport to that player right now.");
        return 1;
    }
    i = find_free_spot(pl->ob->arch, pl->ob->map, pl->ob->x, pl->ob->y, 1, 8);
    if (i == -1)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Can not find a free spot to teleport to.");
        return 1;
    }
    dummy = get_object();
    FREE_AND_ADD_REF_HASH(EXIT_PATH(dummy), pl->ob->map->path);
    EXIT_X(dummy) = pl->ob->x + freearr_x[i];
    EXIT_Y(dummy) = pl->ob->y + freearr_y[i];
    enter_exit(op, dummy);
    /*new_draw_info(NDI_UNIQUE, 0, pl->ob, "You see a portal open.");*/
    new_draw_info(NDI_UNIQUE, 0, op, "OK.");
    return 1;
}

int command_create(object *op, char *params)
{
    object     *tmp = NULL;
    int         nrof, i, magic, set_magic = 0, set_nrof = 0, gotquote, gotspace;
    char        buf[MAX_BUF], *cp, *bp = buf, *bp2, *bp3, *bp4 = NULL, *obp;
    archetype  *at;
    artifact   *art = NULL;

    if (!op)
        return 0;

    if (params == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op,
                      "Usage: create [nr] [magic] <archetype> [ of <artifact>]"
                      " [variable_to_patch setting]");
        return 1;
    }
    bp = params;

    if (sscanf(bp, "%d ", &nrof))
    {
        if ((bp = strchr(params, ' ')) == NULL)
        {
            new_draw_info(NDI_UNIQUE, 0, op,
                          "Usage: create [nr] [magic] <archetype> [ of <artifact>]"
                          " [variable_to_patch setting]");
            return 1;
        }
        bp++;
        set_nrof = 1;
        LOG(llevDebug, "%s creates: (%d) %s\n", op->name, nrof, bp);
    }
    if (sscanf(bp, "%d ", &magic))
    {
        if ((bp = strchr(bp, ' ')) == NULL)
        {
            new_draw_info(NDI_UNIQUE, 0, op,
                          "Usage: create [nr] [magic] <archetype> [ of <artifact>]"
                          " [variable_to_patch setting]");
            return 1;
        }
        bp++;
        set_magic = 1;
        LOG(llevDebug, "%s creates: (%d) (%d) %s\n", op->name, nrof, magic, bp);
    }
    if ((cp = strstr(bp, " of ")) != NULL)
    {
        *cp = '\0';
        cp += 4;
    }

    for (bp2 = bp; *bp2; bp2++)
        if (*bp2 == ' ')
        {
            *bp2 = '\0';
            bp2++;
            break;
        }

    /* ok - first step: browse the archtypes for the name - perhaps it is a base item */
    if ((at = find_archetype(bp)) == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such archetype or artifact name.");
        return 1;
    }

    if (cp)
    {
        if (find_artifactlist(at->clone.type) == NULL)
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "No artifact list for type %d\n", at->clone.type);
        }
        else
        {
            art = find_artifactlist(at->clone.type)->items;

            do
            {
                if (art->flags&ARTIFACT_FLAG_HAS_DEF_ARCH && !strcmp(art->def_at.clone.name, cp))
                    break;
                art = art->next;
            }
            while (art != NULL);
            if (!art)
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "No such artifact ([%d] of %s)", at->clone.type, cp);
            }
        }
        LOG(llevDebug, "%s creates: (%d) (%d) (%s) of (%s)\n", op->name, set_nrof ? nrof : 0, set_magic ? magic : 0, bp,
            cp);
    } /* if cp */

    if (at->clone.nrof)
    {
        tmp = arch_to_object(at);
        tmp->x = op->x,tmp->y = op->y;
        if (set_nrof)
            tmp->nrof = nrof;
        tmp->map = op->map;
        if (set_magic)
            set_abs_magic(tmp, magic);
        if (art)
            give_artifact_abilities(tmp, art);
        if (need_identify(tmp))
        {
            SET_FLAG(tmp, FLAG_IDENTIFIED);
            CLEAR_FLAG(tmp, FLAG_KNOWN_MAGICAL);
        }
		
        while (*bp2)
        {
			bp4 = NULL;
				
            /* find the first quote */
            for (bp3 = bp2, gotquote = 0, gotspace = 0; *bp3 && gotspace < 2; bp3++)
            {
                if (*bp3 == '"')
                {
                    *bp3 = ' ';
                    gotquote++;
                    bp3++;
                    for (bp4 = bp3; *bp4; bp4++)
                        if (*bp4 == '"')
                        {
                            *bp4 = '\0';
                            break;
                        }
                    break;
                }
                else if (*bp3 == ' ')
                    gotspace++;
            }
			
            if (!gotquote)
            {
                /* then find the second space */
                for (bp3 = bp2; *bp3; bp3++)
                {
                    if (*bp3 == ' ')
                    {
                        bp3++;
                        for (bp4 = bp3; *bp4; bp4++)
                        {
                            if (*bp4 == ' ')
                            {
                                *bp4 = '\0';
                                break;
                            }
                        }
                        break;
                    }
                }
            }
			
			if(bp4 == NULL)
			{
                new_draw_info_format(NDI_UNIQUE, 0, op, "No parameter value for variable %s", bp2);
				break;
			}
			
            /* now bp3 should be the argument, and bp2 the whole command */
            if (set_variable(tmp, bp2) == -1)
                new_draw_info_format(NDI_UNIQUE, 0, op, "Unknown variable %s", bp2);
            else
                new_draw_info_format(NDI_UNIQUE, 0, op, "(%s#%d)->%s=%s", tmp->name, tmp->count, bp2, bp3);
			
            if (gotquote)
                bp2 = bp4 + 2;
            else
                bp2 = bp4 + 1;
			
            /* WARNING: got a warning msg by compiler here - using obp without init. */
            /*if (obp == bp2)
            break;*/ /* invalid params */
            obp = bp2;
        }
        tmp = insert_ob_in_ob(tmp, op);
        esrv_send_item(op, tmp);
        return 1;
    }

    for (i = 0 ; i < (set_nrof ? nrof : 1); i++)
    {
        archetype      *atmp;
        object*prev =   NULL, *head = NULL;
        for (atmp = at; atmp != NULL; atmp = atmp->more)
        {
				
            tmp = arch_to_object(atmp);
            if (head == NULL)
                head = tmp;
            tmp->x = op->x + tmp->arch->clone.x;
            tmp->y = op->y + tmp->arch->clone.y;
            tmp->map = op->map;
            if (set_magic)
                set_abs_magic(tmp, magic);
            if (art)
                give_artifact_abilities(tmp, art);
            if (need_identify(tmp))
            {
                SET_FLAG(tmp, FLAG_IDENTIFIED);
                CLEAR_FLAG(tmp, FLAG_KNOWN_MAGICAL);
            }
			
            while (*bp2)
            {
				bp4=NULL;
				
                /* find the first quote */
                for (bp3 = bp2, gotquote = 0, gotspace = 0; *bp3 && gotspace < 2; bp3++)
                {
                    if (*bp3 == '"')
                    {
                        *bp3 = ' ';
                        gotquote++;
                        bp3++;
                        for (bp4 = bp3; *bp4; bp4++)
                            if (*bp4 == '"')
                            {
                                *bp4 = '\0';
                                break;
                            }
                        break;
                    }
                    else if (*bp3 == ' ')
                        gotspace++;
                }

                if (!gotquote)
                {
                    /* then find the second space */
                    for (bp3 = bp2; *bp3; bp3++)
                    {
                        if (*bp3 == ' ')
                        {
                            bp3++;
                            for (bp4 = bp3; *bp4; bp4++)
                            {
                                if (*bp4 == ' ')
                                {
                                    *bp4 = '\0';
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }

				if(bp4 == NULL)
				{
               		new_draw_info_format(NDI_UNIQUE, 0, op, "No parameter value for variable %s", bp2);
					break;
				}
			
                /* now bp3 should be the argument, and bp2 the whole command */
                if (set_variable(tmp, bp2) == -1)
                    new_draw_info_format(NDI_UNIQUE, 0, op, "Unknown variable '%s'", bp2);
                else
                    new_draw_info_format(NDI_UNIQUE, 0, op, "(%s#%d)->%s=%s", tmp->name, tmp->count, bp2, bp3);
                if (gotquote)
                    bp2 = bp4 + 2;
                else
                    bp2 = bp4 + 1;
                /* WARNING: got a warning msg by compiler here - using obp without init. */
                /*if (obp == bp2)
                   break;*/ /* invalid params */
                obp = bp2;
            }
            if (head != tmp)
                tmp->head = head,prev->more = tmp;
            prev = tmp;
        }
        if (IS_LIVE(head))
            insert_ob_in_map(head, op->map, op, INS_NO_MERGE | INS_NO_WALK_ON);
        else
            head = insert_ob_in_ob(head, op);
        if (at->clone.randomitems)
            create_treasure_list(at->clone.randomitems, head, GT_APPLY, get_enviroment_level(head),ART_CHANCE_UNSET, 0);
        esrv_send_item(op, head);
    }
    return 1;
}

/* if(<not socket>) */

/*
 * Now follows dm-commands which are also acceptable from sockets
 */

int command_inventory(object *op, char *params)
{
    object *tmp;
    int     i;

	/* allowed for GM and DM only */
	if(CONTR(op)->gmaster_mode < GMASTER_MODE_GM)
		return 0;

    if (!params)
    {
        inventory(op, NULL);
        return 0;
    }

    if (!sscanf(params, "%d", &i) || (tmp = find_object(i)) == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Inventory of what object (nr)?");
        return 1;
    }
    inventory(op, tmp);
    return 1;
}


int command_dump(object *op, char *params)
{
    int     i;
    object *tmp;

    if (params != NULL && !strcmp(params, "me"))
        tmp = op;
    else if (params == NULL || !sscanf(params, "%d", &i) || (tmp = find_object(i)) == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Dump what object (nr)?");
        return 1;
    }
    dump_object(tmp);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    return 1;
}

int command_patch(object *op, char *params)
{
    int     i;
    char   *arg, *arg2;
    char    buf[MAX_BUF];
    object *tmp;

    tmp = NULL;
    if (params != NULL)
    {
        if (!strncmp(params, "me", 2))
            tmp = op;
        else if (sscanf(params, "%d", &i))
            tmp = find_object(i);
        else if (sscanf(params, "%s", buf))
            tmp = find_object_name(buf);
    }
    if (tmp == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Patch what object (nr)?");
        return 1;
    }
    arg = strchr(params, ' ');
    if (arg == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Patch what values?");
        return 1;
    }
    if ((arg2 = strchr(++arg, ' ')))
        arg2++;
    if (set_variable(tmp, arg) == -1)
        new_draw_info_format(NDI_UNIQUE, 0, op, "Unknown variable %s", arg);
    else
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "(%s#%d)->%s=%s", tmp->name, tmp->count, arg, arg2);
    }
    return 1;
}

int command_remove(object *op, char *params)
{
    int     i;
    object *tmp;

    if (params == NULL || !sscanf(params, "%d", &i) || (tmp = find_object(i)) == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Remove what object (nr)?");
        return 1;
    }
    remove_ob(tmp);
    check_walk_off(tmp, NULL, MOVE_APPLY_VANISHED);
    return 1;
}

int command_free(object *op, char *params)
{
    int     i;
    object *tmp;

    if (params == NULL || !sscanf(params, "%d", &i) || (tmp = find_object(i)) == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Free what object (nr)?");
        return 1;
    }
    /* free_object(tmp); TODO: remove me*/

    return 1;
}

int command_addexp(object *op, char *params)
{
    char    buf[MAX_BUF];
    int     exp, snr;
    object *exp_skill, *exp_ob;
    player *pl;

    if (params == NULL || sscanf(params, "%s %d %d", buf, &snr, &exp) != 3)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Usage: addexp [who] [skill nr] [how much].");
        return 1;
    }

    pl = find_player(buf);

    if (pl == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");
        return 1;
    }

    /* Bug 0000100: /addexp Cher 101 100000 crashes server */
    /* Safety check */
    if (snr < 0 || snr >= NROFSKILLS)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such skill.");
        return 1;
    }
    
    exp_skill = pl->skill_ptr[snr];

    if (!exp_skill) /* safety check */
    {
        /* our player don't have this skill?
         * This can happens when group exp is devided.
         * We must get a useful sub or we skip the exp.
         */
        LOG(llevDebug, "TODO: add_exp(): called for %s with skill nr %d / %d exp - object has not this skill.\n",
            query_name(pl->ob), snr, exp);
        return 0; /* TODO: groups comes later  - now we skip all times */
    }

    /* if we are full in this skill, then nothing is to do */
    if (exp_skill->level >= MAXLEVEL)
        return 0;

    pl->update_skills = 1; /* we will sure change skill exp, mark for update */
    exp_ob = exp_skill->exp_obj;

    if (!exp_ob)
    {
        LOG(llevBug, "BUG: add_exp() skill:%s - no exp_op found!!\n", query_name(exp_skill));
        return 0;
    }

    exp = adjust_exp(pl->ob, exp_skill, exp);   /* first we see what we can add to our skill */

    /* adjust_exp has adjust the skill and all exp_obj and player exp */
    /* now lets check for level up in all categories */
    player_lvl_adj(pl->ob, exp_skill);
    player_lvl_adj(pl->ob, exp_ob);
    player_lvl_adj(pl->ob, NULL);

    return 1;
}

int command_speed(object *op, char *params)
{
    long i;
    if (params == NULL || !sscanf(params, "%ld", &i))
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "Current speed is %ld ums (%f ticks/second)", pticks_ums, pticks_second);
        return 1;
    }
    set_pticks_time(i);
    reset_sleep();
    new_draw_info_format(NDI_UNIQUE, 0, op, "Set speed to %ld ums (%f ticks/second)", pticks_ums, pticks_second);

    return 1;
}


/**************************************************************************/
/* Mods made by Tyler Van Gorder, May 10-13, 1992.                        */
/* CSUChico : tvangod@cscihp.ecst.csuchico.edu                            */
/**************************************************************************/

int command_stats(object *op, char *params)
{
    player *pl;
    char    buf[MAX_BUF];

    if (params == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Who?");
        return 1;
    }
    if (!(pl = find_player(params)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");
        return 1;
    }

    sprintf(buf, "Str : %-2d      H.P. : %-4d  MAX : %d", pl->ob->stats.Str, pl->ob->stats.hp, pl->ob->stats.maxhp);
    new_draw_info(NDI_UNIQUE, 0, op, buf);
    sprintf(buf, "Dex : %-2d      S.P. : %-4d  MAX : %d", pl->ob->stats.Dex, pl->ob->stats.sp, pl->ob->stats.maxsp) ;
    new_draw_info(NDI_UNIQUE, 0, op, buf);
    sprintf(buf, "Con : %-2d        AC : %-4d  WC  : %d", pl->ob->stats.Con, pl->ob->stats.ac, pl->ob->stats.wc) ;
    new_draw_info(NDI_UNIQUE, 0, op, buf);
    sprintf(buf, "Wis : %-2d       EXP : %d", pl->ob->stats.Wis, pl->ob->stats.exp);
    new_draw_info(NDI_UNIQUE, 0, op, buf);
    sprintf(buf, "Cha : %-2d      Food : %d", pl->ob->stats.Cha, pl->ob->stats.food) ;
    new_draw_info(NDI_UNIQUE, 0, op, buf);
    sprintf(buf, "Int : %-2d    Damage : %d", pl->ob->stats.Int, pl->ob->stats.dam) ;
    sprintf(buf, "Pow : %-2d    Grace : %d", pl->ob->stats.Pow, pl->ob->stats.grace) ;
    new_draw_info(NDI_UNIQUE, 0, op, buf);

    return 1;
}

int command_abil(object *op, char *params)
{
    char    thing[20], thing2[20];
    int     iii;
    player *pl;
    char    buf[MAX_BUF];

    iii = 0;
    thing[0] = '\0';
    thing2[0] = '\0';
    if (params == NULL || !sscanf(params, "%s %s %d", thing, thing2, &iii) || thing == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Who?");
        return 1;
    }
    if (thing2 == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't change that.");
        return 1;
    }

    if (!(pl = find_player(thing)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");
        return 1;
    }


    if (iii<MIN_STAT || iii>MAX_STAT)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Illegal range of stat.\n");
        return 1;
    }

    if (!strcmp("str", thing2))
        pl->ob->stats.Str = iii,pl->orig_stats.Str = iii;
    if (!strcmp("dex", thing2))
        pl->ob->stats.Dex = iii,pl->orig_stats.Dex = iii;
    if (!strcmp("con", thing2))
        pl->ob->stats.Con = iii,pl->orig_stats.Con = iii;
    if (!strcmp("wis", thing2))
        pl->ob->stats.Wis = iii,pl->orig_stats.Wis = iii;
    if (!strcmp("cha", thing2))
        pl->ob->stats.Cha = iii,pl->orig_stats.Cha = iii;
    if (!strcmp("int", thing2))
        pl->ob->stats.Int = iii,pl->orig_stats.Int = iii;
    if (!strcmp("pow", thing2))
        pl->ob->stats.Pow = iii,pl->orig_stats.Pow = iii;
    sprintf(buf, "%s has been altered.", pl->ob->name);
    new_draw_info(NDI_UNIQUE, 0, op, buf);
    fix_player(pl->ob);
    return 1;
}

int command_reset(object *op, char *params)
{
    int             count;
    mapstruct      *m;
    player         *pl;
    object*dummy =  NULL;
    const char     *mapfile_sh;

    if (params == NULL)
        m = has_been_loaded_sh(op->map->path);
    else
    {
        mapfile_sh = add_string(params);
        m = has_been_loaded_sh(mapfile_sh);
        free_string_shared(mapfile_sh);
    }

    if (m == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such map.");
        return 1;
    }

    dummy = get_object();
    dummy->map = NULL;
    FREE_AND_ADD_REF_HASH(EXIT_PATH(dummy), m->path);

    if (m->in_memory != MAP_SWAPPED)
    {
        if (m->in_memory != MAP_IN_MEMORY)
        {
            LOG(llevBug, "BUG: Tried to swap out map which was not in memory.\n");
            return 0;
        }

        new_draw_info_format(NDI_UNIQUE, 0, op, "Start reseting map %s.", STRING_SAFE(m->path));
        /* remove now all players from this map - flag them so we can
             * put them back later.
             */
        count = 0;
        for (pl = first_player; pl != NULL; pl = pl->next)
        {
            if (pl->ob->map == m)
            {
                count++;
                remove_ob(pl->ob); /* no walk off check */
                pl->dm_removed_from_map = 1;
                /*tmp=op;*/
            }
            else
                pl->dm_removed_from_map = 0;
        }
        new_draw_info_format(NDI_UNIQUE, 0, op, "removed %d players from map. Swap map.", count);
        swap_map(m, 1);
    }

    if (m->in_memory == MAP_SWAPPED)
    {
        LOG(llevDebug, "Resetting map %s.\n", m->path);
        clean_tmp_map(m);
        if (m->tmpname)
            free(m->tmpname);
        m->tmpname = NULL;
        /* setting this effectively causes an immediate reload */
        m->reset_time = 1;
        new_draw_info(NDI_UNIQUE, 0, op, "Swap successful. Inserting players.");

        for (pl = first_player; pl != NULL; pl = pl->next)
        {
            if (pl->dm_removed_from_map)
            {
                EXIT_X(dummy) = pl->ob->x;
                EXIT_Y(dummy) = pl->ob->y;
                enter_exit(pl->ob, dummy);
                if (pl->ob != op)
                {
                    if (QUERY_FLAG(pl->ob, FLAG_WIZ))
                        new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "Map reset by %s.", op->name);
                    else /* Write a nice little confusing message to the players */
                        new_draw_info(NDI_UNIQUE, 0, pl->ob,
                                      "Your surroundings seem different but still familiar. Haven't you been here before?");
                }
            }
        }

        new_draw_info(NDI_UNIQUE, 0, op, "resetmap done.");
    }
    else
    {
        /* Need to re-insert player if swap failed for some reason */
        for (pl = first_player; pl != NULL; pl = pl->next)
        {
            if (pl->dm_removed_from_map)
                insert_ob_in_map(pl->ob, m, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
        }
        new_draw_info(NDI_UNIQUE, 0, op, "Reset failed, couldn't swap map!");
    }

    return 1;
}

/* warning: these function is for heavy debugging.
 * Its somewhat useless under windows.
 */
int command_check_fd(object *op, char *params)
{
    struct _stat    buf;
    int             handle_max  = socket_info.max_filedescriptor < 100 ? 100 : socket_info.max_filedescriptor, fh;

    /* remember, max_filedescriptor don't works under windows */
    new_draw_info_format(NDI_UNIQUE, 0, op, "check file handles from 0 to %d.", handle_max);
    LOG(llevSystem, "check file handles from 0 to %d.", handle_max);
    for (fh = 0; fh <= handle_max; fh++)
    {
        /* Check if statistics are valid: */
        if (!_fstat(fh, &buf))
        {
            /* no ttyname() under windows... well,
                     * debugging fh's is always more clever on linux.
                     */
#ifdef WIN32
            LOG(llevSystem, "FH %d ::(%d) size     : %ld\n", fh, _isatty(fh), buf.st_size);
#else
            player *pp;
            char   *name1   = NULL;

            /* collect some senseless handle numbers... */
            for (pp = first_player; pp; pp = pp->next)
            {
                if (pp->socket.fd == fh)
                    break;
            }
            name1 = ttyname(_isatty(fh));

            LOG(llevSystem, "FH %d ::(%s) (%s) size: %ld\n", fh, name1 ? name1 : "><",
                pp ? (pp->ob ? query_name(pp->ob) : ">player<") : "", buf.st_size);
#endif
        }
    }
    return 1;
}

/* a muted player can't shout/say/tell/reply for the
 * amount of time.
 * we have 2 kinds of mute: shout/say & tell/gsay/reply.
 * a player can be muted through this command and/or 
 * automatic by the spam agent.
 */
int command_mute(object *op, char *params)
{

	return 1;
}

/* a silenced player can't shout or tell or say
 * but the he don't know it. A own shout will be shown
 * to him as normal but not to others. 
 */
int command_silence(object *op, char *params)
{
	
	return 1;
}

/* a player can be banned forever or for a time.
 * We can ban a player or a IP/host.
 * Format is /ban player <name> <time> or
 * /ban ip <host> <time>.
 * <time> can be 1m, 1h, 3d or *. m= minutes, h=hours, d= days
 * and '*' = permanent.
 * This is something we should move later to the login server.
 */
int command_ban(object *op, char *params)
{
	objectlink *ol;
	char *str;
	
	if(CONTR(op)->gmaster_mode == GMASTER_MODE_NO)
		return 0;
	
	if (!params)
		goto ban_usage;		

	/* list all entries of gmaster_file
	 */
	if(!strcmp(params,"list"))
	{	
	
		new_draw_info(NDI_UNIQUE, 0, op, "ban list");
		new_draw_info(NDI_UNIQUE, 0, op, "--- --- ---");
		for(ol = ban_list_player;ol;ol=ol->next)
			new_draw_info_format(NDI_UNIQUE, 0, op, "P: %s (%d)", ol->objlink.ban->tag ,ol->objlink.ban->ticks);

		for(ol = ban_list_ip;ol;ol=ol->next)
			new_draw_info_format(NDI_UNIQUE, 0, op, "IP:%s (%d)", ol->objlink.ban->tag ,ol->objlink.ban->ticks);

		return 1;
	}
    if (!(str = strchr(params, ' ')))
		goto bane_usage;		
	
    /* kill the space, and set string to the next param */
    *str++ = '\0';
	if(!strcmp(params,"add"))
	{
		int ticks=-1;
		char mode=0, name[MAX_BUF]="";
		
        if (sscanf(str, "%s %c %d", name, &mode, &ticks) == 3)
		{
			if(name && name[0]!='\0' && (mode == 'i' || mode =='p'))
			{
				objectlink *ol, *ol_tmp;
				int flag=FALSE;

				for(ol = ban_list_player;ol;ol=ol_tmp)
				{
					ol_tmp = ol->next;
					if(!strcasecmp(ol->objlink.ban->tag,name))
					{
						flag=TRUE;
						remove_ban_entry(ol);
					}
					break;
				}
				if(!flag)
				{
					for(ol = ban_list_player;ol;ol=ol_tmp)
					{
						ol_tmp = ol->next;
						if(!strcasecmp(ol->objlink.ban->tag,name))
						{
							flag=TRUE;
							remove_ban_entry(ol);
						}
						break;
					}
				}
				
				if(flag) 
				{
					new_draw_info_format(NDI_UNIQUE, 0, op, "You updated the ban entry %s (%d ticks banned)", str, ticks);
					LOG(llevSystem,"/ban: %s updated the ban entry %s (%c - %d)\n", query_name(op), str, mode, ticks);
				}
				else
				{
					new_draw_info_format(NDI_UNIQUE, 0, op, "You added the ban entry %s (%d ticks banned)", str, ticks);
					LOG(llevSystem,"/ban: %s added the ban entry %s (%c - %d)\n", query_name(op), str, mode, ticks);
				}
				add_ban_entry(name, ticks, ticks, mode);
				return 1;
			}
		}
	}
	else if(!strcmp(params,"remove"))
	{
		/* we assume that a host/ip entry has not the same name
		 * as a player name... so we don't care and compare just all
		 * against the same string.
		 */
		for(ol = ban_list_player;ol;ol=ol->next)
		{
			if(!strcmp(ol->objlink.ban->tag,str))
			{
				LOG(llevSystem,"/ban: %s removed the ban entry %s\n", query_name(op), str);
				new_draw_info_format(NDI_UNIQUE, 0, op, "You removed the ban entry %s", str);
				remove_ban_entry(ol);
				return 1;
			}
		}
		for(ol = ban_list_ip;ol;ol=ol->next)
		{
			if(!strcmp(ol->objlink.ban->tag,str))
			{
				LOG(llevSystem,"/ban: %s removed the ban entry %s\n", query_name(op), str);
				new_draw_info_format(NDI_UNIQUE, 0, op, "You removed the ban entry %s", str);
				remove_ban_entry(ol);
				return 1;
			}
		}
			
	}
		
ban_usage:
    new_draw_info(NDI_UNIQUE, 0, op, "Usage: /ban  list | add | remove <entry>");
	return 1;
}
	
/* become a VOL
 */
int command_vol(object *op, char *params)
{
	if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL) /* turn off ? */
		remove_gmaster_mode(CONTR(op)); 
	else if(check_gmaster_list(CONTR(op), GMASTER_MODE_VOL))
		set_gmaster_mode(CONTR(op), GMASTER_MODE_VOL);
	
    return 1;
}

/* become a GM
 */
int command_gm(object *op, char *params)
{
	if(CONTR(op)->gmaster_mode == GMASTER_MODE_GM) /* turn off ? */
		remove_gmaster_mode(CONTR(op)); 
	else if(check_gmaster_list(CONTR(op), GMASTER_MODE_GM))
		set_gmaster_mode(CONTR(op), GMASTER_MODE_GM);

    return 1;	
}

/* Actual command to perhaps become dm.  Changed aroun a bit in version 0.92.2
 * - allow people on sockets to become dm, and allow better dm file
 */

int command_dm(object *op, char *params)
{
	if(CONTR(op)->gmaster_mode == GMASTER_MODE_DM) /* turn off ? */
		remove_gmaster_mode(CONTR(op)); 
	else if(check_gmaster_list(CONTR(op), GMASTER_MODE_DM))
		set_gmaster_mode(CONTR(op), GMASTER_MODE_DM);

	return 1;
}

/* list all active GM/VOL/DM 
 * gmaster actives only
 */
int command_dm_list(object *op, char *params)
{
	objectlink *ol;

	if(CONTR(op)->gmaster_mode == GMASTER_MODE_NO)
		return 0;
	
	new_draw_info(NDI_UNIQUE, 0, op, "DM/GM/VOL online");
	new_draw_info(NDI_UNIQUE, 0, op, "--- --- ---");
	for(ol = gmaster_list_VOL;ol;ol=ol->next)
		new_draw_info_format(NDI_UNIQUE, 0, op, "%s (%d)", CONTR(ol->objlink.ob)->quick_name, ol->objlink.ob->level);
	
	for(ol = gmaster_list_GM;ol;ol=ol->next)
		new_draw_info_format(NDI_UNIQUE, 0, op, "%s (%d)", CONTR(ol->objlink.ob)->quick_name, ol->objlink.ob->level);
	
	for(ol = gmaster_list_DM;ol;ol=ol->next)
	{
		if(!CONTR(ol->objlink.ob)->dm_stealth)
			new_draw_info_format(NDI_UNIQUE, 0, op, "%s (%d)", CONTR(ol->objlink.ob)->quick_name, ol->objlink.ob->level);
	}
	return 1;
	
}

/* /dm_set command
 */
int command_dm_set(object *op, char *params)
{
	char *str;


	if(CONTR(op)->gmaster_mode != GMASTER_MODE_DM)
		return 0;

	if (!params)
		goto d_set_usage;		

	/* list all entries of gmaster_file
	 */
	if(!strcmp(params,"list"))
	{	
		objectlink *ol;

		new_draw_info(NDI_UNIQUE, 0, op, "gmaster_file");
		new_draw_info(NDI_UNIQUE, 0, op, "--- --- ---");
		for(ol = gmaster_list;ol;ol=ol->next)
			new_draw_info_format(NDI_UNIQUE, 0, op, "%s", ol->objlink.gm->entry);
			
		return 1;
	}

    if (!(str = strchr(params, ' ')))
		goto d_set_usage;		


    /* kill the space, and set string to the next param */
    *str++ = '\0';

	if(!strcmp(params,"add"))
	{
		char name[MAX_BUF], passwd[MAX_BUF], host[MAX_BUF], mode[MAX_BUF], dummy[MAX_BUF];

        if (sscanf(str, "%[^:]:%[^:]:%[^:]:%s%[\n\r]", name, passwd, host, mode, dummy) < 3)
			new_draw_info(NDI_UNIQUE, 0, op, "/dm_set: invalid parameter.");
		else
		{	
			int mode_id = check_gmaster_file_entry(name, passwd, host, mode);
			
			if(mode_id == GMASTER_MODE_NO)
			{
				new_draw_info(NDI_UNIQUE, 0, op, "/dm_set: invalid parameter.");
				return 1;
			}
			
			/* all ok, setup the gmaster node and add it to our list */
			LOG(llevSystem, "GMASTER:: /dm_set add %s invoked by %s\n", str, query_name(op));
			new_draw_info_format(NDI_UNIQUE, 0, op, "/dm_set: add entry %s", str);
			add_gmaster_file_entry(name, passwd, host, mode_id);			
			new_draw_info(NDI_UNIQUE, 0, op, "write back gmaster_file...");
			write_gmaster_file(); /* create a new file */
		}
		return 1;
	}
	else if(!strcmp(params,"remove"))
	{
		objectlink *ol;

		for(ol = gmaster_list;ol;ol=ol->next)
		{
			if(!strcmp(str,ol->objlink.gm->entry)) /* found a entry */
			{
				/* delete the entry... */
				LOG(llevSystem, "GMASTER:: /dm_set remove %s invoked by %s\n", str, query_name(op));

				new_draw_info_format(NDI_UNIQUE, 0, op, "/dm_set: remove entry %s", str);
				remove_gmaster_file_entry(ol);
				new_draw_info(NDI_UNIQUE, 0, op, "write back gmaster_file...");
				write_gmaster_file(); /* create a new file */
				new_draw_info(NDI_UNIQUE, 0, op, "update gmaster rights...");
				update_gmaster_file(); /* control rights of all active VOL/GM/DM */
				new_draw_info(NDI_UNIQUE, 0, op, "done.");
				
				return 1;
			}
		}

		return 1;
	}

d_set_usage:
    new_draw_info(NDI_UNIQUE, 0, op, "Usage: /dm_set  list | add | remove <entry>");
	
	return 1;
}

	
int command_invisible(object *op, char *params)
{
    if (!op)
        return 0;


    if (IS_SYS_INVISIBLE(op))
    {
        CLEAR_FLAG(op, FLAG_SYS_OBJECT);
        new_draw_info(NDI_UNIQUE, 0, op, "You turn visible.");
    }
    else
    {
        SET_FLAG(op, FLAG_SYS_OBJECT);
        new_draw_info(NDI_UNIQUE, 0, op, "You turn invisible.");
    }
    update_object(op, UP_OBJ_FACE);
    return 0;
}


static int command_learn_spell_or_prayer(object *op, char *params, int special_prayer)
{
    int spell;

    if (op->type != PLAYER || CONTR(op) == NULL || params == NULL)
        return 0;

    if ((spell = look_up_spell_name(params)) <= 0)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Unknown spell.");
        return 1;
    }

    do_learn_spell(op, spell, special_prayer);
    return 1;
}

int command_learn_spell(object *op, char *params)
{
    return command_learn_spell_or_prayer(op, params, 0);
}

int command_learn_special_prayer(object *op, char *params)
{
    return command_learn_spell_or_prayer(op, params, 1);
}

int command_forget_spell(object *op, char *params)
{
    int spell;

    if (op->type != PLAYER || CONTR(op) == NULL || params == NULL)
        return 0;

    if ((spell = look_up_spell_name(params)) <= 0)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Unknown spell.");
        return 1;
    }

    do_forget_spell(op, spell);
    return 1;
}

/* GROS */
/* Lists all plugins currently loaded with their IDs and full names.         */
int command_listplugins(object *op, char *params)
{
    displayPluginsList(op);
    return 1;
}

/* GROS */
/* Loads the given plugin. The DM specifies the name of the library to load  */
/* (no pathname is needed). Do not ever attempt to load the same plugin more */
/* than once at a time, or bad things could happen.                          */
int command_loadplugin(object *op, char *params)
{
    char    buf[MAX_BUF];

    strcpy(buf, DATADIR);
    strcat(buf, "/../plugins/");
    strcat(buf, params);
    printf("Requested plugin file is %s\n", buf);
    initOnePlugin(buf);
    return 1;
}
/* GROS */
/* Unloads the given plugin. The DM specified the ID of the library to       */
/* unload. Note that some things may behave strangely if the correct plugins */
/* are not loaded.                                                           */
int command_unloadplugin(object *op, char *params)
{
    removeOnePlugin(params);
    return 1;
}

