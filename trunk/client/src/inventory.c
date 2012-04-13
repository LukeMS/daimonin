/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

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
#include "include.h"
#include "math.h"

/* this function returns number of items and adjust the inventory windows data */
int get_inventory_data(item *op, int *ctag, int *slot, int *start, int *count, int wxlen, int wylen)
{
    register item * tmp, * tmpc;
    register int i = 0;
    int ret = -1;

    cpl.window_weight = 0;
    *ctag = -1;
    *count = 0;
    if (!op)
    {
        *slot = *start = 0;
        return(-1);
    }
    if (!op->inv)
    {
        *slot = *start = 0;
        return(-1);
    }
    if (*slot < 0)
        *slot = 0;
    /* pre count items, and adjust slot cursor*/
    for (tmp = op->inv; tmp; tmp = tmp->next)
    {
        (*count)++;

        cpl.window_weight += tmp->weight;
        if ((int)tmp->tag == cpl.container_tag)
            cpl.container = tmp;
        if (cpl.container && cpl.container->tag == tmp->tag)
        {
            tmpc = cpl.sack->inv;
            for (; tmpc; tmpc = tmpc->next)
                (*count)++;
        }
    }
    if (!*count)
        *slot = 0;
    else if (*slot >= *count)
        *slot = *count - 1;
    /* now find tag*/
    for (tmp = op->inv; tmp; tmp = tmp->next)
    {
        if (*slot == i)
            ret = tmp->tag;
        i++;
        if (cpl.container && cpl.container->tag == tmp->tag)
        {
            tmpc = cpl.sack->inv;
            for (; tmpc; tmpc = tmpc->next)
            {
                if (*slot == i)
                {
                    *ctag = cpl.container->tag;
                    ret = tmpc->tag;
                }
                i++;
            }
        }
    }
    /* and adjust the slot/start position of the window*/
    if (*slot < *start)
        *start = *slot - (*slot % wxlen);
    else if (*slot > *start + (wxlen * wylen) - 1)
        *start = ((int) (*slot / wxlen)) * wxlen - (wxlen * (wylen - 1));
    return(ret);
}

void examine_range_inv(void)
{
    register item * op, * tmp;

    op = cpl.ob;
    if (!op->inv)
        return;
    fire_mode.weapon = FIRE_ITEM_NO;
    fire_mode.ammo = FIRE_ITEM_NO;

    for (tmp = op->inv; tmp; tmp = tmp->next)
    {
        if (tmp->applied && (tmp->itype == TYPE_BOW || (tmp->itype == TYPE_ARROW && tmp->stype >= 128)
            || tmp->itype == TYPE_WAND || tmp->itype == TYPE_ROD || tmp->itype == TYPE_HORN))
            fire_mode.weapon = tmp->tag;
        else if(tmp->applied && tmp->itype == TYPE_ARROW && tmp->stype < 128)
            fire_mode.ammo = tmp->tag;
    }
}
