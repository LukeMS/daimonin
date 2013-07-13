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

static void show_inventory_item_stats(item *tmp, int x, int y)
{
    char        buf[256];
    SDL_Rect    tmp_rect;
    tmp_rect.w = 222;

    if (tmp->nrof > 1)
        sprintf(buf, "%d %s", tmp->nrof, tmp->s_name);
    else
        sprintf(buf, "%s", tmp->s_name);
    string_blt(ScreenSurface, &font_small, buf, x + 23, y - 26, COLOR_HGOLD, &tmp_rect, NULL);

    sprintf(buf, "weight: %4.3f", (float)tmp->weight/1000.0f);
    string_blt(ScreenSurface, &font_small, buf, x + 170, y - 14, COLOR_HGOLD, NULL, NULL);


    if (tmp->item_qua == 255) /* this comes from server when not identified */
        string_blt(ScreenSurface, &font_small, "not identified", x + 23, y - 14, COLOR_RED, NULL, NULL);
    else
    {
        string_blt(ScreenSurface, &font_small, "con: ", x + 23, y - 14, COLOR_HGOLD, NULL, NULL);
        sprintf(buf, "%d", tmp->item_qua);
        string_blt(ScreenSurface, &font_small, buf, x + 57 - string_width(&font_small, buf), y - 14,
                  COLOR_HGOLD, NULL, NULL);
        sprintf(buf, "(%d)", tmp->item_con);
        string_blt(ScreenSurface, &font_small, buf, x + 60, y - 14, COLOR_HGOLD, NULL, NULL);

        if (tmp->item_level)
        {
            sprintf(buf, "allowed: lvl %d %s", tmp->item_level, skill_level_name[tmp->item_skill]);
            if ((!tmp->item_skill && tmp->item_level <= cpl.stats.level)
                    || (tmp->item_skill && tmp->item_level <= cpl.stats.skill_level[tmp->item_skill - 1]))
                string_blt(ScreenSurface, &font_small, buf, x + 100 , y - 14, COLOR_HGOLD, NULL, NULL);
            else
                string_blt(ScreenSurface, &font_small, buf, x + 100, y - 14, COLOR_RED, NULL, NULL);
        }
        else
            string_blt(ScreenSurface, &font_small, "allowed: all", x + 100, y - 14, COLOR_HGOLD, NULL, NULL);
    }
}


void widget_inventory_event(int x, int y, SDL_Event event)
{
    int mx=0, my=0;
    mx = x - cur_widget[MAIN_INV_ID].x1;
    my = y - cur_widget[MAIN_INV_ID].y1;

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
    register int i;
    int     invxlen, invylen;
    item   *op, *tmp, *tmpx = NULL;
    item   *tmpc;
    char            buf[256];




    if (cpl.inventory_win != IWIN_INV)
    {
        if (!options.playerdoll)
            cur_widget[PDOLL_ID].show = 0;
        cur_widget[MAIN_INV_ID].ht = 32;
        sprite_blt(Bitmaps[BITMAP_INV_BG], x, y, NULL, NULL);
        string_blt(ScreenSurface, &font_small, "Carry", x+140, y+4, COLOR_HGOLD, NULL, NULL);
        sprintf(buf, "%4.3f kg", (float)cpl.real_weight/1000.0f);
        string_blt(ScreenSurface, &font_small, buf, x+140 + 35, y+4, COLOR_DEFAULT, NULL, NULL);
        string_blt(ScreenSurface, &font_small, "Limit", x+140, y+15, COLOR_HGOLD, NULL, NULL);
        sprintf(buf, "%4.3f kg", (float) cpl.weight_limit / 1000.0);
        string_blt(ScreenSurface, &font_small, buf, x+140 + 35, y+15, COLOR_DEFAULT, NULL, NULL);
        string_blt(ScreenSurface, &font_tiny_out, "(SHIFT for inventory)", x+32, y+ 9, COLOR_DEFAULT, NULL, NULL);
        return;
    }

    if (!options.playerdoll)
       cur_widget[PDOLL_ID].show = 1;
    cur_widget[MAIN_INV_ID].ht = 129;


    invxlen = INVITEMXLEN;
    invylen = INVITEMYLEN;


    sprite_blt(Bitmaps[BITMAP_INVENTORY], x, y, NULL, NULL);

    y += 26+3;
    x += 3;
    blt_window_slider(Bitmaps[BITMAP_INV_SCROLL], ((cpl.win_inv_count - 1) / invxlen) + 1, invylen,
                      cpl.win_inv_start / invxlen, -1, x + 226, y + 11);

    if (!cpl.ob)
        return;
    op = cpl.ob;
    for (tmpc = NULL,i = 0,tmp = op->inv; tmp && i < cpl.win_inv_start; tmp = tmp->next)
    {
        i++;
        if (cpl.container && cpl.container->tag == tmp->tag)
        {
            tmpx = tmp;
            tmpc = cpl.sack->inv;
            for (; tmpc && i < cpl.win_inv_start; tmpc = tmpc->next,i++)
                ;
            if (tmpc)
                break;
        }
    }

    i = 0;
    if (tmpc)
    {
        tmp = tmpx;
        goto jump_in_container1;
    }

    for (; tmp && i < invxlen*invylen; tmp = tmp->next)
    {
        if ((int) tmp->tag == cpl.mark_count)
            sprite_blt(Bitmaps[BITMAP_INVSLOT_MARKED], x + (i % invxlen) * 32, y + (i / invxlen) * 32, NULL, NULL);
        blt_inv_item(tmp, x + (i % invxlen) * 32 + 1, y + (i / invxlen) * 32 + 1);

        if (cpl.inventory_win != IWIN_BELOW && i + cpl.win_inv_start == cpl.win_inv_slot)
        {
            sprite_blt(Bitmaps[BITMAP_INVSLOT], x + (i % invxlen) * 32, y + (i / invxlen) * 32, NULL, NULL);
            show_inventory_item_stats(tmp, x, y);
        }
        i++;

        /* we have a open container - 'insert' the items inside in the panel */
        if (cpl.container && cpl.container->tag == tmp->tag)
        {
            sprite_blt(Bitmaps[BITMAP_CMARK_START], x + ((i - 1) % invxlen) * 32 + 1, y + ((i - 1) / invxlen) * 32 + 1,
                       NULL, NULL);
            tmpc = cpl.sack->inv;
jump_in_container1:
            for (; tmpc && i < invxlen*invylen; tmpc = tmpc->next)
            {
                if ((int)tmpc->tag == cpl.mark_count)
                    sprite_blt(Bitmaps[BITMAP_INVSLOT_MARKED], x + (i % invxlen) * 32, y + (i / invxlen) * 32, NULL, NULL);

                blt_inv_item(tmpc, x + (i % invxlen) * 32 + 1, y + (i / invxlen) * 32 + 1);
                if (cpl.inventory_win != IWIN_BELOW && i + cpl.win_inv_start == cpl.win_inv_slot)
                {
                    sprite_blt(Bitmaps[BITMAP_INVSLOT], x + (i % invxlen) * 32, y + (i / invxlen) * 32, NULL, NULL);

                    show_inventory_item_stats(tmpc, x, y);
                }
                sprite_blt(Bitmaps[BITMAP_CMARK_MIDDLE], x + (i % invxlen) * 32 + 1, y + (i / invxlen) * 32 + 1, NULL,
                           NULL);
                i++;
            }
            if (!tmpc)
                sprite_blt(Bitmaps[BITMAP_CMARK_END], x + ((i - 1) % invxlen) * 32 + 1,
                           y + ((i - 1) / invxlen) * 32 + 1, NULL, NULL);
        }
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
            if (y >= cur_widget[BELOW_INV_ID].y1+19 &&
                y <= cur_widget[BELOW_INV_ID].y1 + cur_widget[BELOW_INV_ID].ht - 4 &&
                x > cur_widget[BELOW_INV_ID].x1+4 &&
                x < cur_widget[BELOW_INV_ID].x1 + cur_widget[BELOW_INV_ID].wd - 12)
            {
//                if (cpl.inventory_win == IWIN_INV)
//                    cpl.inventory_win = IWIN_BELOW;
                cpl.win_below_slot = (x-cur_widget[BELOW_INV_ID].x1 - 5) / 32;
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
            else if (y >= cur_widget[BELOW_INV_ID].y1+20 &&
                    y <= cur_widget[BELOW_INV_ID].y1+29 &&
                    x > cur_widget[BELOW_INV_ID].x1+262 &&
                    x < cur_widget[BELOW_INV_ID].x1+269 &&
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
            else if (y >= cur_widget[BELOW_INV_ID].y1+42 &&
                    y <= cur_widget[BELOW_INV_ID].y1+51 &&
                    x > cur_widget[BELOW_INV_ID].x1+262 &&
                    x < cur_widget[BELOW_INV_ID].x1+269 &&
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
    register int i, slot,at;
    item       *tmp, *tmpc, *tmpx = NULL;
    char        buf[256];
    SDL_Rect    tmp_rect;
    tmp_rect.w = 265;

    sprite_blt(Bitmaps[BITMAP_BELOW], x, y, NULL, NULL);


    blt_window_slider(Bitmaps[BITMAP_BELOW_SCROLL], ((cpl.win_below_count - 1) / INVITEMBELOWXLEN) + 1,
                      INVITEMBELOWYLEN, cpl.win_below_start / INVITEMBELOWXLEN, -1, x + 263, y + 30);

    if (!op)
        return;
    for (i = 0,tmpc = NULL,tmp = op->inv; tmp && i < cpl.win_below_start; tmp = tmp->next)
    {
        i++;
        tmpx = tmp;
        if (cpl.container && cpl.container->tag == tmp->tag)
        {
            tmpc = cpl.sack->inv;
            for (; tmpc && i < cpl.win_below_start; tmpc = tmpc->next,i++)
                ;
            if (tmpc)
                break;
        }
    }

    i = 0;
    if (tmpc)
    {
        tmp = tmpx;
        goto jump_in_container2;
    }

    for (; tmp && i < INVITEMBELOWXLEN*INVITEMBELOWYLEN; tmp = tmp->next)
    {
        at = tmp->applied;
        if ((int)tmp->tag != cpl.container_tag)
            tmp->applied = 0;
        blt_inv_item(tmp, x + (i % INVITEMBELOWXLEN) * 32 + 5, y + (i / INVITEMBELOWXLEN) * 32 + 19);
        tmp->applied = at;
        if (i + cpl.win_below_start == cpl.win_below_slot)
        {
            if (cpl.inventory_win == IWIN_BELOW)
                slot = BITMAP_INVSLOT;
            else
                slot = BITMAP_INVSLOT_U;
            sprite_blt(Bitmaps[slot], x + (i % INVITEMBELOWXLEN) * 32 + 5, y + (i / INVITEMBELOWXLEN) * 32 + 19, NULL,
                       NULL);

            if (tmp->nrof > 1)
                sprintf(buf, "%d %s", tmp->nrof, tmp->s_name);
            else
                sprintf(buf, "%s", tmp->s_name);
            string_blt(ScreenSurface, &font_small, buf, x + 6, y + 5, COLOR_HGOLD, &tmp_rect, NULL);
        }
        i++;
        /* we have a open container - 'insert' the items inside in the panel */
        if (cpl.container && cpl.container->tag == tmp->tag)
        {
            sprite_blt(Bitmaps[BITMAP_CMARK_START], x + ((i - 1) % INVITEMBELOWXLEN) * 32 + 5,
                       y + ((i - 1) / INVITEMBELOWXLEN) * 32 + 19, NULL, NULL);
            tmpc = cpl.sack->inv;
jump_in_container2:
            for (; tmpc && i < INVITEMBELOWXLEN*INVITEMBELOWYLEN; tmpc = tmpc->next)
            {
                blt_inv_item(tmpc, x + (i % INVITEMBELOWXLEN) * 32 + 5, y + (i / INVITEMBELOWXLEN) * 32 + 19);
                if (i + cpl.win_below_start == cpl.win_below_slot)
                {
                    if (cpl.inventory_win == IWIN_BELOW)
                        slot = BITMAP_INVSLOT;
                    else
                        slot = BITMAP_INVSLOT_U;
                    sprite_blt(Bitmaps[slot], x + (i % INVITEMBELOWXLEN) * 32 + 5, y + (i / INVITEMBELOWXLEN) * 32 + 19,
                               NULL, NULL);
                    if (tmpc->nrof > 1)
                        sprintf(buf, "%d %s", tmpc->nrof, tmpc->s_name);
                    else
                        sprintf(buf, "%s", tmpc->s_name);
                    string_blt(ScreenSurface, &font_small, buf, x + 6, y + 4, COLOR_HGOLD, &tmp_rect, NULL);
                }
                sprite_blt(Bitmaps[BITMAP_CMARK_MIDDLE], x + (i % INVITEMBELOWXLEN) * 32 + 5,
                           y + (i / INVITEMBELOWXLEN) * 32 + 19, NULL, NULL);
                i++;
            }
            if (!tmpc)
                sprite_blt(Bitmaps[BITMAP_CMARK_END], x + ((i - 1) % INVITEMBELOWXLEN) * 32 + 5,
                           y + ((i - 1) / INVITEMBELOWXLEN) * 32 + 19, NULL, NULL);
        }
    }
    /*   sprite_blt(Bitmaps[BITMAP_MIDDLE_OVERLAY],242+143, 472+78, NULL, NULL); */
}

uint8 blt_inv_item_centered(item *tmp, int x, int y)
{
    register int temp;
    int         xstart, xlen, ystart, ylen;
    int         itemdmg_percent;
    sint16      anim1;
    SDL_Rect    box;
    _BLTFX      bltfx;
    bltfx.flags = 0;
    bltfx.dark_level = 0;
    bltfx.surface = NULL;
    bltfx.alpha = 128;
    /*
       bltfx.flags=1;
       bltfx.dark_level=0;*/

    if (!FaceList[tmp->face].sprite)
        return 0;

    if (FaceList[tmp->face].sprite->status != SPRITE_STATUS_LOADED)
        return 0;

    anim1 = tmp->face;

    /* this is part of a animation... Because iso items have different offsets and sizes,
     * we must use ONE sprite base offset to center a animation over a animation.
     * we use the first frame of a animation for it.*/


/* TODO: map it to new animtable */
    if (tmp->anim && (tmp->anim->animnum > 0))
    {
        if (new_anim_load_and_check(tmp->anim->animnum, tmp->anim->sequence, tmp->anim->dir))
            anim1 = animation[tmp->anim->animnum].aSeq[tmp->anim->sequence]->dirs[tmp->anim->dir].faces[0];
    }

    if (!FaceList[anim1].sprite) /* fallback: first ani bitmap not loaded */
        anim1 = tmp->face;

    /* also fallback here */
    if (FaceList[anim1].sprite->status != SPRITE_STATUS_LOADED)
        anim1 = tmp->face;
    xstart = FaceList[anim1].sprite->border_left;
    xlen = FaceList[anim1].sprite->bitmap->w - xstart - FaceList[anim1].sprite->border_right;
    ystart = FaceList[anim1].sprite->border_up;
    ylen = FaceList[anim1].sprite->bitmap->h - ystart - FaceList[anim1].sprite->border_down;

    if (xlen > 32)
    {
        box.w = 32;
        temp = (xlen - 32) / 2;
        box.x = xstart + temp;
        xstart = 0;
    }
    else
    {
        box.w = xlen;
        box.x = xstart;
        xstart = (32 - xlen) / 2;
    }

    if (ylen > 32)
    {
        box.h = 32;
        temp = (ylen - 32) / 2;
        box.y = ystart + temp;
        ystart = 0;
    }
    else
    {
        box.h = ylen;
        box.y = ystart;
        ystart = (32 - ylen) / 2;
    }

    /* now we have a perfect centered sprite.
     * but: if this is the start pos of our
     * first ani and not of our sprite, we
     * must shift it a bit to insert our
     * face exactly
     */

    if (anim1 != tmp->face)
    {
        temp = xstart - box.x;

        box.x = 0;
        box.w = FaceList[tmp->face].sprite->bitmap->w;
        xstart = temp;

        temp = ystart - box.y + (FaceList[anim1].sprite->bitmap->h - FaceList[tmp->face].sprite->bitmap->h);
        box.y = 0;
        box.h = FaceList[tmp->face].sprite->bitmap->h;
        ystart = temp;

        if (xstart < 0)
        {
            box.x = -xstart;
            box.w = FaceList[tmp->face].sprite->bitmap->w + xstart;
            if (box.w > 32)
                box.w = 32;
            xstart = 0;
        }
        else
        {
            if (box.w + xstart > 32)
                box.w -= ((box.w + xstart) - 32);
        }
        if (ystart < 0)
        {
            box.y = -ystart;
            box.h = FaceList[tmp->face].sprite->bitmap->h + ystart;
            if (box.h > 32)
                box.h = 32;
            ystart = 0;
        }
        else
        {
            if (box.h + ystart > 32)
                box.h -= ((box.h + ystart) - 32);
        }
    }

    if (tmp->flagsval & F_INVISIBLE)
        bltfx.flags = BLTFX_FLAG_SRCALPHA | BLTFX_FLAG_GREY;
    if (tmp->flagsval & F_ETHEREAL)
        bltfx.flags = BLTFX_FLAG_SRCALPHA;

    sprite_blt(FaceList[tmp->face].sprite, x + xstart, y + ystart, &box, &bltfx);

    if (options.showqc)
    {
        box.w=2;
        box.h=(int) (((double) tmp->item_con / (double) tmp->item_qua)*30);
        if (box.h>30)
            box.h = 30;
        box.x=x + 30;
        box.y=y+ (31 - box.h);


         if (tmp->item_con < tmp->item_qua)
        {
            itemdmg_percent = (int) (((double) tmp->item_con / (double) tmp->item_qua)*100);
            if (itemdmg_percent >= options.itemdmg_limit_orange)
                SDL_FillRect(ScreenSurface,&box,SDL_MapRGB(ScreenSurface->format,255,255,0));
            else if (itemdmg_percent >= options.itemdmg_limit_red)
                SDL_FillRect(ScreenSurface,&box,SDL_MapRGB(ScreenSurface->format,255,109,0));
            else
                SDL_FillRect(ScreenSurface,&box,SDL_MapRGB(ScreenSurface->format,255,0,0));
        }
     }

    return 1;
}

void blt_inv_item(item *tmp, int x, int y)
{
    blt_inv_item_centered(tmp, x, y);

    if (tmp->nrof > 1)
    {
        char buf[TINY_BUF];

        if (tmp->nrof > 9999)
        {
            sprintf(buf, "many");
        }
        else
        {
            sprintf(buf, "%d", tmp->nrof);
        }

        string_blt(ScreenSurface, &font_tiny_out, buf, x +
                   skindef.item_size / 2 -
                   string_width(&font_tiny_out, buf) / 2, y + 18, COLOR_WHITE,
                   NULL, NULL);
    }

    /* bottom left */
    if (tmp->locked)
    {
        sprite_blt(Bitmaps[BITMAP_LOCK], x, y + skindef.item_size -
                   skindef.icon_size, NULL, NULL);
    }

    /* top left */
    /* applied and unpaid some spot - can't apply unpaid items */
    if (tmp->applied)
    {
        sprite_blt(Bitmaps[BITMAP_APPLY], x, y, NULL, NULL);
    }
    else if (tmp->unpaid)
    {
        sprite_blt(Bitmaps[BITMAP_UNPAID], x, y, NULL, NULL);
    }

    /* right side, top to bottom */
    if (tmp->item_qua == 255)
    {
        sprite_blt(Bitmaps[BITMAP_UNIDENTIFIED],
                   x + skindef.item_size - skindef.icon_size - 2, y, NULL, NULL);
    }

    if (tmp->magical)
    {
        sprite_blt(Bitmaps[BITMAP_MAGIC],
                   x + skindef.item_size - skindef.icon_size - 2,
                   y + skindef.icon_size, NULL, NULL);
    }

    if (tmp->cursed)
    {
        sprite_blt(Bitmaps[BITMAP_CURSED],
                   x + skindef.item_size - skindef.icon_size - 2,
                   y + skindef.icon_size * 2, NULL, NULL);
    }

    if (tmp->damned)
    {
        sprite_blt(Bitmaps[BITMAP_DAMNED],
                   x + skindef.item_size - skindef.icon_size - 2,
                   y + skindef.icon_size * 3, NULL, NULL);
    }

    /* central */
    if (tmp->traped)
    {
        sprite_blt(Bitmaps[BITMAP_TRAPED], x + 8, y + 7, NULL, NULL);
    }
}

void examine_range_inv(void)
{
    register item * op, * tmp;

    op = cpl.ob;
    if (!op->inv)
        return;
    fire_mode_tab[FIRE_MODE_BOW].item = FIRE_ITEM_NO;
    fire_mode_tab[FIRE_MODE_BOW].amun = FIRE_ITEM_NO;

    for (tmp = op->inv; tmp; tmp = tmp->next)
    {
        if (tmp->applied && (tmp->itype == TYPE_BOW || (tmp->itype == TYPE_ARROW && tmp->stype >= 128)
            || tmp->itype == TYPE_WAND || tmp->itype == TYPE_ROD || tmp->itype == TYPE_HORN))
            fire_mode_tab[FIRE_MODE_BOW].item = tmp->tag;
        else if(tmp->applied && tmp->itype == TYPE_ARROW && tmp->stype < 128)
            fire_mode_tab[FIRE_MODE_BOW].amun = tmp->tag;
    }
}
