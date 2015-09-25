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


#include <global.h>

#define OBFUSCATE_MESSAGE(_C_, _M_) \
    for ((_C_) = (_M_); (_C_) && *(_C_) != '\0'; (_C_)++) \
    { \
        if (*(_C_) != ' ') \
        { \
            *(_C_) = '*'; \
        } \
    }

/* this function does 3 things: controlling we have
 * a legal string - if not, return NULL  - if return string*
 * - remove all whitespace in front (if all are whitespace
 *   we return NULL)
 * - change and/or process all our control chars like '^' or '§'
 */
static char * cleanup_chat_string(char *ustring)
{
    char *ptr;
    int   flag; // 0/1 whether non-space has been found

    if (!ustring)
        return NULL;

    /* kill all whitespace */
    while (*ustring != '\0' && isspace(*ustring))
        ustring++;

    /* this happens when whitespace only string was submited */
    if (!ustring || *ustring == '\0')
        return NULL;

    /* now clear all control chars */
    for (ptr = ustring, flag = 0; *ptr != '\0'; ptr++)
    {
        switch(*ptr)
        {
            case ECC_HYPERTEXT:
                *ptr = ' ';
                break;
            default:
                if (!isspace(*ptr))
                    flag = 1;
                break;
        }
    }

    return (flag) ? ustring : NULL;
}

/* check the player communication commands for a simple time based mute.
 * This will catch for example the annoying spams by key bound shouts,
 * dropped on purpose or accidently. It also will nicely limit the speed
 * of shouts & say commands and avoid emote spams.
 * Of course it don't can handle language or other abuses - for that we have
 * the mute command itself.
 */
static int check_mute(object_t *op, int mode)
{
    if(op->type != PLAYER || CONTR(op)==NULL)
        return TRUE;

    /* players less than settings.shout_lvl cannot shout due to spam problems.
     * VOLs, GMs, and SAs are exempt. */
    if (mode == MUTE_MODE_SHOUT &&
        op->level < settings.mutelevel &&
        !(CONTR(op)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM | GMASTER_MODE_VOL)))
    {
        ndi(NDI_UNIQUE | NDI_ORANGE, 0, op, "You need to be level %d or higher for shout/tell!",
                      settings.mutelevel);

        return FALSE;
    }

    if(CONTR(op)->mute_counter)
    {
        if(CONTR(op)->mute_counter <= pticks) /* its ok */
            CONTR(op)->mute_counter=0;
        else /* player is muted */
        {
            if(CONTR(op)->mute_msg_count<=pticks)
            {
                unsigned long tmp = (CONTR(op)->mute_counter-pticks)/(1000000/MAX_TIME);

                ndi( NDI_UNIQUE, 0, op, "You are still muted for %d second(s).", (int)(tmp?tmp:1));
                CONTR(op)->mute_msg_count = pticks+MUTE_MSG_FREQ;
            }
            return FALSE;
        }
    }
    else /* no old mute - lets check for new*/
    {
        if(mode == MUTE_MODE_SHOUT) /* say or shout? */
        {
            if(CONTR(op)->mute_freq_shout<=pticks) /* can we do a shout class communication ?*/
            {
                /* yes, all fine */
                CONTR(op)->mute_freq_shout=pticks+MUTE_FREQ_SHOUT;
                CONTR(op)->mute_flags &= ~(MUTE_FLAG_SHOUT|MUTE_FLAG_SHOUT_WARNING);
            }
            else /* nope - don't process the comm. action and tell the player why */
            {
                if(!(CONTR(op)->mute_flags & MUTE_FLAG_SHOUT)) /* friendly message not to spam */
                {
                    ndi( NDI_UNIQUE, 0, op, "Please wait 2 seconds between shout like commands.");
                    CONTR(op)->mute_flags |= MUTE_FLAG_SHOUT;
                    return FALSE;
                }
                else if(!(CONTR(op)->mute_flags & MUTE_FLAG_SHOUT_WARNING)) /* first & last warning */
                {
                    ndi( NDI_UNIQUE|NDI_ORANGE, 0, op, "Auto-Mute Warning: Please wait 2 seconds!");
                    CONTR(op)->mute_flags |= MUTE_FLAG_SHOUT_WARNING;
                    return FALSE;
                }
                else /* mute him */
                {
                    ndi( NDI_UNIQUE|NDI_RED, 0, op, "Auto-Mute: Don't spam! You are muted for %d seconds!",(int)(MUTE_AUTO_NORMAL/(1000000/MAX_TIME)));
                    CONTR(op)->mute_counter = pticks+MUTE_AUTO_NORMAL;
                    return FALSE;
                }
            }
        }
        else
        {
            if(CONTR(op)->mute_freq_say<=pticks) /* can we do a say class command? (say/emote)*/
            {
                /* yes, all fine */
                CONTR(op)->mute_freq_say=pticks+MUTE_FREQ_SAY;
                CONTR(op)->mute_flags &= ~(MUTE_FLAG_SAY|MUTE_FLAG_SAY_WARNING);
            }
            else /* nope - don't process the comm. action and tell the player why */
            {
                if(!(CONTR(op)->mute_flags & MUTE_FLAG_SAY)) /* friendly message not to spam */
                {
                    ndi( NDI_UNIQUE, 0, op, "Please wait 2 seconds between say like commands.");
                    CONTR(op)->mute_flags |= MUTE_FLAG_SAY;
                    return FALSE;
                }
                else if(!(CONTR(op)->mute_flags & MUTE_FLAG_SAY_WARNING)) /* first & last warning */
                {
                    ndi( NDI_UNIQUE|NDI_ORANGE, 0, op, "Auto-Mute Warning: Please wait 2 seconds!");
                    CONTR(op)->mute_flags |= MUTE_FLAG_SAY_WARNING;
                    return FALSE;
                }
                else /* mute him */
                {
                    ndi( NDI_UNIQUE|NDI_RED, 0, op, "Auto-Mute: Don't spam! You are muted for %d seconds!",(int)(MUTE_AUTO_NORMAL/(1000000/MAX_TIME)));
                    CONTR(op)->mute_counter = pticks+MUTE_AUTO_NORMAL;
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}


int command_say(object_t *op, char *params)
{
    if(!check_mute(op, MUTE_MODE_SAY))
        return 0;

    /* this happens when whitespace only string was submited */
    if (!params || !(params = cleanup_chat_string(params)))
        return 1;

    CHATLOG("SAY:%s >%s<\n", STRING_OBJ_NAME(op), params);
    communicate(op, params);

    return 0;
}


int command_gsay(object_t *op, char *params)
{
    char            buf[MEDIUM_BUF],
                   *cp;
    object_t         *member;
#if 0 // def USE_CHANNELS
    sockbuf_struct *sb;
#endif

    if (!check_mute(op, MUTE_MODE_SAY))
    {
        return 0;
    }

    if (!(CONTR(op)->group_status & GROUP_STATUS_GROUP) ||
        !params ||
        !(params = cleanup_chat_string(params)))
    {
        return 1;
    }

/* Disabled because 0.10.5 and earlier clients do not log channel chat and it
 * is necessary that clients keep gsay logs as the server no longer does
 * (r7282, 0.10.5-y). */
#if 0 // def USE_CHANNELS
    sprintf(buf, "%c%cGroup %s:%s",
            2, NDI_YELLOW, QUERY_SHORT_NAME(op, NULL), params);
    sb = SOCKBUF_COMPOSE(SERVER_CMD_CHANNELMSG, buf, strlen(buf + 2) + 2, 0);

    for(member = CONTR(op)->group_leader; member; member = CONTR(member)->group_next)
    {
        SOCKBUF_ADD_TO_SOCKET(&CONTR(member)->socket, sb);
    }

    SOCKBUF_COMPOSE_FREE(sb);
#else
    sprintf(buf, " gsays: %s", params);

    for(member = CONTR(op)->group_leader; member; member = CONTR(member)->group_next)
    {
        ndi(NDI_GSAY | NDI_PLAYER | NDI_UNIQUE | NDI_YELLOW, 0, member, "%s %s",
            QUERY_SHORT_NAME(op, member), buf);
    }
#endif

    OBFUSCATE_MESSAGE(cp, params);
    CHATLOG("GSAY:%s >%s<\n", STRING_OBJ_NAME(op), params);

    return 0;
}

#ifndef USE_CHANNELS
int command_shout(object_t *op, char *params)
{
#ifdef PLUGINS
    int     evtid;
    CFParm  CFP;
#endif

    if(!check_mute(op, MUTE_MODE_SHOUT))
        return 0;

    /* this happens when whitespace only string was submited */
    if (!params || !(params = cleanup_chat_string(params)))
        return 1;

    /* moved down, cause if whitespace is shouted, then no need to log it */
    CHATLOG("SHOUT:%s >%s<\n", STRING_OBJ_NAME(op), params);

    ndi(NDI_SHOUT | NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_ORANGE, 1, NULL, "%s shouts: %s",
        QUERY_SHORT_NAME(op, NULL), params);

#ifdef PLUGINS
    /* GROS : Here we handle the SHOUT global event */
    evtid = EVENT_SHOUT;
    CFP.Value[0] = (void *) (&evtid);
    CFP.Value[1] = (void *) (op);
    CFP.Value[2] = (void *) (params);
    GlobalEvent(&CFP);
#endif

    return 0;
}
#else
// Redirect /shouts to the new B5 channel system.
int command_shout(object_t *op, char *params)
{
    char *newparams;

    /* this happens when whitespace only string was submited */
    if (!params ||
        !(params = cleanup_chat_string(params)))
    {
        return 1;
    }

    MALLOC(newparams, strlen(params) + 9);
    sprintf(newparams, "general %s", params);
    command_channel(op, newparams);
    FREE(newparams);

    return 0;
}
#endif

#ifndef USE_CHANNELS
int command_describe(object_t *op, char *params)
{
    /* TODO: Make this support channels, like "/describe tell XYZ", "/describe auction".
     * ATM shout is all that's supported.
     */

    if(!check_mute(op, MUTE_MODE_SHOUT))
        return 0;

    object_t *targetob = CONTR(op)->mark;

    if (!targetob) // Don't do anything if there is no marked item.
        return 0;

    ndi(NDI_SHOUT | NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_ORANGE, 1, NULL, "%s describes: %s (examine worth: %s)",
        QUERY_SHORT_NAME(op, NULL),
        query_name(targetob, op, targetob->nrof, 1),
        cost_string_from_value(targetob->value, COSTSTRING_SHORT));

    return 0;
}
#endif

int command_tell(object_t *op, char *params)
{
    char   *msg;
    player_t *pl;

    if (!check_mute(op, MUTE_MODE_SHOUT))
    {
        return 0;
    }
    else if (!params ||
             !(params = cleanup_chat_string(params)) ||
             !(msg = strchr(params, ' ')))
    {
        return 1;
    }

    *(msg++) = '\0';

    if (!(pl = find_player(params)))
    {
        ndi(NDI_UNIQUE, 0, op, "No such player.");
    }
    else if (pl->ob == op)
    {
        ndi(NDI_UNIQUE, 0, op, "You tell yourself the news. Very smart.");
    }
    else
    {
        char *cp;

        /* If pl has requested privacy we send the msg but we don't reveal his
         * presence, EXCEPT to VOLs, GMs, and SAs, UNLESS he is an SA! */
        if (pl->privacy)
        {
            if ((CONTR(op)->gmaster_mode & GMASTER_MODE_SA) ||
                (!(pl->gmaster_mode & GMASTER_MODE_SA) &&
                 (CONTR(op)->gmaster_mode & (GMASTER_MODE_GM | GMASTER_MODE_VOL))))
            {
                ndi(NDI_PLAYER | NDI_UNIQUE, 0, op, "You tell |%s| (~privacy mode~): %s",
                    pl->quick_name, msg);
            }
            else
            {
                ndi(NDI_UNIQUE, 0, op, "No such player.");
            }

            ndi(NDI_TELL | NDI_PLAYER | NDI_UNIQUE | NDI_NAVY, 0, pl->ob, "%s tells you (~privacy mode~): %s",
                QUERY_SHORT_NAME(op, pl->ob), msg);
        }
        else
        {
            ndi(NDI_PLAYER | NDI_UNIQUE, 0, op, "You tell |%s|: %s",
                pl->quick_name, msg);
            ndi(NDI_TELL | NDI_PLAYER | NDI_UNIQUE | NDI_NAVY, 0, pl->ob, "%s tells you: %s",
                QUERY_SHORT_NAME(op, pl->ob), msg);
        }

        OBFUSCATE_MESSAGE(cp, msg);
        CHATLOG("TELL:%s >%s %s<\n",
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(pl->ob), msg);
    }

    return 0;
}

static void emote_other(object_t *op, object_t *target, char *str, char *buf, char *buf2, char *buf3, int emotion)
{
    const char *name = str;
    char *who, *own;

    if(QUERY_FLAG(op, FLAG_IS_MALE) == QUERY_FLAG(op, FLAG_IS_FEMALE))
    {
        own = "their";
        who = "they ARE";
    }
    else if (QUERY_FLAG(op, !FLAG_IS_MALE) == QUERY_FLAG(op, !FLAG_IS_FEMALE))
    {
        own = "its";
        who = "it IS";
    }
    else if (QUERY_FLAG(op, FLAG_IS_MALE))
    {
        own = "his";
        who = "he IS";
    }
    else
    {
       own = "her";
       who = "she IS";
    }

    if (target && target->name)
        name = target->name;



#if 0
    /* Only GMs and SAs can emote stealthed SAs. */
    if ((target->type == PLAYER &&
         CONTR(target) &&
         CONTR(target)->privacy) &&
        (op->type == PLAYER &&
         CONTR(op) &&
         (CONTR(op)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM))))
       {
          ndi(NDI_UNIQUE, 0, op, "No such player.");
          return;
       }
#endif

    switch (emotion)
    {
        case EMOTE_NOD:
          sprintf(buf, "You nod solemnly to %s.", name);
          sprintf(buf2, "%s nods solemnly to you.", op->name);
          sprintf(buf3, "%s nods solemnly to %s.", op->name, name);
          break;
        case EMOTE_DANCE:
          sprintf(buf, "You grab %s and begin doing the Cha-Cha!", name);
          sprintf(buf2, "%s grabs you, and begins dancing!", op->name);
          sprintf(buf3, "Yipe! %s and %s are doing some really dirty dancing!", op->name, name);
          break;
        case EMOTE_KISS:
          sprintf(buf, "You kiss %s.", name);
          sprintf(buf2, "%s kisses you.", op->name);
          sprintf(buf3, "%s kisses %s.", op->name, name);
          break;
        case EMOTE_BOUNCE:
          sprintf(buf, "You bounce around the room with %s.", name);
          sprintf(buf2, "%s bounces around the room with you.", op->name);
          sprintf(buf3, "%s bounces around the room with %s.", op->name, name);
          break;
        case EMOTE_SMILE:
          sprintf(buf, "You smile at %s.", name);
          sprintf(buf2, "%s smiles at you.", op->name);
          sprintf(buf3, "%s beams a smile at %s.", op->name, name);
          break;
        case EMOTE_LAUGH:
          sprintf(buf, "You take one look at %s and fall down " "laughing.", name);
          sprintf(buf2, "%s looks at you and falls down on the " "ground laughing.", op->name);
          sprintf(buf3, "%s looks at %s and falls down on the " "ground laughing.", op->name, name);
          break;
        case EMOTE_SHAKE:
          sprintf(buf, "You shake %s's hand.", name);
          sprintf(buf2, "%s shakes your hand.", op->name);
          sprintf(buf3, "%s shakes %s's hand.", op->name, name);
          break;
        case EMOTE_PUKE:
          sprintf(buf, "You puke on %s.", name);
          sprintf(buf2, "%s pukes on your clothes!", op->name);
          sprintf(buf3, "%s pukes on %s.", op->name, name);
          break;
        case EMOTE_HUG:
          sprintf(buf, "You hug %s.", name);
          sprintf(buf2, "%s hugs you.", op->name);
          sprintf(buf3, "%s hugs %s.", op->name, name);
          break;
        case EMOTE_CRY:
          sprintf(buf, "You cry on %s's shoulder.", name);
          sprintf(buf2, "%s cries on your shoulder.", op->name);
          sprintf(buf3, "%s cries on %s's shoulder.", op->name, name);
          break;
        case EMOTE_POKE:
          sprintf(buf, "You poke %s in the ribs.", name);
          sprintf(buf2, "%s pokes you in the ribs.", op->name);
          sprintf(buf3, "%s pokes %s in the ribs.", op->name, name);
          break;
        case EMOTE_ACCUSE:
          sprintf(buf, "You look accusingly at %s.", name);
          sprintf(buf2, "%s looks accusingly at you.", op->name);
          sprintf(buf3, "%s looks accusingly at %s.", op->name, name);
          break;
        case EMOTE_GRIN:
          sprintf(buf, "You grin at %s.", name);
          sprintf(buf2, "%s grins evilly at you.", op->name);
          sprintf(buf3, "%s grins evilly at %s.", op->name, name);
          break;
        case EMOTE_BOW:
          sprintf(buf, "You bow before %s.", name);
          sprintf(buf2, "%s bows before you.", op->name);
          sprintf(buf3, "%s bows before %s.", op->name, name);
          break;
        case EMOTE_FROWN:
          sprintf(buf, "You frown darkly at %s.", name);
          sprintf(buf2, "%s frowns darkly at you.", op->name);
          sprintf(buf3, "%s frowns darkly at %s.", op->name, name);
          break;
        case EMOTE_GLARE:
          sprintf(buf, "You glare icily at %s.", name);
          sprintf(buf2, "%s glares icily at you, you feel cold to" " your bones.", op->name);
          sprintf(buf3, "%s glares at %s.", op->name, name);
          break;
        case EMOTE_LICK:
          sprintf(buf, "You lick %s.", name);
          sprintf(buf2, "%s licks you.", op->name);
          sprintf(buf3, "%s licks %s.", op->name, name);
          break;
        case EMOTE_SHRUG:
          sprintf(buf, "You shrug at %s.", name);
          sprintf(buf2, "%s shrugs at you.", op->name);
          sprintf(buf3, "%s shrugs at %s.", op->name, name);
          break;
        case EMOTE_SLAP:
          sprintf(buf, "You slap %s.", name);
          sprintf(buf2, "%s slapped you!", op->name);
          sprintf(buf3, "%s slaps %s.", op->name, name);
          break;
        case EMOTE_SNEEZE:
          sprintf(buf, "You sneeze at %s and a film of snot shoots" " onto him.", name);
          sprintf(buf2, "%s sneezes on you, you feel the snot cover" " you. EEEEEEW.", op->name);
          sprintf(buf3, "%s sneezes on %s and a film of snot covers" " him.", op->name, name);
          break;
        case EMOTE_SNIFF:
          sprintf(buf, "You sniff %s.", name);
          sprintf(buf2, "%s sniffs you.", op->name);
          sprintf(buf3, "%s sniffs %s", op->name, name);
          break;
        case EMOTE_SPIT:
          sprintf(buf, "You spit on %s.", name);
          sprintf(buf2, "%s spits in your face!", op->name);
          sprintf(buf3, "%s spits in %s's face.", op->name, name);
          break;
        case EMOTE_THANK:
          sprintf(buf, "You thank %s heartily.", name);
          sprintf(buf2, "%s thanks you heartily.", op->name);
          sprintf(buf3, "%s thanks %s heartily.", op->name, name);
          break;
        case EMOTE_WAVE:
          sprintf(buf, "You wave goodbye to %s.", name);
          sprintf(buf2, "%s waves goodbye to you. Have a good" " journey.", op->name);
          sprintf(buf3, "%s waves goodbye to %s.", op->name, name);
          break;
        case EMOTE_WHISTLE:
          sprintf(buf, "You whistle at %s.", name);
          sprintf(buf2, "%s whistles at you.", op->name);
          sprintf(buf3, "%s whistles at %s.", op->name, name);
          break;
        case EMOTE_WINK:
          sprintf(buf, "You wink suggestively at %s.", name);
          sprintf(buf2, "%s winks suggestively at you.", op->name);
          sprintf(buf3, "%s winks at %s.", op->name, name);
          break;
        case EMOTE_BEG:
          sprintf(buf, "You beg %s for mercy.", name);
          sprintf(buf2, "%s begs you for mercy! Show no quarter!", op->name);
          sprintf(buf3, "%s begs %s for mercy!", op->name, name);
          break;
        case EMOTE_BLEED:
          sprintf(buf, "You slash your wrist and bleed all over %s", name);
          sprintf(buf2, "%s slashes %s wrist and bleeds all over" " you.", op->name, own);
          sprintf(buf3, "%s slashes %s wrist and bleeds all " "over %s.", op->name, own, name);
          break;
        case EMOTE_CRINGE:
          sprintf(buf, "You cringe away from %s.", name);
          sprintf(buf2, "%s cringes away from you.", op->name);
          sprintf(buf3, "%s cringes away from %s in mortal terror.", op->name, name);
          break;
        default:
          sprintf(buf, "You are still nuts.");
          sprintf(buf2, "%s looks nuts. You get the distinct feeling that %s nuts.", op->name, who);
          sprintf(buf3, "%s is eyeing %s quizzically.", name, op->name);
          break;
    }
}

static void emote_self(object_t *op, char *buf, char *buf2, int emotion)
{
    char *self, *own, *who;
    if(QUERY_FLAG(op, FLAG_IS_MALE) == QUERY_FLAG(op, FLAG_IS_FEMALE))
    {
        self = "themself";
        own = "their";
        who = "they ARE";
    }
    else if (QUERY_FLAG(op, !FLAG_IS_MALE) == QUERY_FLAG(op, !FLAG_IS_FEMALE))
    {
        self = "itself";
        own = "its";
        who = "it IS";
    }
    else if (QUERY_FLAG(op, FLAG_IS_MALE))
    {
        self = "himself";
        own = "his";
        who = "he IS";
    }
    else
    {
        self = "herself";
        own = "her";
        who = "she IS";
    }

    switch (emotion)
    {
        case EMOTE_DANCE:
          sprintf(buf, "You skip and dance around by yourself.");
          sprintf(buf2, "%s embraces %s and begins to dance!", op->name, self);
          break;
        case EMOTE_LAUGH:
          sprintf(buf, "Laugh at yourself all you want, the others " "won't understand.");
          sprintf(buf2, "%s is laughing at something.", op->name);
          break;
        case EMOTE_SHAKE:
          sprintf(buf, "You are shaken by yourself.");
          sprintf(buf2, "%s shakes and quivers like a bowlful of " "jelly.", op->name);
          break;
        case EMOTE_PUKE:
          sprintf(buf, "You puke on yourself.");
          sprintf(buf2, "%s pukes on %s clothes.", op->name, own);
          break;
        case EMOTE_HUG:
          sprintf(buf, "You hug yourself.");
          sprintf(buf2, "%s hugs %s.", op->name, self);
          break;
        case EMOTE_CRY:
          sprintf(buf, "You cry to yourself.");
          sprintf(buf2, "%s sobs quietly to %s.", op->name, self);
          break;
        case EMOTE_POKE:
          sprintf(buf, "You poke yourself in the ribs, feeling very" " silly.");
          sprintf(buf2, "%s pokes %s in the ribs, looking very" " sheepish.", op->name, self);
          break;
        case EMOTE_ACCUSE:
          sprintf(buf, "You accuse yourself.");
          sprintf(buf2, "%s seems to have a bad conscience.", op->name);
          break;
        case EMOTE_BOW:
          sprintf(buf, "You kiss your toes.");
          sprintf(buf2, "%s folds up like a jackknife and kisses %s" " own toes.", op->name, own);
          break;
        case EMOTE_FROWN:
          sprintf(buf, "You frown at yourself.");
          sprintf(buf2, "%s frowns at %s.", op->name, self);
          break;
        case EMOTE_GLARE:
          sprintf(buf, "You glare icily at your feet, they are " "suddenly very cold.");
          sprintf(buf2, "%s glares at %s feet and seems bothered.", op->name, own);
          break;
        case EMOTE_LICK:
          sprintf(buf, "You lick yourself.");
          sprintf(buf2, "%s licks %s - YUCK.", op->name, self);
          break;
        case EMOTE_SLAP:
          sprintf(buf, "You slap yourself, silly you.");
          sprintf(buf2, "%s slaps %s, really strange...", op->name, self);
          break;
        case EMOTE_SNEEZE:
          sprintf(buf, "You sneeze on yourself, what a mess!");
          sprintf(buf2, "%s sneezes, and covers %s in a slimy" " substance.", op->name, self);
          break;
        case EMOTE_SNIFF:
          sprintf(buf, "You sniff yourself.");
          sprintf(buf2, "%s sniffs %s.", op->name, self);
          break;
        case EMOTE_SPIT:
          sprintf(buf, "You drool all over yourself.");
          sprintf(buf2, "%s drools all over %s.", op->name, self);
          break;
        case EMOTE_THANK:
          sprintf(buf, "You thank yourself since nobody else " "wants to!");
          sprintf(buf2, "%s thanks %s since you won't.", op->name, self);
          break;
        case EMOTE_WAVE:
          sprintf(buf, "Are you going on adventures as well??");
          sprintf(buf2, "%s waves goodbye to %s.", op->name, self);
          break;
        case EMOTE_WHISTLE:
          sprintf(buf, "You whistle while you work.");
          sprintf(buf2, "%s whistles to %s in boredom.", op->name, self);
          break;
        case EMOTE_WINK:
          sprintf(buf, "You wink at yourself?? What are you up to?");
          sprintf(buf2, "%s winks at %s - something strange " "is going on...", op->name, self);
          break;
        case EMOTE_BLEED:
          sprintf(buf, "Very impressive! You wipe your blood all " "over yourself.");
          sprintf(buf2, "%s performs some satanic ritual while " "wiping %s blood on %s.", op->name, own, self);
          break;
        default:
          sprintf(buf, "My god! is that LEGAL?");
          sprintf(buf2, "%s looks nuts. You get the distinct feeling %s nuts.", op->name, who);
          break;
    }/*case*/
}

/*
 * This function covers basic emotions a player can have.  An emotion can be
 * one of three things currently.  Directed at oneself, directed at someone,
 * or directed at nobody.  The first set is nobody, the second at someone, and
 * the third is directed at oneself.  Every emotion does not have to be
 * filled out in every category.  The default case will take care of the ones
 * that are not.  Helper functions will call basic_emote with the proper
 * arguments, translating them into commands.  Adding a new emotion can be
 * done by editing command.c and command.h.
 * [garbled 09-25-2001]
 */

static int basic_emote(object_t *op, char *params, int emotion)
{
    char    buf[HUGE_BUF] = "", buf2[HUGE_BUF] = "", buf3[HUGE_BUF] = "";
    player_t *pl;
    char *self, *own;

    if(!check_mute(op, MUTE_MODE_SAY))
        return 0;

    LOG(llevDebug, "EMOTE: %p (%s) (params: >%s<) (t: %s) %d\n", op, STRING_OBJ_NAME(op), STRING_SAFE(params),
        (op->type == PLAYER && CONTR(op) && OBJECT_VALID(CONTR(op)->target_ob, CONTR(op)->target_tag))
      ? STRING_OBJ_NAME(CONTR(op)->target_ob)
      : "NO CTRL!!",
        emotion);


    if (!params || (params = cleanup_chat_string(params)))
    {
        /* name is ok but be sure we have something like "Xxxxx" */
        if (emotion != EMOTE_ME)
        {
            /* But only capitalize on those emotes that are for sure a player name param. */
            if(op->type == PLAYER)
                transform_player_name_string(params);
        }
    }

    if (!params)
    {
        /* if we are a player with legal target, use it as target for the emote */
        if (op->type == PLAYER
         && CONTR(op)->target_ob != op
         && OBJECT_VALID(CONTR(op)->target_ob, CONTR(op)->target_tag)
         && CONTR(op)->target_ob->name)
        {
            rv_t   rv;
            RV_GET_OBJ_TO_OBJ(op, CONTR(op)->target_ob, &rv, 0);

            if (rv.distance <= 4)
            {
                emote_other(op, CONTR(op)->target_ob, NULL, buf, buf2, buf3, emotion);
                ndi(NDI_UNIQUE, 0, op, "%s", buf);
                if (CONTR(op)->target_ob->type == PLAYER)
                    ndi(NDI_EMOTE | NDI_PLAYER | NDI_UNIQUE | NDI_YELLOW,
                                  0, CONTR(op)->target_ob, "%s", buf2);
                ndi_map(NDI_EMOTE | NDI_PLAYER | NDI_YELLOW, MSP_KNOWN(op), MAP_INFO_NORMAL, op, CONTR(op)->target_ob, "%s", buf3);
                return 0;
            }
            ndi(NDI_UNIQUE, 0, op, "The target is not in range for this emote action.");
            return 0;
        }

        if(QUERY_FLAG(op, FLAG_IS_MALE) == QUERY_FLAG(op, FLAG_IS_FEMALE))
        {
            self = "itself"; /* neuter or hermaphrodite */
            own = "its";
        }
        else if (QUERY_FLAG(op, FLAG_IS_MALE))
        {
            self = "himself";
            own = "his";
        }
        else
        {
            self = "herself";
            own = "her";
        }

        switch (emotion)
        {
            case EMOTE_NOD:
              sprintf(buf, "%s nods solemnly.", op->name);
              sprintf(buf2, "You nod solemnly.");
              break;
            case EMOTE_DANCE:
              sprintf(buf, "%s expresses %s through interpretive dance.", op->name, self);
              sprintf(buf2, "You dance with glee.");
              break;
            case EMOTE_KISS:
              sprintf(buf, "%s makes a weird facial contortion", op->name);
              sprintf(buf2, "All the lonely people..");
              break;
            case EMOTE_BOUNCE:
              sprintf(buf, "%s bounces around.", op->name);
              sprintf(buf2, "BOIINNNNNNGG!");
              break;
            case EMOTE_SMILE:
              sprintf(buf, "%s smiles happily.", op->name);
              sprintf(buf2, "You smile happily.");
              break;
            case EMOTE_CACKLE:
              sprintf(buf, "%s throws back %s head and cackles with insane " "glee!", op->name, own);
              sprintf(buf2, "You cackle gleefully.");
              break;
            case EMOTE_LAUGH:
              sprintf(buf, "%s falls down laughing.", op->name);
              sprintf(buf2, "You fall down laughing.");
              break;
            case EMOTE_GIGGLE:
              sprintf(buf, "%s giggles.", op->name);
              sprintf(buf2, "You giggle.");
              break;
            case EMOTE_SHAKE:
              sprintf(buf, "%s shakes %s head.", op->name, own);
              sprintf(buf2, "You shake your head.");
              break;
            case EMOTE_PUKE:
              sprintf(buf, "%s pukes.", op->name);
              sprintf(buf2, "Bleaaaaaghhhhhhh!");
              break;
            case EMOTE_GROWL:
              sprintf(buf, "%s growls.", op->name);
              sprintf(buf2, "Grrrrrrrrr....");
              break;
            case EMOTE_SCREAM:
              sprintf(buf, "%s screams at the top of %s lungs!", op->name, own);
              sprintf(buf2, "ARRRRRRRRRRGH!!!!!");
              break;
            case EMOTE_SIGH:
              sprintf(buf, "%s sighs loudly.", op->name);
              sprintf(buf2, "You sigh.");
              break;
            case EMOTE_SULK:
              sprintf(buf, "%s sulks in the corner.", op->name);
              sprintf(buf2, "You sulk.");
              break;
            case EMOTE_CRY:
              sprintf(buf, "%s bursts into tears.", op->name);
              sprintf(buf2, "Waaaaaaahhh..");
              break;
            case EMOTE_GRIN:
              sprintf(buf, "%s grins evilly.", op->name);
              sprintf(buf2, "You grin evilly.");
              break;
            case EMOTE_BOW:
              sprintf(buf, "%s bows deeply.", op->name);
              sprintf(buf2, "You bow deeply.");
              break;
            case EMOTE_CLAP:
              sprintf(buf, "%s gives a round of applause.", op->name);
              sprintf(buf2, "Clap, clap, clap.");
              break;
            case EMOTE_BLUSH:
              sprintf(buf, "%s blushes.", op->name);
              sprintf(buf2, "Your cheeks are burning.");
              break;
            case EMOTE_BURP:
              sprintf(buf, "%s burps loudly.", op->name);
              sprintf(buf2, "You burp loudly.");
              break;
            case EMOTE_CHUCKLE:
              sprintf(buf, "%s chuckles politely.", op->name);
              sprintf(buf2, "You chuckle politely");
              break;
            case EMOTE_COUGH:
              sprintf(buf, "%s coughs loudly.", op->name);
              sprintf(buf2, "Yuck, try to cover your mouth next time!");
              break;
            case EMOTE_FLIP:
              sprintf(buf, "%s flips head over heels.", op->name);
              sprintf(buf2, "You flip head over heels.");
              break;
            case EMOTE_FROWN:
              sprintf(buf, "%s frowns.", op->name);
              sprintf(buf2, "What's bothering you?");
              break;
            case EMOTE_GASP:
              sprintf(buf, "%s gasps in astonishment.", op->name);
              sprintf(buf2, "You gasp in astonishment.");
              break;
            case EMOTE_GLARE:
              sprintf(buf, "%s glares around him.", op->name);
              sprintf(buf2, "You glare at nothing in particular.");
              break;
            case EMOTE_GROAN:
              sprintf(buf, "%s groans loudly.", op->name);
              sprintf(buf2, "You groan loudly.");
              break;
            case EMOTE_HICCUP:
              sprintf(buf, "%s hiccups.", op->name);
              sprintf(buf2, "*HIC*");
              break;
            case EMOTE_LICK:
              sprintf(buf, "%s licks %s mouth and smiles.", op->name, own);
              sprintf(buf2, "You lick your mouth and smile.");
              break;
            case EMOTE_POUT:
              sprintf(buf, "%s pouts.", op->name);
              sprintf(buf2, "Aww, don't take it so hard.");
              break;
            case EMOTE_SHIVER:
              sprintf(buf, "%s shivers uncomfortably.", op->name);
              sprintf(buf2, "Brrrrrrrrr.");
              break;
            case EMOTE_SHRUG:
              sprintf(buf, "%s shrugs helplessly.", op->name);
              sprintf(buf2, "You shrug.");
              break;
            case EMOTE_SMIRK:
              sprintf(buf, "%s smirks.", op->name);
              sprintf(buf2, "You smirk.");
              break;
            case EMOTE_SNAP:
              sprintf(buf, "%s snaps %s fingers.", op->name, own);
              sprintf(buf2, "PRONTO! You snap your fingers.");
              break;
            case EMOTE_SNEEZE:
              sprintf(buf, "%s sneezes.", op->name);
              sprintf(buf2, "Gesundheit!");
              break;
            case EMOTE_SNICKER:
              sprintf(buf, "%s snickers softly.", op->name);
              sprintf(buf2, "You snicker softly.");
              break;
            case EMOTE_SNIFF:
              sprintf(buf, "%s sniffs sadly.", op->name);
              sprintf(buf2, "You sniff sadly. *SNIFF*");
              break;
            case EMOTE_SNORE:
              sprintf(buf, "%s snores loudly.", op->name);
              sprintf(buf2, "Zzzzzzzzzzzzzzz.");
              break;
            case EMOTE_SPIT:
              sprintf(buf, "%s spits over %s left shoulder.", op->name, own);
              sprintf(buf2, "You spit over your left shoulder.");
              break;
            case EMOTE_STRUT:
              sprintf(buf, "%s struts proudly.", op->name);
              sprintf(buf2, "Strut your stuff.");
              break;
            case EMOTE_TWIDDLE:
              sprintf(buf, "%s patiently twiddles %s thumbs.", op->name, own);
              sprintf(buf2, "You patiently twiddle your thumbs.");
              break;
            case EMOTE_WAVE:
              sprintf(buf, "%s waves happily.", op->name);
              sprintf(buf2, "You wave.");
              break;
            case EMOTE_WHISTLE:
              sprintf(buf, "%s whistles appreciatively.", op->name);
              sprintf(buf2, "You whistle appreciatively.");
              break;
            case EMOTE_WINK:
              sprintf(buf, "%s winks suggestively.", op->name);
              sprintf(buf2, "Have you got something in your eye?");
              break;
            case EMOTE_YAWN:
              sprintf(buf, "%s yawns sleepily.", op->name);
              sprintf(buf2, "You open up your yap and let out a big breeze " "of stale air.");
              break;
            case EMOTE_CRINGE:
              sprintf(buf, "%s cringes in terror!", op->name);
              sprintf(buf2, "You cringe in terror.");
              break;
            case EMOTE_BLEED:
              sprintf(buf, "%s is bleeding all over the carpet - got a spare tourniquet?", op->name);
              sprintf(buf2, "You bleed all over your nice new armour.");
              break;
            case EMOTE_THINK:
              sprintf(buf, "%s closes %s eyes and thinks really hard.", op->name, own);
              sprintf(buf2, "Anything in particular that you'd care to think " "about?");
              break;
            case EMOTE_ME:
//              sprintf(buf2, "usage: /me <emote to display>");
//              if (op->type == PLAYER)
//                  ndi(NDI_UNIQUE, 0, op, "%s", buf2);
                /* do nothing, since we specified nothing to do */
              return 1;
            default:
              sprintf(buf, "%s looks nuts - you get the distinct feeling %s is nuts!",
                      op->name, op->name);
              sprintf(buf2, "You are a nut.");
              break;
        } /*case*/
        ndi_map(NDI_YELLOW, MSP_KNOWN(op), MAP_INFO_NORMAL, op, op, "%s", buf);
        if (op->type == PLAYER)
            ndi(NDI_UNIQUE, 0, op, "%s", buf2);
        return 0;
    }
    else /* we have params */
    {
        if (emotion == EMOTE_ME)        /* catch special case emote /me */
        {
            CHATLOG("EMOTE:%s >%s<\n", STRING_OBJ_NAME(op), params);
            ndi_map(NDI_EMOTE | NDI_PLAYER | NDI_YELLOW, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, NULL, "%s %s",
                QUERY_SHORT_NAME(op, NULL), params);

            return 0;
        }
        else /* here we handle player & npc emotes with parameter */
        {
            const char *name_hash   = find_string(params);

            if (op->type == PLAYER && op->name == name_hash) /* targeting ourself */
            {
                emote_self(op, buf, buf2, emotion);
                ndi(NDI_UNIQUE, 0, op, "%s", buf);
                ndi_map(NDI_EMOTE | NDI_PLAYER | NDI_YELLOW, MSP_KNOWN(op), MAP_INFO_NORMAL, op, op, "%s", buf2);
                return 0;
            }

            if (name_hash) /* no hash no player */
            {
                for (pl = first_player; pl != NULL; pl = pl->next)
                {
                    if (pl->ob->name == name_hash && (pl->state & ST_PLAYING) && !
                        QUERY_FLAG(pl->ob,FLAG_REMOVED) &&
                        !pl->privacy) // FIXME: same deal as tells
                    {
                        rv_t   rv; /* lets check range */
                        RV_GET_OBJ_TO_OBJ(op, pl->ob, &rv, 0);

                        emote_other(op, NULL, params, buf, buf2, buf3, emotion);

                        if (rv.distance <= 4)
                        {
                            if (op->type == PLAYER)
                            {
                                emote_other(op, pl->ob, NULL, buf, buf2, buf3, emotion);
                                ndi(NDI_UNIQUE, 0, op, "%s", buf);
                                ndi(NDI_EMOTE | NDI_PLAYER |NDI_UNIQUE | NDI_YELLOW,
                                              0, pl->ob, "%s", buf2);
                                ndi_map(NDI_EMOTE | NDI_PLAYER |NDI_YELLOW, MSP_KNOWN(op), MAP_INFO_NORMAL, op, pl->ob, "%s", buf3);
                            }
                            else /* npc */
                            {
                                ndi(NDI_UNIQUE | NDI_YELLOW, 0,
                                              pl->ob, "%s", buf2);
                                ndi_map(NDI_YELLOW, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, pl->ob, "%s", buf3);
                            }
                        }
                        else if (op->type == PLAYER)
                            ndi(NDI_UNIQUE, 0, op, "The target is not in range for this emote action.");

                        return 0;
                    }
                }
            }

            /* we had a non player name as params!
                     * Now, we *can* do check for a possible non player target.
                     * Like a npc or a mob. But for that we need to analyze the
                     * surrounding area. Bad idea. Also, the name is for a mob not unique.
                     * There are perhaps several ants or whatever around the player.
                     * For emoting mobs and npcs we simply use the target system.
                     */
            if (op->type == PLAYER)
            {
                emote_self(op, buf, buf2, -1); /* force neutral default emote */
                ndi(NDI_UNIQUE, 0, op, "%s", buf);
                ndi_map(NDI_EMOTE | NDI_PLAYER | NDI_YELLOW, MSP_KNOWN(op), MAP_INFO_NORMAL, op, op, "%s", buf2);
            }
            else
            {
                /* our target is perhaps a npc or whatever */
                emote_other(op, NULL, params, buf, buf2, buf3, emotion);
                ndi_map(NDI_YELLOW, MSP_KNOWN(op), MAP_INFO_NORMAL, NULL, NULL, "%s", buf3);
            }
        }
    }

    return 0;
}

/*
 * everything from here on out are just wrapper calls to basic_emote
 */

int command_nod(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_NOD));
}

int command_dance(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_DANCE));
}

int command_kiss(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_KISS));
}

int command_bounce(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_BOUNCE));
}

int command_smile(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SMILE));
}

int command_cackle(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_CACKLE));
}

int command_laugh(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_LAUGH));
}

int command_giggle(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_GIGGLE));
}

int command_shake(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SHAKE));
}

int command_puke(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_PUKE));
}

int command_growl(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_GROWL));
}

int command_scream(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SCREAM));
}

int command_sigh(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SIGH));
}

int command_sulk(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SULK));
}

int command_hug(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_HUG));
}

int command_cry(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_CRY));
}

int command_poke(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_POKE));
}

int command_accuse(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_ACCUSE));
}

int command_grin(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_GRIN));
}

int command_bow(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_BOW));
}

int command_clap(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_CLAP));
}

int command_blush(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_BLUSH));
}

int command_burp(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_BURP));
}

int command_chuckle(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_CHUCKLE));
}

int command_cough(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_COUGH));
}

int command_flip(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_FLIP));
}

int command_frown(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_FROWN));
}

int command_gasp(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_GASP));
}

int command_glare(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_GLARE));
}

int command_groan(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_GROAN));
}

int command_hiccup(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_HICCUP));
}

int command_lick(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_LICK));
}

int command_pout(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_POUT));
}

int command_shiver(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SHIVER));
}

int command_shrug(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SHRUG));
}

int command_slap(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SLAP));
}

int command_smirk(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SMIRK));
}

int command_snap(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SNAP));
}

int command_sneeze(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SNEEZE));
}

int command_snicker(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SNICKER));
}

int command_sniff(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SNIFF));
}

int command_snore(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SNORE));
}

int command_spit(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_SPIT));
}

int command_strut(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_STRUT));
}

int command_thank(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_THANK));
}

int command_twiddle(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_TWIDDLE));
}

int command_wave(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_WAVE));
}

int command_whistle(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_WHISTLE));
}

int command_wink(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_WINK));
}

int command_yawn(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_YAWN));
}

int command_beg(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_BEG));
}

int command_bleed(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_BLEED));
}

int command_cringe(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_CRINGE));
}

int command_think(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_THINK));
}

int command_me(object_t *op, char *params)
{
    return(basic_emote(op, params, EMOTE_ME));
}
