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

#include <newclient.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

/* WARNING: the whole module must rewritten to fix it for the new money
 * system in daimonin. 32bit will be broken when a player has ~250 mithril coins.
 */

#define NUM_COINS 4	/* number of coin types */
static char *coins[] = {"mitcoin", "goldcoin", "silvercoin","coppercoin", NULL};

/* i reworked this function in most parts. I removed every part which try to recalculate
 * the item value. Now we always use the real value or the clone value and only adjust 
 * it by charisma or buy/sell base modifiers.
 */
double query_cost(object *tmp, object *who, int flag) 
{
	double val, diff;
	int number;	/* used to better calculate value */
	int charisma=11; /* thas a neutral base value */

	if ((number = tmp->nrof)==0) 
	  number=1;


	if (tmp->type==MONEY) /* money is always identified */
		return((double)number * (double)tmp->value);

	/* handle identified items */
	if (QUERY_FLAG(tmp, FLAG_IDENTIFIED) || !need_identify(tmp)) 
	{
		if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))
			return 0.0;
		else
		val=(double)tmp->value * (double)number;
	}
	else /* This area deals with objects that are not identified, but can be */
	{
		if (tmp->arch != NULL) 
		{
			if (flag == F_BUY) 
			{
				LOG(llevBug, "BUG: Asking for buy-value of unidentified object %s.\n", query_name(tmp));
				val = (double)tmp->arch->clone.value * (double)number;
			}
			else	/* Trying to sell something, or get true value */
			{
				if(tmp->type == GEM || tmp->type == TYPE_JEWEL|| tmp->type == TYPE_NUGGET) /* selling unidentified gems is *always* stupid */
					val = (double)number * 3.0;
				else if (tmp->type == POTION)
					val = (double)number * 50.0; /* Don't want to give anything away */
				else 
					val = (double)number * (double)tmp->arch->clone.value;
			}
		}
		else 
		{ /* No archetype with this object - we generate some dummy values to avoid server break */
			LOG(llevBug,"BUG: In sell item: Have object with no archetype: %s\n", query_name(tmp));
			if (flag == F_BUY)
			{
				LOG(llevBug, "BUG: Asking for buy-value of unidentified object without arch.\n");
				val = (double)number * 100.0;
			}
			else
				val = (double)number * 80.0;
		}
	}

	/* wands will count special. The base value is for a wand with one charge */
	if (tmp->type==WAND)
		val+=((val*(double)tmp->level)/1.0) *(double)tmp->stats.food;
	else if (tmp->type==ROD || tmp->type == HORN || tmp->type == POTION || tmp->type == SCROLL)
		val+=(val*(double)tmp->level)/1.0;

	/* we are done if we only want get the real value */
	if(flag == F_TRUE)
		return val;

	/* ok, we handle buy or sell values.
	 * If we buy, the price is nearly true value.
	 * If we sell, its about20% of the true value.
	 * This value can be altered from charisma and skills to.
	 */

	/* first, we adjust charisma for players and count skills in */
	if (who!=NULL && who->type==PLAYER) 
	{
		charisma = who->stats.Cha;  /* used for SK_BARGAINING modification */

		/* this skill give us a charisma boost */
		if (find_skill(who,SK_BARGAINING)) 
		{
			charisma += 4;
			if(charisma>30) 
				charisma = 30;
		}
	}

	/* now adjust for sell or buy the multiplier */
	if(flag == F_BUY)
			diff = 1.0-(double)cha_bonus[charisma];
	else
			diff = 0.20+(double)cha_bonus[charisma];  

	return val*diff;
}

/* Find the coin type that is worth more the 'c'.  Starts at the
 * cointype placement.
 */

static archetype *find_next_coin(double c, int *cointype) {
  archetype *coin;

  do {
    if (coins[*cointype]==NULL) return NULL;
    coin = find_archetype(coins[*cointype]);
    if (coin == NULL)
      return NULL;
    *cointype += 1;
  } while ((double)coin->clone.value > c);

  return coin;
}

/* This returns a string of how much somethign is worth based on
 * an integer being passed.
 */
char *cost_string_from_value(double cost)
{
  static char buf[MAX_BUF];
  archetype *coin, *next_coin;
  char *endbuf;
  int num, cointype = 0;

  coin = find_next_coin(cost, &cointype);
  if (coin == NULL)
    return "nothing";

  num = (int)(cost / (double)coin->clone.value);
  cost -= (double)num * (double)coin->clone.value;
  if (num == 1)
    sprintf(buf, "1 %s%s", material_real[coin->clone.material_real].name,coin->clone.name);
  else
    sprintf(buf, "%d %s%ss", num, material_real[coin->clone.material_real].name, coin->clone.name);

  next_coin = find_next_coin(cost, &cointype);
  if (next_coin == NULL)
    return buf;

  do {
    endbuf = buf + strlen(buf);

    coin = next_coin;
    num = (int)(cost / (double)coin->clone.value);
    cost -= (double)num * (double)coin->clone.value;

    if (cost == 0.0)
      next_coin = NULL;
    else
      next_coin = find_next_coin(cost, &cointype);

    if (next_coin) {
      /* There will be at least one more string to add to the list,
       * use a comma.
       */
      strcat(endbuf, ", "); endbuf += 2;
    } else {
      strcat(endbuf, " and "); endbuf += 5;
    }
    if (num == 1)
      sprintf(endbuf, "1 %s%s",material_real[coin->clone.material_real].name, coin->clone.name);
    else
      sprintf(endbuf, "%d %s%ss", num, material_real[coin->clone.material_real].name, coin->clone.name);
  } while (next_coin);

  return buf;
}

char *query_cost_string(object *tmp,object *who,int flag) {
  return cost_string_from_value(query_cost(tmp,who,flag));
}

/* This function finds out how much money the player is carrying,	*
 * and returns that value						*/
/* Now includes any coins in active containers -- DAMN			*/
int query_money(object *op) {
    object *tmp;
    int	total=0;

    if (op->type!=PLAYER && op->type!=CONTAINER) {
	LOG(llevBug, "BUG: Query money called with non player/container.\n");
	return 0;
    }
    for (tmp = op->inv; tmp; tmp= tmp->below) {
	if (tmp->type==MONEY) {
	    total += tmp->nrof * tmp->value;
	} else if (tmp->type==CONTAINER &&
		   QUERY_FLAG(tmp,FLAG_APPLIED) &&
		   (tmp->race==NULL || strstr(tmp->race,"gold"))) {
	    total += query_money(tmp);
	}
    }
    return total;
}
/* TCHIZE: This function takes the amount of money from the             *
 * the player inventory and from it's various pouches using the         *
 * pay_from_container function.                                         *
 * returns 0 if not possible. 1 if success                              */
int pay_for_amount(int to_pay,object *pl) {
    object *pouch;

    if (to_pay==0) return 1;
    if(to_pay>query_money(pl)) return 0;

    to_pay = pay_from_container(NULL, pl, to_pay);

    for (pouch=pl->inv; (pouch!=NULL) && (to_pay>0); pouch=pouch->below) {
	if (pouch->type == CONTAINER
	    && QUERY_FLAG(pouch, FLAG_APPLIED)
	    && (pouch->race == NULL || strstr(pouch->race, "gold"))) {
	    to_pay = pay_from_container(NULL, pouch, to_pay);
	}
    }

#ifndef REAL_WIZ
    if(QUERY_FLAG(pl,FLAG_WAS_WIZ))
      SET_FLAG(op, FLAG_WAS_WIZ);
#endif
    fix_player(pl);
    return 1;
}

/* DAMN: This is now a wrapper for pay_from_container, which is 	*
 * called for the player, then for each active container that can hold	*
 * money until op is paid for.  Change will be left wherever the last	*
 * of the price was paid from.						*/
int pay_for_item(object *op,object *pl) {
    int to_pay = (int)query_cost(op,pl,F_BUY);
    object *pouch;

    if (to_pay==0.0) return 1;
    if(to_pay>query_money(pl)) return 0;

    to_pay = pay_from_container(op, pl, to_pay);

    for (pouch=pl->inv; (pouch!=NULL) && (to_pay>0); pouch=pouch->below) {
	if (pouch->type == CONTAINER
	    && QUERY_FLAG(pouch, FLAG_APPLIED)
	    && (pouch->race == NULL || strstr(pouch->race, "gold"))) {
	    to_pay = pay_from_container(op, pouch, to_pay);
	}
    }

#ifndef REAL_WIZ
    if(QUERY_FLAG(pl,FLAG_WAS_WIZ))
      SET_FLAG(op, FLAG_WAS_WIZ);
#endif
    fix_player(pl);
    return 1;
}

/* This pays for the item, and takes the proper amount of money off
 * the player.
 * CF 0.91.4 - this function is mostly redone in order to fix a bug
 * with weight not be subtracted properly.  We now remove and
 * insert the coin objects -  this should update the weight
 * appropriately
 */
/* DAMN: This function is used for the player, then for any active	*
 * containers that can hold money, until the op is paid for.		*/
int pay_from_container(object *op, object *pouch, int to_pay) {
    int count, i, remain;
    object *tmp, *coin_objs[NUM_COINS], *next;
    archetype *at;
    object *who;

    if (pouch->type != PLAYER && pouch->type != CONTAINER) return to_pay;

    remain = to_pay;
    for (i=0; i<NUM_COINS; i++) coin_objs[i] = NULL;

    /* This hunk should remove all the money objects from the player/container */
    for (tmp=pouch->inv; tmp; tmp=next) {
	next = tmp->below;

	if (tmp->type == MONEY) {
	    for (i=0; i<NUM_COINS; i++) {
		if (!strcmp(coins[NUM_COINS-1-i], tmp->arch->name) &&
		    (tmp->value == tmp->arch->clone.value) ) {

		    /* This should not happen, but if it does, just		*
		     * merge the two.						*/
		    if (coin_objs[i]!=NULL) {
			LOG(llevBug,"BUG: %s has two money entries of (%s)\n", query_name(pouch), coins[NUM_COINS-1-i]);
			remove_ob(tmp);
			coin_objs[i]->nrof += tmp->nrof;
			esrv_del_item(pouch->contr, tmp->count, tmp->env);
			free_object(tmp);
		    }
		    else {
			remove_ob(tmp);
			if(pouch->type==PLAYER) esrv_del_item(pouch->contr, tmp->count,tmp->env);
			coin_objs[i] = tmp;
		    }
		    break;
		}
	    }
	    if (i==NUM_COINS)
		LOG(llevBug,"BUG: in pay_for_item: Did not find string match for %s\n", tmp->arch->name);
	}
    }

    /* Fill in any gaps in the coin_objs array - needed to make change. */
    /* Note that the coin_objs array goes from least value to greatest value */
    for (i=0; i<NUM_COINS; i++)
	if (coin_objs[i]==NULL) {
	    at = find_archetype(coins[NUM_COINS-1-i]);
	    if (at==NULL) LOG(llevBug, "BUG: Could not find %s archetype", coins[NUM_COINS-1-i]);
	    coin_objs[i] = get_object();
	    copy_object(&at->clone, coin_objs[i]);
	    coin_objs[i]->nrof = 0;
	}

    for (i=0; i<NUM_COINS; i++) {
	int num_coins;

	if (coin_objs[i]->nrof*coin_objs[i]->value> (uint32) remain) {
	    num_coins = remain / coin_objs[i]->value;
	    if (num_coins*coin_objs[i]->value< remain) num_coins++;
	} else {
	    num_coins = coin_objs[i]->nrof;
	}

	remain -= num_coins * coin_objs[i]->value;
	coin_objs[i]->nrof -= num_coins;
	/* Now start making change.  Start at the coin value
	 * below the one we just did, and work down to
	 * the lowest value.
	 */
	count=i-1;
	while (remain<0 && count>=0) {
		num_coins = -remain/ coin_objs[count]->value;
		coin_objs[count]->nrof += num_coins;
		remain += num_coins * coin_objs[count]->value;
		count--;
	}
    }
    for (i=0; i<NUM_COINS; i++) {
	if (coin_objs[i]->nrof) {
	    object *tmp = insert_ob_in_ob(coin_objs[i], pouch);
	    for (who = pouch; who && who->type!=PLAYER && who->env!=NULL; who=who->env) {}
	    esrv_send_item(who, tmp);
	    esrv_send_item (who, pouch);
	    esrv_update_item (UPD_WEIGHT, who, pouch);
	    if (pouch->type != PLAYER) {
		esrv_send_item (who, who);
		esrv_update_item (UPD_WEIGHT, who, who);
	    }
	} else {
	    free_object(coin_objs[i]);
	}
    }
    return(remain);
}

/* Eneq(@csd.uu.se): Better get_payment, descends containers looking for
   unpaid items. get_payment is now used as a link. To make it simple
   we need the player-object here. */

int get_payment2 (object *pl, object *op) {
    char buf[MAX_BUF];
    int ret=1;

    if (op!=NULL&&op->inv)
        ret = get_payment2(pl, op->inv);

    if (!ret)
        return 0;

    if (op!=NULL&&op->below)
        ret = get_payment2 (pl, op->below);

    if (!ret) 
        return 0;
   
    if(op!=NULL&&QUERY_FLAG(op,FLAG_UNPAID)) {
        strncpy(buf,query_cost_string(op,pl,F_BUY),MAX_BUF);
        if(!pay_for_item(op,pl)) {
            int i=(int)query_cost(op,pl,F_BUY) - query_money(pl);
	    CLEAR_FLAG(op, FLAG_UNPAID);
	    new_draw_info_format(NDI_UNIQUE, 0, pl,
		"You lack %s to buy %s.", cost_string_from_value(i),
		query_name(op));
	    SET_FLAG(op, FLAG_UNPAID);
            return 0;
        } else {
	    object *tmp, *c_cont = op->env;
	    tag_t c = op->count;

	    CLEAR_FLAG(op, FLAG_UNPAID);
	    CLEAR_FLAG(op, FLAG_STARTEQUIP);
	    tmp=merge_ob(op,NULL);
	    if (pl->type == PLAYER) {
		    new_draw_info_format(NDI_UNIQUE, 0, pl,
						"You paid %s for %s.",buf,query_name(op));
		if (tmp) {      /* it was merged */
		    esrv_del_item (pl->contr, c, c_cont);
		    op = tmp;
		}
	        esrv_send_item(pl, op);
	    }
	}
    }
    return 1;
}

int get_payment(object *pl) {
  int ret;

  ret = get_payment2 (pl, pl->inv);

  return ret;
}

/* Modified function to give out platinum coins.  This function is	*
 * not as general as pay_for_item in finding money types - each		*
 * new money type needs to be explicity code in here.			*/
/* Modified to fill available race: gold containers before dumping	*
 * remaining coins in character's inventory. -- DAMN 			*/
void sell_item(object *op, object *pl) {
  int i=(int)query_cost(op,pl,F_SELL), count;
  object *tmp;
  object *pouch;
  archetype *at;

  if(pl==NULL||pl->type!=PLAYER) {
    LOG(llevDebug,"Object other than player tried to sell something.\n");
    return;
  }
  if(!i) {
    new_draw_info_format(NDI_UNIQUE, 0, pl,
					"We're not interested in %s.",query_name(op));

    /* Even if the character doesn't get anything for it, it may still be
     * worth something.  If so, make it unpaid
     */
	/* don't id or mark it unpaid - so we can get it back in clone shops
    if (op->value)
      SET_FLAG(op, FLAG_UNPAID);
    identify(op);
	*/
    return;
  }
  for (count=0; coins[count]!=NULL; count++) {
      at = find_archetype(coins[count]);
      if (at==NULL) LOG(llevBug, "BUG: Could not find %s archetype", coins[count]);
      else if ((i/at->clone.value) > 0) {
	  for ( pouch=pl->inv ; pouch ; pouch=pouch->below ) {
	      if ( pouch->type==CONTAINER && QUERY_FLAG(pouch, FLAG_APPLIED) && pouch->race && strstr(pouch->race, "gold") ) {
		  int w = at->clone.weight * (100-pouch->stats.Str)/100;
		  int n = i/at->clone.value;

		  if (w==0) w=1;    /* Prevent divide by zero */
		  if ( n>0 && (!pouch->weight_limit || pouch->carrying+w<=(sint32)pouch->weight_limit)) {
		      if (pouch->weight_limit && ((sint32)pouch->weight_limit-pouch->carrying)/w<n) {
			  n = (pouch->weight_limit-pouch->carrying)/w;
		      }
		      tmp = get_object();
		      copy_object(&at->clone, tmp);
		      tmp->nrof = n;
		      i -= tmp->nrof * tmp->value;
		      tmp = insert_ob_in_ob(tmp, pouch);
		      esrv_send_item (pl, tmp);
		      esrv_send_item (pl, pouch);
		      esrv_update_item (UPD_WEIGHT, pl, pouch);
		      esrv_send_item (pl, pl);
		      esrv_update_item (UPD_WEIGHT, pl, pl);
		  }
	      }
	  }
	  if (i/at->clone.value > 0) {
	      tmp = get_object();
	      copy_object(&at->clone, tmp);
	      tmp->nrof = i/tmp->value;
	      i -= tmp->nrof * tmp->value;
	      tmp = insert_ob_in_ob(tmp, pl);
	      esrv_send_item (pl, tmp);
	      esrv_send_item (pl, pl);
	      esrv_update_item (UPD_WEIGHT, pl, pl);
	  }
      }
  }

  if (i!=0) 
	LOG(llevBug,"BUG: Warning - payment not zero: %d\n", i);

  new_draw_info_format(NDI_UNIQUE, 0, pl,
	"You receive %s for %s.",query_cost_string(op,pl,1),
          query_name(op));
  SET_FLAG(op, FLAG_UNPAID);
/* TODO: unique item shop will work like old CF shops
  identify(op);
 */
}


typedef struct shopinv {
    char	*item_sort;
    char	*item_real;
    uint16	type;
    uint32	nrof;
} shopinv;

/* There are a lot fo extra casts in here just to suppress warnings - it
 * makes it look uglier than it really it.
 * The format of the strings we get is type:name.  So we first want to
 * sort by type (numerical) - if the same type, then sort by name.
 */
    /* the type is the same (what atoi gets), so do a strcasecmp to sort
     * via alphabetical order
	 */
/*
static int shop_sort(const void *a1, const void *a2)
{
    shopinv *s1 = (shopinv*)a1, *s2= (shopinv*)a2;

    if (s1->type<s2->type) return -1;
    if (s1->type>s2->type) return 1;
    return strcasecmp(s1->item_sort, s2->item_sort);
}
*/
    /* clear unpaid flag so that doesn't come up in query
     * string.  We clear nrof so that we can better sort
     * the object names.
     */
/*
static void add_shop_item(object *tmp, shopinv *items, int *numitems, int *numallocated)
{
    char buf[MAX_BUF];


    CLEAR_FLAG(tmp, FLAG_UNPAID);
    items[*numitems].nrof=tmp->nrof;
    items[*numitems].type=tmp->type;

    switch (tmp->type) {
	case RING:
	case AMULET:
	case BRACERS:
	case BOOTS:
	case GLOVES:
	case GIRDLE:
	    sprintf(buf,"%s %s",query_base_name(tmp),describe_item(tmp));
	    items[*numitems].item_sort = strdup_local(buf);
	    sprintf(buf,"%s %s",query_name(tmp),describe_item(tmp));
	    items[*numitems].item_real = strdup_local(buf);
	    (*numitems)++;
	    break;

	default:
	    items[*numitems].item_sort = strdup_local(query_base_name(tmp));
	    items[*numitems].item_real = strdup_local(query_base_name(tmp));
	    (*numitems)++;
	    break;
    }
    SET_FLAG(tmp, FLAG_UNPAID);
}
*/

/* This listing generator need really performance.
 * He collect, sort and generate full names for every item in the shop and
 * transfer it to the player. This cost cpu time & bandwith. 
 * With 10 players, this will not count... with 100+, this need to be reworked.
 */
void shop_listing(object *op)
{
	/* i removed this code - there was to much artifacts in i don't want */
}

