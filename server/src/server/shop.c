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

static sint64 PayFrom(object_t *where, sint64 amount);
static void   InsertCoin(uint8 cointype, uint32 nrof, object_t *into);

/* query_cost() will return the real value of an item
 * Thats not always ->value - and in some cases the value 
 * is calced using the default arch
 */
sint64 query_cost(object_t *tmp, object_t *who, int flag)
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
                LOG(llevMapbug, "MAPBUG:: Asking for buy-value of unidentified object %s.\n", STRING_OBJ_NAME(tmp));
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
            LOG(llevBug, "BUG: In sell item: Have object with no archetype: %s\n", STRING_OBJ_NAME(tmp));
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
static inline archetype_t * find_next_coin(sint64 c, int *cointype)
{
    archetype_t  *coin;

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
    archetype_t  *coin, *next_coin;
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

char * query_cost_string(object_t *tmp, object_t *who, int flag, int mode)
{
    return cost_string_from_value(query_cost(tmp, who, flag), mode);
}

/* query_money() calculates the total value of coins in (including containers
 * in) where's inventory.
 *
 * If money is non-NULL, the exact nrof coin denominations is recorded here.
 *
 * The return is the total value. */
sint64 query_money(object_t *where, moneyblock_t *money)
{
    object_t *this,
           *next;
    sint64  total = 0;

    if (!where)
    {
        return 0;
    }

    FOREACH_OBJECT_IN_OBJECT(this, where, next)
    {
        if (this->type == CONTAINER)
        {
            total += query_money(this, money);
            continue;
        }

        if (this->type == MONEY)
        {
            total += this->nrof * this->value;

            if (money)
            {
                uint32 *cointype = NULL;

                if (this->value == coins_arch[0]->clone.value)
                {
                    cointype = &money->mithril;
                }
                else if (this->value == coins_arch[1]->clone.value)
                {
                    cointype = &money->gold;
                }
                else if (this->value == coins_arch[2]->clone.value)
                {
                    cointype = &money->silver;
                }
                else if (this->value == coins_arch[3]->clone.value)
                {
                    cointype = &money->copper;
                }

                if (cointype)
                {
                    money->mode == MONEY_MODE_AMOUNT;
                    *cointype += (this->nrof * this->value);
                }
            }
        }
    }

    return total;
}

/* Tries to pay to_pay if op can afford it. Returns 1 or 0 on success or
 * failure. */
uint8 shop_pay_amount(sint64 amount, object_t *op)
{
    /* Sanity check. */
    if (!op)
    {
        return 0;
    }

    if (amount <= 0)                   // hurray, it's free!
    {
        return 1;
    }
    else if (amount > query_money(op, NULL)) // can't afford it
    {
        return 0;
    }

    if (op->type == PLAYER)
    {
        SET_FLAG(op, FLAG_NO_FIX_PLAYER);
    }

    if ((amount = PayFrom(op, amount)) > 0)
    {
        object_t *this,
               *next;

        FOREACH_OBJECT_IN_OBJECT(this, op, next)
        {
            if (this->type == CONTAINER &&
                (amount = PayFrom(this, amount)) <= 0)
            {
                break;
            }
        }
    }

    /* Negative means we're due change. */
    if (amount < 0)
    {
        moneyblock_t  money;
        object_t       *loot;

        (void)enumerate_coins(ABS(amount), &money);
        loot = create_financial_loot(&money, op, MODE_NO_INVENTORY);
        FREE_AND_COPY_HASH(loot->name, "your change");
        (void)pick_up(op, loot, NULL, 1);
    }

    return 1;
}

static sint64 PayFrom(object_t *where, sint64 amount)
{
    object_t *this;

    /* Sanity checks. */
    if (!where ||
        amount <= 0)
    {
        return amount;
    }

    this = where->inv;

    while (this)
    {
        object_t *next = this->below;

        if (amount <= 0)
        {
            break;
        }

        if (this->type == MONEY)
        {
            amount -= (this->nrof * this->value);
            remove_ob(this);
        }

        this = next;
    }

    return amount;
}

/* Tries to buy every unpaid item in the inv of or below this (which should
 * originally be op->inv). */
uint8 shop_checkout(object_t *op, object_t *this)
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
            ndi(NDI_UNIQUE, 0, op, "You lack the funds to buy %s.",
                          QUERY_SHORT_NAME(this, op));
        }
        else
        {
            CLEAR_FLAG(this, FLAG_UNPAID);
            ndi(NDI_UNIQUE, 0, op, "You paid %s for %s.",
                          cost_string_from_value(price, COSTSTRING_SHORT),
                          QUERY_SHORT_NAME(this, op));
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

/* shop_return_unpaid(). */
void shop_return_unpaid(object_t *who, msp_t *msp)
{
    object_t *this,
             *next;

    FOREACH_OBJECT_IN_OBJECT(this, who, next)
    {
        if (QUERY_FLAG(this, FLAG_SYS_OBJECT))
        {
            continue;
        }

        if (this->type == PLAYER ||
            this->type == MONSTER ||
            this->type == GOLEM ||
            this->type == CONTAINER)
        {
            shop_return_unpaid(this, msp);
        }

        if (QUERY_FLAG(this, FLAG_UNPAID))
        {
            remove_ob(this);
        }
    }
}

/* get_money_from_string() parses string to money.
 *
 * string should include a sequence of tokens of the basic form '# cointype'.
 * The space is optional and cointype is case insensitive and cointype may be
 * any abbreviation of the full word. For example: '1g', '1gold' '1GoL', '1 g',
 * '1g, 2 silver and 3CoP' are all valid. Alternatively string may be the
 * keyword 'all'.
 *
 * The return is money-> mode (so MONEY_MODE_NOTHING, MONEY_MODE_AMOUNT, or
 * MONEY_MODE_ALL depending on string).  */
int get_money_from_string(char *string, struct moneyblock_t *money)
{
    char *cp = string,
          buf[MEDIUM_BUF];

    memset(money, 0, sizeof(struct moneyblock_t));
    (void)get_token(string, buf, 0);

    if (!strncasecmp(buf, "all", 3))
    {
        money->mode = MONEY_MODE_ALL;
        return money->mode;
    }

    do
    {
        sint64  value;
        char   *endp;

        cp = get_token(cp, buf, 0);

        if (buf[0] != '\0' &&
            (value = strtol(buf, &endp, 10)))
        {
            if (*endp == '\0')
            {
                cp = get_token(cp, buf, 0);
                endp = buf;
            }

            if (*endp != '\0')
            {
                size_t len = strspn(endp, "cdeghilmoiprstvCDEGHILMOPRSTV");

                /* There is no way to test the coin arches directly for the
                 * name -- they get the "silver", "gold" part from material. */
                if (!strncasecmp("mithril", endp, len))
                {
                    money->mode = MONEY_MODE_AMOUNT;
                    money->mithril += value;
                }
                else if (!strncasecmp("gold", endp, len))
                {
                    money->mode = MONEY_MODE_AMOUNT;
                    money->gold += value;
                }
                else if (!strncasecmp("silver", endp, len))
                {
                    money->mode = MONEY_MODE_AMOUNT;
                    money->silver += value;
                }
                else if (!strncasecmp("copper", endp, len))
                {
                    money->mode = MONEY_MODE_AMOUNT;
                    money->copper += value;
                }
            }
        }
    }
    while (cp);

    return money->mode;
}

int query_money_type(object_t *op, int value)
{
    object_t *tmp,
           *next;
    sint64  total = 0;

    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
    {
        if (tmp->type == MONEY && tmp->value == value)
            total += tmp->nrof;
        else if (tmp->type == CONTAINER && !tmp->slaying)
            total += query_money_type(tmp, value);

        if(total >= (sint64) value)
            break;
    }
    return (int) total;
}

/* A simple function to calculate the optimum number of coins of each
 * denomination for a given value. */
int enumerate_coins(sint64 value, struct moneyblock_t *money)
{
    memset(money, 0, sizeof(struct moneyblock_t));
    money->mode = MONEY_MODE_NOTHING;

    if ((money->mithril = value / 10000000))
    {
        money->mode = MONEY_MODE_AMOUNT;
        value -= money->mithril * 10000000;
    }
    if ((money->gold = value / 10000))
    {
        money->mode = MONEY_MODE_AMOUNT;
        value -= money->gold * 10000;
    }
    if ((money->silver = value / 100))
    {
        money->mode = MONEY_MODE_AMOUNT;
        value -= money->silver * 100;
    }
    if ((money->copper = value))
        money->mode = MONEY_MODE_AMOUNT;

    return money->mode;
}

/* create_financial_loot() creates a loot_container and fills it with coins
 * according to money.
 *
 * The precise distribution of coins depends on money->mode. If this is
 * MONEY_MODE_AMOUNT, the exact amount of coins of each cointype specified in
 * money will be used. If MONEY_MODE_ALL, the optimum number of coins for the
 * total value of money will be used. If any other value (in theory this should
 * only be MONEY_MODE_NOTHING) or if money is NULL, the function does nothing
 * and returns NULL.
 *
 * If who is non-NULL and mode is MODE_NO_INVENTORY, the loot will be inserted
 * at who->map, who->x, who->y (or the map of the environment of who). If mode
 * is MODE_INVENTORY, the loot will be inderted in the inventory of who.
 *
 * The return is this loot object. Remember that the loot_container arch is a
 * sys object and is not detectable to normal players, so neither are it's
 * contents. IOW something should be done with it immediately following this
 * function to extract those contents. */
/* TODO: This can form the basis of the business end of a pickpocket skill. */
object_t *create_financial_loot(moneyblock_t *money, object_t *who, uint8 mode)
{
    object_t *loot;

    if (money &&
        money->mode == MONEY_MODE_ALL)
    {
        sint64 total = 0;

        total += (money->mithril * 10000000); 
        total += (money->gold * 10000); 
        total += (money->silver * 100); 
        total += (money->copper * 1); 
        (void)enumerate_coins(total, money); // resets money->mode
    }
    else if (!money ||
             money->mode != MONEY_MODE_AMOUNT)
    {
        return NULL;
    }

    loot = arch_to_object(archetype_global._loot_container);
    InsertCoin(0, money->mithril, loot);
    InsertCoin(1, money->gold, loot);
    InsertCoin(2, money->silver, loot);
    InsertCoin(3, money->copper, loot);

    if (who)
    {
        if (mode == MODE_NO_INVENTORY)
        {
            object_t *this;

            for (this = who; this->env; this = this->env)
            {
                ;
            }

            loot->x = this->x;
            loot->y = this->y;
            (void)insert_ob_in_map(loot, this->map, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
        }
        else
        {
            (void)insert_ob_in_ob(loot, who);
        }
    }

    return loot;
}

/* InsertCoin() inserts nrof * coins of cointype into into. */
static void InsertCoin(uint8 cointype, uint32 nrof, object_t *into)
{
     object_t *coin = clone_object(&coins_arch[cointype]->clone, MODE_NO_INVENTORY);

     coin->nrof = nrof;
     (void)insert_ob_in_ob(coin, into);
}
