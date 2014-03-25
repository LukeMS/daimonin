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

static char     numbers[21][20]         =
{
    "no", "", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen",
    "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen", "twenty"
};

static char     numbers_10[10][20]      =
{
    "zero", "ten", "twenty", "thirty", "fourty", "fifty", "sixty", "seventy", "eighty", "ninety"
};

static char     levelnumbers[21][20]    =
{
    "zeroth", "first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth", "ninth", "tenth", "eleventh",
    "twelfth", "thirteenth", "fourteenth", "fifteenth", "sixteenth", "seventeenth", "eighteen", "nineteen", "twentieth"
};

static char     levelnumbers_10[11][20] =
{
    "zeroth", "tenth", "twentieth", "thirtieth", "fortieth", "fiftieth", "sixtieth", "seventieth", "eightieth",
    "ninetieth"
};

static object *CanReach(object *who, object *what);
static object *CanPickUp(object *who, object *what, object *where, uint32 nrof);
static void    FixAsNeeded(object *who, object *to, object *from, object *what);
static object *CanDiscard(object *who, object *what);
static object *NoDiscardContainer(object *who, object *what);

/* describe_resistance generates the visible naming for resistances.
 * returns a static array of the description.  This can return
 * a big buffer.
 * if newline is true, we don't put parens around the description
 * but do put a newline at the end.  Useful when dumping to files
 */
char * describe_resistance(const object *const op, int newline)
{
    static char buf[LARGE_BUF];
    char        buf1[LARGE_BUF];
    int         tmpvar, flag = 1;

    buf[0] = 0;

    for (tmpvar = 0; tmpvar < NROFATTACKS; tmpvar++)
    {
        if (op->resist[tmpvar] && (op->type != FLESH || atnr_is_dragon_enabled(tmpvar) == 1))
        {
            if (flag)
            {
                if (!newline)
                    strcat(buf, "(resists: ");
            }
            if (!newline)
            {
                if (!flag)
                    strcat(buf, ", ");
                sprintf(buf1, "%s %+d%%", attack_name[tmpvar], op->resist[tmpvar]);
            }
            else
                sprintf(buf1, "%s %d%%\n", attack_name[tmpvar], op->resist[tmpvar]);
            flag = 0;
            strcat(buf, buf1);
        }
    }
    if (!newline && !flag)
        strcat(buf, ") ");
    return buf;
}

/* describe_attacks generates the visible naming for attack forms.
 * returns a static array of the description.  This can return
 * a big buffer.
 * if newline is true, we don't put parens around the description
 * but do put a newline at the end.  Useful when dumping to files
 */
char * describe_attack(const object *const op, int newline)
{
    static char buf[LARGE_BUF];
    char        buf1[LARGE_BUF];
    int         tmpvar, flag = 1;

    buf[0] = 0;

    for (tmpvar = 0; tmpvar < NROFATTACKS; tmpvar++)
    {
        if (op->attack[tmpvar])
        {
            if (flag)
            {
                if (!newline)
                    strcat(buf, "(Attacks: ");
            }
            if (!newline)
            {
                if (!flag)
                    strcat(buf, ", ");
                sprintf(buf1, "%s %+d%%", attack_name[tmpvar], op->attack[tmpvar]);
            }
            else
                sprintf(buf1, "%s %+d%%\n", attack_name[tmpvar], op->attack[tmpvar]);
            flag = 0;
            strcat(buf, buf1);
        }
    }
    if (!newline && !flag)
        strcat(buf, ") ");
    return buf;
}

/*
 * query_weight(object) returns a character pointer to a static buffer
 * containing the text-representation of the weight of the given object.
 * The buffer will be overwritten by the next call to query_weight().
 */

char * query_weight(object *op)
{
    static char buf[10];
    unsigned int i = op->nrof ? op->nrof *op->weight : (unsigned int) (op->weight + op->carrying);

    if (op->weight < 0)
        return "      ";
    if (i % 1000)
        sprintf(buf, "%6.1f", (float) i / 1000.0f);
    else
        sprintf(buf, "%4d  ", i / 1000);
    return buf;
}

/*
 * Returns the pointer to a static buffer containing
 * the number requested (of the form first, second, third...)
 */

char * get_levelnumber(int i)
{
    static char buf[MEDIUM_BUF];
    if (i > 99)
    {
        sprintf(buf, "%d.", i);
        return buf;
    }
    if (i < 21)
        return levelnumbers[i];
    if (!(i % 10))
        return levelnumbers_10[i / 10];
    strcpy(buf, numbers_10[i / 10]);
    strcat(buf, levelnumbers[i % 10]);
    return buf;
}


/*
 * get_number(integer) returns the text-representation of the given number
 * in a static buffer.  The buffer might be overwritten at the next
 * call to get_number().
 * It is currently only used by the query_name() function.
 */

char * get_number(int i)
{
    if (i <= 20)
        return numbers[i];
    else
    {
        static char buf[MEDIUM_BUF];
        sprintf(buf, "%d", i);
        return buf;
    }
}

/*
 * query_short_name(object) is similar to query_name, but doesn't
 * contain any information about object status (worn/cursed/etc.)
 */
char * query_short_name(const object *const op, const object *const caller)
{
    static char buf[HUGE_BUF];
    char        buf2[HUGE_BUF];
    int         len = 0;

    buf[0] = 0;
    if (!op || !op->name)
        return buf;

    /* To speed things up (or make things slower?)
       if(!op->nrof && !op->weight && !op->title && !is_magical(op))
    return op->name;
    */
    if (op->nrof)
    {
        safe_strcat(buf, get_number(op->nrof), &len, sizeof(buf));

        if (op->nrof != 1)
            safe_strcat(buf, " ", &len, sizeof(buf));

        if(!QUERY_FLAG(op, FLAG_IS_NAMED)) /* named items has no prefix */
        {
            if (!IS_LIVE(op) && op->type != TYPE_BASE_INFO)
                safe_strcat(buf, item_race_table[op->item_race].name, &len, sizeof(buf));

            if (op->material_real>0 && QUERY_FLAG(op, FLAG_IDENTIFIED))
                safe_strcat(buf, material_real[op->material_real].name, &len, sizeof(buf));
        }
        safe_strcat(buf, op->name, &len, sizeof(buf));
        if (op->nrof != 1)
        {
            char   *buf3    = strstr(buf, " of ");
            if (buf3 != NULL)
            {
                strcpy(buf2, buf3);
                *buf3 = '\0';   /* also changes value in buf */
            }
            len = strlen(buf);

            /* If buf3 is set, then this was a string that contained
                    * something of something (potion of dexterity.)  The part before
                    * the of gets made plural, so now we need to copy the rest
                    * (after and including the " of "), to the buffer string.
                    */
            if (buf3)
                safe_strcat(buf, buf2, &len, sizeof(buf));
        }
    }
    else
    {
        /* if nrof is 0, the object is not mergable, and thus, op->name
            should contain the name to be used. */

        if(!QUERY_FLAG(op, FLAG_IS_NAMED)) /* named items has no prefix */
        {
            if (!IS_LIVE(op) && op->type != TYPE_BASE_INFO)
                safe_strcat(buf, item_race_table[op->item_race].name, &len, sizeof(buf));

            if (op->material_real>0 && QUERY_FLAG(op, FLAG_IDENTIFIED))
                safe_strcat(buf, material_real[op->material_real].name, &len, sizeof(buf));
        }
        safe_strcat(buf, op->name, &len, sizeof(buf));
    }

    switch (op->type)
    {
        case CONTAINER:
          if (QUERY_FLAG(op, FLAG_IDENTIFIED))
          {
              if (op->title)
              {
                  safe_strcat(buf, " ", &len, sizeof(buf));
                  safe_strcat(buf, op->title, &len, sizeof(buf));
              }
          }

          if (op->sub_type1 >= ST1_CONTAINER_NORMAL_group)
          {
              if (op->sub_type1 == ST1_CONTAINER_CORPSE_group)
              {
                  if(!caller)
                      safe_strcat(buf, " (bounty of a group)", &len, sizeof(buf));
                  else if(CONTR(caller)->group_status & GROUP_STATUS_GROUP &&
                              CONTR(CONTR(caller)->group_leader)->group_id == op->stats.maxhp)
                  {
                      safe_strcat(buf, " (bounty of your group", &len, sizeof(buf));
                      if (QUERY_FLAG(op, FLAG_BEEN_APPLIED))
                          safe_strcat(buf, ", SEARCHED", &len, sizeof(buf));
                      safe_strcat(buf, ")", &len, sizeof(buf));
                  }
                  else /* its a different group */
                      safe_strcat(buf, " (bounty of another group)", &len, sizeof(buf));
              }
          }
          else if (op->sub_type1 >= ST1_CONTAINER_NORMAL_player)
          {
              if (op->sub_type1 == ST1_CONTAINER_CORPSE_player)
              {
                  if (op->slaying)
                  {
                      safe_strcat(buf, " (bounty of ", &len, sizeof(buf));
                      safe_strcat(buf, op->slaying, &len, sizeof(buf));
                      if ((caller && caller->name == op->slaying) &&
                          QUERY_FLAG(op, FLAG_BEEN_APPLIED))
                          safe_strcat(buf, ", SEARCHED", &len, sizeof(buf));
                      safe_strcat(buf, ")", &len, sizeof(buf));
                  }
                  else if (QUERY_FLAG(op, FLAG_BEEN_APPLIED))
                      safe_strcat(buf, " (SEARCHED)", &len, sizeof(buf));
              }
          }
          break;

        case SPELLBOOK:
          if (QUERY_FLAG(op, FLAG_IDENTIFIED) || QUERY_FLAG(op, FLAG_BEEN_APPLIED))
          {
              if (!op->title)
              {
                  safe_strcat(buf, " of ", &len, sizeof(buf));
                  if (op->slaying)
                      safe_strcat(buf, op->slaying, &len, sizeof(buf));
                  else
                  {
                      if (op->stats.sp == SP_NO_SPELL)
                          safe_strcat(buf, "nothing", &len, sizeof(buf));
                      else
                          safe_strcat(buf, spells[op->stats.sp].name, &len, sizeof(buf));
                  }
              }
              else
              {
                  safe_strcat(buf, " ", &len, sizeof(buf));
                  safe_strcat(buf, op->title, &len, sizeof(buf));
              }
          }
          break;

        case SCROLL:
        case WAND:
        case ROD:
        case HORN:
        case POTION:
          if (QUERY_FLAG(op, FLAG_IDENTIFIED) || QUERY_FLAG(op, FLAG_BEEN_APPLIED))
          {
              if (!op->title)
              {
                  if (op->stats.sp != SP_NO_SPELL)
                  {
                      safe_strcat(buf, " of ", &len, sizeof(buf));
                      safe_strcat(buf, spells[op->stats.sp].name, &len, sizeof(buf));
                  }
                  else
                      safe_strcat(buf, " of nothing", &len, sizeof(buf));
              }
              else
              {
                  safe_strcat(buf, " ", &len, sizeof(buf));
                  safe_strcat(buf, op->title, &len, sizeof(buf));
              }
              sprintf(buf2, " (lvl %d)", op->level);
              safe_strcat(buf, buf2, &len, sizeof(buf));
          }
          break;

        case TYPE_SKILL:
        case AMULET:
        case RING:
          if (QUERY_FLAG(op, FLAG_IDENTIFIED))
          {
 
              if (!op->title)
              {
                  /* If ring has a title or is specially named, full description isn't so useful */

                  char     *s   = describe_item(op);
                  if (s[0])
                  {
                      safe_strcat(buf, " ", &len, sizeof(buf));
                      safe_strcat(buf, s, &len, sizeof(buf));
                  }
              }
              else
              {
                  safe_strcat(buf, " ", &len, sizeof(buf));
                  safe_strcat(buf, op->title, &len, sizeof(buf));
              }
          }
          break;

        default:
          if (op->magic && (!need_identify(op) || QUERY_FLAG(op, FLAG_BEEN_APPLIED) || QUERY_FLAG(op, FLAG_IDENTIFIED)))
          {
              if (!IS_LIVE(op) && op->type != TYPE_BASE_INFO)
              {
                  sprintf(buf2, " %+d", op->magic);
                  safe_strcat(buf, buf2, &len, sizeof(buf));
              }
          }
          if (op->title && QUERY_FLAG(op, FLAG_IDENTIFIED))
          {
              safe_strcat(buf, " ", &len, sizeof(buf));
              safe_strcat(buf, op->title, &len, sizeof(buf));
          }
          if (op->type == ARROW && op->slaying)
          {
              safe_strcat(buf, " ", &len, sizeof(buf));
              safe_strcat(buf, op->slaying, &len, sizeof(buf));
          }
    }
    return buf;
}

/*
 * query_name(object) returns a character pointer pointing to a static
 * buffer which contains a verbose textual representation of the name
 * of the given object.
 * cf 0.92.6:  Put in 5 buffers that it will cycle through.  In this way,
 * you can make several calls to query_name before the bufs start getting
 * overwritten.  This may be a bad thing (it may be easier to assume the value
 * returned is good forever.)  However, it makes printing statements that
 * use several names much easier (don't need to store them to temp variables.)
 *
 */
char  *query_name_full(const object *op, const object *caller)
{
    static char buf[5][HUGE_BUF];
    static int  use_buf = 0;
    int         len     = 0;

    use_buf++;
    use_buf %= 5;

    if (!op || !op->name)
    {
        buf[use_buf][0] = 0;
        return buf[use_buf];
    }
    safe_strcat(buf[use_buf], query_short_name(op, caller), &len, HUGE_BUF);

    if (QUERY_FLAG(op, FLAG_ONE_DROP))
        safe_strcat(buf[use_buf], " (~one-drop~)", &len, HUGE_BUF);
    else if (QUERY_FLAG(op, FLAG_QUEST_ITEM))
        safe_strcat(buf[use_buf], " (~quest~)", &len, HUGE_BUF);

    if (QUERY_FLAG(op, FLAG_INV_LOCKED))
        safe_strcat(buf[use_buf], " *", &len, HUGE_BUF);
    if (op->type == CONTAINER && QUERY_FLAG(op, FLAG_APPLIED))
    {
        if (op->attacked_by && op->attacked_by->type == PLAYER)
            safe_strcat(buf[use_buf], " (open)", &len, HUGE_BUF);
        else
            safe_strcat(buf[use_buf], " (ready)", &len, HUGE_BUF);
    }
    if (QUERY_FLAG(op, FLAG_KNOWN_CURSED))
    {
        if (QUERY_FLAG(op, FLAG_PERM_DAMNED))
            safe_strcat(buf[use_buf], " (perm. damned)", &len, HUGE_BUF);
        else if (QUERY_FLAG(op, FLAG_DAMNED))
            safe_strcat(buf[use_buf], " (damned)", &len, HUGE_BUF);
        else if (QUERY_FLAG(op, FLAG_PERM_CURSED))
            safe_strcat(buf[use_buf], " (perm. cursed)", &len, HUGE_BUF);
        else if (QUERY_FLAG(op, FLAG_CURSED))
            safe_strcat(buf[use_buf], " (cursed)", &len, HUGE_BUF);
    }

    if (QUERY_FLAG(op, FLAG_KNOWN_MAGICAL) && is_magical(op))
        safe_strcat(buf[use_buf], " (magical)", &len, HUGE_BUF);
    if (QUERY_FLAG(op, FLAG_APPLIED))
    {
        switch (op->type)
        {
            case BOW:
            case WAND:
            case ROD:
            case HORN:
              safe_strcat(buf[use_buf], " (readied)", &len, HUGE_BUF);
              break;
            case WEAPON:
              safe_strcat(buf[use_buf], " (wielded)", &len, HUGE_BUF);
              break;
            case ARMOUR:
            case SHOULDER:
            case LEGS:
            case HELMET:
            case SHIELD:
            case RING:
            case BOOTS:
            case GLOVES:
            case AMULET:
            case GIRDLE:
            case BRACERS:
            case CLOAK:
              safe_strcat(buf[use_buf], " (worn)", &len, HUGE_BUF);
              break;
            case CONTAINER:
              safe_strcat(buf[use_buf], " (active)", &len, HUGE_BUF);
              break;
            case TYPE_SKILL:
            default:
              safe_strcat(buf[use_buf], " (applied)", &len, HUGE_BUF);
        }
    }
    if (QUERY_FLAG(op, FLAG_UNPAID))
        safe_strcat(buf[use_buf], " (unpaid)", &len, HUGE_BUF);

    return buf[use_buf];
}

/*
 * query_base_name(object) returns a character pointer pointing to a static
 * buffer which contains a verbose textual representation of the name
 * of the given object.  The buffer will be overwritten at the next
 * call to query_base_name().   This is a lot like query_name, but we
 * don't include the item count or item status.  Used for inventory sorting
 * and sending to client.
 */
char *query_base_name(object *op, object *caller)
{
    static char buf[MEDIUM_BUF];
    char        buf2[32];
    int         len;

    buf[0] = '\0';
    if (op->name == NULL)
        return "(null)";

    if(op->sub_type1 == ARROW && op->type == MISC_OBJECT) /* special neutralized arrow! */
        strcat(buf, "broken ");

    if(!QUERY_FLAG(op, FLAG_IS_NAMED)) /* named items has no prefix */
    {
        /* add the item race name */
        if (!IS_LIVE(op) && op->type != TYPE_BASE_INFO)
            strcat(buf, item_race_table[op->item_race].name);
        /* we add the real material name as prefix. Because real_material == 0 is
         * "" (clear string) we don't must check item types for adding something here
         * or not (artifacts for example has normally no material prefix)
         */
        if (op->material_real>0 && QUERY_FLAG(op, FLAG_IDENTIFIED))
            strcat(buf, material_real[op->material_real].name);
    }

    strcat(buf, op->name);

    if (!op->weight && !op->title && !is_magical(op))
        return buf; /* To speed things up (or make things slower?) */

    len = strlen(buf);

    switch (op->type)
    {
        case CONTAINER:
          if (QUERY_FLAG(op, FLAG_IDENTIFIED))
          {
              if (op->title)
              {
                  safe_strcat(buf, " ", &len, sizeof(buf));
                  safe_strcat(buf, op->title, &len, sizeof(buf));
              }
          }

          if (op->sub_type1 >= ST1_CONTAINER_NORMAL_group)
          {
              if (op->sub_type1 == ST1_CONTAINER_CORPSE_group)
              {
                  if(!caller)
                      safe_strcat(buf, " (bounty of a group)", &len, sizeof(buf));
                  else if(CONTR(caller)->group_status & GROUP_STATUS_GROUP &&
                              CONTR(CONTR(caller)->group_leader)->group_id == op->stats.maxhp)
                  {
                      safe_strcat(buf, " (bounty of your group", &len, sizeof(buf));
                      if (QUERY_FLAG(op, FLAG_BEEN_APPLIED))
                          safe_strcat(buf, ", SEARCHED", &len, sizeof(buf));
                      safe_strcat(buf, ")", &len, sizeof(buf));
                  }
                  else /* its a different group */
                      safe_strcat(buf, " (bounty of another group)", &len, sizeof(buf));
              }
          }
          else if (op->sub_type1 >= ST1_CONTAINER_NORMAL_player)
          {
              if (op->sub_type1 == ST1_CONTAINER_CORPSE_player)
              {
                  if (op->slaying)
                  {
                      safe_strcat(buf, " (bounty of ", &len, sizeof(buf));
                      safe_strcat(buf, op->slaying, &len, sizeof(buf));
                      if ((caller && caller->name == op->slaying) &&
                          QUERY_FLAG(op, FLAG_BEEN_APPLIED))
                          safe_strcat(buf, ", SEARCHED", &len, sizeof(buf));
                      safe_strcat(buf, ")", &len, sizeof(buf));
                  }
                  else if (QUERY_FLAG(op, FLAG_BEEN_APPLIED))
                      safe_strcat(buf, " (SEARCHED)", &len, sizeof(buf));
              }
          }
          break;

        case SPELLBOOK:
          if (QUERY_FLAG(op, FLAG_IDENTIFIED))
          {
              if (!op->title)
              {
                  safe_strcat(buf, " of ", &len, sizeof(buf));
                  if (op->slaying)
                      safe_strcat(buf, op->slaying, &len, sizeof(buf));
                  else
                  {
                      if (op->stats.sp == SP_NO_SPELL)
                          safe_strcat(buf, "nothing", &len, sizeof(buf));
                      else
                          safe_strcat(buf, spells[op->stats.sp].name, &len, sizeof(buf));
                  }
              }
              else
              {
                  safe_strcat(buf, " ", &len, sizeof(buf));
                  safe_strcat(buf, op->title, &len, sizeof(buf));
              }
          }
          break;

        case SCROLL:
        case WAND:
        case ROD:
        case HORN:
        case POTION:
          if (QUERY_FLAG(op, FLAG_IDENTIFIED))
          {
              if (!op->title)
              {
                  if (op->stats.sp != SP_NO_SPELL)
                  {
                      safe_strcat(buf, " of ", &len, sizeof(buf));
                      safe_strcat(buf, spells[op->stats.sp].name, &len, sizeof(buf));
                  }
                  else
                      safe_strcat(buf, " of nothing", &len, sizeof(buf));
              }
              else
              {
                  safe_strcat(buf, " ", &len, sizeof(buf));
                  safe_strcat(buf, op->title, &len, sizeof(buf));
              }
              sprintf(buf2, " (lvl %d)", op->level);
              safe_strcat(buf, buf2, &len, sizeof(buf));
          }
          break;

        case TYPE_SKILL:
        case AMULET:
        case RING:
          if (QUERY_FLAG(op, FLAG_IDENTIFIED))
          {
              if (!op->title )
              {
                  /* If ring has a title or is specially named, full description isn't so useful */
                  char     *s   = describe_item(op);
                  if (s[0])
                  {
                      safe_strcat(buf, " ", &len, sizeof(buf));
                      safe_strcat(buf, s, &len, sizeof(buf));
                  }
              }
              else
              {
                  safe_strcat(buf, " ", &len, sizeof(buf));
                  safe_strcat(buf, op->title, &len, sizeof(buf));
              }
          }
          break;
            case MISC_OBJECT:
            if(op->sub_type1 == ARROW) /* special neutralized arrow! */
            {
                if(QUERY_FLAG(op, FLAG_IDENTIFIED))
                {
                    if (op->magic)
                    {
                        sprintf(buf2, " %+d", op->magic);
                        safe_strcat(buf, buf2, &len, sizeof(buf));
                    }
                    if (op->title)
                    {
                        safe_strcat(buf, " ", &len, sizeof(buf));
                        safe_strcat(buf, op->title, &len, sizeof(buf));
                    }
                    if (op->slaying)
                    {
                        safe_strcat(buf, " ", &len, sizeof(buf));
                        safe_strcat(buf, op->slaying, &len, sizeof(buf));
                    }
                }
                return buf;
            }

        default:
          if (op->magic && (!need_identify(op) || QUERY_FLAG(op, FLAG_BEEN_APPLIED) || QUERY_FLAG(op, FLAG_IDENTIFIED)))
          {
              if (!IS_LIVE(op) && op->type != TYPE_BASE_INFO)
              {
                  sprintf(buf2, " %+d", op->magic);
                  safe_strcat(buf, buf2, &len, sizeof(buf));
              }
          }

          if (op->title && (need_identify(op) && QUERY_FLAG(op, FLAG_IDENTIFIED)))
          {
              safe_strcat(buf, " ", &len, sizeof(buf));
              safe_strcat(buf, op->title, &len, sizeof(buf));
          }
          if (op->type == ARROW && op->slaying)
          {
              safe_strcat(buf, " ", &len, sizeof(buf));
              safe_strcat(buf, op->slaying, &len, sizeof(buf));
          }
    } /* switch */

    return buf;
}


/* describe terrain flags
 * we use strcat only - prepare the retbuf before call.
 */
static void describe_terrain(const object *const op, char *const retbuf)
{
    if (op->terrain_flag & TERRAIN_AIRBREATH)
        strcat(retbuf, "(air breathing)");
    if (op->terrain_flag & TERRAIN_WATERWALK)
        strcat(retbuf, "(water walking)");
    if (op->terrain_flag & TERRAIN_FIREWALK)
        strcat(retbuf, "(fire walking)");
    if (op->terrain_flag & TERRAIN_CLOUDWALK)
        strcat(retbuf, "(cloud walking)");
    if (op->terrain_flag & TERRAIN_WATERBREATH)
        strcat(retbuf, "(water breathing)");
    if (op->terrain_flag & TERRAIN_FIREBREATH)
        strcat(retbuf, "(fire breathing)");
}

/*
 * Returns a pointer to a static buffer which contains a
 * description of the given object.
 * If it is a monster, lots of information about its abilities
 * will be returned.
 * If it is an item, lots of information about which abilities
 * will be gained about its user will be returned.
 * If it is a player, it writes out the current abilities
 * of the player, which is usually gained by the items applied.
 */
/* i rewrite this to describe *every* object in the game.
 * That includes a description of every flag, etc.
 * MT-2003 */
char * describe_item(const object *const op)
{
    object     *tmp;
    int         attr, val, more_info = 0, id_true = FALSE;
    char        buf[MEDIUM_BUF];
    static char retbuf[LARGE_BUF*3];

    retbuf[0] = '\0';

    /* we start with living objects like mobs */
    if (op->type == PLAYER)
    {
        player *pl = CONTR(op);

        describe_terrain(op, retbuf);

        if (pl->gen_grace)
        {
            sprintf(buf, "(grace gain%+d%%)", pl->gen_grace);
            strcat(retbuf, buf);
        }
        if (pl->gen_sp)
        {
            sprintf(buf, "(mana gain%+d%%)", pl->gen_sp);
            strcat(retbuf, buf);
        }
        if (pl->gen_hp)
        {
            sprintf(buf, "(regeneration%+d%%)", pl->gen_hp);
            strcat(retbuf, buf);
        }
    }
    else if (QUERY_FLAG(op, FLAG_MONSTER))
    {
        describe_terrain(op, retbuf);

        if (QUERY_FLAG(op, FLAG_UNDEAD))
            strcat(retbuf, "(undead)");
        if (QUERY_FLAG(op, FLAG_INVULNERABLE))
            strcat(retbuf, "(invulnerable)");
        if (QUERY_FLAG(op, FLAG_NO_ATTACK))
            strcat(retbuf, "(never attacks)");
        if (QUERY_FLAG(op, FLAG_CAN_PASS_THRU))
            strcat(retbuf, "(pass through doors)");
        if (QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
            strcat(retbuf, "(see invisible)");

        if (QUERY_FLAG(op, FLAG_USE_WEAPON))
            strcat(retbuf, "(melee attack)");
        if (QUERY_FLAG(op, FLAG_READY_BOW))
            strcat(retbuf, "(range attack)");
        if (QUERY_FLAG(op, FLAG_USE_ARMOUR))
            strcat(retbuf, "(wear armour)");
        if (QUERY_FLAG(op, FLAG_USE_RING))
            strcat(retbuf, "(wear ring)");
        if (QUERY_FLAG(op, FLAG_FRIENDLY))
            strcat(retbuf, "(NPC)");
        if (QUERY_FLAG(op, FLAG_UNAGGRESSIVE))
            strcat(retbuf, "(unaggressive)");
        else
            strcat(retbuf, "(aggressive)");
        if (QUERY_FLAG(op, FLAG_HITBACK))
            strcat(retbuf, "(hitback)");
        if (QUERY_FLAG(op, FLAG_READY_SPELL))
            strcat(retbuf, "(spellcaster)");
        if (QUERY_FLAG(op, FLAG_CAN_OPEN_DOOR))
            strcat(retbuf, "(open doors)");
        if (QUERY_FLAG(op, FLAG_RANDOM_MOVE))
            strcat(retbuf, "(wandering)");

        /* lets check the inv for spell ABILITY objects.
             * This was previous a randomitems search, but that
             * was wrong because its possible we use a random list to
             * generate different instances of this mob/item
             */
        for (tmp = op->inv; tmp ; tmp = tmp->below)
        {
            if (tmp && (tmp->type == ABILITY))
            {
                strcat(retbuf, "(");
                strcat(retbuf, STRING_SAFE(tmp->name));
                strcat(retbuf, ")");
            }
        }

        if (FABS(op->speed) > MIN_ACTIVE_SPEED)
        {
            switch ((int) ((FABS(op->speed)) * 15))
            {
                case 0:
                  strcat(retbuf, "(very slow movement)");
                  break;
                case 1:
                  strcat(retbuf, "(slow movement)");
                  break;
                case 2:
                  strcat(retbuf, "(normal movement)");
                  break;
                case 3:
                case 4:
                  strcat(retbuf, "(fast movement)");
                  break;
                case 5:
                case 6:
                  strcat(retbuf, "(very fast movement)");
                  break;
                case 7:
                case 8:
                case 9:
                case 10:
                  strcat(retbuf, "(extremely fast movement)");
                  break;
                default:
                  strcat(retbuf, "(lightning fast movement)");
                  break;
            }
        }
    }
    else /* here we handle items... */
    {
        if (QUERY_FLAG(op, FLAG_IDENTIFIED) || QUERY_FLAG(op, FLAG_BEEN_APPLIED) || !need_identify(op))
            id_true = TRUE; /* we only need calculate this one time */

        /* terrain flags have no double use... if valid, show them */
        if (id_true && op->terrain_type)
            describe_terrain(op, retbuf);

        /* now lets deal with special cases */
        switch (op->type)
        {
            case WAND:
            case ROD:
            case HORN:
              if (id_true)
              {
                  sprintf(buf, "(%1.2f sec)", ((float) op->last_grace / pticks_second));
                  strcat(retbuf, buf);
              }
              break;

              /* Armor type objects */
            case ARMOUR:
            case HELMET:
            case SHOULDER:
            case LEGS:
            case SHIELD:
            case BOOTS:
            case GLOVES:
            case GIRDLE:
            case BRACERS:
            case CLOAK:
              if (id_true)
              {
                  if (ARMOUR_SPEED(op))
                  {
                      sprintf(buf, "(encumbrance%+.1f%%)", ARMOUR_SPEED(op) / 10.0);
                      strcat(retbuf, buf);
                  }
                  /* Do this in all cases - otherwise it gets confusing - does that
                             * item have no penality, or is it not fully identified for example.
                             */
                  if (ARMOUR_SPELLS(op))
                  {
                      sprintf(buf, "(casting penalty%+.1f%%)", ARMOUR_SPELLS(op)/10.0);
                      strcat(retbuf, buf);
                  }
              }
            case WEAPON:
            case RING:
            case AMULET:
            case FORCE:
              more_info = 1;

            case BOW:
            case ARROW:
              if (id_true)
              {
                  float dps_swing = 0.0f;

                  if (op->type == BOW || op->type == ARROW)
                      dps_swing = ((float) op->last_grace * WEAPON_SWING_TIME);

                  if (op->stats.dam)
                  {
                      int i, tmp_dam=0;

                      /* Show DPS */
                      /* These are "direct" damage dealers where we have a swing speed & dmg */
                      if(op->type == WEAPON || (op->type == ARROW && op->sub_type1 > 127) || op->type == BOW ||
                          op->type == PLAYER || op->type == MONSTER)
                      {
                          float dps = (float)op->stats.dam;

                          /* adjust damage with quality. DON'T adjust by /10 - this will
                          * give us a better value. Docs has to explain that the DPS of weapons
                          * is shown for a (ideal) level 10 char. Note that ->magic will be added
                          * to dam AND wc - for dam it will be handled as 0.1 additional damage.
                          * For this items this is only the "base DPS"
                          */
                          if(op->type == WEAPON || op->type == ARROW || op->type == BOW)
                              dps = (dps * (op->item_quality / 100.0f)) + op->magic;

                          if(!dps_swing) /* not a bow or arrow then use weapon_speed as swing speed */
                              dps_swing = op->weapon_speed;

                          if(dps_swing)
                              dps /= dps_swing;

                          /* we can have more as one attack - and so more as 100% damage (or less)
                          * so we need to "collect" and multiply the attack damage
                          * ONLY use the real damage attack forms
                          */
                          for(i=0;i<=LAST_ATNR_ATTACK;i++)
                              tmp_dam += op->attack[i];
                          tmp_dam += op->attack[ATNR_INTERNAL];

                          if(tmp_dam)
                              dps *= ((float)tmp_dam)/100.0f;

                          sprintf(buf, "(dps %.1f)", dps);
                      }
                      /* all what a player can apply has dam*10 value */
                      else if( op->type == AMULET || op->type == RING ||
                          op->type == BOOTS || op->type == HELMET || op->type == BRACERS || op->type == GIRDLE ||
                          op->type == CLOAK || op->type == ARMOUR || op->type == SHIELD || op->type == GLOVES ||
                          op->type == SHOULDER || op->type == LEGS || (op->type == ARROW && op->sub_type1 < 127))
                          sprintf(buf, "(dam%+.1f)", ((float)op->stats.dam)/10.0f);
                      else
                          sprintf(buf, "(dam%+d)", op->stats.dam);
                      strcat(retbuf, buf);
                  }

                  if (op->stats.wc)
                  {
                      sprintf(buf, "(wc%+d)", op->stats.wc);
                      strcat(retbuf, buf);
                  }
                  if (op->stats.thac0)
                  {
                      sprintf(buf, "(hit chancet%+d)", op->stats.thac0);
                      strcat(retbuf, buf);
                  }
                  if (op->stats.thacm)
                  {
                      sprintf(buf, "(fumble%+d)", op->stats.thacm);
                      strcat(retbuf, buf);
                  }
                  if (op->stats.ac)
                  {
                      sprintf(buf, "(ac%+d)", op->stats.ac);
                      strcat(retbuf, buf);
                  }

                  if (op->type == ARROW || op->type == BOW)
                  {
                      if(op->last_grace)
                      {
                          sprintf(buf, "(%1.2f sec)", dps_swing);
                          strcat(retbuf, buf);
                      }

                      if (op->last_sp)
                      {
                          sprintf(buf, "(range%+d)", op->last_sp);
                          strcat(retbuf, buf);
                      }
                  }
                  else if (op->type == WEAPON)
                  {
                      sprintf(buf, "(%1.2f sec)", op->weapon_speed);
                      strcat(retbuf, buf);

                      if (op->level > 0)
                      {
                          sprintf(buf, "(improved %d/%d)", op->last_eat, op->level);
                          strcat(retbuf, buf);
                      }
                  }
              }
              break;

            case FOOD:
            case FLESH:
            case DRINK:
              if (id_true)
              {
                  if (op->type == FLESH && op->last_eat > 0 && atnr_is_dragon_enabled(op->last_eat))
                  {
                      sprintf(strchr(retbuf, '\0'), "(%s metabolism)", attack_name[op->last_eat]);
                  }

                  if (op->stats.thac0)
                  {
                      sprintf(strchr(retbuf, '\0'), "(hit%+d)", op->stats.thac0);
                  }

                  if (op->stats.thacm)
                  {
                      sprintf(strchr(retbuf, '\0'), "(miss%+d)", op->stats.thacm);
                  }

                  if (op->last_eat <= 0)
                  {
                      strcat(retbuf, "\nIt has no restorative qualities.");
                  }
                  else
                  {
                      strcat(retbuf, "\nIf consumed it will ");

                      if (op->stats.hp > 0)
                      {
                          sprintf(strchr(retbuf, '\0'), "restore %d hp%s",
                                  op->last_eat * op->stats.hp,
                                  (op->stats.sp > 0 && op->stats.grace > 0) ? "," : "");
                      }

                      if (op->stats.sp > 0)
                      {
                          sprintf(strchr(retbuf, '\0'), "%s%d mana%s",
                                  (op->stats.hp > 0) ? " and " : "restore ",
                                  op->last_eat * op->stats.sp,
                                  (op->stats.hp > 0 && op->stats.grace > 0) ? "," : "");
                      }

                      if (op->stats.grace > 0)
                      {
                          sprintf(strchr(retbuf, '\0'), "%s%d grace",
                                  (op->stats.hp > 0 || op->stats.sp > 0) ? " and " : "restore ",
                                  op->last_eat * op->stats.grace);
                      }

                      if (op->stats.hp < 0)
                      {
                          sprintf(strchr(retbuf, '\0'), " but reduce %d hp%s",
                                  ABS(op->last_eat * op->stats.hp),
                                  (op->stats.sp < 0 && op->stats.grace < 0) ? "," : "");
                      }

                      if (op->stats.sp < 0)
                      {
                          sprintf(strchr(retbuf, '\0'), "%s%d mana%s",
                                  (op->stats.hp < 0) ? " and " : " but reduce ",
                                  ABS(op->last_eat * op->stats.sp),
                                  (op->stats.hp < 0 && op->stats.grace < 0) ? "," : "");
                      }

                      if (op->stats.grace < 0)
                      {
                          sprintf(strchr(retbuf, '\0'), "%s%d grace",
                                  (op->stats.hp < 0 || op->stats.sp < 0) ? " and " : " but reduce ",
                                  ABS(op->last_eat * op->stats.grace));
                      }

                      sprintf(strchr(retbuf, '\0'), " over %d seconds.", op->last_eat);
                  }
              }
              break;

            case POTION:
              if (id_true)
              {
                  if (op->stats.thac0)
                  {
                      sprintf(buf, "(hit%+d)", op->stats.thac0);
                      strcat(retbuf, buf);
                  }
                  if (op->stats.thacm)
                  {
                      sprintf(buf, "(miss%+d)", op->stats.thacm);
                      strcat(retbuf, buf);
                  }
                  if (op->last_sp)
                  {
                      sprintf(buf, "(range%+d)", op->last_sp);
                      strcat(retbuf, buf);
                  }
              }
              /* potions, scrolls, ...*/
            default:
              /* no more infos for all items we don't have handled in the switch */
              return retbuf;
        }

        /* these counts for every "normal" item a player deals with - most times equipement */
        if (id_true)
        {
            for (attr = 0; attr < 7; attr++)
            {
                if ((val = get_stat_value(&(op->stats), attr)) != 0)
                {
                    sprintf(buf, "(%s%+d)", short_stat_name[attr], val);
                    strcat(retbuf, buf);
                }
            }

            if (op->stats.exp)
            {
                sprintf(buf, "(speed %+d)", op->stats.exp);
                strcat(retbuf, buf);
            }
        }
    }

    /* some special info for some kind of identified items */
    if (id_true && more_info)
    {
        if (op->stats.sp)
        {
            sprintf(buf, "(mana gain%+d%%)", op->stats.sp);
            strcat(retbuf, buf);
        }
        if (op->stats.grace)
        {
            sprintf(buf, "(grace gain%+d%%)", op->stats.grace);
            strcat(retbuf, buf);
        }
        if (op->stats.hp)
        {
            sprintf(buf, "(regeneration%+d%%)", op->stats.hp);
            strcat(retbuf, buf);
        }
    }

    /* here we deal with all the special flags */
    if (id_true || QUERY_FLAG(op, FLAG_MONSTER) || op->type == PLAYER)
    {
        if (QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
            strcat(retbuf, "(see invisible)");
        if (QUERY_FLAG(op, FLAG_MAKE_ETHEREAL))
            strcat(retbuf, "(makes ethereal)");
        if (QUERY_FLAG(op, FLAG_IS_ETHEREAL))
            strcat(retbuf, "(ethereal)");
        if (QUERY_FLAG(op, FLAG_MAKE_INVISIBLE))
            strcat(retbuf, "(makes invisible)");
        if (QUERY_FLAG(op, FLAG_IS_INVISIBLE))
            strcat(retbuf, "(invisible)");
        if (QUERY_FLAG(op, FLAG_XRAYS))
            strcat(retbuf, "(xray-vision)");
        if (QUERY_FLAG(op, FLAG_SEE_IN_DARK))
            strcat(retbuf, "(infravision)");
        if (QUERY_FLAG(op, FLAG_LIFESAVE))
            strcat(retbuf, "(lifesaving)");
        if (QUERY_FLAG(op, FLAG_REFL_SPELL) || QUERY_FLAG(op, FLAG_CAN_REFL_SPELL))
            strcat(retbuf, "(reflect spells)");
        if (QUERY_FLAG(op, FLAG_REFL_MISSILE) || QUERY_FLAG(op, FLAG_CAN_REFL_MISSILE))
            strcat(retbuf, "(reflect missiles)");
        if (QUERY_FLAG(op, FLAG_STEALTH))
            strcat(retbuf, "(stealth)");
        if (QUERY_FLAG(op, FLAG_FLYING))
            strcat(retbuf, "(flying)");
        if (QUERY_FLAG(op, FLAG_LEVITATE))
            strcat(retbuf, "(levitate)");
    }
    if (id_true)
    {
        if (op->slaying != NULL)
        {
            strcat(retbuf, "(slay ");
            strcat(retbuf, op->slaying);
            strcat(retbuf, ")");
        }

        /* for dragon players display the attacks from clawing skill */
        /* must convert this for AV dragons later... */
        /*
            if (is_dragon_pl(op)) {
            object *tmp;
                for (tmp=op->inv; tmp!=NULL && !(tmp->type == TYPE_SKILL &&
                strcmp(tmp->name, "clawing")==0); tmp=tmp->below);
            */

        strcat(retbuf, describe_attack(op, 0));

        /* resistance on flesh is only visible for quetzals */
        if (op->type != FLESH || QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
            strcat(retbuf, describe_resistance(op, 0));
        DESCRIBE_PATH(retbuf, op->path_attuned, "Attuned");
        DESCRIBE_PATH(retbuf, op->path_repelled, "Repelled");
        DESCRIBE_PATH(retbuf, op->path_denied, "Denied");

        if (op->stats.maxhp && (op->type != HORN && op->type != ROD && op->type != WAND))
        {
            sprintf(buf, "(hp%+d)", op->stats.maxhp);
            strcat(retbuf, buf);
        }
        if (op->stats.maxsp)
        {
            sprintf(buf, "(mana%+d)", op->stats.maxsp);
            strcat(retbuf, buf);
        }
        if (op->stats.maxgrace)
        {
            sprintf(buf, "(grace%+d)", op->stats.maxgrace);
            strcat(retbuf, buf);
        }
    }
    return retbuf;
}

/* need_identify returns true if the item should be identified.  This
 * function really should not exist - by default, any item not identified
 * should need it.
 */

int need_identify(const object *const op)
{
    switch (op->type)
    {
        case RING:
        case WAND:
        case ROD:
        case HORN:
        case SCROLL:
        case TYPE_SKILL:
        case SPELLBOOK:
        case FOOD:
        case POTION:
        case BOW:
        case ARROW:
        case WEAPON:
        case ARMOUR:
        case SHIELD:
        case HELMET:
        case SHOULDER:
        case LEGS:
        case AMULET:
        case BOOTS:
        case GLOVES:
        case BRACERS:
        case GIRDLE:
        case CONTAINER:
        case DRINK:
        case FLESH:
        case INORGANIC:
        case CLOSE_CON:
        case CLOAK:
        case GEM:
        case TYPE_PEARL:
        case TYPE_JEWEL:
        case TYPE_NUGGET:
        case POWER_CRYSTAL:
        case POISON:
        case BOOK:
        case TYPE_LIGHT_APPLY:
        case TYPE_LIGHT_REFILL:
          return 1;
    }

    return 0;
}


/*
 * Supposed to fix face-values as well here, but later.
 */

void identify(object *op)
{
    if (!op)
    {
        return;
    }

    SET_FLAG(op, FLAG_IDENTIFIED);
    CLEAR_FLAG(op, FLAG_KNOWN_CURSED);
    CLEAR_FLAG(op, FLAG_KNOWN_MAGICAL);
    CLEAR_FLAG(op, FLAG_NO_SKILL_IDENT);

    /*
     * We want autojoining of equal objects:
     */
    if (is_cursed_or_damned(op))
        SET_FLAG(op, FLAG_KNOWN_CURSED);

    if (is_magical(op))
        SET_FLAG(op, FLAG_KNOWN_MAGICAL);

    if (op->type == POTION && op->arch != (archetype *) NULL)
    {
        /*op->face = op->arch->clone.face; */
        FREE_AND_ADD_REF_HASH(op->name, op->arch->clone.name);
    }
    else if (op->type == SPELLBOOK && op->slaying != NULL)
    {
        if ((op->stats.sp = look_up_spell_name(op->slaying)) < 0)
        {
            char    buf[256];
            op->stats.sp = -1;
            sprintf(buf, "Spell formula for %s", op->slaying);
            FREE_AND_COPY_HASH(op->name, buf);
        }
        else
        {
            /* clear op->slaying since we no longer need it */
            FREE_AND_CLEAR_HASH(op->slaying);
        }
    }

    if (op->map) /* The shop identifies items before they hit the ground */
        /* I don't think identification will change anything but face */
        update_object(op, UP_OBJ_FACE);
    else
    {
        /* A lot of the values can change from an update - might as well send
         * it all. */
        /* Seems very cavalier with BW -- looks to me like an
         * esrv_update_item() with one or more of UPD_FLAGS | UPD_FACE |
         * UPD_NAME would do it.
         *
         * -- Smacky 20130131 */
        esrv_send_item(op);
    }
}

/* check a object marked with
 * FLAG_IS_TRAPED has still a known
 * trap in it!
 */
void set_traped_flag(object *op)
{
    object *tmp;
    int     flag;

    if (!op)
        return;

    /* player & monsters are not marked */
    if (op->type == PLAYER || op->type == MONSTER)
        return;

    flag = QUERY_FLAG(op, FLAG_IS_TRAPED);
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
    {
        /* must be a rune AND visible */
        if (tmp->type == RUNE && tmp->stats.Cha <= 1)
        {
            SET_FLAG(op, FLAG_IS_TRAPED);
            if (!flag)
                goto set_traped_view;
            return;
        }
    }

    CLEAR_FLAG(op, FLAG_IS_TRAPED); /* clean */
    if (!flag)
        return;

    set_traped_view:
    if (!op->env) /* env object is on map */
        update_object(op, UP_OBJ_FACE);
    else /* somewhere else - if visible, update */
    {
        esrv_update_item(UPD_FLAGS, op);
    }
}

/* We don't want allow to put a magical container inside another
 * magical container. This function returns FALSE when op can be
 * put in env of TRUE when both are magical.
 */
int check_magical_container(const object *op, const object *env)
{
    /* is op a magical container? */
    if(!op || op->type != CONTAINER || op->weapon_speed == 1.0f)
    {
        return FALSE;
    }

    for(;env;env = env->env)
    {
        if (env->type == CONTAINER && env->weapon_speed != 1.0f)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/* pick_up() is called when who attempts to pick up nrof * what. If where is
 * non-NULL it is a container into which what will be put (pick up to
 * container) otherwise what is moved into who's inv (pick up to inventory).
 *
 * The return is what (but possibly with a different ->nrof, certainly with a
 * different ->env, etc than before the function) if what was picked up or NULL
 * if it wasn't. */
object *pick_up(object *who, object *what, object *where, uint32 nrof)
{
    player   *pl = NULL;
    object   *from,
             *to = who,
             *outof;
    MapSpace *msp;
    object   *shop;

    /* STEP 1: Check that the parameters are sensible. */
    /* Sanity checks: must be a who and only players and monsters can pick
     * things up; must be a what; if there is a where, it must be a
     * container (and for non-gmaster wiz players, it must be their linked
     * container). */
    if (!who ||
        (who->type != MONSTER &&
         (who->type != PLAYER ||
          !(pl = CONTR(who)))) ||
        !what ||
        (where &&
         (((!pl ||
            pl->gmaster_wiz) &&
           where->type != CONTAINER) ||
          (pl &&
           where != pl->container))))
    {
        return NULL;
    }

    /* Make sure who can reach what (and where if there is one) -- see
     * CanReach(). */
    if (!(from = CanReach(who, what)) ||
        (where &&
         !(to = CanReach(who, where))))
    {
        return NULL;
    }

    outof = what->env;
    msp = GET_MAP_SPACE_PTR(who->map, who->x, who->y);
    GET_MAP_SPACE_SYS_OBJ(msp, SHOP_FLOOR, shop);

    /* Ensure nrof is a sensible value. */
    nrof = MAX(1, MIN(nrof, MAX_OBJ_NROF));

    if (nrof > what->nrof)
    {
        /* In shops any number of unpaid items can be picked up off the floor,
         * regardless of stack size. */
        if (!shop ||
            !what->map ||
            !QUERY_FLAG(what, FLAG_UNPAID))
        {
            nrof = what->nrof;
        }
    }

    /* STEP 2: Check that who actually can pick up nrof * what. */
    /* There are several reasons why what can't be picked up -- see
     * CanPickUp(). */
    if (!CanPickUp(who, what, where, nrof))
    {
        return NULL;
    }

    /* STEP 3: Sort out special cases of where. */
    /* For a pick up to inv (ie, where is NULL), who might have an appropriate
     * readied container in his inventory, so check that. */
    if (!where &&
        from != who)
    {
        object *this;

        for (this = who->inv; this; this = this->below)
        {
            /* The container must be applied (ready/open). */
            if (this->type == CONTAINER &&
                QUERY_FLAG(this, FLAG_APPLIED))
            {
                /* It must have enough space for what. */
                if (this->weight_limit == 0 ||
                    (this->weight_limit > 0 &&
                     this->weight_limit > this->carrying + WEIGHT_OVERALL(what)))
                {
                    /* Some containers can only hold specific classes of item
                     * (eg, pouches for coins) while others can hold anything.
                     * If we find a specific container that matches the item
                     * class being picked up, use that. Otherwise, use the
                     * first generic one found. */
                    /* TODO: Improve and expand these specific containers. */
                    if (!where &&
                        !this->race)
                    {
                        where = this;
                    }
                    else if (this->race == what->race)
                    {
                        where = this;
                        break;
                    }
                }
            }
        }
    }

    /* STEP 4: At this point we've determined that its theoretically OK for who
     * to pick up nrof * what. Now we need to determine the type of pick up (to
     * inv or to container), either of which involves further side
     * effects/checks. */
    /* If where is (still) NULL (see above) we're trying to pick up what to
     * who's inv. */
    if (!where)
    {
        /* If what is a container be sure to unlink (close) it for all players
         * viewing it. */
        if (what->type == CONTAINER)
        {
            container_unlink(NULL, what);
        }
    }
    /* When where is non-NULL we're trying to put what in it. */
    else
    {
        /* Containers can't be put in other containers. */
        if (what->type == CONTAINER)
        {
            if (pl)
            {
                new_draw_info(NDI_UNIQUE, 0, who, "Containers can't be put in other containers!");
            }

            return NULL;
        }
        /* where may be a container which cannot hold what. */
        else if ((where->sub_type1 & 1) != ST1_CONTAINER_CORPSE &&
                 where->race &&
                 where->race != what->race)
        {
            if (pl)
            {
                new_draw_info(NDI_UNIQUE, 0, who, "Only %s can be put into %s!",
                    where->race, query_name(where));
            }

            return NULL;
        }

        /* If an applyable light is lit (has glow_radius) force it to be
         * FLAG_APPLIED so we can unapply it below. This prevents players storing
         * lit torches in bags. */
        if (what->type == TYPE_LIGHT_APPLY &&
            what->glow_radius)
        {
            SET_FLAG(what, FLAG_APPLIED);
            apply_light(who, what);
        }

        /* Picking up to a container not in who's inventory is similar to
         * dropping in some respects so do the necessary checks -- see
         * CanDiscard(). */
        if (to != who &&
            !CanDiscard(who, what))
        {
            return NULL;
        }
    }

    /* STEP 5: Now we've determined the pick up is possible, so lets do it! */
    /* what can have a PICKUP script on it which, if it returns true, aborts
     * the actual pick up. */
    if (trigger_object_plugin_event(EVENT_PICKUP, what, who, where, NULL,
            (int *)&nrof, NULL, NULL, SCRIPT_FIX_ALL))
    {
        return NULL;
    }

    /* where can also have a PICKUP script on it which, if it returns true,
     * aborts the actual pick up. */
    if (where &&
        trigger_object_plugin_event(EVENT_PICKUP, where, who, what, NULL,
            (int *)&nrof, NULL, NULL, SCRIPT_FIX_ALL))
    {
        return NULL;
    }

    /* Picking things up from or to outside of who's inventory interrupts
     * resting. */
    if ((from != who ||
         to != who) &&
        pl)
    {
        pl->rest_mode = 0;
    }

    /* Below here is the actual code to move what to a new environment (either
     * where or who). Also we send some appropriate messages to the client. */
    /* Picking up unpaid items from a shop floor is preliminary to buying
     * them. */
    if (shop &&
        what->map &&
        QUERY_FLAG(what, FLAG_UNPAID) &&
        !QUERY_FLAG(what, FLAG_SYS_OBJECT) &&
        what->layer)
    {
        what = clone_object(what, CLONE_WITHOUT_INVENTORY);
        what->nrof = nrof;

        if (pl)
        {
            new_draw_info(NDI_UNIQUE, 0, who, "You can buy %s for ~%s~ from %s.\n",
                query_name(what),
                query_cost_string(what, who, F_BUY, COSTSTRING_SHORT),
                query_name(shop));
        }
    }
    else
    {
        if (nrof != what->nrof)
        {
            what = get_split_ob(what, nrof);
        }
        else if (!QUERY_FLAG(what, FLAG_REMOVED))
        {
            if (what->map)
            {
                remove_ob(what);

                if (check_walk_off(what, NULL, MOVE_APPLY_VANISHED) != CHECK_WALK_OK)
                {
                    return NULL;
                }
            }
            else
            {
                remove_ob(what);
            }
        }
    }

    /* Give players an appropriate message to tell them what they've done. */
    /* TODO: Currently the client plays a pick up sound whenever a pick up is
     * attempted. This means when the server decides the attempt has failed, a
     * sound is still played. This should be moved server-side. Then also the
     * client can handle the 'you pick up...' messages on receipt of the sound
     * cmd. */
    if (pl)
    {
        char buf[LARGE_BUF];

        if (outof &&
            outof != who)
        {
            sprintf(buf, "Taking %s out of %s, you ",
                (nrof > 1) ? "them" : "it", query_name(outof));
        }
        else
        {
            sprintf(buf, "You ");
        }

        if (from == who)
        {
            sprintf(strchr(buf, '\0'), "%s %s into %s",
                (to == who) ? "transfer" : "put", query_name(what),
                (where) ? query_name(where) : "your inventory");
        }
        else
        {
            sprintf(strchr(buf, '\0'), "pick up %s", query_name(what));

            if (where)
            {
                sprintf(strchr(buf, '\0'), " and put %s in %s",
                    (nrof > 1) ? "them" : "it", query_name(where));
            }
        }

        new_draw_info(NDI_UNIQUE, 0, who, "%s.", buf);
    }

    /* Yay! we're finally done. So insert what into its new environment. */
    what = insert_ob_in_ob(what, (where) ? where : who);

    FixAsNeeded(who, to, from, what);

    return what;
}

/* CanReach() ascertains if what is close enough that who can touch it. That
 * means what must be either (in a container) in who's inventory or (in a
 * container) on the same map square as who -- see below for some exceptions to
 * this basic rule.
 *
 * The return is NULL is what is not in reach of who or, if it is, an object
 * which is one of: what if what is directly on a map; a player or monster
 * (which may be who) if what is (in a container) in that creature's inv; a
 * container which is directly on a map.
 *
 * If who is a player an ndi is sent to the player when what is out of
 * reach. */
static object *CanReach(object *who, object *what)
{
    object *this;

    /* Walk up what's environments until the one before the map. In a textbook
     * scenario if what is (in a container) in who's inv then this should be
     * who (players and monters must not have envs).
     *
     * Afterwards this points to an object, one of: what if what is directly on
     * a map; a player or monster (which may be who) if what is (in a
     * container) in that creature's inv; a container which is directly on a
     * map. */
    for (this = what; this->env; this = this->env)
    {
        ;
    }

    /* If either who or this are airborne and the other is not (and what is not
     * in who's inventory), who can't reach what. */
    /* TODO: This makes no distinction between flying/levitating. */
    if (this != who &&
        IS_AIRBORNE(who) != IS_AIRBORNE(this))
    {
        if (who->type == PLAYER)
        {
            new_draw_info(NDI_UNIQUE, 0, who, "%s is floating out of your reach!",
                query_name(what));
        }

        return NULL;
    }

    /* Gmaster wiz players can reach things in other creature's invs
     * (including in containers in those invs). All others can only fiddle with
     * themselves. */
    if (this->type == PLAYER ||
        this->type == MONSTER)
    {
        if (this != who &&
            (who->type == MONSTER ||
             !CONTR(who) ||
             !CONTR(who)->gmaster_wiz))
        {
            this = NULL;
        }
    }
    /* Gmaster wiz players can reach things in any container (open or closed).
     * For convenience so can monsters. Other players are limited to their
     * linked container. */
    else if (this != what &&
             this->type == CONTAINER)
    {
        if (who->type == PLAYER &&
            !CONTR(who)->gmaster_wiz &&
            this != CONTR(who)->container)
        {
            this = NULL;
        }
    }

    /* If this does not point to who it must be directly on a map so check that
     * it occupies the same space as who. */
    if (this &&
        this != who)
    {
        if (this->map != who->map ||
            this->x != who->x ||
            this->y != who->y)
        {
            this = NULL;
        }
    }

    if (!this &&
        who->type == PLAYER)
    {
        new_draw_info(NDI_UNIQUE, 0, who, "%s is out of your reach!",
            query_name(what));
    }

    return this;
}

/* CanPickUp() determines if who can pick up (if appropriate, nrof *) what
 * (into where if not NULL)>
 *
 * The return is what if who can pick it up, or NULL if who can't.
 *
 * If who is a player an ndi is sent to the player when what can't be picked
 * up. */
static object *CanPickUp(object *who, object *what, object *where, uint32 nrof)
{
    /* Multiparts are absolute no-nos because the server cannot cope with such
     * objects having an environment. Also it would be logically ridiculous to
     * have such physically large objects being pocketed. */
    /* TODO: This restriction will be lifted fo gmaster wiz players. */
    if (what->more ||
        what->head)
    {
        if (who->type == PLAYER)
        {
            new_draw_info(NDI_UNIQUE, 0, who, "%s is too large for you to pick up!",
                query_name(what));
        }

        return NULL;
    }

    /* Normal players and mobs cannot pick up any of these items but gmaster
     * wiz's can. */
    if (!IS_GMASTER_WIZ(who))
    {
        int     ego_mode;
        object *this;

        /* If what is somebody else's ego item then who can't pick it up. */
        if ((ego_mode = check_ego_item(who, what)) &&
            ego_mode == EGO_ITEM_BOUND_PLAYER)
        {
            if (who->type == PLAYER)
            {
                new_draw_info (NDI_UNIQUE, 0, who, "%s is not your ego item!",
                    query_name(what));
            }

            return NULL;
        }

        /* Layer 0/sys objects can't be picked up (really the one should imply the
         * other but JIC). TODO: Perhaps be strict about all layers? Really only
         * layers 3 and 4 should be pickable. */
        if (what->layer == 0 ||
            QUERY_FLAG(what, FLAG_SYS_OBJECT) ||
        /* No_picks can't be picked up. */
            QUERY_FLAG(what, FLAG_NO_PICK) ||
        /* If you can't see it, you can't pick it up. */
            IS_NORMAL_INVIS_TO(what, who))
        {
            if (who->type == PLAYER)
            {
                new_draw_info(NDI_UNIQUE, 0, who, "%s is not something you can pick up!",
                   query_name(what));
            }

            return NULL;
        }

        for (this = what; this->env; this = this->env)
        {
            ;
        }

        /* When */
        if (this != who)
        {
            uint32 limit = (who->type == PLAYER) ?
                CONTR(who)->weight_limit :
                ((who->weight_limit > 0) ? who->weight_limit : 20000);
            sint32 extra_weight = WEIGHT_OVERALL(what);

            if (limit < who->carrying + extra_weight)
            {
                if (who->type == PLAYER)
                {
                    new_draw_info(NDI_UNIQUE, 0, who, "%s is too heavy to pick up!",
                        query_name(what));
                }

                return NULL;
            }
            else if (where &&
                     where->weight_limit < where->carrying + extra_weight)
            {
                if (who->type == PLAYER)
                {
                    new_draw_info(NDI_UNIQUE, 0, who, "%s is too heavy to fit in %s!",
                       query_name(what), query_name(where));
                }

                return NULL;
            }
        }
    }

    return what;
}

/* FixAsNeeded() carries out any nnecessary fixing post-pick up/drop. */
/* TODO: While I think this works well enough, ALL fixing needs
 * reworking/making more efficient. But that is a separate job. */
/* TODO: Currently monster encumbrance probably isn't handled. */
static void FixAsNeeded(object *who, object *to, object *from, object *what)
{
    object *needfixing[4];
    uint8   i;
    int     is_sys_object = QUERY_FLAG(what, FLAG_SYS_OBJECT);

    if (from == who ||
        from == to)
    {
        from = NULL;
    }

    if (to == who)
    {
        to = NULL;
    }

    needfixing[0] = who;
    needfixing[1] = to;
    needfixing[2] = from;
    needfixing[3] = NULL;

    for (i = 0; i <= 3; i++)
    {
        object *this = needfixing[i];

        if (!this)
        {
            return;
        }

        if (is_sys_object)
        {
            FIX_PLAYER(this, "pick up/drop"); // also fixes monsters
        }
        else if (this->type == PLAYER)
        {
            fix_player_weight(this);
        }
    }
}

/* drop_to_floor() is called when who attempts to drop nrof * what.
 *
 * The return is what (but possibly with a different ->nrof, certainly with a
 * different ->env and ->map, etc than before the function) if what was dropped
 * or NULL if it wasn't. */
object *drop_to_floor(object *who, object *what, uint32 nrof)
{
    player   *pl = NULL;
    MapSpace *msp;
    object   *shop,
             *from,
             *outof;
    int       reinsert = 1;

    /* STEP 1: Check that the parameters are sensible. */
    /* Sanity checks: must be a who and only players and monsters can drop
     * things; must be a what. */
    if (!who ||
        (who->type != MONSTER &&
         (who->type != PLAYER ||
          !(pl = CONTR(who)))) ||
        !what)
    {
        return NULL;
    }

    for (from = what; from->env; from = from->env)
    {
        ;
    }

    /* who can't drop what unless it is in (a container in) who's inventory. */
    if (from != who)
    {
        return NULL;
    }

    outof = what->env;
    msp = GET_MAP_SPACE_PTR(who->map, who->x, who->y);
    GET_MAP_SPACE_SYS_OBJ(msp, SHOP_FLOOR, shop);

    /* STEP 2: Check that who can actually drop what. */
    /* In a shop a container that is not empty cannot be dropped. */
    if (shop &&
        what->type == CONTAINER &&
        what->inv)
    {
        if (pl)
        {
            new_draw_info(NDI_UNIQUE, 0, who, "First take everything out of %s!",
                query_name(what));
        }

        return NULL;
    }

    /* Sometimes who cannot discard what -- see CanDiscard(). */
    if (!CanDiscard(who, what))
    {
        return NULL;
    }

    /* STEP 3: Now we've determined the drop is possible, so lets do it! */
    /* If what is a container be sure to unlink (close) it for all players
     * viewing it. */
    if (what->type == CONTAINER)
    {
        container_unlink(NULL, what);
    }

    /* what can have a DROP script on it which, if it returns true, aborts the
     * actual drop. */
    if (trigger_object_plugin_event(EVENT_DROP, what, who, NULL, NULL,
            (int *)&nrof, NULL, NULL, SCRIPT_FIX_ALL))
    {
        return NULL;
    }

    /* Dropping things interrupts resting. */
    if (pl)
    {
        pl->rest_mode = 0;
    }

    /* Below here is the actual code to move what to the map (specifically the
     * sqaure beneath who's feet). Also we send some appropriate messages to
     * the client. */
    if (nrof &&
        what->nrof != nrof)
    {
        what = get_split_ob(what, nrof);
    }
    else if (!QUERY_FLAG(what, FLAG_REMOVED))
    {
        remove_ob(what);
    }

    /* Give players an appropriate message to tell them what they've done. */
    /* TODO: Currently the client plays a drop sound whenever a drop is
     * attempted. This means when the server decides the attempt has failed, a
     * sound is still played. This should be moved server-side. Then also the
     * client can handle the 'you drop...' message on receipt of the sound
     * cmd. */
    if (pl)
    {
        char buf[MEDIUM_BUF];

        if (outof &&
            outof != who)
        {
            sprintf(buf, "Taking %s out of %s, you",
                (nrof > 1) ? "them" : "it", query_name(outof));
        }
        else
        {
            sprintf(buf, "You ");
        }

        new_draw_info(NDI_UNIQUE, 0, who, "%s drop %s.",
            buf, query_name(what));
    }

    /* No drops vanish for non-gmaster wiz players. */
    if (pl &&
        !pl->gmaster_wiz &&
        QUERY_FLAG(what, FLAG_NO_DROP))
    {
        new_draw_info(NDI_UNIQUE, 0, who, "~NO-DROP~: %s vanishes to nowhere!",
            query_name(what));
        reinsert = 0;
    }
    /* In a shop there are special rules. */
    else if (shop &&
             !QUERY_FLAG(what, FLAG_SYS_OBJECT) &&
             what->layer)
    {
        sint64 price;

        /* An unpaid item is just put back in the shop. */
        if (QUERY_FLAG(what, FLAG_UNPAID))
        {
            if (pl)
            {
                new_draw_info(NDI_UNIQUE, 0, who, "The shop magic puts %s back in storage.",
                    query_name(what));
            }

            reinsert = 0;
        }
        /* Coins and worthless items just drop to the floor like normal but
         * everything else is sold to the shop. */
        else if (what->type != MONEY &&
                 (price = query_cost(what, who, F_SELL)) > 0)
        {
            uint8 i;

            /* TODO: This results in each coin type being picked up separately (so 4g,
             * 8s, and 6c means 3 pick ups). We need a 'cash' object into which the
             * coins can be inserted here. Then after this loop that object is inserted
             * to the map and picked up, with pick_up() knowing to extract its contents
             * and remove the empty husk within (a container in) who's inv. */
            for (i = 0; coins_arch[i]; i++)
            {
                archetype *at = coins_arch[i];
                uint32     nr;

                if (price <= 0)
                {
                    break;
                }

                if ((nr = price / at->clone.value) > 0)
                {
                     object *new = clone_object(&at->clone, CLONE_WITHOUT_INVENTORY);

                     new->nrof = nr;
                     new->x = who->x;
                     new->y = who->y;
                     (void)insert_ob_in_map(new, who->map, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
                     (void)pick_up(who, new, NULL, nr);
                     price -= (nr * at->clone.value);
                }
            }

            if (pl)
            {
                new_draw_info(NDI_UNIQUE, 0, who, "You receive ~%s~ for %s.",
                    query_cost_string(what, who, F_SELL, COSTSTRING_FULL),
                    query_name(what));
            }

            reinsert = 0;
        }
    }

    /* Yay! we're finally done. So insert what into its new environment if
     * necessary. */
    if (reinsert)
    {
        what->x = who->x;
        what->y = who->y;
        what = insert_ob_in_map(what, who->map, who, 0);

        /* Reinsert who at the top of the object list. */
        SET_FLAG(who, FLAG_NO_APPLY);
        remove_ob(who);
        (void)insert_ob_in_map(who, who->map, who, INS_NO_MERGE | INS_NO_WALK_ON);
        CLEAR_FLAG(who, FLAG_NO_APPLY);
    }

    FixAsNeeded(who, NULL, NULL, what);

    return what;
}

/* CanDiscard() determines if who can discard what.
 *
 * The return is what if who can discard it, or NULL if who can't.
 *
 * If who is a player an ndi is sent to the player when what can't be
 * discarded. */
static object *CanDiscard(object *who, object *what)
{
    /* Players have a few specific rules. */
    if (who->type == PLAYER)
    {
        /* Locked items can't be dropped. */
        if (QUERY_FLAG(what, FLAG_INV_LOCKED))
        {
            new_draw_info(NDI_UNIQUE, 0, who, "%s %s locked!",
                query_name(what), (what->nrof > 1) ? "are" : "is");
            return NULL;
        }

        /* Under some circumstances a player cannot discard a container with
         * something in it. */
        if (what->type == CONTAINER)
        {
            /* Always, certain contents prevent dropping. */
            if (NoDiscardContainer(who, what))
            {
                return NULL;
            }
        }
    }
    /* Monsters can't drop no drops. */
    else if (QUERY_FLAG(what, FLAG_NO_DROP))
    {
        return NULL;
    }

    /* If what is applied and can't be unapplied (eg, it's cursed) then it
     * can't be discarded. */
    if (QUERY_FLAG(what, FLAG_APPLIED))
    {
        int cantunapply = apply_special(who, what, AP_UNAPPLY | AP_NO_MERGE);

        if (cantunapply)
        {
            return NULL;
        }

        if (!QUERY_FLAG(what, FLAG_SYS_OBJECT)) // else called later
        {
            FIX_PLAYER(who, "CanDiscard");
        }
    }

    return what;
}

/* NoDiscardContainer() recursively checks containers for no drops and locked
 * items inside, returning a pointer to the first such object found (container
 * cannot be discarded) or NULL if none are found (container can be
 * discarded).
 *
 * An ndi is sent to who (this function is only called for players) when the
 * container cannot be discarded. */
static object *NoDiscardContainer(object *who, object *what)
{
    object *this;

    for (this = what->inv; this; this = this->below)
    {
        if (this->type == CONTAINER &&
            this->inv)
        {
            object *that = NoDiscardContainer(who, this);

            if (that)
            {
                return that;
            }
        }

        if (QUERY_FLAG(this, FLAG_NO_DROP))
        {
            new_draw_info(NDI_UNIQUE, 0, who, "First remove all ~NO-DROP~ items from %s!",
               query_name(what));
            return this;
        }
        else if (QUERY_FLAG(this, FLAG_INV_LOCKED))
        {
            new_draw_info(NDI_UNIQUE, 0, who, "First remove all locked items from %s!",
               query_name(what));
            return this;
        }
    }

    return NULL;
}
