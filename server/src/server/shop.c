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

static sint64 pay_from_container(object *op, object *pouch, sint64 to_pay);

/* query_cost() will return the real value of an item
 * Thats not always ->value - and in some cases the value 
 * is calced using the default arch
 */
sint64 query_cost(object *tmp, object *who, int flag)
{
    sint64  val;
    int     number; /* used to better calculate value */

    if ((number = tmp->nrof) == 0)
        number = 1;

    if (tmp->type == MONEY) /* money is always identified */
        return(number * tmp->value);

    /* handle identified items */
    if (QUERY_FLAG(tmp, FLAG_IDENTIFIED) || !need_identify(tmp))
    {
        if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))
            return 0;
		else if (tmp->type == GEM || tmp->type == TYPE_JEWEL || tmp->type == TYPE_PEARL || tmp->type == TYPE_NUGGET) /* selling unidentified gems is *always* stupid */
			val = tmp->value * number;
        else
		{
			if (flag == F_BUY)
				val = (sint64)((float)(tmp->value * number) *1.0f);
			else
				val = (sint64)((float)(tmp->value * number) *0.8f);
		}
    }
    else /* This area deals with objects that are not identified, but can be */
    {
        if (tmp->arch != NULL)
        {
            if (flag == F_BUY)
            {
                LOG(llevBug, "BUG: Asking for buy-value of unidentified object %s.\n", query_name(tmp));
                val = tmp->arch->clone.value * number;
            }
            else    /* Trying to sell something, or get true value */
            {
                if (tmp->type == GEM || tmp->type == TYPE_JEWEL || tmp->type == TYPE_PEARL || tmp->type == TYPE_NUGGET) /* selling unidentified gems is *always* stupid */
                    val = number * 3;
                else if (tmp->type == POTION)
                    val = number * 50; /* Don't want to give anything away */
                else
				{
					if (flag == F_BUY)
						val = (sint64)((float)(tmp->arch->clone.value * number) *1.0f);
					else
						val = (sint64)((float)(tmp->arch->clone.value * number) *0.8f);
				}
            }
        }
        else
        {
            /* No archetype with this object - we generate some dummy values to avoid server break */
            LOG(llevBug, "BUG: In sell item: Have object with no archetype: %s\n", query_name(tmp));
            if (flag == F_BUY)
            {
                LOG(llevBug, "BUG: Asking for buy-value of unidentified object without arch.\n");
                val = number * 100;
            }
            else
                val = number * 80;
        }
    }

    /* wands will count special. The base value is for a wand with one charge */
    if (tmp->type == WAND)
        val += (val * tmp->level) * tmp->stats.food;
    else if (tmp->type == ROD || tmp->type == HORN || tmp->type == POTION || tmp->type == SCROLL)
        val += val * tmp->level;

    return val;
}

/* Find the coin type that is worth more the 'c'.  Starts at the
 * cointype placement.
 */
static inline archetype * find_next_coin(sint64 c, int *cointype)
{
    archetype  *coin;

    do
    {
        coin = coins_arch[*cointype];
        if (coin == NULL)
            return NULL;
        *cointype += 1;
    }
    while (coin->clone.value > c);

    return coin;
}

/* This returns a string of how much something is worth based on
 * an integer being passed.
 */
char * cost_string_from_value(sint64 cost, int mode)
{
    static char buf[MAX_BUF];
    archetype  *coin, *next_coin;
    char       *endbuf;
    uint32      num;
    int         cointype = 0;

    coin = find_next_coin(cost, &cointype);
    if (coin == NULL)
        return "nothing";

    num = (uint32) (cost / coin->clone.value);
    cost -= num * coin->clone.value;
    /* careful - never set a coin arch to material_real = -1 ! */
    if (mode == COSTSTRING_SHORT)
        sprintf(buf, "%d%c", num, material_real[coin->clone.material_real].name[0]);
    else if (mode == COSTSTRING_FULL)
        sprintf(buf, "%d %s", num, material_real[coin->clone.material_real].name);
    next_coin = find_next_coin(cost, &cointype);
    if (next_coin == NULL)
    {
        if (mode != COSTSTRING_SHORT)
        {
            if (num == 1)
                strcat(buf, "coin");
            else
                strcat(buf, "coins");
        }
        return buf;
    }

    do
    {
        endbuf = buf + strlen(buf);

        coin = next_coin;
        num = (uint32) (cost / coin->clone.value);
        cost -= num * coin->clone.value;

        if (cost == 0)
            next_coin = NULL;
        else
            next_coin = find_next_coin(cost, &cointype);

        if (next_coin)
        {
            /* There will be at least one more string to add to the list,
             * use a comma.
             */
            /* Can't work out how, but we need to check for and remove any
             * trailing space. Stupid material names. */
            strcat(endbuf, ", "); endbuf += 2;
        }
        else
        {
            if (mode == COSTSTRING_SHORT)
                strcat(endbuf++, " ");
            strcat(endbuf, "and "); endbuf += 4;
        }
        if (mode == COSTSTRING_SHORT)
            sprintf(endbuf, "%d%c", num, material_real[coin->clone.material_real].name[0]);
        else if (mode == COSTSTRING_FULL)
            sprintf(endbuf, "%d %s", num, material_real[coin->clone.material_real].name);
    }
    while (next_coin);

    if (mode != COSTSTRING_SHORT)
    {
        if (num == 1)
            strcat(buf, "coin");
        else
            strcat(buf, "coins");
    }

    return buf;
}

char * query_cost_string(object *tmp, object *who, int flag, int mode)
{
    return cost_string_from_value(query_cost(tmp, who, flag), mode);
}




/* This function finds out how much money the player is carrying,   *
 * and returns that value                       */
/* Now includes any coins in active containers -- DAMN          */
/* or every gold type container (even not applied) */
sint64 query_money(object *op)
{
    object *tmp;
    sint64     total   = 0;

    if (op->type != PLAYER && op->type != CONTAINER)
    {
        LOG(llevBug, "BUG: Query money called with non player/container.\n");
        return 0;
    }
    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        if (tmp->type == MONEY)
            total += tmp->nrof * tmp->value;
        else if (tmp->type == CONTAINER && ((!tmp->race || strstr(tmp->race, "gold")) || QUERY_FLAG(tmp, FLAG_APPLIED)))
            total += query_money(tmp);
    }
    return total;
}


/* TCHIZE: This function takes the amount of money from the             *
 * the player inventory and from it's various pouches using the         *
 * pay_from_container function.                                         *
 * returns 0 if not possible. 1 if success                              */
int pay_for_amount(sint64 to_pay, object *pl)
{
    if (to_pay == 0)
        return 1;
    if (to_pay > query_money(pl))
        return 0;

    pay_from_container(pl, pl, to_pay);

    FIX_PLAYER(pl ,"pay for amount");
    return 1;
}

/* DAMN: This is now a wrapper for pay_from_container, which is     *
 * called for the player, then for each active container that can hold  *
 * money until op is paid for.  Change will be left wherever the last   *
 * of the price was paid from.                      */
int pay_for_item(object *op, object *pl)
{
    sint64     to_pay  = query_cost(op, pl, F_BUY);

    if (to_pay == 0.0)
        return 1;
    if (to_pay > query_money(pl))
        return 0;

    pay_from_container(pl, pl, to_pay);

    FIX_PLAYER(pl ,"pay for item");
    return 1;
}

/* This pays for the item, and takes the proper amount of money off
 * the player.
 * CF 0.91.4 - this function is mostly redone in order to fix a bug
 * with weight not be subtracted properly.  We now remove and
 * insert the coin objects -  this should update the weight
 * appropriately
 */
/* DAMN: This function is used for the player, then for any active  *
 * containers that can hold money, until the op is paid for.        */
static sint64 pay_from_container(object *op, object *pouch, sint64 to_pay)
{
    sint64        remain;
    int         count, i;
    object     *tmp, *coin_objs[NUM_COINS], *next;
    archetype  *at;
    object     *who;

    if (pouch->type != PLAYER && pouch->type != CONTAINER)
        return to_pay;

    remain = to_pay;
    for (i = 0; i < NUM_COINS; i++)
        coin_objs[i] = NULL;

    /* This hunk should remove all the money objects from the player/container */
    for (tmp = pouch->inv; tmp; tmp = next)
    {
        next = tmp->below;

        if (tmp->type == MONEY)
        {
            for (i = 0; i < NUM_COINS; i++)
            {
                if (coins_arch[NUM_COINS - 1 - i]->name  == tmp->arch->name && (tmp->value == tmp->arch->clone.value))
                {
                    /* This should not happen, but if it does, just merge the two */
					if(tmp->env)
					{
						if (tmp->env->type == PLAYER)
							esrv_del_item(CONTR(tmp->env), tmp->count, tmp->env);
						if (tmp->env->type == CONTAINER)
							esrv_del_item(NULL, tmp->count, tmp->env);
					}
					remove_ob(tmp);
					if (coin_objs[i] != NULL)
                    {
                        LOG(llevBug, "BUG: %s has two money entries of (%s)\n", query_name(pouch),
                            coins_arch[NUM_COINS - 1 - i]->name);
                        coin_objs[i]->nrof += tmp->nrof;
                    }
                    else
                    {
                        coin_objs[i] = tmp;
                    }

                    break;
                }
            }
            if (i == NUM_COINS)
                LOG(llevBug, "BUG: in pay_for_item: Did not find string match for %s\n", tmp->arch->name);
        }
    }

	/* ugly, but in this way we ensure the whole root ->inv is searched,
	 * before we go recursive inside the containers
	 */
	for (tmp = pouch->inv; tmp; tmp = next)
	{
		next = tmp->below;

		if (tmp->type == CONTAINER)
			remain = pay_from_container(op, tmp, remain);
	}

    /* Fill in any gaps in the coin_objs array - needed to make change. */
    /* Note that the coin_objs array goes from least value to greatest value */
    for (i = 0; i < NUM_COINS; i++)
    {
        if (coin_objs[i] == NULL)
        {
            at = coins_arch[NUM_COINS - 1 - i];
            if (at == NULL)
                LOG(llevBug, "BUG: Could not find %s archetype", coins_arch[NUM_COINS - 1 - i]->name);
            coin_objs[i] = get_object();
            copy_object(&at->clone, coin_objs[i]);
            coin_objs[i]->nrof = 0;
        }
    }

    for (i = 0; i < NUM_COINS; i++)
    {
        sint64 num_coins;

        if (coin_objs[i]->nrof * coin_objs[i]->value > remain)
        {
            num_coins = remain / coin_objs[i]->value;
            if (num_coins * coin_objs[i]->value < remain)
                num_coins++;
        }
        else
        {
            num_coins = coin_objs[i]->nrof;
        }

        if(num_coins> ((sint64)1 << 31))
        {
            LOG(llevDebug,"shop.c (line: %d): money overflow value->nrof: number of coins>2^32 (type coin %d)\n", __LINE__ , i);
            num_coins = ((sint64)1 << 31);
        }

        remain -= num_coins * coin_objs[i]->value;
        coin_objs[i]->nrof -= (uint32) num_coins;

        /* Now start making change.  Start at the coin value
         * below the one we just did, and work down to
         * the lowest value.
         */
        count = i - 1;
        while (remain < 0 && count >= 0)
        {
            num_coins = -remain / coin_objs[count]->value;
            coin_objs[count]->nrof += (uint32)num_coins;
            remain += num_coins * coin_objs[count]->value;
            count--;
        }
    }
    for (i = 0; i < NUM_COINS; i++)
    {
        if (coin_objs[i]->nrof)
        {
            object *tmp = insert_ob_in_ob(coin_objs[i], pouch);
            for (who = pouch; who && who->type != PLAYER && who->env != NULL; who = who->env)
            {
            }
            esrv_send_item(who, tmp);
            esrv_send_item(who, pouch);
            esrv_update_item(UPD_WEIGHT, who, pouch);
            if (pouch->type != PLAYER)
            {
                esrv_send_item(who, who);
                esrv_update_item(UPD_WEIGHT, who, who);
            }
        }
    }
    return(remain);
}

/* Eneq(@csd.uu.se): Better get_payment, descends containers looking for
   unpaid items. get_payment is now used as a link. To make it simple
   we need the player-object here. */

int get_payment2(object *pl, object *op)
{
    char    buf[MAX_BUF];
    int     ret = 1;

    if (op != NULL && op->inv)
        ret = get_payment2(pl, op->inv);

    if (!ret)
        return 0;

    if (op != NULL && op->below)
        ret = get_payment2(pl, op->below);

    if (!ret)
        return 0;

    if (op != NULL && QUERY_FLAG(op, FLAG_UNPAID))
    {
        strncpy(buf, query_cost_string(op, pl, F_BUY, COSTSTRING_SHORT), MAX_BUF);
        if (!pay_for_item(op, pl))
        {
            sint64 i   = query_cost(op, pl, F_BUY) - query_money(pl);
            CLEAR_FLAG(op, FLAG_UNPAID);
            new_draw_info_format(NDI_UNIQUE, 0, pl, "You lack %s to buy %s.", cost_string_from_value(i, COSTSTRING_SHORT), query_name(op));
            SET_FLAG(op, FLAG_UNPAID);
            return 0;
        }
        else
        {
            object *tmp, *c_cont = op->env;
            tag_t   c   = op->count;

            CLEAR_FLAG(op, FLAG_UNPAID);
            CLEAR_FLAG(op, FLAG_STARTEQUIP);
            if (pl->type == PLAYER)
                new_draw_info_format(NDI_UNIQUE, 0, pl, "You paid %s for %s.", buf, query_name(op));
            tmp = merge_ob(op, NULL);
            if (pl->type == PLAYER)
            {
                if (tmp)
                {
                    /* it was merged */
                    esrv_del_item(CONTR(pl), c, c_cont);
                    op = tmp;
                }
                esrv_send_item(pl, op);
            }
        }
    }
    return 1;
}

int get_payment(object *pl)
{
    int ret;

    ret = get_payment2(pl, pl->inv);

    return ret;
}

/* Modified function to give out platinum coins.  This function is  *
 * not as general as pay_for_item in finding money types - each     *
 * new money type needs to be explicity code in here.           */
/* Modified to fill available race: gold containers before dumping  *
 * remaining coins in character's inventory. -- DAMN             */
void sell_item(object *op, object *pl, sint64 value)
{
    sint64      i;
    int            count;
    object     *tmp;
    object     *pouch;
    archetype  *at;

    if (pl == NULL || pl->type != PLAYER)
    {
        LOG(llevDebug, "Object other than player tried to sell something.\n");
        return;
    }

    if (op == NULL)
        i = value;
    else
        i = (int) query_cost(op, pl, F_SELL);

    if (!i)
    {
        if (op)
            new_draw_info_format(NDI_UNIQUE, 0, pl, "We're not interested in %s.", query_name(op));

        /* Even if the character doesn't get anything for it, it may still be
         * worth something.  If so, make it unpaid
         */

        /* don't id or mark it unpaid - so we can get it back in clone shops
          if (op->value)
            SET_FLAG(op, FLAG_UNPAID);
          identify(op);
        */

        /* if this return is enabled, items with value 0 will don't be put
         * in store backroom (aka destroyed) and will stay inside the shop.
         * if disabeld, the shop will remove it but you don't get any money.
         */
        /* return; */
    }
    /* i can't say i understand this... MT-2004 */
    for (count = 0; coins_arch[count] != NULL; count++)
    {
        /* this can be speed up - we have now a prebuild table for this MT-2004 */
        at = coins_arch[count];
        if (at == NULL)
            LOG(llevBug, "BUG: Could not find %s archetype", coins_arch[count]->name);
        else if ((i / at->clone.value) > 0)
        {
            for (pouch = pl->inv ; pouch ; pouch = pouch->below)
            {
                if (pouch->type == CONTAINER && QUERY_FLAG(pouch, FLAG_APPLIED) && pouch->race && strstr(pouch->race,
                                                                                                         "gold"))
                {
                    int w   = (int) ((float) at->clone.weight *pouch->weapon_speed);
                    uint32 n   = (uint32) (i / at->clone.value);

                    if (w == 0)
                        w = 1;    /* Prevent divide by zero */
                    if (n > 0 && (!pouch->weight_limit || pouch->carrying + w <=  pouch->weight_limit))
                    {
			if (pouch->weight_limit && (((pouch->weight_limit - pouch->carrying) / w) < (sint32)n))
                        {
                            n = (pouch->weight_limit - pouch->carrying) / w;
                        }
                        tmp = get_object();
                        copy_object(&at->clone, tmp);
                        tmp->nrof = n;
                        i -= tmp->nrof * tmp->value;
                        tmp = insert_ob_in_ob(tmp, pouch);
                        esrv_send_item(pl, tmp);
                        esrv_send_item(pl, pouch);
                        esrv_update_item(UPD_WEIGHT, pl, pouch);
                        esrv_send_item(pl, pl);
                        esrv_update_item(UPD_WEIGHT, pl, pl);
                    }
                }
            }
            if (i / at->clone.value > 0)
            {
                tmp = get_object();
                copy_object(&at->clone, tmp);
                tmp->nrof = (uint32)(i / tmp->value);
                i -= tmp->nrof * tmp->value;
                tmp = insert_ob_in_ob(tmp, pl);
                esrv_send_item(pl, tmp);
                esrv_send_item(pl, pl);
                esrv_update_item(UPD_WEIGHT, pl, pl);
            }
        }
    }

    if (!op)
        return;

    if (i != 0)
        LOG(llevBug, "BUG: Warning - payment not zero: %d\n", i);

    new_draw_info_format(NDI_UNIQUE, 0, pl, "You receive %s for %s.", query_cost_string(op, pl, COSTSTRING_FULL), query_name(op));
    SET_FLAG(op, FLAG_UNPAID);
    /* TODO: unique item shop will work like old CF shops
      identify(op);
     */
}

/* return 0 = we have nothing found.
 * return 1 = we have some money.
 * return -1 = we have keyword "all"
 */
int get_money_from_string(char *text, struct _money_block *money)
{
    int     pos = 0;
    char   *word;

    memset(money, 0, sizeof(struct _money_block));

    /* kill all whitespace */
    while (*text != '\0' && isspace(*text))
        text++;

    /* easy, special case: all money */
    if (!strncasecmp(text, "all", 3))
    {
        money->mode = MONEYSTRING_ALL;
        return money->mode;
    }

    /* parse that sucker. we simply look for a word
     * which is a number and then we test the next
     * word is like "mithril", "gold", "silver" or "copper".
     * is not, we go on.
     */
    money->mode = MONEYSTRING_NOTHING;

    while ((word = get_word_from_string(text, &pos)))
    {
        int i = 0, flag = *word;


        while (*(word + i) != '\0')
        {
            if (*(word + i) < '0' || *(word + i) > '9')
                flag = 0;
            i++;
        }

        if (flag) /* if still set, we have a valid number in the word string */
        {
            int value   = atoi(word);

            if (value > 0 && value < 1000000) /* a valid number - now lets look we have a valid money keyword */
            {
                if ((word = get_word_from_string(text, &pos)) && *word != '\0')
                {
                    int len = strlen(word);
                    /* there is no way to test the coin arches direct for
                                 * the name - they get the "silver", "gold" part from
                                 * material...
                                 */
                    if (!strncasecmp("mithril", word, len))
                    {
                        money->mode = MONEYSTRING_AMOUNT;
                        money->mithril += value;
                    }
                    else if (!strncasecmp("gold", word, len))
                    {
                        money->mode = MONEYSTRING_AMOUNT;
                        money->gold += value;
                    }
                    else if (!strncasecmp("silver", word, len))
                    {
                        money->mode = MONEYSTRING_AMOUNT;
                        money->silver += value;
                    }
                    else if (!strncasecmp("copper", word, len))
                    {
                        money->mode = MONEYSTRING_AMOUNT;
                        money->copper += value;
                    }
                }
            }
        }
    }

    return money->mode;
}

int query_money_type(object *op, int value)
{
    object *tmp;
    sint64     total   = 0;

    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        if (tmp->type == MONEY && tmp->value == value)
            total += tmp->nrof;
        else if (tmp->type == CONTAINER && !tmp->slaying && ((!tmp->race || strstr(tmp->race, "gold"))))
            total += query_money_type(tmp, value);

        if(total >= (sint64) value)
            break;
    }
    return (int) total;
}

sint64 remove_money_type(object *who, object *op, sint64 value, sint64 amount)
{
    object *tmp, *tmp2;

    for (tmp = op->inv; tmp; tmp = tmp2)
    {
        tmp2 = tmp->below;

        if (!amount && value != -1)
            return amount;
        if (tmp->type == MONEY && (tmp->value == value || value == -1))
        {
            if (tmp->nrof <= amount || value == -1)
            {
                object *env = tmp->env;
                if (value == -1)
                    amount += (tmp->nrof * tmp->value);
                else
                    amount -= tmp->nrof;
                remove_ob(tmp);
                if (op->type == PLAYER)
                    esrv_del_item(CONTR(op), tmp->count, NULL);
                else
                    esrv_del_item(NULL, tmp->count, env);
            }
            else
            {
                tmp->nrof -= (uint32) amount;
                amount = 0;

                esrv_send_item(who, tmp);
                esrv_send_item(who, op);
                esrv_update_item(UPD_WEIGHT, who, op);
                if (op->type != PLAYER)
                {
                    esrv_send_item(who, who);
                    esrv_update_item(UPD_WEIGHT, who, who);
                }
            }
        }
        else if (tmp->type == CONTAINER && !tmp->slaying && ((!tmp->race || strstr(tmp->race, "gold"))))
            amount = remove_money_type(who, tmp, value, amount);
    }
    return amount;
}

/* add number of money coins to a player */
void add_money_to_player(object *pl, int c, int s, int g, int m)
{
    /* we don't handle decrease/remove of coins (<0) atm */
    if(m>0)
        insert_money_in_player(pl, &coins_arch[0]->clone, m);
    if(g>0)
        insert_money_in_player(pl, &coins_arch[1]->clone, g);
    if(s>0)
        insert_money_in_player(pl, &coins_arch[2]->clone, s);
    if(c>0)
        insert_money_in_player(pl, &coins_arch[3]->clone, c);
}

void insert_money_in_player(object *pl, object *money, uint32 nrof)
{
    object *tmp;
    tmp = get_object();
    copy_object(money, tmp);
    tmp->nrof = nrof;
    tmp = insert_ob_in_ob(tmp, pl);
    esrv_send_item(pl, tmp);
    esrv_send_item(pl, pl);
    esrv_update_item(UPD_WEIGHT, pl, pl);
}

/* A simple function to calculate the optimum number of coins of each
 * denomination for a given value. */
int enumerate_coins(sint64 value, struct _money_block *money)
{
    memset(money, 0, sizeof(struct _money_block));
    money->mode = MONEYSTRING_NOTHING;

    if ((money->mithril = value / 10000000))
    {
        money->mode = MONEYSTRING_AMOUNT;
        value -= money->mithril * 10000000;
    }
    if ((money->gold = value / 10000))
    {
        money->mode = MONEYSTRING_AMOUNT;
        value -= money->gold * 10000;
    }
    if ((money->silver = value / 100))
    {
        money->mode = MONEYSTRING_AMOUNT;
        value -= money->silver * 100;
    }
    if ((money->copper = value))
        money->mode = MONEYSTRING_AMOUNT;

    return money->mode;
}
