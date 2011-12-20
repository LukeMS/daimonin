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

char   *skill_level_name[]  =
    {
        "", "Ag", "Pe", "Me", "Ph", "Ma", "Wi"
    };

static void ShowIcons(sint16 ox, sint16 oy, sint16 x, sint16 y, uint8 invxlen, uint8 invylen,
                      _inventory_win iwin, _BLTFX *bltfx);
static void PrintInfo(sint16 x, sint16 y, item *ip, _inventory_win iwin);

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

void widget_inventory_event(int x, int y, SDL_Event event)
{
    int mx=0, my=0;
    mx = x - widget_data[WIDGET_MAIN_INV_ID].x1;
    my = y - widget_data[WIDGET_MAIN_INV_ID].y1;

    switch (event.type)
    {
    case SDL_MOUSEBUTTONUP:

        if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
        {
            /* KEYFUNC_APPLY and KEYFUNC_DROP works only if cpl.inventory_win = IWIN_INV. The tag must
                        be placed in cpl.win_inv_tag. So we do this and after DnD we restore the old values. */
            int   old_inv_win = cpl.inventory_win;
            int   old_inv_tag = cpl.win_inv_tag;
            cpl.inventory_win = IWIN_INV;

            if (draggingInvItem(DRAG_GET_STATUS) == DRAG_PDOLL)
            {
                cpl.win_inv_tag = cpl.win_pdoll_tag;
                /* we dont have to check for the coordinates, if we are here we are in the widget */
                process_macro_keys(KEYFUNC_APPLY, 0); /* drop to inventory */
            }

            cpl.inventory_win = old_inv_win;
            cpl.win_inv_tag = old_inv_tag;
        }
        else if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_BELOW)
        {
            cpl.inventory_win = IWIN_BELOW;
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100);
            process_macro_keys(KEYFUNC_GET, 0);
            cpl.inventory_win = IWIN_INV; // keep inv open.
        }
        draggingInvItem(DRAG_NONE);
        itemExamined = 0; /* ready for next item */
        break;
    case SDL_MOUSEBUTTONDOWN:

        /* inventory (open / close) */
        if (mx >= 4 &&
            mx <= 22 &&
            my >= 4 &&
            my <= 26)
        {
            if (cpl.inventory_win == IWIN_INV)
                cpl.inventory_win = IWIN_BELOW;
            else
                cpl.inventory_win = IWIN_INV;
            break;
        }


        if (mx > 226 && mx < 236)/* scrollbar */
        {
            if (my <= 39 && my >= 30 && cpl.win_inv_slot >= INVITEMXLEN)
                cpl.win_inv_slot -= INVITEMXLEN;
            else if (my >= 116 && my <= 125)
            {
                cpl.win_inv_slot += INVITEMXLEN;
                if (cpl.win_inv_slot > cpl.win_inv_count)
                    cpl.win_inv_slot = cpl.win_inv_count;
            }
        }
        else if (mx > 3)
        {
            /* stuff */
            if (event.button.button == 4 && cpl.win_inv_slot >= INVITEMXLEN) /* Mouseweel */
                cpl.win_inv_slot -= INVITEMXLEN;
            else if (event.button.button == 5) /* Mouseweel */
            {
                cpl.win_inv_slot += INVITEMXLEN;
                if (cpl.win_inv_slot > cpl.win_inv_count)
                    cpl.win_inv_slot = cpl.win_inv_count;
            }
            else if((event.button.button == SDL_BUTTON_LEFT ||
                     event.button.button == SDL_BUTTON_RIGHT ||
                     event.button.button == SDL_BUTTON_MIDDLE) &&
                    my > 29 &&
                    my < 125)
            {
                cpl.win_inv_slot = (my - 30) / 32 * INVITEMXLEN + (mx - 3) / 32 + cpl.win_inv_start;
                cpl.win_inv_tag = get_inventory_data(cpl.ob, &cpl.win_inv_ctag, &cpl.win_inv_slot,
                                                     &cpl.win_inv_start, &cpl.win_inv_count, INVITEMXLEN,
                                                     INVITEMYLEN);
                if (event.button.button == SDL_BUTTON_RIGHT || event.button.button == SDL_BUTTON_MIDDLE)
                    process_macro_keys(KEYFUNC_MARK, 0);
                else if (cpl.win_inv_tag >= 0)
                {
                    if (cpl.inventory_win == IWIN_INV)
                        draggingInvItem(DRAG_IWIN_INV);
                }
            }
        }
        break;


    case SDL_MOUSEMOTION:

        /* scrollbar-sliders */
        if (event.button.button == SDL_BUTTON_LEFT && !draggingInvItem(DRAG_GET_STATUS))
        {
            /* IWIN_INV Slider */
            if (cpl.inventory_win == IWIN_INV &&
                my + 38 &&
                my + 116 &&
                mx + 227 &&
                mx + 236)
            {
                if (old_mouse_y - y > 0)
                    cpl.win_inv_slot -= INVITEMXLEN;
                else if (old_mouse_y - y < 0)
                    cpl.win_inv_slot += INVITEMXLEN;
                if (cpl.win_inv_slot > cpl.win_inv_count)
                    cpl.win_inv_slot = cpl.win_inv_count;
                break;
            }
        }
    }
}


void widget_show_inventory_window(int x, int y)
{
    if (cpl.inventory_win != IWIN_INV)
    {
        char buf[TINY_BUF];

        if (!options.playerdoll)
            WIDGET_SHOW(WIDGET_PDOLL_ID) = 0;
        widget_data[WIDGET_MAIN_INV_ID].ht = 32;
        sprite_blt(skin_sprites[SKIN_SPRITE_INV_BG], x, y, NULL, NULL);
        string_blt(ScreenSurface, &font_small, "Carry", x+140, y+4, skin_prefs.widget_key, NULL, NULL);
        sprintf(buf, "%4.3f kg", (float)cpl.real_weight/1000.0f);
        string_blt(ScreenSurface, &font_small, buf, x + 140 + 35, y + 4,
                   percentage_colr(100 - ((float)cpl.real_weight /
                                          (float)cpl.weight_limit * 100)),
                   NULL, NULL);
        string_blt(ScreenSurface, &font_small, "Limit", x+140, y+15, skin_prefs.widget_key, NULL, NULL);
        sprintf(buf, "%4.3f kg", (float) cpl.weight_limit / 1000.0);
        string_blt(ScreenSurface, &font_small, buf, x+140 + 35, y+15, skin_prefs.widget_valueEq, NULL, NULL);
        string_blt(ScreenSurface, &font_tiny_out, "(SHIFT for inventory)", x+32, y+ 9, skin_prefs.widget_info, NULL, NULL);
        return;
    }

    if (!options.playerdoll)
       WIDGET_SHOW(WIDGET_PDOLL_ID) = 1;

    widget_data[WIDGET_MAIN_INV_ID].ht = 129;
    sprite_blt(skin_sprites[SKIN_SPRITE_INVENTORY], x, y, NULL, NULL);
    blt_window_slider(skin_sprites[SKIN_SPRITE_INV_SCROLL],
                      ((cpl.win_inv_count - 1) / INVITEMXLEN) + 1, INVITEMYLEN,
                      cpl.win_inv_start / INVITEMXLEN, -1, x + 229, y + 40);

    if (cpl.ob)
    {
        ShowIcons(x, y, 4, 30, INVITEMXLEN, INVITEMYLEN, IWIN_INV, NULL);
    }
}

void widget_below_window_event(int x, int y, int MEvent)
{
    switch (MEvent)
    {
        case MOUSE_UP:
            if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
            {
                /* KEYFUNC_APPLY and KEYFUNC_DROP works only if cpl.inventory_win = IWIN_INV. The tag must
                            be placed in cpl.win_inv_tag. So we do this and after DnD we restore the old values. */
                int old_inv_win = cpl.inventory_win;
                int old_inv_tag = cpl.win_inv_tag;

                cpl.inventory_win = IWIN_INV;

#if 0 // surely unnecessary if the server really is secure.
                if (draggingInvItem(DRAG_GET_STATUS) == DRAG_PDOLL)
                {
//                    item *op;

                    cpl.win_inv_tag = cpl.win_pdoll_tag;
                    process_macro_keys(KEYFUNC_APPLY, 0); /* drop to inventory */


//                    /* In case object disappears when unapplieid or is cursed. */
//                   if ((op = locate_item(cpl.win_inv_tag)) &&
//                        !op->applied)
                    {
                        process_macro_keys(KEYFUNC_DROP, 0); /* drop to floor */
                    }
                }
#endif
                if (draggingInvItem(DRAG_GET_STATUS) != DRAG_QUICKSLOT_SPELL)
                {
                    process_macro_keys(KEYFUNC_DROP, 0); /* drop to floor */
                }

                cpl.inventory_win = old_inv_win;
                cpl.win_inv_tag = old_inv_tag;
                draggingInvItem(DRAG_NONE);
                itemExamined = 0; /* ready for next item */
            }

            break;

        case MOUSE_DOWN:
            /* ground ( IWIN_BELOW )  */
            if (y >= widget_data[WIDGET_BELOW_INV_ID].y1+19 &&
                y <= widget_data[WIDGET_BELOW_INV_ID].y1 + widget_data[WIDGET_BELOW_INV_ID].ht - 4 &&
                x > widget_data[WIDGET_BELOW_INV_ID].x1+4 &&
                x < widget_data[WIDGET_BELOW_INV_ID].x1 + widget_data[WIDGET_BELOW_INV_ID].wd - 12)
            {
//                if (cpl.inventory_win == IWIN_INV)
//                    cpl.inventory_win = IWIN_BELOW;
                cpl.win_below_slot = (x-widget_data[WIDGET_BELOW_INV_ID].x1 - 5) / 32;
                cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag, &cpl.win_below_slot,
                                                       &cpl.win_below_start, &cpl.win_below_count, INVITEMBELOWXLEN,
                                                       INVITEMBELOWYLEN);

                if (cpl.win_below_tag >= 0)
                {
                    if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
                        draggingInvItem(DRAG_IWIN_BELOW);
                    else
                        process_macro_keys(KEYFUNC_APPLY, 0);
                }
            }
            else if (y >= widget_data[WIDGET_BELOW_INV_ID].y1+20 &&
                    y <= widget_data[WIDGET_BELOW_INV_ID].y1+29 &&
                    x > widget_data[WIDGET_BELOW_INV_ID].x1+262 &&
                    x < widget_data[WIDGET_BELOW_INV_ID].x1+269 &&
                    MEvent == MOUSE_DOWN)
            {
//                if (cpl.inventory_win == IWIN_INV)
//                    cpl.inventory_win = IWIN_BELOW;
                cpl.win_below_slot = cpl.win_below_slot - INVITEMBELOWXLEN;
                if (cpl.win_below_slot < 0)
                    cpl.win_below_slot = 0;
                cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag, &cpl.win_below_slot,
                                                       &cpl.win_below_start, &cpl.win_below_count, INVITEMBELOWXLEN,
                                                       INVITEMBELOWYLEN);
            }
            else if (y >= widget_data[WIDGET_BELOW_INV_ID].y1+42 &&
                    y <= widget_data[WIDGET_BELOW_INV_ID].y1+51 &&
                    x > widget_data[WIDGET_BELOW_INV_ID].x1+262 &&
                    x < widget_data[WIDGET_BELOW_INV_ID].x1+269 &&
                    MEvent == MOUSE_DOWN)
            {
//                if (cpl.inventory_win == IWIN_INV)
//                    cpl.inventory_win = IWIN_BELOW;
                cpl.win_below_slot = cpl.win_below_slot + INVITEMBELOWXLEN;
                if (cpl.win_below_slot > cpl.win_below_count -1)
                    cpl.win_below_slot = cpl.win_below_count -1;
                cpl.win_below_tag = get_inventory_data(cpl.below, &cpl.win_below_ctag, &cpl.win_below_slot,
                                                       &cpl.win_below_start, &cpl.win_below_count, INVITEMBELOWXLEN,
                                                       INVITEMBELOWYLEN);
            }

            break;
    }
}

void widget_show_below_window(item *op, int x, int y)
{
    sprite_blt(skin_sprites[SKIN_SPRITE_BELOW], x, y, NULL, NULL);
    blt_window_slider(skin_sprites[SKIN_SPRITE_BELOW_SCROLL], ((cpl.win_below_count - 1) / INVITEMBELOWXLEN) + 1,
                      INVITEMBELOWYLEN, cpl.win_below_start / INVITEMBELOWXLEN, -1, x + 263, y + 30);

    if (cpl.below)
    {
        ShowIcons(x, y, 5, 19, INVITEMBELOWXLEN, INVITEMBELOWYLEN, IWIN_BELOW,
                  NULL);
    }
}

static void ShowIcons(sint16 ox, sint16 oy, sint16 x, sint16 y, uint8 invxlen, uint8 invylen,
                      _inventory_win iwin, _BLTFX *bltfx)
{
    uint16  i,
            start = (iwin == IWIN_INV)
                    ? cpl.win_inv_start : cpl.win_below_start,
            slot = (iwin == IWIN_INV) ? cpl.win_inv_slot : cpl.win_below_slot;
    item   *wp = (iwin == IWIN_INV) ? cpl.ob : cpl.below,
           *ip = wp->inv,
           *cip = cpl.sack->inv,
           *tmp = NULL;

    for (i = 0; i < start; i++)
    {
        if (!ip)
        {
            return;
        }

        if (cpl.container &&
            cpl.container->tag == ip->tag)
        {
            tmp = ip;

            for (i += 1; i < start; i++)
            {
                if (!cip)
                {
                    break;
                }

                cip = cip->next;
            }

            if (cip)
            {
                i = 0;
                ip = tmp;

                goto jump_in_container;
            }
        }

        ip = ip->next;
    }

    for (i = 0; i < invxlen * invylen; i++)
    {
        sint16             xi,
                           yi;
        sprite_icon_type_t type;
        uint8              selected,
                           quacon;

        if (!ip)
        {
            return;
        }

        xi = (i % invxlen) * 32 + ox + x;
        yi = (i / invxlen) * 32 + oy + y;
        type = (iwin == IWIN_INV &&
                (int)ip->tag == cpl.mark_count)
               ? SPRITE_ICON_TYPE_ACTIVE : SPRITE_ICON_TYPE_NONE;
        selected = (cpl.inventory_win == iwin &&
                    i + start == slot) ? 1 : 0;
        quacon = (ip->item_qua == 255) ? 255
                 : (float)ip->item_con / (float)ip->item_qua * 100;
        sprite_blt_as_icon(face_list[ip->face].sprite, xi, yi, type, selected,
                           ip->flagsval, (quacon == 100) ? 0 : quacon,
                           (ip->nrof == 1) ? 0 : ip->nrof, bltfx);

        if (selected)
        {
            PrintInfo(ox, oy, ip, iwin);
        }

        if (cpl.container &&
            cpl.container->tag == ip->tag)
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_CMARK_START], xi, yi, NULL, bltfx);

jump_in_container:
            for (i += 1; i < invxlen * invylen; i++)
            {
                if (!cip)
                {
                    i--;

                    break;
                }

                xi = (i % invxlen) * 32 + ox + x;
                yi = (i / invxlen) * 32 + oy + y;
                type = (iwin == IWIN_INV &&
                        (int)cip->tag == cpl.mark_count)
                       ? SPRITE_ICON_TYPE_ACTIVE : SPRITE_ICON_TYPE_NONE;
                selected = (cpl.inventory_win == iwin &&
                            i + start == slot) ? 1 : 0;
                quacon = (cip->item_qua == 255) ? 255
                         : (float)cip->item_con / (float)cip->item_qua * 100;

                sprite_blt_as_icon(face_list[cip->face].sprite, xi, yi, type,
                                   selected, cip->flagsval,
                                   (quacon == 100) ? 0 : quacon, 
                                   (cip->nrof == 1) ? 0 : cip->nrof, bltfx);

                if (selected)
                {
                    PrintInfo(ox, oy, cip, iwin);
                }

                if ((cip = cip->next))
                {
                    sprite_blt(skin_sprites[SKIN_SPRITE_CMARK_MIDDLE], xi, yi, NULL, bltfx);
                }
                else
                {
                    sprite_blt(skin_sprites[SKIN_SPRITE_CMARK_END], xi, yi, NULL, bltfx);
                }
            }
        }

        ip = ip->next;
    }
}

static void PrintInfo(sint16 x, sint16 y, item *ip, _inventory_win iwin)
{
    char         buf[MEDIUM_BUF];
    SDL_Surface *surface = ScreenSurface;
    
    x += ((iwin == IWIN_INV) ? 36 : 4);

    /* Print 'nrof name'. */
    if (ip->nrof == 1)
    {
        sprintf(buf, "%s", ip->s_name);
    }
    else
    {
        sprintf(buf, "%d %s", ip->nrof, ip->s_name);
    }

    string_blt(surface, &font_small, buf, x, y + 4, skin_prefs.widget_title, NULL, NULL);

    /* In the below inv this is all the info we get. This is simply a real
     * estate issue. There just isn't space to squeeze in more info. This could
     * be addressed by using a tooltip or embiggening the window. Arguably
     * though it makes little sense that the player inherently know the weight,
     * condition, etc of items on the floor, but note that this info IS
     * available to the client, so the current restriction is insecure. */
    if (iwin == IWIN_BELOW)
    {
       return;
    }

    sprintf(buf, "weight: ");
    string_blt(surface, &font_small, buf, x, y + 16, skin_prefs.widget_key, NULL, NULL);
    x += string_width(&font_small, buf);
    sprintf(buf, "%4.3f ", (float)ip->weight / 1000.0);
    string_blt(surface, &font_small, buf, x, y + 16, skin_prefs.widget_valueEq, NULL, NULL);
    x += string_width(&font_small, buf);

    if (ip->item_qua == 255) /* this comes from server when not identified */
    {
        string_blt(surface, &font_small, "(not identified)", x, y + 16,
                   skin_prefs.widget_info, NULL, NULL);
    }
    else
    {
        sprintf(buf, "con: ");
        string_blt(surface, &font_small, buf, x, y + 16, skin_prefs.widget_key, NULL,
                   NULL);
        x += string_width(&font_small, buf);
        sprintf(buf, "%d ", ip->item_con);
        string_blt(ScreenSurface, &font_small, buf, x, y + 16,
                   percentage_colr((float)ip->item_con /
                                   (float)ip->item_qua * 100), NULL, NULL);
        x += string_width(&font_small, buf);
        sprintf(buf, "/ %d", ip->item_qua);
        string_blt(surface, &font_small, buf, x, y + 16, skin_prefs.widget_valueHi,
                   NULL, NULL);
        x += string_width(&font_small, buf);
        sprintf(buf, "allowed: ");
        string_blt(surface, &font_small, buf, x, y + 16, skin_prefs.widget_key, NULL,
                   NULL);
        x += string_width(&font_small, buf);

        if (ip->item_level)
        {
            sprintf(buf, "lvl %d %s", ip->item_level, skill_level_name[ip->item_skill]);

            if ((!ip->item_skill &&
                 ip->item_level <= cpl.stats.level) ||
                (ip->item_skill &&
                 ip->item_level <= cpl.stats.skill_level[ip->item_skill - 1]))
            {
                string_blt(surface, &font_small, buf, x, y + 16,
                           skin_prefs.widget_valueEq, NULL, NULL);
            }
            else
            {
                string_blt(surface, &font_small, buf, x, y + 16,
                           skin_prefs.widget_valueLo, NULL, NULL);
            }
        }
        else
        {
            string_blt(surface, &font_small, "all", x, y + 16,
                       skin_prefs.widget_valueHi, NULL, NULL);
        }
    }
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
