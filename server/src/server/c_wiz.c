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

#include <global.h>

#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* end win32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

typedef struct dmload_struct {
    char    name[32];
    char    password[32];
} _dmload_struct;

static struct dmload_struct dmload_value = {"",""};

/* Gecko: we could at least search through the active, friend and player lists here... */
/* Michtoen: When reworking the DM commands - make this browsing all, why not? its a DM command.
* browse active list, all players and even all maps - this IS a super special command,
* don't care about cpu usage. Its important to patch or fix objects online.
*/
/*
 * Helper function get an object somewhere in the game
 * Returns the object which has the count-variable equal to the argument.
 */

static object * find_object(int i)
{
    /* i is the count - browse ALL and return the object */
    return NULL;
}

#if 0 /* disabled because account patch */
/* This command allows to login as a player without we know the password,
* but adding a "temporary" new one defined
*/
int command_dmload(object *op, char *params)
{
    char buf[256], name[256]="", pwd[256]="";

    if (!params || op==NULL || op->type != PLAYER)
        return 0;

    if(!QUERY_FLAG(op,FLAG_WIZ) )
        return 0;

    sscanf(params, "%s %s", name, pwd);

    strncpy(dmload_value.name, name, 32);
    dmload_value.name[30] = 0;
    strncpy(dmload_value.password, pwd, 32);
    dmload_value.password[30] = 0;

    sprintf(buf,"DM_LOAD name:>%s< pwd:>%s<\n", dmload_value.name, dmload_value.password);

    LOG(llevDebug, buf);
    new_draw_info_format(NDI_UNIQUE, 0, op, buf);
    return 1;
}
/* check a dmload struct is set and if, the settings match.
* returns 0 when not and 1 when we want and can override!
*/
int check_dmload(const char*name, const char *pwd)
{

    /* if a name is not set or they don't match - nothing to do */
    if(!dmload_value.name[0] || !name || strcasecmp(dmload_value.name, name))
        return 0;

    if(!dmload_value.password[0] || !pwd || strcmp(dmload_value.password, pwd))
        return 0;

    /* after we report ONE match, we reset the dmload structure for safety reasons */
    dmload_value.name[0] = 0;
    dmload_value.password[0] = 0;

    LOG(llevDebug, "DM_LOAD:: DM_CHECK SUCCESS - >%s< >%s<!\n", name, pwd);
    return 1;
}

#endif

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
    if (!(ob = find_object(atol(params))))
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
     * If its a "technical" kick, the 10 sec is a protection.
     * Perhaps we want reset a map or whatever.
     */
    ticks = (int) (pticks_second*60.0f);
    add_ban_entry(params, NULL, ticks, ticks);

    return 1;
}

/* called command_kick(NULL,NULL) or command_kick(op,<player name>.
 * NULl,NULL will global kick *all* players, the 2nd format only <player name>.
 * op,NULL is invalid
 */
int command_kick(object *ob, char *params)
{
    struct pl_player   *pl;
    const char         *name_hash = NULL;
    int                 ret=0;
    objectlink         *ol;

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

    if(!ob)
    {
        for(pl=first_player;pl!=NULL;pl=pl->next)
        {
            if(player_save(pl->ob))
                LOG(llevInfo, "Saving player %s: Success!\n", query_name(pl->ob));
            else
                LOG(llevInfo, "Saving player %s: FAILED!\n", query_name(pl->ob));
        }

    }
    else
        player_save(ob);

    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (!ob || (pl->ob != ob && pl->ob->name && pl->ob->name == name_hash))
        {
            object *op;
            op = pl->ob;
            ret=1;
            activelist_remove(op);
            remove_ob(op);
            check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
            op->direction = 0;
            if(ob)
                LOG(llevInfo, "KICKCMD: %s issued /kick %s\n", query_name(ob), query_name(op));
            if (params)
                new_draw_info_format(NDI_UNIQUE | NDI_ALL, 5, ob, "%s is kicked out of the game.", query_name(op));
            LOG(llevInfo, "%s is kicked out of the game.\n", query_name(op));
            for(ol = gmaster_list_VOL;ol;ol=ol->next)
                new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob,
                    "KICK: Player %s has been kicked by %s\n", query_name(op), query_name(ob));
            for(ol = gmaster_list_GM;ol;ol=ol->next)
                new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob,
                    "KICK: Player %s has been kicked by %s\n", query_name(op), query_name(ob));
            for(ol = gmaster_list_MM;ol;ol=ol->next)
                new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob,
                    "KICK: Player %s has been kicked by %s\n", query_name(op), query_name(ob));
            container_unlink(CONTR(op), NULL);
            CONTR(op)->socket.status = Ns_Dead;
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
    int x=-1, y=-1;
    char name[MAXPATHLEN] = {"\0"};

    if (!op)
        return 0;

    if (params == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Go to what level?\nUsage: /goto <map path> x y");
        return 1;
    }

    sscanf(params, "%s %d %d", name, &x, &y);
    if(name[0] != '\0')
    {
        shstr *hash_name = add_string(name);

        if(enter_map_by_name(op, hash_name, hash_name, x, y, 0))
            new_draw_info_format(NDI_UNIQUE, 0, op, "Difficulty: %d.", op->map->difficulty);

        FREE_ONLY_HASH(hash_name);
    }
    return 1;
}

/* is this function called from somewhere ? -Tero */
int command_generate(object *op, char *params)
{
    object     *tmp = NULL;
    int         nrof, i, magic, set_magic = 0, set_nrof = 0, gotquote, gotspace;
    char        buf[MAX_BUF], *cp, *bp = buf, *bp2, *bp3, *bp4 = NULL, *obp, *cp2;
    archetype  *at;
    artifact   *art = NULL, *art2 = NULL;

    if (!op)
        return 0;
//    if(op && CONTR(op)->gmaster_mode <= GMASTER_MODE_VOL)
//        return 0;
   if(op && (CONTR(op)->gmaster_mode == GMASTER_MODE_GM || CONTR(op)->gmaster_mode == GMASTER_MODE_MM))
   {
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
    if ((cp = strstr(bp, " amask ")) != NULL)
    {
        *cp = '\0';
        cp += 7;
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
    if (at->clone.type == MONSTER)
    {
      new_draw_info(NDI_UNIQUE, 0, op, "Generate cannot be used to create mobs.");
      return 1;
    }
    if (cp)
    {
        for (cp2 = cp; *cp2; cp2++)
         if (*cp2 == ' ')
           {
            *cp2 = '\0';
            break;
           }
        if (find_artifactlist(at->clone.type) == NULL)
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "No artifact list for type %d\n", at->clone.type);
        }
        else
        {
            art = find_artifact(cp);
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
                if (strstr(bp2, "name") || strstr(bp2, "title") || strstr(bp2, "amask"))
                {
                  /* now bp3 should be the argument, and bp2 the whole command */
                if (set_variable(tmp, bp2) == -1)
                  new_draw_info_format(NDI_UNIQUE, 0, op, "Unknown variable %s", bp2);
                else
                    new_draw_info_format(NDI_UNIQUE, 0, op, "(%s#%d)->%s=%s", tmp->name, tmp->count, bp2, bp3);
                }

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
                if (!cp && (strstr(bp2, "name") || strstr(bp2, "title") || strstr(bp2, "amask")))
                {
                /* now bp3 should be the argument, and bp2 the whole command */
                if (set_variable(tmp, bp2) == -1)
                    new_draw_info_format(NDI_UNIQUE, 0, op, "Unknown variable '%s'", bp2);
                else
                    new_draw_info_format(NDI_UNIQUE, 0, op, "(%s#%d)->%s=%s", tmp->name, tmp->count, bp2, bp3);
                }

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
            head = insert_ob_in_ob(head, op);
			esrv_send_item(op, head);
		
    }
    return 1;
  }
  else
    return 0;
}


int command_mutelevel(object *op, char *params)
{
    char buf[256];
    int lvl = 0;
    objectlink *ol;

    /* allowed for VOL and higher */
    if(CONTR(op)->gmaster_mode < GMASTER_MODE_VOL)
        return 0;

    if (op && params == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Use: /mutelevel <level>");
        return 0;
    }

    /* set shout/mute level from params */
    sscanf(params, "%d", &lvl);
    if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL && lvl > 10)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "WARNING: Maximum mutelevel for VOLs is 10.");
        lvl=10;
    }
    settings.mutelevel = lvl;
    sprintf(buf,"SET: shout level set to %d!\n", lvl);

    for(ol = gmaster_list_VOL;ol;ol=ol->next)
        new_draw_info(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob, buf);
    for(ol = gmaster_list_GM;ol;ol=ol->next)
        new_draw_info(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob, buf);
    for(ol = gmaster_list_MM;ol;ol=ol->next)
        new_draw_info(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob, buf);

    return 1;
}

int command_summon(object *op, char *params)
{
    int     i;
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
    if (!(pl->state & ST_PLAYING))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "That player can't be summoned right now.");
        return 1;
    }
    i = find_free_spot(op->arch, op->map, op->x, op->y, 1, 8);
    if (i == -1)
        i = 0;

    if(enter_map_by_name( pl->ob, op->map->path, op->map->orig_path,
                       op->x + freearr_x[i], op->y + freearr_y[i], MAP_STATUS_TYPE(op->map->map_status)))
    {
        new_draw_info(NDI_UNIQUE, 0, pl->ob, "You are summoned.");
        new_draw_info(NDI_UNIQUE, 0, op, "OK.");
    }
    return 1;
}

/* Teleport next to target player */
/* mids 01/16/2002 */
int command_teleport(object *op, char *params)
{
    int     i;
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
    if (!(pl->state & ST_PLAYING))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't teleport to that player right now.");
        return 1;
    }
    i = find_free_spot(pl->ob->arch, pl->ob->map, pl->ob->x, pl->ob->y, 1, 8);
    if (i == -1)
        i = 0;

    if(enter_map_by_name(op, pl->ob->map->path, pl->ob->map->orig_path,
              pl->ob->x + freearr_x[i], pl->ob->y + freearr_y[i], MAP_STATUS_TYPE(pl->ob->map->map_status) ))
        new_draw_info(NDI_UNIQUE, 0, op, "OK.");

    return 1;
}

int command_create(object *op, char *params)
{
    object     *tmp = NULL;
    int         nrof, i, magic, set_magic = 0, set_nrof = 0, gotquote, gotspace;
    char        buf[MAX_BUF], *cp, *bp = buf, *bp2, *bp3, *bp4 = NULL, *obp, *cp2;
    archetype  *at;
    artifact   *art = NULL;

    if (!op)
        return 0;
    if(op && CONTR(op)->gmaster_mode != GMASTER_MODE_MM)
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
    if ((cp = strstr(bp, " amask ")) != NULL)
    {
        *cp = '\0';
        cp += 7;
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
        for (cp2 = cp; *cp2; cp2++)
         if (*cp2 == ' ')
           {
            *cp2 = '\0';
            break;
           }
        if (find_artifactlist(at->clone.type) == NULL)
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "No artifact list for type %d\n", at->clone.type);
        }
        else
        {
            art = find_artifact(cp);

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

        if (at->clone.randomitems)
            create_treasure_list(at->clone.randomitems, head, GT_APPLY,
                head->type == MONSTER?head->level:get_enviroment_level(head),ART_CHANCE_UNSET, 0);

        if (IS_LIVE(head))
        {
            if(head->type == MONSTER)
                fix_monster(head);
            insert_ob_in_map(head, op->map, op, INS_NO_MERGE | INS_NO_WALK_ON);
        }
        else
        {
            head = insert_ob_in_ob(head, op);
            esrv_send_item(op, head);
        }
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
    object *tmp;

    tmp = NULL;
    if (params != NULL)
    {
        if (!strncmp(params, "me", 2))
            tmp = op;
        else if (sscanf(params, "%d", &i))
            tmp = find_object(i);
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

/* same like addexp, but we set here the skill level explicit
 * If the player don't has the skill we add it.
 * if level is 0 we remove the skill (careful!!)
*/
int command_setskill(object *op, char *params)
{
    char    buf[MAX_BUF];
    int     level, snr;
    object *exp_skill, *exp_ob;
    player *pl;

    if (params == NULL || sscanf(params, "%s %d %d", buf, &snr, &level) != 3)
    {
        int i;
        char buf[HUGE_BUF];

        sprintf(buf, "Usage: setskill [who] [skill nr] [level]\nSkills/Nr: ");
        for(i=0;i<NROFSKILLS;i++)
            sprintf(buf,"%s,%s(%d)", buf,  skills[i].name,i);
        new_draw_info(NDI_UNIQUE, 0, op, buf);
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
    if (!exp_skill) /* we don't have the skill - learn it*/
    {
        learn_skill(op, NULL, NULL, snr, 0);
        exp_skill = pl->skill_ptr[snr];
        FIX_PLAYER(op, "setskill");
    }
    else if(!level)/* if level is 0 we unlearn the skill! */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "removed skill!");
        exp_ob = exp_skill->exp_obj;
        remove_ob(exp_skill);
        player_lvl_adj(op, exp_ob, TRUE);
        player_lvl_adj(op, NULL, TRUE);
        FIX_PLAYER(op,"setskill");
        return 1;
    }

    if (!exp_skill) /* safety check */
    {

        /* our player don't have this skill?
         * This can happens when group exp is devided.
         * We must get a useful sub or we skip the exp.
         */
        new_draw_info(NDI_UNIQUE, 0, op, "No such skill!");
        LOG(llevDebug, "TODO: command_setskill(): called for %s with skill nr %d / %d level - object has not this skill.\n",
            query_name(op), snr, level);
        return 0; /* TODO: groups comes later  - now we skip all times */
    }

    pl->update_skills = 1; /* we will sure change skill exp, mark for update */
    exp_ob = exp_skill->exp_obj;

    if (!exp_ob)
    {
        LOG(llevBug, "BUG: add_exp() skill:%s - no exp_op found!!\n", query_name(exp_skill));
        return 0;
    }

    exp_skill->level = level;
    exp_skill->stats.exp =  new_levels[level];

    /* adjust_exp has adjust the skill and all exp_obj and player exp */
    /* now lets check for level up in all categories */
    adjust_exp(op, exp_skill, 1); /* we add one more so we get a clean call here */
    player_lvl_adj(op, exp_skill, TRUE);
    player_lvl_adj(op, exp_ob, TRUE);
    player_lvl_adj(op, NULL, TRUE);
    FIX_PLAYER(op,"setskill");

    return 1;
}

int command_addexp(object *op, char *params)
{
    char    buf[MAX_BUF];
    int     exp, snr;
    object *exp_skill, *exp_ob;
    player *pl;
    if(op && CONTR(op)->gmaster_mode != GMASTER_MODE_MM)
        return 0;

    if (params == NULL || sscanf(params, "%s %d %d", buf, &snr, &exp) != 3)
    {
        int i;
        char buf[HUGE_BUF];

        sprintf(buf, "Usage: addexp [who] [skill nr] [exp]\nSkills/Nr: ");
        for(i=0;i<NROFSKILLS;i++)
            sprintf(buf,"%s%s(%d), ", buf,  skills[i].name,i);
        new_draw_info(NDI_UNIQUE, 0, op, buf);
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
    player_lvl_adj(pl->ob, exp_skill, TRUE);
    player_lvl_adj(pl->ob, exp_ob, TRUE);
    player_lvl_adj(pl->ob, NULL, TRUE);

    return 1;
}

int command_speed(object *op, char *params)
{
    long i;
    if(op && CONTR(op)->gmaster_mode != GMASTER_MODE_MM)
        return 0;
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
    FIX_PLAYER(pl->ob ,"command abil");
    return 1;
}

int command_reset(object *op, char *params)
{
    int             count;
    mapstruct      *m;
    player         *pl;
    if(op && CONTR(op)->gmaster_mode != GMASTER_MODE_MM)
        return 0;

    if (params == NULL)
        m = has_been_loaded_sh(op->map->path);
    else
    {
        const char *mapfile_sh = add_string(params);
        m = has_been_loaded_sh(mapfile_sh);
        free_string_shared(mapfile_sh);
    }

    if (m == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such map.");
        return 1;
    }

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

                /* With the new activelist, any player on a reset map
                 * was somehow forgotten. This seems to fix it. The
                 * problem isn't analyzed, though. Gecko 20050713 */
                activelist_remove(pl->ob);
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
        FREE_AND_NULL_PTR(m->tmpname);
        /* setting this effectively causes an immediate reload */
        m->reset_time = 1;
        new_draw_info(NDI_UNIQUE, 0, op, "Swap successful. Inserting players.");

        m = ready_map_name(m->path, m->orig_path, MAP_STATUS_TYPE(m->map_status), m->reference);

        for (pl = first_player; pl != NULL; pl = pl->next)
        {
            if (pl->dm_removed_from_map)
            {
                insert_ob_in_map(pl->ob, m, NULL, INS_NO_MERGE);
                activelist_insert(pl->ob); /* See activelist comment above */
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
    char name[256]="";
    int seconds=0;
    player *pl;
    objectlink *ol;

    if (!params)
        return 0;

    if(CONTR(op)->gmaster_mode == GMASTER_MODE_NO)
        return 0;

    sscanf(params, "%s %d", name, &seconds);
    pl= find_player(name);
    if(pl == NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "mute command: can't find player %s", name);
        return 0;
    }
    LOG(llevInfo, "MUTECMD: %s issued /mute %s %d\n", op->name, name, seconds);

    if(seconds<0)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "mute command: illegal seconds parameter (%d)", seconds);
        return 0;
    }

    if(!seconds) /* unmute player */
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "mute command: unmuting player %s!", name);
        pl->mute_counter = 0;
    }
    else
    {
        pl->mute_counter = pticks+seconds*(1000000/MAX_TIME);
        for(ol = gmaster_list_MW;ol;ol=ol->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob,
            "MUTE: Player %s has been muted by %s for %d seconds.\n", query_name(pl->ob), query_name(op), seconds);
        for(ol = gmaster_list_VOL;ol;ol=ol->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob,
            "MUTE: Player %s has been muted by %s for %d seconds.\n", query_name(pl->ob), query_name(op), seconds);
        for(ol = gmaster_list_GM;ol;ol=ol->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob,
            "MUTE: Player %s has been muted by %s for %d seconds.\n", query_name(pl->ob), query_name(op), seconds);
        for(ol = gmaster_list_MM;ol;ol=ol->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ol->objlink.ob,
            "MUTE: Player %s has been muted by %s for %d seconds.\n", query_name(pl->ob), query_name(op), seconds);

    }

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

static void add_banlist_ip(object* op, char *ip, int ticks)
{
    objectlink *ol, *ol_tmp, *ob;

    for(ol = ban_list_ip;ol;ol=ol_tmp)
    {
            if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL && ol->objlink.ban->ticks_init == -1)
             {
                new_draw_info_format(NDI_UNIQUE, 0, op, "Only GMs and DMs can unban permanently banned!");
                return;
             }
        ol_tmp = ol->next;
        if(!strcmp(ol->objlink.ban->ip, ip))
            remove_ban_entry(ol);
    }
    new_draw_info_format(NDI_UNIQUE, 0, op, "IP %s is now banned for %d seconds.", ip, ticks/8);
    LOG(llevSystem, "IP %s is now banned for %d seconds.\n", ip, ticks/8);
    add_ban_entry(NULL, ip, ticks, ticks);
    for(ob = gmaster_list_VOL;ob;ob=ob->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
            "BAN: IP %s has been banned by %s for %d seconds.\n", ip, query_name(op), ticks/8);
    for(ob = gmaster_list_GM;ob;ob=ob->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
            "BAN: IP %s has been banned by %s for %d seconds.\n", ip, query_name(op), ticks/8);
    for(ob = gmaster_list_MM;ob;ob=ob->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
            "BAN: IP %s has been banned by %s for %d seconds.\n", ip, query_name(op), ticks/8);
}

static void add_banlist_name(object* op, char *name, int ticks)
{
    objectlink *ol, *ol_tmp, *ob;
    const char *name_hash;

    transform_name_string(name);

    name_hash = find_string(name); /* we need an shared string to check ban list */
    for(ol = ban_list_player;ol;ol=ol_tmp)
    {
            if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL && ol->objlink.ban->ticks_init == -1)
             {
                new_draw_info_format(NDI_UNIQUE, 0, op, "Only GMs and DMs can unban permanently banned!");
                return;
             }
        ol_tmp = ol->next;
        if(ol->objlink.ban->name == name_hash)
            remove_ban_entry(ol);
    }
    new_draw_info_format(NDI_UNIQUE, 0, op, "Player %s is now banned for %d seconds.", name, ticks/8);
    LOG(llevSystem,"Player %s is now banned for %d seconds.\n", name, ticks/8);
    add_ban_entry(name, NULL, ticks, ticks);
    for(ob = gmaster_list_VOL;ob;ob=ob->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
            "BAN: Player %s has been banned by %s for %d seconds.\n", name, query_name(op), ticks/8);
    for(ob = gmaster_list_GM;ob;ob=ob->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
            "BAN: Player %s has been banned by %s for %d seconds.\n", name, query_name(op), ticks/8);
    for(ob = gmaster_list_MM;ob;ob=ob->next)
        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
            "BAN: Player %s has been banned by %s for %d seconds.\n", name, query_name(op), ticks/8);

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
    objectlink *ol, *ol_tmp, *ob;
    player *pl;
    char *name, name_buf[MAX_BUF]="";
    int ticks=0;
    char *str;

    if(CONTR(op)->gmaster_mode < GMASTER_MODE_VOL)
        return 0;

    if (!params)
        goto ban_usage;

    /* list all entries of gmaster_file
     */
    if(!strcmp(params,"list"))
    {

        new_draw_info(NDI_UNIQUE, 0, op, "ban list");
        new_draw_info(NDI_UNIQUE, 0, op, "--- --- ---");
        for(ol = ban_list_player;ol;ol=ol_tmp)
        {
            ol_tmp = ol->next;
            if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
                remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
            else
                new_draw_info_format(NDI_UNIQUE, 0, op, "%s -> %d left (of %d) sec",
                    ol->objlink.ban->name ,(ol->objlink.ban->ticks-pticks)/8, ol->objlink.ban->ticks_init/8);
        }

        for(ol = ban_list_ip;ol;ol=ol_tmp)
        {
            ol_tmp = ol->next;
            if(ol->objlink.ban->ticks_init != -1 &&  pticks >= ol->objlink.ban->ticks)
                remove_ban_entry(ol); /* is not valid anymore, gc it on the fly */
            else
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "%s :: %d left (of %d) sec",
                        ol->objlink.ban->ip, (ol->objlink.ban->ticks-pticks)/8, ol->objlink.ban->ticks_init/8);
            }
        }
        return 1;
    }
    if (!(str = strchr(params, ' ')))
        goto ban_usage;

    /* kill the space, and set string to the next param */
    *str++ = '\0';

    if(!strcmp(params,"name")) /* ban name */
    {
        if (sscanf(str, "%s %d", name_buf, &ticks) == 2)
        {
            name = cleanup_string((name = name_buf));
            if(name && name[0]!='\0')
            {
                if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL && ticks == -1)
                {
                    goto ban_usage;
                }

                if (ticks != -1)
                    ticks *= 8;     /* convert seconds to ticks */

                add_banlist_name(op, name, ticks);
                save_ban_file();
                command_kick(op, name);
                return 1;
            }
        }
        else goto ban_usage;
    }
    else if(!strcmp(params,"ip")) /* ban IP only */
    {
        if (sscanf(str, "%s %d", name_buf, &ticks) == 2)
        {
            name = cleanup_string((name = name_buf));
            if(name && name[0]!='\0')
            {
                int spot;
                if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL && ticks == -1)
                    goto ban_usage;

                for(spot=0;name[spot] != '\0';spot++)
                {
                    if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL && name[spot] == '*')
                        goto ban_usage;
                    else if(CONTR(op)->gmaster_mode >= GMASTER_MODE_GM && name[spot] == '*' && spot < 11)
                        goto ban_usage;
                }

                if (ticks != -1)
                    ticks *= 8;     /* convert seconds to ticks */

                add_banlist_ip(op, name, ticks);
                save_ban_file();
                return 1;
            }
        }
        else goto ban_usage;
    }
    else if(!strcmp(params,"add")) /* ban name & IP at once */
    {
        if (sscanf(str, "%s %d", name_buf, &ticks) == 2)
        {
            name = cleanup_string((name = name_buf));
            pl = find_player(name);

            if(!pl || !pl->ob)
            {
                new_draw_info_format(NDI_UNIQUE, 0, op,
                    "/ban: can't find the player %s!\nCheck name or use /ban <name><ip> direct.", STRING_SAFE(name));
            }
            else
            {
                if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL && ticks == -1)
                {
                    goto ban_usage;
                }

                if (ticks != -1)
                    ticks *= 8;

                /* add name AND ip to the lists */
                add_banlist_name(op, name, ticks);
                add_banlist_ip(op, pl->socket.ip_host, ticks);
                save_ban_file();
                LOG(llevInfo, "BANCMD: %s issued /ban add %s %s %d seconds\n", op->name, name, name_buf, ticks/8);
                for(ob = gmaster_list_VOL;ob;ob=ob->next)
                    new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
                        "BAN: Player %s and IP %s have been banned by %s for %d seconds.\n",
                        query_name(pl->ob), pl->socket.ip_host, op->name, ticks/8);
                for(ob = gmaster_list_GM;ob;ob=ob->next)
                    new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
                        "BAN: Player %s and IP %s have been banned by %s for %d seconds.\n",
                        query_name(pl->ob), pl->socket.ip_host, op->name, ticks/8);
                for(ob = gmaster_list_MM;ob;ob=ob->next)
                    new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
                        "BAN: Player %s and IP %s have been banned by %s for %d seconds.\n",
                        query_name(pl->ob), pl->socket.ip_host, op->name, ticks/8);
                command_kick(op, query_name(pl->ob));
            }
            return 1;

        }
        else goto ban_usage;
    }
    else if(!strcmp(params,"remove"))
    {
        name = cleanup_string(str);

        if(name && name[0]!='\0')
        {
            const char *name_hash;
            int success = FALSE, sent = FALSE;

            for(ol = ban_list_ip;ol;ol=ol_tmp)
            {
                ol_tmp = ol->next;
                if(!strcmp(ol->objlink.ban->ip, name))
                {
                    if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL && ol->objlink.ban->ticks_init == -1)
                    {
                        new_draw_info_format(NDI_UNIQUE, 0, op, "Only GMs and DMs can unban permanently banned!");
                        return 0;
                    }
                    LOG(llevSystem,"/ban: %s unbanned the IP %s\n", query_name(op), name);
                    new_draw_info_format(NDI_UNIQUE, 0, op, "You unbanned IP %s!", name);
                    for(ob = gmaster_list_VOL;ob;ob=ob->next)
                        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
                        "BAN: IP %s has been unbanned by %s.\n", name, op->name);
                       if(strcmp(query_name(ob->objlink.ob),query_name(op)))
                        sent = TRUE;
                    for(ob = gmaster_list_GM;ob;ob=ob->next)
                       if(!sent)
                        new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
                        "BAN: IP %s has been unbanned by %s.\n", name, op->name);
                       if(strcmp(query_name(ob->objlink.ob),query_name(op)))
                        sent = TRUE;
                    for(ob = gmaster_list_MM;ob;ob=ob->next)
                      if(!sent)
                       new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
                        "BAN: IP %s has been unbanned by %s.\n", name, op->name);
                    remove_ban_entry(ol);
                    success = TRUE;
                }
            }

            transform_name_string(name);
            name_hash = find_string(name); /* we need an shared string to check ban list */

            for(ol = ban_list_player;ol;ol=ol_tmp)
            {
                ol_tmp = ol->next;
                if(ol->objlink.ban->name == name_hash)
                {
                   if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL && ol->objlink.ban->ticks_init == -1)
                    {
                      new_draw_info_format(NDI_UNIQUE, 0, op, "Only GMs and DMs can unban permanently banned!");
                      return 0;
                    }
                    LOG(llevSystem,"/ban: %s unbanned the player %s\n", query_name(op), name);
                    new_draw_info_format(NDI_UNIQUE, 0, op, "You unbanned player %s!", name);
                  for(ob = gmaster_list_VOL;ob;ob=ob->next)
                    new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
                        "BAN: Player %s has been unbanned by %s.\n", name, op->name);
                    if(strcmp(query_name(ob->objlink.ob),query_name(op)))
                     sent = TRUE;
                  for(ob = gmaster_list_GM;ob;ob=ob->next)
                   if(!sent)
                    new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
                        "BAN: Player %s has been unbanned by %s.\n", name, op->name);
                       if(strcmp(query_name(ob->objlink.ob),query_name(op)))
                        sent = TRUE;
                  for(ob = gmaster_list_MM;ob;ob=ob->next)
                   if(!sent)
                    new_draw_info_format(NDI_PLAYER | NDI_UNIQUE | NDI_RED, 0, ob->objlink.ob,
                        "BAN: Player %s has been unbanned by %s.\n", name, op->name);
                    remove_ban_entry(ol);
                    success = TRUE;
                }
            }
            if(success)
                save_ban_file();
            return 1;
        }
    }

ban_usage:
    new_draw_info(NDI_UNIQUE, 0, op, "Usage: /ban  list");
    new_draw_info(NDI_UNIQUE, 0, op, "Usage: /ban  remove <name or ip>");
    new_draw_info(NDI_UNIQUE, 0, op, "Usage: /ban  name <name> <second>");
    new_draw_info(NDI_UNIQUE, 0, op, "Usage: /ban  ip <IP> <second>");
    new_draw_info(NDI_UNIQUE, 0, op, "Usage: /ban  add <player name> <second>");
    new_draw_info(NDI_UNIQUE, 0, op, "Note: /ban add will ban ip AND name. Player must be still online.");
    return 1;
}

/* become a MW */
int command_mw(object *op, char *params)
{
    if(CONTR(op)->gmaster_mode == GMASTER_MODE_MW) /* turn off ? */
        remove_gmaster_mode(CONTR(op));
    else if(check_gmaster_list(CONTR(op), GMASTER_MODE_MW))
    {
        /* remove from other lists when we change mode */
        if(CONTR(op)->gmaster_mode != GMASTER_MODE_NO)
            remove_gmaster_mode(CONTR(op));
        set_gmaster_mode(CONTR(op), GMASTER_MODE_MW);
    }
    return 1;
}

/* become a VOL */
int command_vol(object *op, char *params)
{
    if(CONTR(op)->gmaster_mode == GMASTER_MODE_VOL) /* turn off ? */
        remove_gmaster_mode(CONTR(op));
    else if(check_gmaster_list(CONTR(op), GMASTER_MODE_VOL))
    {
        /* remove from other lists when we change mode */
        if(CONTR(op)->gmaster_mode != GMASTER_MODE_NO)
            remove_gmaster_mode(CONTR(op));
        set_gmaster_mode(CONTR(op), GMASTER_MODE_VOL);
    }

    return 1;
}

/* become a GM */
int command_gm(object *op, char *params)
{
    if(CONTR(op)->gmaster_mode == GMASTER_MODE_GM) /* turn off ? */
        remove_gmaster_mode(CONTR(op));
    else if(check_gmaster_list(CONTR(op), GMASTER_MODE_GM))
    {
        /* remove from other lists when we change mode */
        if(CONTR(op)->gmaster_mode != GMASTER_MODE_NO)
            remove_gmaster_mode(CONTR(op));
        set_gmaster_mode(CONTR(op), GMASTER_MODE_GM);
    }

    return 1;
}

/* Actual command to perhaps become dm.  Changed around a bit in version 0.92.2
 * - allow people on sockets to become dm, and allow better dm file */
int command_mm(object *op, char *params)
{
    if(CONTR(op)->gmaster_mode == GMASTER_MODE_MM) /* turn off ? */
        remove_gmaster_mode(CONTR(op));
    else if(check_gmaster_list(CONTR(op), GMASTER_MODE_MM))
    {
        /* remove from other lists when we change mode */
        if(CONTR(op)->gmaster_mode != GMASTER_MODE_NO)
            remove_gmaster_mode(CONTR(op));
        set_gmaster_mode(CONTR(op), GMASTER_MODE_MM);
    }

    return 1;
}

/* list all active GM/VOL/DM
 * gmaster actives only */
int command_dm_list(object *op, char *params)
{
    objectlink *ol;

    if(CONTR(op)->gmaster_mode == GMASTER_MODE_NO)
        return 0;

    new_draw_info(NDI_UNIQUE, 0, op, "MM/GM/VOL online");
    new_draw_info(NDI_UNIQUE, 0, op, "--- --- ---");
    for(ol = gmaster_list_MW;ol;ol=ol->next)
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s (%d)", CONTR(ol->objlink.ob)->quick_name, ol->objlink.ob->level);

    for(ol = gmaster_list_VOL;ol;ol=ol->next)
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s (%d)", CONTR(ol->objlink.ob)->quick_name, ol->objlink.ob->level);

    for(ol = gmaster_list_GM;ol;ol=ol->next)
        new_draw_info_format(NDI_UNIQUE, 0, op, "%s (%d)", CONTR(ol->objlink.ob)->quick_name, ol->objlink.ob->level);

    for(ol = gmaster_list_MM;ol;ol=ol->next)
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


    if(CONTR(op)->gmaster_mode != GMASTER_MODE_MM)
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

int command_gm_set(object *op, char *params)
{
    char *str;


    if(CONTR(op)->gmaster_mode < GMASTER_MODE_GM)
        return 0;

    if (!params)
        goto g_set_usage;

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
        goto g_set_usage;


    /* kill the space, and set string to the next param */
    *str++ = '\0';

    if(!strcmp(params,"add"))
    {
        char name[MAX_BUF], passwd[MAX_BUF], host[MAX_BUF], mode[MAX_BUF], dummy[MAX_BUF];

        if (sscanf(str, "%[^:]:%[^:]:%[^:]:%s%[\n\r]", name, passwd, host, mode, dummy) < 3)
            new_draw_info(NDI_UNIQUE, 0, op, "/gm_set: invalid parameter.");
        else
        {
            int mode_id = check_gmaster_file_entry(name, passwd, host, mode);

            if(mode_id == GMASTER_MODE_NO || mode_id == GMASTER_MODE_MM)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "/gm_set: invalid parameter.");
                return 1;
            }

            /* all ok, setup the gmaster node and add it to our list */
            LOG(llevSystem, "GMASTER:: /gm_set add %s invoked by %s\n", str, query_name(op));
            new_draw_info_format(NDI_UNIQUE, 0, op, "/gm_set: add entry %s", str);
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
               char name[MAX_BUF], passwd[MAX_BUF], host[MAX_BUF], mode[MAX_BUF];
               int mode_id = check_gmaster_file_entry(name, passwd, host, mode);
              if(mode_id < GMASTER_MODE_MM)
              {
                /* delete the entry... */
                LOG(llevSystem, "GMASTER:: /gm_set remove %s invoked by %s\n", str, query_name(op));

                new_draw_info_format(NDI_UNIQUE, 0, op, "/gm_set: remove entry %s", str);
                remove_gmaster_file_entry(ol);
                new_draw_info(NDI_UNIQUE, 0, op, "write back gmaster_file...");
                write_gmaster_file(); /* create a new file */
                new_draw_info(NDI_UNIQUE, 0, op, "update gmaster rights...");
                update_gmaster_file(); /* control rights of all active VOL/GM/DM */
                new_draw_info(NDI_UNIQUE, 0, op, "done.");

                return 1;
               }
               else
               {
                 new_draw_info(NDI_UNIQUE, 0, op, "/gm_set: invalid parameter.");
                 return 1;
               }
            }
        }

        return 1;
    }

g_set_usage:
    new_draw_info(NDI_UNIQUE, 0, op, "Usage: /gm_set  list | add | remove <entry>");

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

    if (CONTR(op)->gmaster_mode != GMASTER_MODE_MM)
        return 1;

    if (!params) /* fix crash bug with no paramaters -- Gramlath 3/30/2007 */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Usage: plugin [file name]");
        return 1;
    }

    sprintf(buf, "%s/../plugins/%s", DATADIR, params);
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
    if (CONTR(op)->gmaster_mode != GMASTER_MODE_MM)
        return 1;

    if (!params) /* fix crash bug with no paramaters -- Gramlath 3/30/2007 */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Usage: plugout [plugin name]");
        return 1;
    }

    removeOnePlugin(params);
    return 1;
}
int command_ip(object *op, char *params)
{
    player *pl;
     if(CONTR(op)->gmaster_mode < GMASTER_MODE_VOL)
      return 0;


    pl = find_player(params);

    if(!pl)
     {
       new_draw_info(NDI_UNIQUE, 0, op, "IP of Who?");
       return 0;
     }
    else
     {
       new_draw_info_format(NDI_UNIQUE, 0, op, "IP of %s is %s", params, pl->socket.ip_host);
       return 1;
     }
}
