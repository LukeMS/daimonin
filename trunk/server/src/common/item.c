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
#include <funcpoint.h>
#include <living.h>
#include <spells.h>

static float weapon_speed_table[19]={	20.0f, 18.0f, 10.0f, 8.0f, 5.5f, 4.25f, 3.50f, 3.05f, 2.70f, 2.35f, 2.15f, 
										1.95f ,1.80f, 1.60f, 1.52f, 1.44f, 1.32f, 1.25f, 1.20f};

static char numbers[21][20] = {
  "no","","two","three","four","five","six","seven","eight","nine","ten",
  "eleven","twelve","thirteen","fourteen","fifteen","sixteen","seventeen",
  "eighteen","nineteen","twenty"
};

static char numbers_10[10][20] = {
  "zero","ten","twenty","thirty","fourty","fifty","sixty","seventy",
  "eighty","ninety"
};

static char levelnumbers[21][20] = {
  "zeroth","first", "second", "third", "fourth", "fifth", "sixth", "seventh",
  "eighth", "ninth", "tenth", "eleventh", "twelfth", "thirteenth",
  "fourteenth", "fifteenth", "sixteenth", "seventeenth", "eighteen",
  "nineteen", "twentieth"
};

static char levelnumbers_10[11][20] = {
  "zeroth","tenth","twentieth","thirtieth","fortieth","fiftieth","sixtieth",
  "seventieth","eightieth","ninetieth"
};

/* describe_resistance generates the visible naming for resistances.
 * returns a static array of the description.  This can return
 * a big buffer.
 * if newline is true, we don't put parens around the description
 * but do put a newline at the end.  Useful when dumping to files
 */
char *describe_resistance(object *op, int newline)
{
    static char buf[VERY_BIG_BUF];
    char    buf1[VERY_BIG_BUF];
    int tmpvar,flag=1;

    buf[0]=0;
                
    for (tmpvar=0; tmpvar<NROFATTACKS; tmpvar++) {
    if (op->resist[tmpvar] && (op->type != FLESH || atnr_is_dragon_enabled(tmpvar)==1)) {
        if(flag)
        {
            if (!newline)
                strcat(buf,"(Resists: ");
        }
	    if (!newline)
        {
            if(!flag)
                strcat(buf,", ");
    		sprintf(buf1,"%s %+d", resist_plus[tmpvar], op->resist[tmpvar]);
        }
	    else
		sprintf(buf1,"%s %d\n", resist_plus[tmpvar], op->resist[tmpvar]);
        flag=0;        
	    strcat(buf, buf1);
       }
    }
    if (!newline && !flag)
        strcat(buf,") ");
    return buf;
}

/* describe_attacks generates the visible naming for attack forms.
 * returns a static array of the description.  This can return
 * a big buffer.
 * if newline is true, we don't put parens around the description
 * but do put a newline at the end.  Useful when dumping to files
 */
char *describe_attack(object *op, int newline)
{
    static char buf[VERY_BIG_BUF];
    char    buf1[VERY_BIG_BUF];
    int tmpvar,flag=1;

    buf[0]=0;

    for (tmpvar=0; tmpvar<NROFATTACKS; tmpvar++) {
    if (op->attack[tmpvar]){
        if(flag)
        {
            if (!newline)
                strcat(buf,"(Attacks: ");
        }
	    if (!newline)
        {
            if(!flag)
                strcat(buf,", ");
            sprintf(buf1,"%s %+d", attacktype_desc[tmpvar],op->attack[tmpvar]);
        }
	    else
            sprintf(buf1,"%s %+d\n", attacktype_desc[tmpvar],op->attack[tmpvar]);
        flag=0;        
	    strcat(buf, buf1);
       }
    }
    if (!newline && !flag)
        strcat(buf,") ");
    return buf;
}

/* As above, but it list the protections */
char *describe_protections(object *op, int newline)
{
    static char buf[VERY_BIG_BUF];
    char    buf1[VERY_BIG_BUF];
    int tmpvar,flag=1;

    buf[0]=0;
                
    for (tmpvar=0; tmpvar<NROFPROTECTIONS; tmpvar++) {
        if (op->protection[tmpvar]) {
            if(flag)
            {
                if (!newline)
                    strcat(buf,"(Protections: ");
            }
            if (!newline)
        {
            if(!flag)
                strcat(buf,", ");
    		sprintf(buf1,"%s %+d", protection_name[tmpvar], op->protection[tmpvar]);
        }
	    else
		sprintf(buf1,"%s %d\n", protection_name[tmpvar], op->protection[tmpvar]);
        flag=0;
        
	    strcat(buf, buf1);
       }
    }
    if (!newline && !flag)
        strcat(buf,") ");
    return buf;
}

/*
 * query_weight(object) returns a character pointer to a static buffer
 * containing the text-representation of the weight of the given object.
 * The buffer will be overwritten by the next call to query_weight().
 */

char *query_weight(object *op) {
  static char buf[10];
  int i=op->nrof?op->nrof*op->weight:op->weight+op->carrying;

  if(op->weight<0)
    return "      ";
  if(i%1000)
    sprintf(buf,"%6.1f",(float)i/1000.0f);
  else
    sprintf(buf,"%4d  ",i/1000);
  return buf;
}

/*
 * Returns the pointer to a static buffer containing
 * the number requested (of the form first, second, third...)
 */

char *get_levelnumber(int i) {
  static char buf[MAX_BUF];
  if (i > 99) {
    sprintf(buf, "%d.", i);
    return buf;
  }
  if(i < 21)
    return levelnumbers[i];
  if(!(i%10))
    return levelnumbers_10[i/10];
  strcpy(buf, numbers_10[i/10]);
  strcat(buf, levelnumbers[i%10]);
  return buf;
}


/*
 * get_number(integer) returns the text-representation of the given number
 * in a static buffer.  The buffer might be overwritten at the next
 * call to get_number().
 * It is currently only used by the query_name() function.
 */

char *get_number(int i) {
  if(i<=20)
    return numbers[i];
  else {
    static char buf[MAX_BUF];
    sprintf(buf,"%d",i);
    return buf;
  }
}

/*
 * query_short_name(object) is similar to query_name, but doesn't 
 * contain any information about object status (worn/cursed/etc.)
 */
char *query_short_name(object *op) 
{
    static char buf[HUGE_BUF];
    char buf2[HUGE_BUF];
    int len=0;

    if(op->name == NULL)
	return "(null)";
	
	/* To speed things up (or make things slower?) 
    if(!op->nrof && !op->weight && !op->title && !is_magical(op)) 
	return op->name; 
	*/
    if(op->nrof) {
		safe_strcat(buf, get_number(op->nrof), &len, HUGE_BUF);

		if (op->nrof!=1) 
			safe_strcat(buf, " ", &len, HUGE_BUF);
		/* add the item race name */
		safe_strcat(buf, item_race_table[op->item_race].name, &len, HUGE_BUF);

		if(op->material_real && QUERY_FLAG(op,FLAG_IDENTIFIED))
			safe_strcat(buf,material_real[op->material_real].name, &len, HUGE_BUF);

		safe_strcat(buf,op->name, &len, HUGE_BUF);
		if (op->nrof != 1)
		{
			char *buf3 = strstr(buf, " of ");
			if (buf3!=NULL)
			{
				strcpy(buf2, buf3);
				*buf3 = '\0';	/* also changes value in buf */
			}
			len=strlen(buf);			
			
			/* If buf3 is set, then this was a string that contained
			* something of something (potion of dexterity.)  The part before
			* the of gets made plural, so now we need to copy the rest
			* (after and including the " of "), to the buffer string.
			*/
			if (buf3)
				safe_strcat(buf, buf2, &len, HUGE_BUF);
		}
	} 
	else 
	{
		/* if nrof is 0, the object is not mergable, and thus, op->name
		should contain the name to be used. */

		safe_strcat(buf, item_race_table[op->item_race].name, &len, HUGE_BUF);

		if(op->material_real && QUERY_FLAG(op,FLAG_IDENTIFIED))
			safe_strcat(buf,material_real[op->material_real].name, &len, HUGE_BUF);

		safe_strcat(buf,op->name, &len, HUGE_BUF);
    }

	switch(op->type) 
	{
		case CONTAINER:
			if (QUERY_FLAG(op,FLAG_IDENTIFIED))
			{
				if(op->title) 
				{
					safe_strcat(buf, " ", &len, MAX_BUF);
					safe_strcat(buf, op->title, &len, MAX_BUF);
				}
			}
			if(op->sub_type1>=ST1_CONTAINER_NORMAL_player)
			{
				if(op->sub_type1 == ST1_CONTAINER_CORPSE_player && op->slaying)
				{
					safe_strcat(buf," (bounty of ", &len, HUGE_BUF);	
					safe_strcat(buf,op->slaying, &len, HUGE_BUF);	
					safe_strcat(buf,")", &len, HUGE_BUF);	
				}
			}
		break;

		case SPELLBOOK:
			if (QUERY_FLAG(op,FLAG_IDENTIFIED)||QUERY_FLAG(op,FLAG_BEEN_APPLIED))
			{
				if(!op->title) 
				{
					safe_strcat(buf," of ", &len, HUGE_BUF);
					if(op->slaying) 
						safe_strcat(buf,op->slaying, &len, HUGE_BUF);
					else
					{
						if(op->stats.sp == SP_NO_SPELL)
							safe_strcat(buf,"nothing", &len, HUGE_BUF);
						else
							safe_strcat(buf,spells[op->stats.sp].name, &len, MAX_BUF);
					}
				}
				else
				{
					safe_strcat(buf, " ", &len, HUGE_BUF);
					safe_strcat(buf, op->title, &len, HUGE_BUF);
				}
			}
		break;

		case SCROLL:
		case WAND:
		case ROD:
		case HORN:
		case POTION:
			if (QUERY_FLAG(op,FLAG_IDENTIFIED)||QUERY_FLAG(op,FLAG_BEEN_APPLIED)) 
			{
				if(!op->title)
				{
					if(op->stats.sp != SP_NO_SPELL)
					{
						safe_strcat(buf," of ", &len, HUGE_BUF);
						safe_strcat(buf,spells[op->stats.sp].name, &len, HUGE_BUF);
					}
					else
						safe_strcat(buf," of nothing", &len, HUGE_BUF);
			    }
				else
				{
					safe_strcat(buf, " ", &len, HUGE_BUF);
					safe_strcat(buf, op->title, &len, HUGE_BUF);
				}
				sprintf(buf2, " (lvl %d)", op->level);
				safe_strcat(buf, buf2, &len, HUGE_BUF);
			}
		break;

		case SKILL:
		case AMULET:
		case RING:
			if (!op->title) 
			{
				/* If ring has a title, full description isn't so useful */ 
				char *s = describe_item(op);
				if (s[0])
				{
					safe_strcat (buf, " ", &len, HUGE_BUF);
					safe_strcat(buf, s, &len, HUGE_BUF);
				}
			}
			else
			{
				safe_strcat(buf, " ", &len, HUGE_BUF);
				safe_strcat(buf, op->title, &len, HUGE_BUF);
			}
		break;

		default:
		if(op->magic && ((QUERY_FLAG(op,FLAG_BEEN_APPLIED) && 
					need_identify(op)) || QUERY_FLAG(op,FLAG_IDENTIFIED)))
		{
			sprintf(buf2, " %+d", op->magic);
			safe_strcat(buf, buf2, &len, HUGE_BUF);
		}
	    if (op->title && QUERY_FLAG(op,FLAG_IDENTIFIED))
		{
			safe_strcat(buf, " ", &len, HUGE_BUF);
			safe_strcat(buf, op->title, &len, HUGE_BUF);
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
char *query_name(object *op) {
    static char buf[5][HUGE_BUF];
    static int use_buf=0;
    int len=0;

    use_buf++;
    use_buf %=5;

	if(!op || !op->name)
	{
		buf[use_buf][0]=0;
		return buf[use_buf];
	}

    safe_strcat(buf[use_buf], query_short_name(op), &len, HUGE_BUF);

    if (QUERY_FLAG(op,FLAG_INV_LOCKED))
	safe_strcat(buf[use_buf], " *", &len, HUGE_BUF);
    if (op->type == CONTAINER && ((op->env && op->env->container == op) || 
	(!op->env && QUERY_FLAG(op,FLAG_APPLIED))))
	safe_strcat(buf[use_buf]," (open)", &len, HUGE_BUF);

    if (QUERY_FLAG(op,FLAG_KNOWN_CURSED)||QUERY_FLAG(op,FLAG_IDENTIFIED)) {
	if(QUERY_FLAG(op,FLAG_DAMNED))
	    safe_strcat(buf[use_buf], " (damned)", &len, HUGE_BUF);
	else if(QUERY_FLAG(op,FLAG_CURSED))
	    safe_strcat(buf[use_buf], " (cursed)", &len, HUGE_BUF);
    }

    if ((QUERY_FLAG(op,FLAG_KNOWN_MAGICAL&&QUERY_FLAG(op,FLAG_IS_MAGICAL))) ||  (QUERY_FLAG(op,FLAG_IS_MAGICAL) && QUERY_FLAG(op,FLAG_IDENTIFIED)))
	safe_strcat(buf[use_buf], " (magical)", &len, HUGE_BUF);
    if(QUERY_FLAG(op,FLAG_APPLIED)) {
	switch(op->type) {
	  case BOW:
	  case WAND:
	  case ROD:
	  case HORN:
	    safe_strcat(buf[use_buf]," (readied)", &len, HUGE_BUF);
	    break;
	  case WEAPON:
	    safe_strcat(buf[use_buf]," (wielded)", &len, HUGE_BUF);
	    break;
	  case ARMOUR:
	  case HELMET:
	  case SHIELD:
	  case RING:
	  case BOOTS:
	  case GLOVES:
	  case AMULET:
	  case GIRDLE:
	  case BRACERS:
	  case CLOAK:
	    safe_strcat(buf[use_buf]," (worn)", &len, HUGE_BUF);
	    break;
	  case CONTAINER:
	    safe_strcat(buf[use_buf]," (active)", &len, HUGE_BUF);
	    break;
	  case SKILL:
	  default:
	    safe_strcat(buf[use_buf]," (applied)", &len, HUGE_BUF);
	}
    }
    if(QUERY_FLAG(op, FLAG_UNPAID))
	safe_strcat(buf[use_buf]," (unpaid)", &len, HUGE_BUF);

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
char *query_base_name(object *op) {
    static char buf[MAX_BUF];
	char buf2[32];
    int len;

    if(op->name == NULL)
	return "(null)";

    /* add the item race name */
    strcpy(buf, item_race_table[op->item_race].name);

	/* we add the real material name as prefix. Because real_material == 0 is
	 * "" (clear string) we don't must check item types for adding something here
	 * or not (artifacts for example has normally no material prefix)
	 */
    if(op->material_real && QUERY_FLAG(op,FLAG_IDENTIFIED))
        strcat(buf,material_real[op->material_real].name);

    strcat(buf,op->name);

	if(!op->weight && !op->title && !is_magical(op)) 
		return buf; /* To speed things up (or make things slower?) */

    len=strlen(buf);

	switch(op->type) 
	{

		case CONTAINER:
			if (QUERY_FLAG(op,FLAG_IDENTIFIED))
			{
				if(op->title) 
				{
					safe_strcat(buf, " ", &len, MAX_BUF);
					safe_strcat(buf, op->title, &len, MAX_BUF);
				}
			}

			if(op->sub_type1>=ST1_CONTAINER_NORMAL_player)
			{
				if(op->sub_type1 == ST1_CONTAINER_CORPSE_player && op->slaying)
				{
					safe_strcat(buf," (bounty of ", &len, HUGE_BUF);	
					safe_strcat(buf,op->slaying, &len, HUGE_BUF);	
					safe_strcat(buf,")", &len, HUGE_BUF);	
				}
			}
		break;

		case SPELLBOOK:
			if (QUERY_FLAG(op,FLAG_IDENTIFIED)||QUERY_FLAG(op,FLAG_BEEN_APPLIED))
			{
				if(!op->title) 
				{
					safe_strcat(buf," of ", &len, MAX_BUF);
					if(op->slaying) 
						safe_strcat(buf,op->slaying, &len, MAX_BUF);
					else
					{
						if(op->stats.sp == SP_NO_SPELL)
							safe_strcat(buf,"nothing", &len, HUGE_BUF);
						else
							safe_strcat(buf,spells[op->stats.sp].name, &len, MAX_BUF);
					}
				}
				else
				{
					safe_strcat(buf, " ", &len, MAX_BUF);
					safe_strcat(buf, op->title, &len, MAX_BUF);
				}
			}
		break;

		case SCROLL:
		case WAND:
		case ROD:
		case HORN:
		case POTION:
			if (QUERY_FLAG(op,FLAG_IDENTIFIED)||QUERY_FLAG(op,FLAG_BEEN_APPLIED))
			{
			    if(!op->title) 
				{	
					if(op->stats.sp != SP_NO_SPELL)
					{
						safe_strcat(buf," of ", &len, HUGE_BUF);
						safe_strcat(buf,spells[op->stats.sp].name, &len, HUGE_BUF);
					}
					else
						safe_strcat(buf," of nothing", &len, HUGE_BUF);
			    }
				else
				{
					safe_strcat(buf, " ", &len, MAX_BUF);
					safe_strcat(buf, op->title, &len, MAX_BUF);
				}
				sprintf(buf2, " (lvl %d)", op->level);
				safe_strcat(buf, buf2, &len, MAX_BUF);
			}
		break;

      case SKILL:
      case AMULET:
      case RING:
	if (!op->title) {
	    /* If ring has a title, full description isn't so useful */ 
	    char *s = describe_item(op);
	    if (s[0]) {
		safe_strcat (buf, " ", &len, MAX_BUF);
		safe_strcat (buf, s, &len, MAX_BUF);
	    }
	}
				else
				{
					safe_strcat(buf, " ", &len, MAX_BUF);
					safe_strcat(buf, op->title, &len, MAX_BUF);
				}
	break;
      default:
		if(op->magic && ((QUERY_FLAG(op,FLAG_BEEN_APPLIED) && 
							need_identify(op)) || QUERY_FLAG(op,FLAG_IDENTIFIED)))
		{
			sprintf(buf + strlen(buf), " %+d", op->magic);
		}
	    if (op->title && QUERY_FLAG(op,FLAG_IDENTIFIED)) 
		{
			safe_strcat(buf, " ", &len, MAX_BUF);
			safe_strcat(buf, op->title, &len, MAX_BUF);
		}
    } /* switch */

    return buf;
}


/* describe terrain flags 
 * we use strcat only - prepare the retbuf before call.
 */
static describe_terrain(object *op, char *retbuf)
{
	if(op->terrain_flag & TERRAIN_AIRBREATH)
		strcat(retbuf,"(air breathing)");
	if(op->terrain_flag & TERRAIN_WATERWALK)
		strcat(retbuf,"(water walking)");
	if(op->terrain_flag & TERRAIN_FIREWALK)
		strcat(retbuf,"(fire walking)");
	if(op->terrain_flag & TERRAIN_CLOUDWALK)
		strcat(retbuf,"(cloud walking)");
	if(op->terrain_flag & TERRAIN_WATERBREATH)
		strcat(retbuf,"(water breathing)");
	if(op->terrain_flag & TERRAIN_FIREBREATH)
		strcat(retbuf,"(fire breathing)");
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
char *describe_item(object *op) 
{
	int attr,val, more_info = 0, id_true=FALSE;
	char buf[MAX_BUF];
	static char retbuf[VERY_BIG_BUF*3];

	retbuf[0]='\0';        

	/* we start with living objects like mobs */
  if(op->type == PLAYER) {

	describe_terrain(op, retbuf);

    if(op->contr->digestion) {
      if(op->contr->digestion>0)
        sprintf(buf,"(sustenance%+d)",op->contr->digestion);
	else if(op->contr->digestion<0)
		sprintf(buf,"(hunger%+d)",-op->contr->digestion);
      strcat(retbuf,buf);
    }
    if(op->contr->gen_grace) {
      sprintf(buf,"(grace%+d)",op->contr->gen_grace);
      strcat(retbuf,buf);
    }
    if(op->contr->gen_sp) {
      sprintf(buf,"(magic%+d)",op->contr->gen_sp);
      strcat(retbuf,buf);
    }
    if(op->contr->gen_hp) {
      sprintf(buf,"(regeneration%+d)",op->contr->gen_hp);
      strcat(retbuf,buf);
    }
  }
	else
	if(QUERY_FLAG(op,FLAG_MONSTER)) 
	{

		describe_terrain(op, retbuf);

	    if(QUERY_FLAG(op,FLAG_UNDEAD))
			strcat(retbuf,"(undead)");
		if(QUERY_FLAG(op,FLAG_CAN_PASS_THRU))
			strcat(retbuf,"(pass through doors)");
		if(QUERY_FLAG(op,FLAG_SEE_INVISIBLE))
			strcat(retbuf,"(see invisible)");

		if(QUERY_FLAG(op,FLAG_USE_WEAPON))
			strcat(retbuf,"(wield weapon)");
		if(QUERY_FLAG(op,FLAG_USE_BOW))
			strcat(retbuf,"(archer)");
		if(QUERY_FLAG(op,FLAG_USE_ARMOUR))
			strcat(retbuf,"(wear armour)");
		if(QUERY_FLAG(op,FLAG_USE_RING))
			strcat(retbuf,"(wear ring)");
		if(QUERY_FLAG(op,FLAG_USE_SCROLL))
			strcat(retbuf,"(literated)");
		if(QUERY_FLAG(op,FLAG_USE_RANGE))
			strcat(retbuf,"(use range device)");
		if(QUERY_FLAG(op,FLAG_CAN_USE_SKILL))
			strcat(retbuf,"(skill user)");
		if(QUERY_FLAG(op,FLAG_CAST_SPELL))
			strcat(retbuf,"(spellcaster)");
		if(QUERY_FLAG(op,FLAG_FRIENDLY))
			strcat(retbuf,"(friendly)");
		if(QUERY_FLAG(op,FLAG_UNAGGRESSIVE))
			strcat(retbuf,"(unaggressive)");
		if(QUERY_FLAG(op,FLAG_HITBACK))
			strcat(retbuf,"(hitback)");

		if(op->randomitems != NULL)
		{
			treasure *t;
			int first = 1;
			for(t=op->randomitems->items; t != NULL; t=t->next)
			{
				if(t->item && (t->item->clone.type == ABILITY))
				{
					if(first)
					{
						first = 0;
						strcat(retbuf,"(Spell abilities:)");
					}
					strcat(retbuf,"(");
					strcat(retbuf,t->item->clone.name);
					strcat(retbuf,")");
				}
			}
		}

		if(FABS(op->speed)>MIN_ACTIVE_SPEED)
		{
			switch((int)((FABS(op->speed))*15))
			{
				case 0:
					strcat(retbuf,"(very slow movement)");
				break;
				case 1:
					strcat(retbuf,"(slow movement)");
				break;
				case 2:
					strcat(retbuf,"(normal movement)");
				break;
				case 3:
				case 4:
					strcat(retbuf,"(fast movement)");
				break;
				case 5:
				case 6:
					strcat(retbuf,"(very fast movement)");
				break;
				case 7:
				case 8:
				case 9:
				case 10:
					strcat(retbuf,"(extremely fast movement)");
				break;
				default:
					strcat(retbuf,"(lightning fast movement)");
				break;
			}
		}
	} 
	else /* here we handle items... */ 
	{
		if( QUERY_FLAG(op,FLAG_IDENTIFIED) || QUERY_FLAG(op,FLAG_BEEN_APPLIED) || !need_identify(op) )
			id_true=TRUE; /* we only need calculate this one time */

		/* terrain flags have no double use... if valid, show them */
		if(id_true && op->terrain_type)
			describe_terrain(op, retbuf);

		/* now lets deal with special cases */
		switch(op->type)
		{

			case WAND:
			case ROD:
			case HORN:
			break;

			/* Armor type objects */
			case ARMOUR:
			case HELMET:
			case SHIELD:
			case BOOTS:
			case GLOVES:
			case GIRDLE:
			case BRACERS:
			case CLOAK:
				if(id_true)
				{
					if (ARMOUR_SPEED(op))
					{
						sprintf(buf,"(speed cap %1.2f)", ARMOUR_SPEED(op) / 10.0);
						strcat(retbuf, buf);
					}
					/* Do this in all cases - otherwise it gets confusing - does that
					 * item have no penality, or is it not fully identified for example.
			         */
					if(ARMOUR_SPELLS(op))
					{
						sprintf(buf,"(spell reg %d)", -1*ARMOUR_SPELLS(op));
						strcat(retbuf, buf);
					}
				}
			case WEAPON:
			case RING:
			case AMULET:
			case FORCE:
				more_info=1;
			
			case BOW:
			case ARROW:
				if(id_true)
				{
					if(op->stats.wc)
					{
						sprintf(buf,"(wc%+d)",op->stats.wc);
						strcat(retbuf,buf);
					}
					if(op->stats.dam)
					{
						sprintf(buf,"(dam%+d)",op->stats.dam);
						strcat(retbuf,buf);
					}
					if(op->stats.ac)
					{
						sprintf(buf,"(ac%+d)",op->stats.ac);
						strcat(retbuf,buf);
					}

					if (op->type==WEAPON)
					{
						/* this is ugly to calculate because its a curve which increase heavily 
						 * with lower weapon_speed... so, we use a table
						 */
						int ws_temp = (int)(op->weapon_speed/0.0025f);
						if(ws_temp <0)
							ws_temp=0;
						else if(ws_temp > 18)
							ws_temp=18;
						sprintf(buf,"(%3.2f sec)", weapon_speed_table[ws_temp]);
						strcat(retbuf, buf);

						if(op->level>0)
						{
							sprintf(buf,"(improved %d/%d)",op->last_eat,op->level);
							strcat(retbuf,buf);
						}
					}
				}
			break;

			case FOOD:
			case FLESH:
			case DRINK:
				if(id_true)
				{
					sprintf(buf,"(food+%d)", op->stats.food);
					strcat(retbuf, buf);
					if (op->type == FLESH && op->last_eat>0 && atnr_is_dragon_enabled(op->last_eat))
					{
						sprintf(buf, "(%s metabolism)", change_resist_msg[op->last_eat]);
						strcat(retbuf, buf);
					}
					if (!QUERY_FLAG(op,FLAG_CURSED))
					{
						if (op->stats.hp)
							strcat(retbuf,"(heals)");
						if (op->stats.sp)
							strcat(retbuf,"(spellpoint regen)");
					}
					else 
					{
						if (op->stats.hp)
							strcat(retbuf,"(damages)");
						if (op->stats.sp)
							strcat(retbuf,"(spellpoint depletion)");
					}
				}
			break;

			/* potions, scrolls, ...*/
		    default: /* no more infos for all items we don't have handled in the switch */
			 return retbuf;
		}

		/* these counts for every "normal" item a player deals with - most times equipement */
		if(id_true)
		{
			for (attr=0; attr<7; attr++) 
			{
				if ((val=get_attr_value(&(op->stats),attr))!=0) 
				{
					sprintf(buf, "(%s%+d)", short_stat_name[attr], val);
					strcat(retbuf,buf);
      			}
			}

			if(op->stats.exp)
			{
				sprintf(buf,"(speed %+d)",op->stats.exp);
				strcat(retbuf,buf);
			}
		}
	}
  
	/* some special info for some kind of identified items */
	if (id_true && more_info)
	{
	    if(op->stats.sp)
		{
			sprintf(buf,"(magic%+d)",op->stats.sp);
			strcat(retbuf,buf);
	    }
	    if(op->stats.grace)
		{
			sprintf(buf,"(grace%+d)",op->stats.grace);
			strcat(retbuf,buf);
	    }
	    if(op->stats.hp)
		{
			sprintf(buf,"(regeneration%+d)",op->stats.hp);
			strcat(retbuf,buf);
	    }
	    if(op->stats.food)
		{
			if(op->stats.food>0)
				sprintf(buf,"(sustenance%+d)",op->stats.food);
			else if(op->stats.food<0)
				sprintf(buf,"(hunger%+d)",-op->stats.food);
			strcat(retbuf,buf);
	    }
	}

	/* here we deal with all the special flags */
	if(id_true) 
	{
		if(op->stats.luck)
		{
			sprintf(buf,"(luck%+d)",op->stats.luck);
			strcat(retbuf,buf);
		}
	    if(QUERY_FLAG(op,FLAG_SEE_INVISIBLE))
			strcat(retbuf,"(see invisible)");
	    if(QUERY_FLAG(op,FLAG_MAKE_ETHEREAL))
			strcat(retbuf,"(makes ethereal)");
	    if(QUERY_FLAG(op,FLAG_IS_ETHEREAL))
			strcat(retbuf,"(ethereal)");
	    if(QUERY_FLAG(op,FLAG_MAKE_INVISIBLE))
			strcat(retbuf,"(makes invisible)");
	    if(QUERY_FLAG(op,FLAG_IS_INVISIBLE))
			strcat(retbuf,"(invisible)");
	    if(QUERY_FLAG(op,FLAG_XRAYS))
			strcat(retbuf,"(xray-vision)");
		if(QUERY_FLAG(op,FLAG_SEE_IN_DARK))
			strcat(retbuf,"(infravision)");
		if(QUERY_FLAG(op,FLAG_LIFESAVE))
			strcat(retbuf,"(lifesaving)");
		if(QUERY_FLAG(op,FLAG_REFL_SPELL))
			strcat(retbuf,"(reflect spells)");
		if(QUERY_FLAG(op,FLAG_REFL_MISSILE))
			strcat(retbuf,"(reflect missiles)");
		if(QUERY_FLAG(op,FLAG_STEALTH))
			strcat(retbuf,"(stealth)");
		if(QUERY_FLAG(op,FLAG_FLYING))
			strcat(retbuf,"(levitate)");

		if(op->slaying!=NULL)
		{
			strcat(retbuf,"(slay ");
			strcat(retbuf,op->slaying);
			strcat(retbuf,")");
		}
    
      /* for dragon players display the attacks from clawing skill */
      /*
      /* must convert this for AV dragons later... */
		/*
		if (is_dragon_pl(op)) {
		object *tmp;
			for (tmp=op->inv; tmp!=NULL && !(tmp->type == SKILL &&
			strcmp(tmp->name, "clawing")==0); tmp=tmp->below);	
		*/

		strcat(retbuf,describe_attack(op, 0));

		/* resistance on flesh is only visible for quetzals */
		if (op->type != FLESH || QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
          strcat(retbuf,describe_resistance(op, 0));
		strcat(retbuf,describe_protections(op, 0));
		DESCRIBE_PATH(retbuf, op->path_attuned, "Attuned");
		DESCRIBE_PATH(retbuf, op->path_repelled, "Repelled");
		DESCRIBE_PATH(retbuf, op->path_denied, "Denied");
	}
	return retbuf;
}

/* need_identify returns true if the item should be identified.  This 
 * function really should not exist - by default, any item not identified
 * should need it.
 */

int need_identify(object *op) {
  switch(op->type) {
  case RING:
  case WAND:
  case ROD:
  case HORN:
  case SCROLL:
  case SKILL:
  case SKILLSCROLL:
  case SPELLBOOK:
  case FOOD:
  case POTION:
  case BOW:
  case ARROW:
  case WEAPON:
  case ARMOUR:
  case SHIELD:
  case HELMET:
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

void identify(object *op) {
  object *pl;

  if(!op)
	  return;
  
  SET_FLAG(op,FLAG_IDENTIFIED);
  CLEAR_FLAG(op, FLAG_KNOWN_MAGICAL);
  CLEAR_FLAG(op, FLAG_NO_SKILL_IDENT);

/*
 * We want autojoining of equal objects:
 */
  if (QUERY_FLAG(op,FLAG_CURSED) || QUERY_FLAG(op,FLAG_DAMNED))
    SET_FLAG(op,FLAG_KNOWN_CURSED);

  if (op->type == POTION && op->arch != (archetype *) NULL) {
    /*op->face = op->arch->clone.face; */
      free_string(op->name);
      op->name = add_refcount(op->arch->clone.name);
  } else if( op->type == SPELLBOOK && op->slaying != NULL){
       if((op->stats.sp = look_up_spell_name( op->slaying )) <0 ){
	  char buf[256];
          op->stats.sp = -1;  
          sprintf(buf, "Spell formula for %s", op->slaying);
	  if(op->name != NULL) 
		free_string(op->name);
	  op->name = add_string(buf);
       } else {
         /* clear op->slaying since we no longer need it */
         free_string(op->slaying);
         op->slaying=NULL;
       }
  }

  if (op->map) /* The shop identifies items before they hit the ground */
    /* I don't think identification will change anything but face */
    update_object(op,UP_OBJ_FACE);
  else {
    pl = is_player_inv(op->env);
    if (pl)
	/* A lot of the values can change from an update - might as well send
	 * it all.
	 */
	esrv_send_item_func(pl, op);
  }
}
