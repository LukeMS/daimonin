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

/* define the following for skills utility debuging */

#include <global.h>
#include "skillist.h"

static char *exp_group_arch_name[NROFSKILLGROUPS] = {
    "experience_agility",
    "experience_charisma",
    "experience_mental",
    "experience_physical",
    "experience_power",
    "experience_wis",
    "experience_misc"
};

/* link_player_skills() - linking skills with experience objects
 * and creating a linked list of skills for later fast access.
 * adjusting exp when needed.
 */
void link_player_skills(object *op)
{
    int i;
    object *tmp;
    player *pl = CONTR(op);

    /* browse the player inv and put all EXPERIENCE and SKILL
     * objects in the player pointer shutcut arrays.
     * These speeds up the whole skill system alot. MT-2005
     */
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
    {
        if(tmp->type == EXPERIENCE)
            pl->exp_obj_ptr[tmp->sub_type1] = tmp;
        else if (tmp->type == SKILL)
        {
            /* important: we need to have all skill unapplied = unused.
             * or the server will fail at runtime to setup the enviroment of a new loaded obj
             * when a player is start using a specific skill (because it thinks its done before)
             */
            CLEAR_FLAG(tmp, FLAG_APPLIED);
            pl->skill_ptr[tmp->stats.sp] = tmp;
        }
    }

    /* lets check there are missing exp group object.
     * This was given to player in the past with the treasure list,
     * but we have to control it here so or so.
     */
    for(i=0;i<NROFSKILLGROUPS;i++)
    {
        if(!pl->exp_obj_ptr[i])
        {
            /*LOG(llevDebug,"link_player_skills(): adding %s to player %s\n",exp_group_arch_name[i],query_name(tmp));*/
            pl->exp_obj_ptr[i] = insert_ob_in_ob(arch_to_object(find_archetype(exp_group_arch_name[i])), op);
        }
        CLEAR_FLAG(pl->exp_obj_ptr[i], FLAG_APPLIED);

        /* we should not need that exp adjustment
        */
        /*
        if (pl->stats.exp < pl->exp_obj_ptr[i]->stats.exp)
        {
            pl->stats.exp = pl->exp_obj_ptr[i]->stats.exp;
            pl->level = pl->exp_obj_ptr[i]->level;
        }
        player_lvl_adj(NULL, pl->exp_obj_ptr[i]);
        */
    }

    /* now loop the found skills and link them to exp group objects */
    for(i=0;i<NROFSKILLS;i++)
    {
        if(pl->skill_ptr[i])
        {
            pl->skill_ptr[i]->exp_obj = pl->exp_obj_ptr[pl->skill_ptr[i]->magic];

            if( !pl->highest_skill[pl->skill_ptr[i]->magic] ||
                 pl->skill_ptr[i]->stats.exp > pl->highest_skill[pl->skill_ptr[i]->magic]->stats.exp)
                pl->highest_skill[pl->skill_ptr[i]->magic] = pl->skill_ptr[i];
        }


    }
}

/* find_skill()
 * looks for the skill and returns a pointer to it if found
 */
object * find_skill(object *op, int skillnr)
{
    /* we do some sanity checks - this is called from scripts for example */
    if(op->type != PLAYER || !CONTR(op))
    {
        LOG(llevDebug, "BUG: find_skill() called for non player/no CONTR() object %s (%d)\n", query_name(op), skillnr);
        return NULL;
    }

    return CONTR(op)->skill_ptr[skillnr];
}

/* do_skill() - Main skills use function-similar in scope to cast_spell().
 * We handle all requests for skill use outside of some combat here.
 * We require a separate routine outside of fire() so as to allow monsters
 * to utilize skills. Success should be the amount of experience gained
 * from the activity. Also, any skills that monster will use, will need
 * to return a positive value of success if performed correctly. Usually
 * this is handled in calc_skill_exp(). Thus failed skill use re-
 * sults in a 0, or false value of 'success'.
 *  - b.t.  thomas@astro.psu.edu
 */

int do_skill(object *op, int dir, char *string)
{
    int success = 0;        /* needed for monster_skill_use() too */
    int skill   = op->chosen_skill->stats.sp;

    /*LOG(-1,"DO SKILL: skill %s ->%d\n", op->chosen_skill->name, get_skill_time(op,skill));*/

    switch (skill)
    {
        case SK_LEVITATION:
          if (QUERY_FLAG(op, FLAG_LEVITATE))
          {
              CLEAR_MULTI_FLAG(op, FLAG_LEVITATE);
              new_draw_info(NDI_UNIQUE, 0, op, "You come to earth.");
          }
          else
          {
              SET_MULTI_FLAG(op, FLAG_LEVITATE);
              new_draw_info(NDI_UNIQUE, 0, op, "You rise into the air!.");
          }

          break;
        case SK_STEALING:
          success = steal(op, dir);
          break;
        case SK_LOCKPICKING:
          success = pick_lock(op, dir);
          break;
        case SK_HIDING:
          success = hide(op);
          break;
        case SK_JUMPING:
          success = jump(op, dir);
          break;
        case SK_INSCRIPTION:
          success = write_on_item(op, string);
          break;
        case SK_MEDITATION:
          meditate(op);
          break;
          /* note that the following 'attack' skills gain exp through hit_player() */
        case SK_KARATE:
          (void) attack_hth(op, dir, "karate-chopped");
          break;
        case SK_BOXING:
          (void) attack_hth(op, dir, "punched");
          break;
        case SK_FLAME_TOUCH:
          (void) attack_hth(op, dir, "flamed");
          break;
        case SK_CLAWING:
          (void) attack_hth(op, dir, "clawed");
          break;
        case SK_MELEE_WEAPON:
        case SK_SLASH_WEAP:
        case SK_CLEAVE_WEAP:
        case SK_PIERCE_WEAP:
          (void) attack_melee_weapon(op, dir, NULL);
          break;
        case SK_FIND_TRAPS:
          success = find_traps(op, op->chosen_skill->level);
          break;
        case SK_MUSIC:
          success = singing(op, dir);
          break;
        case SK_ORATORY:
          success = use_oratory(op, dir);
          break;
        case SK_SMITH:
        case SK_BOWYER:
        case SK_JEWELER:
        case SK_ALCHEMY:
        case SK_THAUMATURGY:
        case SK_LITERACY:
        case SK_DET_MAGIC:
        case SK_DET_CURSE:
        case SK_WOODSMAN:
          success = skill_ident(op);
          break;
        case SK_REMOVE_TRAP:
          success = remove_trap(op, dir, op->chosen_skill->level);
          break;
        case SK_THROWING:
          new_draw_info(NDI_UNIQUE, 0, op, "This skill is not usable in this way.");
          return success;
          break;

        case SK_SET_TRAP:
          new_draw_info(NDI_UNIQUE, 0, op, "This skill is not currently implemented.");
          return success;
          break;
        case SK_USE_MAGIC_ITEM:
        case SK_MISSILE_WEAPON:
          new_draw_info(NDI_UNIQUE, 0, op, "There is no special attack for this skill.");
          return success;
          break;
        case SK_PRAYING:
          new_draw_info(NDI_UNIQUE, 0, op, "This skill is not usable in this way.");
          return success;
          /*success = pray(op);*/
          break;
        case SK_SPELL_CASTING:
        case SK_CLIMBING:
        case SK_BARGAINING:
          new_draw_info(NDI_UNIQUE, 0, op, "This skill is already in effect.");
          return success;
          break;
        default:
          LOG(llevDebug, "%s attempted to use unknown skill: %d\n", query_name(op), op->chosen_skill->stats.sp);
          return success;
          break;
    }

    /* For players we now update the speed_left from using the skill.
     * Monsters have no skill use time because of the random nature in
     * which use_monster_skill is called already simulates this. -b.t.
     */

    if (op->type == PLAYER)
	{
		LOG(llevDebug, "AC-skills(%d): %d\n", skill, skills[skill].time);
		set_action_time(op, skills[skill].time);
	}

    /* this is a good place to add experience for successfull use of skills.
     * Note that add_exp() will figure out player/monster experience
     * gain problems.
     */

    if (success && skills[skill].category < NROFSKILLGROUPS_ACTIVE)
        add_exp(op, success, op->chosen_skill->stats.sp);

    return success;
}


/* find relevant stats or a skill then return their weighted sum.
 * I admit the calculation is done in a retarded way.
 * If stat1==NO_STAT_VAL this isnt an associated stat. Returns
 * zero then. -b.t.
 */

int get_weighted_skill_stat_sum(object *who, int sk)
{
    float   sum;
    int     number  = 1;

    if (skills[sk].stat1 == NO_STAT_VAL)
    {
        return 0;
    }
    else
        sum = get_attr_value(&(who->stats), skills[sk].stat1);

    if (skills[sk].stat2 != NO_STAT_VAL)
    {
        sum += get_attr_value(&(who->stats), skills[sk].stat2);
        number++;
    }

    if (skills[sk].stat3 != NO_STAT_VAL)
    {
        sum += get_attr_value(&(who->stats), skills[sk].stat3);
        number++;
    }

    return ((int) sum / number);
}

void dump_skills()
{

    /*  dump_all_objects(); */
    /*
    char    buf[MAX_BUF];
    int     i;
    LOG(llevInfo, "exper_catgry \t str \t dex \t con \t wis \t cha \t int \t pow \n");
    for (i = 0; i < NROFSKILLGROUPS; i++)
        LOG(llevInfo, "%d-%s \t %d \t %d \t %d \t %d \t %d \t %d \t %d \n", i, exp_cat[i]->name, exp_cat[i]->stats.Str,
            exp_cat[i]->stats.Dex, exp_cat[i]->stats.Con, exp_cat[i]->stats.Wis, exp_cat[i]->stats.Cha,
            exp_cat[i]->stats.Int, exp_cat[i]->stats.Pow);

    LOG(llevInfo, "\n");
    sprintf(buf, "%20s  %12s  %4s %4s %4s  %5s %5s %5s\n", "sk#       Skill name", "ExpCat", "Time", "Base", "xlvl",
            "Stat1", "Stat2", "Stat3");
    LOG(llevInfo, buf);
    sprintf(buf, "%20s  %12s  %4s %4s %4s  %5s %5s %5s\n", "---       ----------", "------", "----", "----", "----",
            "-----", "-----", "-----");
    LOG(llevInfo, buf);
    for (i = 0; i < NROFSKILLS; i++)
    {
        sprintf(buf, "%2d-%17s  %12s  %4d %4ld %4g  %5s %5s %5s\n", i, skills[i].name,
                exp_cat[skills[i].category] != NULL ? exp_cat[skills[i].category]->name : "NONE", skills[i].time,
                skills[i].bexp, skills[i].lexp,
                skills[i].stat1 != NO_STAT_VAL ? short_stat_name[skills[i].stat1] : "---",
                skills[i].stat2 != NO_STAT_VAL ? short_stat_name[skills[i].stat2] : "---",
                skills[i].stat3 != NO_STAT_VAL ? short_stat_name[skills[i].stat3] : "---");
        LOG(llevInfo, buf);
    }
    */
}

/* read_skill_params() - based on init_spell_params(). This
 * function should read a file 'skill_params' in the /lib
 * directory.  -b.t.
 *
 *  format of the file 'skill_params' is:
 *  name
 *      EXP_CAT, bexp, lexp, stat1, stat2, stat3
 *
 *  see the values in lib/skill_params for more inspiration/direction
 */

void read_skill_params()
{
    FILE   *skill_params;
    char    fname[MAX_BUF];
    char    skill_name[256];
    char    skill_attrib[256];
    int     i, cat, bexp, time, stat1, stat2, stat3, skillindex;
    float   lexp;

    sprintf(fname, "%s/%s", settings.datadir, "skill_params");
    LOG(llevDebug, "Reading skill_params from %s...", fname);
    if (!(skill_params = fopen(fname, "r")))
    {
        LOG(llevError, "ERROR: read_skill_params(): error fopen(%s)\n", fname);
        return;
    }

    while (!feof(skill_params))
    {
        /* Large buf, so that long comments don't kill it. */
        fgets(skill_name, 255, skill_params);
        if (*skill_name == '#' || *skill_name == '\n' || *skill_name == '\r')
            continue;
        skillindex = lookup_skill_by_name(skill_name);
        if (skillindex == -1)
            LOG(llevError, "ERROR: skill_params has unrecognized skill: %s", skill_name);
        fgets(skill_attrib, 255, skill_params);
        sscanf(skill_attrib, "%d %d %d %f %d %d %d", &cat, &time, &bexp, &lexp, &stat1, &stat2, &stat3);
        skills[skillindex].category = cat;
        skills[skillindex].time = time;
        skills[skillindex].bexp = bexp;
        skills[skillindex].lexp = lexp;
        skills[skillindex].stat1 = stat1;
        skills[skillindex].stat2 = stat2;
        skills[skillindex].stat3 = stat3;
    }
    fclose(skill_params);

    for (i = 0; i < NROFSKILLS; i++)
    {
        /* link the skill archetype ptr to skill list for fast access.
        * now we can access the skill archetype by skill number or skill name.
        */
        if (!(skills[i].at = get_skill_archetype(i)))
            LOG(llevError, "ERROR: Aborting! Skill #%d (%s) not found in archlist!\n", i, skills[i].name);
    }

    LOG(llevDebug, "done.\n");
}


/* lookup_skill_by_name() - based on look_up_spell_by_name - b.t.
 * Given name, we return the index of skill 'string' in the skill
 * array, -1 if no skill is found.
 */

int lookup_skill_by_name(char *string)
{
    int     skillnr = 0, nmlen;
    char    name[MAX_BUF];

    if (!string)
        return -1;

    strcpy(name, string);
    nmlen = strlen(name);

    for (skillnr = 0; skillnr < NROFSKILLS; skillnr++)
    {
        if (strlen(name) >= strlen(skills[skillnr].name)) /* GROS - This is to prevent strings like "hi" to be matched as "hiding" */
            if (!strncmp(name, skills[skillnr].name, MIN((int) strlen(skills[skillnr].name), nmlen)))
                return skillnr;
    }
    return -1;
}

/* check_skill_to_apply() - When a player tries to use an
 * object which requires a skill this function is called.
 * (examples are weapons like swords and bows)
 * It does 2 things: checks for appropriate skill in player inventory
 * and alters the player status by making the appropriate skill
 * the new chosen_skill.
 * -bt. thomas@astro.psu.edu
 */

/* This function was strange used.
 * I changed it, so it is used ONLY when a player try to apply something.
 * This function now sets no apply flag or needed one - but it checks and sets
 * the right skill.
 * TODO: monster skills and flags (can_use_xxx) including here.
 * For unapply, only call change_skill(object, NO_SKILL_READY) and fix_player() after it.
 * MT - 4.10.2002
 */
int check_skill_to_apply(object *who, object *item)
{
    int skill = 0, tmp;
    int add_skill   = NO_SKILL_READY; /* perhaps we need a additional skill to use */

    if (who->type != PLAYER)
        return 1; /* this fctn only for players */

    /* first figure out the required skills from the item */
    switch (item->type)
    {
        case WEAPON:
          tmp = item->sub_type1;
          if (tmp >= WEAP_POLE_IMPACT) /* we have a polearm! */
          {
              tmp = item->sub_type1 - WEAP_POLE_IMPACT; /* lets select the right weapon type */
              add_skill = SK_POLEARMS;
          }
          else if (tmp >= WEAP_2H_IMPACT) /* no, we have a 2h! */
          {
              tmp = item->sub_type1 - WEAP_2H_IMPACT; /* lets select the right weapon type */
              add_skill = SK_TWOHANDS;
          }

          if (tmp == WEAP_1H_IMPACT)
              skill = SK_MELEE_WEAPON;
          else if (tmp == WEAP_1H_SLASH)
              skill = SK_SLASH_WEAP;
          else if (tmp == WEAP_1H_CLEAVE)
              skill = SK_CLEAVE_WEAP;
          else if (tmp == WEAP_1H_PIERCE)
              skill = SK_PIERCE_WEAP;
          break;
		case ARROW:
			if(item->sub_type1 > 127)
			{
				skill = SK_THROWING;
				break;
			}

        case BOW:
          tmp = item->sub_type1;
          if (tmp == RANGE_WEAP_BOW)
              skill = SK_MISSILE_WEAPON;
          else if (tmp == RANGE_WEAP_XBOWS)
              skill = SK_XBOW_WEAP;
          else
              skill = SK_SLING_WEAP;
          break;
        case POTION:
          skill = SK_USE_MAGIC_ITEM; /* hm, this can be tricky when a player kills himself
                                     * applying a bomb potion... must watch it */
          break;
        case SCROLL:
          skill = SK_USE_MAGIC_ITEM; /* not literacy atm - we will change scroll back! */
          break;
        case ROD:
          skill = SK_USE_MAGIC_ITEM;
          break;
        case WAND:
          skill = SK_USE_MAGIC_ITEM;
          break;
        case HORN:
          skill = SK_USE_MAGIC_ITEM;
          break;
        default:
          LOG(llevDebug, "Warning: bad call of check_skill_to_apply()\n");
          LOG(llevDebug, "No skill exists for item: %s\n", query_name(item));
          return 0;
    }

    /* this should not happen */
    if (skill == NO_SKILL_READY)
        LOG(llevBug, "BUG: check_skill_to_apply() called for %s and item %s with skill NO_SKILL_READY\n",
            query_name(who), query_name(item));

    /* lets check the additional skill if there is one */
    if (add_skill != NO_SKILL_READY)
    {
        if (!change_skill(who, add_skill))
        {
            /*new_draw_info_format(NDI_UNIQUE, 0,who,"You don't have the needed skill '%s'!", skills[add_skill].name);*/
            return 0;
        }
        change_skill(who, NO_SKILL_READY);
    }

    /* if this skill is ready, all is fine. if not, ready it, if it can't readied, we
     * can't apply/use/do it */
    if (!who->chosen_skill || (who->chosen_skill && who->chosen_skill->stats.sp != skill))
    {
        if (!change_skill(who, skill))
        {
            /*new_draw_info_format(NDI_UNIQUE, 0,who,"You don't have the needed skill '%s'!", skills[skill].name);*/
            return 0;
        }
    }
    return 1;
}


/* unlink_skill() - removes skill from a player skill list and
 * unlinks the pointer to the exp object */

void unlink_skill(object *skillop)
{
    object *op  = skillop ? skillop->env : NULL;

    if (!op || op->type != PLAYER)
    {
        LOG(llevBug, "BUG: unlink_skill() called for non-player %s!\n", query_name(op));
        return;
    }
    send_skilllist_cmd(op, skillop, SPLIST_MODE_REMOVE);
    skillop->exp_obj = NULL;
}


/* link_player_skill() - links a  skill to exp object when applied or learned by
 * a player. Returns true if can link. Returns false if got misc
 * skill - bt.
 */

int link_player_skill(object *pl, object *skillop)
{
    skillop->exp_obj = CONTR(pl)->exp_obj_ptr[skillop->magic];

    if(!skillop->exp_obj)
    {
        LOG(llevDebug," link_player_skill(): player %s has for skill obj %s has no valid exp obj\n",
                        query_name(pl), query_name(skillop));
        return 0;
    }

    return 1;
}

/* Learn skill. This inserts the requested skill in the player's
 * inventory. The 'slaying' field of the scroll should have the
 * exact name of the requested archetype (there should be a better way!?)
 * added skillnr - direct access to archetype in skills[].
 * -bt. thomas@nomad.astro.psu.edu
 */

int learn_skill(object *pl, object *scroll, char *name, int skillnr, int scroll_flag)
{
    player      *p;
    object     *tmp;
    archetype  *skill           = NULL;
    int         has_meditation  = 0;


    if(pl->type != PLAYER)
        return 2;

    p = CONTR(pl);
    if(skillnr!= -1)
        skill = skills[skillnr].at;
    else if (scroll)
        skill = find_archetype(scroll->slaying);
    else if (name)
        skill = find_archetype(name);

    if (!skill)
        return 2;

    skillnr = skill->clone.stats.sp;
    if(find_skill(pl,skillnr))
    {
        if(p && (p->state & ST_PLAYING))
            new_draw_info_format(NDI_UNIQUE, 0, pl, "You already know the skill '%s'!", query_name(&skill->clone));
        return 0;
    }

    tmp = arch_to_object(skill);
    if (!tmp)
        return 2;

    /* Special check - if the player has meditation (monk), they can not
     * learn melee weapons.  Prevents monk from getting this
     * skill.
     */
    /* disabled the meditation check for now - MT-2005 */
    if (tmp->stats.sp == SK_MELEE_WEAPON && has_meditation)
    {
        if(p && (p->state & ST_PLAYING))
            new_draw_info(NDI_UNIQUE, 0, pl, "Your knowledge of inner peace prevents you from learning about melee weapons.");
        return 2;
    }
    /* now a random change to learn, based on player Int */

    /* Everything is cool. Give'em the skill */
    insert_ob_in_ob(tmp, pl);
    CONTR(pl)->skill_ptr[tmp->stats.sp] = tmp;
    link_player_skill(pl, tmp);

    if(p && (p->state & ST_PLAYING))
    {
        play_sound_player_only(CONTR(pl), SOUND_LEARN_SPELL, SOUND_NORMAL, 0, 0);
        new_draw_info_format(NDI_UNIQUE, 0, pl, "You have learned the skill %s!", tmp->name);
        send_skilllist_cmd(pl, tmp, SPLIST_MODE_ADD);
        esrv_send_item(pl, tmp);
    }

    return 1;
}

/* use_skill() - similar to invoke command, it executes the skill in the
 * direction that the user is facing. Returns false if we are unable to
 * change to the requested skill, or were unable to use the skill properly.
 * -b.t.
 */

int use_skill(object *op, char *string)
{
    int sknum   = -1;

    /* the skill name appears at the begining of the string,
     * need to reset the string to next word, if it exists. */
    /* first eat name of skill and then eat any leading spaces */

    if (string && (sknum = lookup_skill_by_name(string)) >= 0)
    {
        int len;

        if (sknum == -1)
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "Unable to find skill by name %s", string);
            return 0;
        }

        len = strlen(skills[sknum].name);

        /* All this logic goes and skips over the skill name to find any
         * options given to the skill.
         */
        if (len >= (int) strlen(string))
        {
            *string = 0x0;
        }
        else
        {
            while (len--)
            {
                string++;
            }
            while (*string == 0x20)
            {
                string++;
            }
        }
        if (strlen(string) == 0)
            string = NULL;
    }

#ifdef SKILL_UTIL_DEBUG
    LOG(llevDebug, "use_skill() got skill: %s\n", sknum > -1 ? skills[sknum].name : "none");
#endif

    /* Change to the new skill, then execute it. */
    if (change_skill(op, sknum))
    {
        if (op->chosen_skill->sub_type1 != ST1_SKILL_USE)
            new_draw_info(NDI_UNIQUE, 0, op, "You can't use this skill in this way.");
        else
		{
			if (!check_skill_action_time(op, op->chosen_skill)) /* are we idle from other action? */
				return 0;

			if (do_skill(op, op->facing, string))
            return 1;
		}
    }
    return 0;
}


/* change_skill() - returns true if we are able to change to the requested
 * skill. Ignore the 'pl' designation, this code is useful for players and
 * monsters.  -bt. thomas@astro.psu.edu
 *
 * sk_index == -1 means that old skill should be unapplied, and no new skill
 * applied.
 */
/* please note that change skill change set_skill_weapon && set_skill_bow are ONLY
 * set in fix_players() */
int change_skill(object *who, int sk_index)
{
    object *tmp;

	if (who->chosen_skill && who->chosen_skill->stats.sp == sk_index)
        return 1;

	LOG(llevDebug, "APPLYcs: %s change %s to %s.\n", query_name(who), query_name(who->chosen_skill), 
																sk_index>=0?skills[sk_index].name:"INVALID");

    if (sk_index >= 0 && sk_index < NROFSKILLS && (tmp = find_skill(who, sk_index)) != NULL)
    {
        if (apply_special(who, tmp, AP_APPLY))
        {
            /*LOG(llevDebug, "BUG: change_skill(): can't apply new skill (%s - %d)\n", who->name, sk_index);*/
            return 0;
        }
        return 1;
    }

    if (who->chosen_skill)
	{
        if (apply_special(who, who->chosen_skill, AP_UNAPPLY))
            LOG(llevBug, "BUG: change_skill(): can't unapply old skill (%s - %d)\n", who->name, sk_index);
		FIX_PLAYER(who, "change_skill AP_UNAPPLY");
	}
    if (sk_index >= 0)
        new_draw_info_format(NDI_UNIQUE, 0, who, "You have no knowledge of %s.", skills[sk_index].name);
    return 0;
}

/* This is like change_skill above, but it is given that
 * skill is already in who's inventory - this saves us
 * time if the caller has already done the work for us.
 * return 0 on success, 1 on failure.
 */

int change_skill_to_skill(object *who, object *skl)
{
    if (!skl)
        return 1;       /* Quick sanity check */


	if (who->chosen_skill == skl)
        return 0;

	/*LOG(llevDebug, "APPLYcsts: %s change %s to %s.\n", query_name(who), query_name(who->chosen_skill), query_name(skl));*/

	if (skl->env != who)
    {
        /*LOG(llevDebug, "BUG: change_skill_to_skill: skill is not in players inventory (%s - %s)\n", query_name(who), query_name(skl));*/
        return 1;
    }

    if (apply_special(who, skl, AP_APPLY))
    {
        LOG(llevBug, "BUG: change_skill(): can't apply new skill (%s - %s)\n", query_name(who), query_name(skl));
        return 1;
    }
    return 0;
}

/* attack_melee_weapon() - this handles melee weapon attacks -b.t.
 * For now we are just checking to see if we have a ready weapon here.
 * But there is a real neato possible feature of this scheme which
 * bears mentioning:
 * Since we are only calling this from do_skill() in the future
 * we may make this routine handle 'special' melee weapons attacks
 * (like disarming manuever with sai) based on player SK_level and
 * weapon type.
 */

int attack_melee_weapon(object *op, int dir, char *string)
{
    if (!QUERY_FLAG(op, FLAG_READY_WEAPON))
    {
        if (op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "You have no ready weapon to attack with!");
        return 0;
    }

    return skill_attack(NULL, op, dir, string);
}

/* attack_hth() - this handles all hand-to-hand attacks -b.t. */

/* July 5, 1995 - I broke up attack_hth() into 2 parts. In the first
 * (attack_hth) we check for weapon use, etc in the second (the new
 * function skill_attack() we actually attack.
 */

int attack_hth(object *pl, int dir, char *string)
{
    object*enemy =  NULL, *weapon;

    if (QUERY_FLAG(pl, FLAG_READY_WEAPON))
        for (weapon = pl->inv; weapon; weapon = weapon->below)
        {
            if (weapon->type != WEAPON || !QUERY_FLAG(weapon, FLAG_APPLIED))
                continue;
            CLEAR_FLAG(weapon, FLAG_APPLIED);
            CLEAR_FLAG(pl, FLAG_READY_WEAPON);
            FIX_PLAYER(pl ,"attack hth");
            if (pl->type == PLAYER)
            {
                new_draw_info(NDI_UNIQUE, 0, pl, "You unwield your weapon in order to attack.");
                esrv_update_item(UPD_FLAGS, pl, weapon);
            }
            break;
        }

    return skill_attack(enemy, pl, dir, string);
}

/* skill_attack() - Core routine for use when we attack using a skills
 * system. There are'nt too many changes from before, basically this is
 * a 'wrapper' for the old attack system. In essence, this code handles
 * all skill-based attacks, ie hth, missile and melee weapons should be
 * treated here. If an opponent is already supplied by move_player(),
 * we move right onto do_skill_attack(), otherwise we find if an
 * appropriate opponent exists.
 *
 * This is called by move_player() and attack_hth()
 *
 * Initial implementation by -bt thomas@astro.psu.edu
 */

int skill_attack(object *tmp, object *pl, int dir, char *string)
{
    int         xt, yt;
    mapstruct  *m;

    if (!dir)
        dir = pl->facing;

    /* If we don't yet have an opponent, find if one exists, and attack.
     * Legal opponents are the same as outlined in move_player()
     */

    if (tmp == NULL)
    {
        xt = pl->x + freearr_x[dir];
        yt = pl->y + freearr_y[dir];
        if (!(m = out_of_map(pl->map, &xt, &yt)))
            return 0;

        /* rewrite this for new "head only" multi arches and battlegrounds. MT. */
        for (tmp = get_map_ob(m, xt, yt); tmp; tmp = tmp->above)
        {
            if ((IS_LIVE(tmp) && (tmp->head == NULL ? tmp->stats.hp > 0 : tmp->head->stats.hp > 0))
             || QUERY_FLAG(tmp, FLAG_CAN_ROLL) || tmp->type == LOCKED_DOOR)
            {
                /* lets skip pvp outside battleground (pvp area) */
                if (pl->type == PLAYER && tmp->type == PLAYER && !op_on_battleground(tmp, NULL, NULL))
                    continue;
                break;
            }
        }
    }
    if (tmp != NULL)
        return do_skill_attack(tmp, pl, string);

    if (pl->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, pl, "There is nothing to attack!");

    return 0;
}

/* do_skill_attack() - We have got an appropriate opponent from either
 * move_player() or skill_attack(). In this part we get on with
 * attacking, take care of messages from the attack and changes in invisible.
 * Returns true if the attack damaged the opponent.
 * -b.t. thomas@astro.psu.edu
 */

int do_skill_attack(object *tmp, object *op, char *string)
{
    int     success;
    char    buf[MAX_BUF], *name = query_name(tmp);

    if (op->type == PLAYER)
    {
        if (!CONTR(op)->selected_weapon) /* ok... lets change to our hth skill */
        {
            if (CONTR(op)->skill_weapon)
            {
                if (change_skill_to_skill(op, CONTR(op)->skill_weapon))
                {
                    LOG(llevBug, "BUG: do_skill_attack() could'nt give new hth skill to %s\n", query_name(op));
                    return 0;
                }
            }
            else
            {
                LOG(llevBug, "BUG: do_skill_attack(): no hth skill in player %s\n", query_name(op));
                return 0;
            }
        }
    }
    /* if we have 'ready weapon' but no 'melee weapons' skill readied
     * this will flip to that skill. This is only window dressing for
     * the players--no need to do this for monsters.
     */
    if ( op->type == PLAYER && QUERY_FLAG(op, FLAG_READY_WEAPON)
         && (!op->chosen_skill || op->chosen_skill->stats.sp != CONTR(op)->set_skill_weapon))
    {
        change_skill(op, CONTR(op)->set_skill_weapon);
    }

    success = attack_ob(tmp, op, NULL);

    /* print appropriate  messages to the player */

    if (success && string != NULL)
    {
        sprintf(buf, string);
        if (op->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE, 0, op, "You %s %s!", buf, name);
        else if (tmp->type == PLAYER)
            new_draw_info_format(NDI_UNIQUE, 0, tmp, "%s %s you!", query_name(op), buf);
    }

    return success;
}


/* This is in the same spirit as the similar routine for spells
 * it should be used anytime a function needs to check the user's
 * level.
 */
int SK_level(object *op)
{
    object *head    = op->head ? op->head : op;
    int     level;

    if (head->type == PLAYER && head->chosen_skill && head->chosen_skill->level != 0)
    {
        level = head->chosen_skill->level;
    }
    else
    {
        level = head->level;
    }

    if (level <= 0)
    {
        LOG(llevBug, "BUG: SK_level(arch %s, name %s): level <= 0\n", op->arch->name, query_name(op));
        level = 1;   /* safety */
    }

    return level;
}

/* sets the action timer for skills like throwing, archery, casting... */
void set_action_time(object *op, int t)
{
	if (op->type != PLAYER)
		return;

	CONTR(op)->action_timer = ROUND_TAG + t;
	LOG(llevDebug, "ActionTimer for %s (skill %s): +%d\n", query_name(op), query_name(op->chosen_skill), t);
}

/* player only: we check the action time for a skill.
 * if skill action is possible, return true.
 * if the action is not possible, drop the right message
 * and/or queue the command.
 */
int check_skill_action_time(object *op, object *skill)
{
    if(!skill)
        return FALSE;

    switch (skill->stats.sp)
    {
        /* spells */
        case SK_PRAYING:
        case SK_SPELL_CASTING:
            if (CONTR(op)->action_timer > ROUND_TAG)
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "You can cast in %2.2f seconds again.",
                        (float) (CONTR(op)->action_timer - ROUND_TAG) / pticks_second);
                return FALSE;
            }
            break;

            /* archery */
        case SK_SLING_WEAP:
        case SK_XBOW_WEAP:
        case SK_MISSILE_WEAPON:
            if (CONTR(op)->action_timer > ROUND_TAG)
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "You can shoot again in %2.2f seconds.",
                        (float) (CONTR(op)->action_timer - ROUND_TAG) / pticks_second);
                return FALSE;
            }
            break;

        case SK_USE_MAGIC_ITEM:
          if (CONTR(op)->action_timer > ROUND_TAG)
          {
              new_draw_info_format(NDI_UNIQUE, 0, op, "You can use a device again in %2.2f seconds.",
                                   (float) (CONTR(op)->action_timer - ROUND_TAG) / pticks_second);
              return FALSE;
          }

        case SK_THROWING:
          if (CONTR(op)->action_timer > ROUND_TAG)
          {
              new_draw_info_format(NDI_UNIQUE, 0, op, "You can throw again in %2.2f seconds.",
                                   (float) (CONTR(op)->action_timer - ROUND_TAG) / pticks_second);
              return FALSE;
          }
        default:
			if (CONTR(op)->action_timer > ROUND_TAG)
			{
				new_draw_info_format(NDI_UNIQUE, 0, op, "You can use this skill in %2.2f seconds again.",
					(float) (CONTR(op)->action_timer - ROUND_TAG) / pticks_second);
				return FALSE;
			}
          break;
    }

    return TRUE;
}

/* get_skill_stat1() - returns the value of the primary skill
 * stat. Used in various skills code. -b.t.
 */

int get_skill_stat1(object *op)
{
    int stat_value = 0, stat = NO_STAT_VAL;

    if ((op->chosen_skill) && ((stat = skills[op->chosen_skill->stats.sp].stat1) != NO_STAT_VAL))
        stat_value = get_attr_value(&(op->stats), stat);

    return stat_value;
}

/* get_skill_stat2() - returns the value of the secondary skill
 * stat. Used in various skills code. -b.t.
 */

int get_skill_stat2(object *op)
{
    int stat_value = 0, stat = NO_STAT_VAL;

    if ((op->chosen_skill) && ((stat = skills[op->chosen_skill->stats.sp].stat2) != NO_STAT_VAL))
        stat_value = get_attr_value(&(op->stats), stat);

    return stat_value;
}

/* get_skill_stat3() - returns the value of the tertiary skill
 * stat. Used in various skills code. -b.t.
 */

int get_skill_stat3(object *op)
{
    int stat_value = 0, stat = NO_STAT_VAL;

    if ((op->chosen_skill) && ((stat = skills[op->chosen_skill->stats.sp].stat3) != NO_STAT_VAL))
        stat_value = get_attr_value(&(op->stats), stat);

    return stat_value;
}

/*get_weighted_skill_stats() - */

int get_weighted_skill_stats(object *op)
{
    int value   = 0;

    value = (get_skill_stat1(op) / 2) + (get_skill_stat2(op) / 4) + (get_skill_stat3(op) / 4);

    return value;
}
