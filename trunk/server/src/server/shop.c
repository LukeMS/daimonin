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

static sint64 PayFrom(object *op, object *root, sint64 amount);

/* query_cost() will return the real value of an item
 * Thats not always ->value - and in some cases the value 
 * is calced using the default arch
 */
sint64 query_cost(object *tmp, object *who, int flag)
{
    sint64  val;
    int     number; /* used to better calculate value */
    float   bon=0.0f;
    const float                     stats_penalty[10] = {0.1f, 0.15f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f}; /* used for stats 0-9 */

    if(who->stats.Cha < 10)
        bon = (-1.0f + stats_penalty[who->stats.Cha]);
    else if(who->stats.Cha > 10)
        bon = ((float)(who->stats.Cha - 10) / 100.0f);

    if ((number = tmp->nrof) == 0)
        number = 1;

    if (tmp->type == MONEY) /* money is always identified */
        return(number * tmp->value);

    /* handle identified items */
    if (QUERY_FLAG(tmp, FLAG_IDENTIFIED) || !need_identify(tmp))
    {
        if (is_cursed_or_damned(tmp))
            return 0;
		else if (tmp->type == GEM || tmp->type == TYPE_JEWEL || tmp->type == TYPE_PEARL || tmp->type == TYPE_NUGGET) /* selling unidentified gems is *always* stupid */
			val = tmp->value * number;
        else
		{
			if (flag == F_BUY)
			    val = (sint64)((float)(tmp->value * number) *(1.0f - bon));
			else if (flag == F_SELL)
			    val = (sint64)((float)(tmp->value * number) *(0.5f + bon));
                        else // F_TRUE
                            val = tmp->value * number;
		}
    }
    else /* This area deals with objects that are not identified, but can be */
    {
        if (tmp->arch != NULL)
        {
            if (flag == F_BUY)
            {
                LOG(llevMapbug, "MAPBUG:: Asking for buy-value of unidentified object %s.\n", query_name(tmp));
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
						val = (sint64)((float)(tmp->arch->clone.value * number) *(1.0f - bon));
					else
						val = (sint64)((float)(tmp->arch->clone.value * number) *(0.5f + bon));
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

    /* Because cursed stuff should always be 0, it should have a return statement all
     * of its own, so that it doesn't use this code and end up as 1.
     */
    if (val < 1)
        val = 1;

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
    static char buf[MEDIUM_BUF];
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
    while (next_coin)
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
    };

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

/* Returns the total value of coins 'carried' by op. Here carried means
 * directly in op's inventory or in an active container, or in (not
 * necessarily active) gold pouches within those two environments. */
sint64 query_money(object *op)
{
    object *this;
    sint64  total = 0;

    if (!op ||
        QUERY_FLAG(op, FLAG_SYS_OBJECT))
    {
        return 0;
    }

    for (this = op->inv; this; this = this->below)
    {
        if (this->type == MONEY)
        {
            total += this->nrof * this->value;
        }
        else if (this->type == CONTAINER &&
                 (QUERY_FLAG(this, FLAG_APPLIED) ||
                  (!this->race ||
                   strstr(this->race, "gold")))) // TODO: use shstr
        {
            total += query_money(this);
        }
    }

    return total;
}

/* Tries to pay to_pay if op can afford it. Returns 1 or 0 on success or
 * failure. */
uint8 shop_pay_amount(sint64 amount, object *op)
{
    sint64 remain;

    /* Sanity check. */
    if (!op)
    {
        return 0;
    }

    if (amount <= 0)                   // hurray, it's free!
    {
        return 1;
    }
    else if (amount > query_money(op)) // can't afford it
    {
        return 0;
    }

    if (op->type == PLAYER)
    {
        SET_FLAG(op, FLAG_NO_FIX_PLAYER);
    }

    /* Negative means we're due change. */
    if ((remain = PayFrom(op, op->inv, amount)) < 0)
    {
        uint8  i;
        sint64 change = ABS(remain);

        for (i = 0; i < NUM_COINS; i++)
        {
            if (coins_arch[i]->clone.value <= change)
            {
                sint32 nrof = 2;

                while (change >= coins_arch[i]->clone.value * nrof)
                {
                    nrof++;
                }

                nrof--;
                insert_money_in_player(op, &coins_arch[i]->clone, nrof);
                change -= coins_arch[i]->clone.value * nrof;
            }

            if (change <= 0)
            {
                break;
            }
        }

        if (change < 0)
        {
#ifdef WIN32
            LOG(llevBug, "BUG:: %s:shop_pay_amount(): Attempt to give %s %I64d too much change!\n",
                __FILE__, STRING_OBJ_NAME(op), ABS(change));
#elif SIZEOF_LONG == 8
            LOG(llevBug, "BUG:: %s:shop_pay_amount(): Attempt to give %s %ld too much change!\n",
                __FILE__, STRING_OBJ_NAME(op), ABS(change));
#else
            LOG(llevBug, "BUG:: %s:shop_pay_amount(): Attempt to give %s %lld too much change!\n",
                __FILE__, STRING_OBJ_NAME(op), ABS(change));
#endif
        }
    }

    if (op->type == PLAYER)
    {
        CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
        FIX_PLAYER(op, "pay amount");
    }

    return 1;
}

/* Takes MONEY in/below root (and then in CONTAINERs) until amount is paid
 * (it is assumed to be pre-determined that op actually has enough cash).
 * Returns amount - what was actually paid (should be <= 0, if negative,
 * change is due). */
static sint64 PayFrom(object *op, object *root, sint64 amount)
{
    object *this,
           *next;

    /* Sanity checks. */
    if (!op ||
        !root ||
        amount <= 0)
    {
        return amount;
    }

    /* This hunk should remove all the money objects from the player/container */
    for (this = root; this; this = next)
    {
        next = this->below;

        if (amount <= 0)
        {
            break;
        }

        /* FIXME: This grabs coins in reverse inventory order. So if root has
         * a stack of 50c followed by a stack of 2s and buys an item for 50c,
         * 1s will be taken and 50c chanve given, leaving root with 100c 1s
         * after the transaction. Technically this is fine but humanly it makes
         * little sense.
         *
         * This could be fixed by adding an extra step to the change-giving
         * cycle (in shop_pay_amount()) to go back through the purchaser's inv
         * and change ALL coins for their optimum denominations according to
         * total value. But this is pretty labour-intensive just to make things
         * seem 'nicer'.
         *
         * Maybe a better fix is to force a server-side ordering of coins so
         * higher value always goes before lower value. But this is pretty much
         * as much work as above.
         *
         * A third option would be to just REMEMBER all the coins in root and
         * then actually pay from this memory later on. That was how the old
         * function worked but it was huge and unwieldy.
         *
         * -- Smacky 20130126 */
        if (this->type == MONEY)
        {
            sint32 needed = (amount > this->value)
                            ? (sint32)(amount / this->value + ((amount % this->value) ? 1 : 0))
                            : 1,
                   used = MIN((sint32)this->nrof, needed);

            amount -= this->value * used;
            (void)decrease_ob_nr(this, used);
        }
    }

    /* Ugly, but in this way we ensure the whole root is searched before we go
     * recursive inside the containers. */
    for (this = root; this; this = next)
    {
        next = this->below;

        if (amount <= 0)
        {
            break;
        }

        if (this->type == CONTAINER &&
            this->inv)
        {
            amount = PayFrom(op, this->inv, amount);
        }
    }

    return amount;
}

/* Tries to buy every unpaid item in the inv of or below this (which should
 * originally be op->inv). */
uint8 shop_checkout(object *op, object *this)
{
    uint8 success;

    /* Always report success in these cases even though we are not actually
     * making a purchase. */
    if (!this ||
        QUERY_FLAG(this, FLAG_SYS_OBJECT))
    {
        return 1;
    }

    if (QUERY_FLAG(this, FLAG_UNPAID))
    {
        sint64 price = query_cost(this, op, F_BUY);

        if (!(success = shop_pay_amount(price, op)))
        {
            CLEAR_FLAG(this, FLAG_UNPAID);
            new_draw_info(NDI_UNIQUE, 0, op, "You lack the funds to buy %s.",
                          query_name(this));
            SET_FLAG(this, FLAG_UNPAID);
        }
        else
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You paid %s for %s.",
                          cost_string_from_value(price, COSTSTRING_SHORT),
                          query_name(this));
            CLEAR_FLAG(this, FLAG_UNPAID);
            (void)merge_ob(this, NULL);
            esrv_update_item(UPD_WEIGHT | UPD_NROF | UPD_FLAGS, this);
        }
    }
    else
    {
        success = 1;
    }

    /* Recursively go through EVERY item in the inv, no matter how deeply
     * buried in closed containers. TODO: In future we can modify this function
     * to perhaps allow a chance for shoplifting. */
    if (success &&
        (success = shop_checkout(op, this->inv)))
    {
        success = shop_checkout(op, this->below);
    }

    return success;
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

/* FIXME: My god this is a stupid function! I won't enumerate the problems.
 * Suffice to say it'll be remove entirly soon. ATM only used for bank deposits
 * anyway.
 *
 * -- Smacky 20130125 */
uint32 remove_money_type(object *who, object *op, sint64 value, uint32 amount)
{
    object *this,
           *next;

    for (this = op->inv; this; this = next)
    {
        next = this->below;

        if (!amount &&
            value != -1)
        {
            break;
        }

        if (this->type == MONEY &&
            (this->value == value ||
             value == -1))
        {
            if (value == -1)
            {
                amount += (uint32)(this->nrof * this->value);
                remove_ob(this);
            }
            else // don't think thiis calcs properly but this is never used anyway
            {
                sint32 preop = this->nrof;

                if ((this = decrease_ob_nr(this, amount)))
                {
                    amount -= (preop - this->nrof);
                }
                else
                {
                    amount = 0;
                }
            }
        }
        else if (this->type == CONTAINER &&
                 !this->slaying &&
                 (!this->race ||
                  strstr(this->race, "gold"))) // TODO: use shstr
        {
            amount = remove_money_type(who, this, value, amount);
        }
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
    (void)insert_ob_in_ob(tmp, pl);
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
