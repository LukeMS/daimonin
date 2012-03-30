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
#include <include.h>

static void PrintResist(char *name, int x, int y, int num);

_media_file             media_file[MEDIA_MAX];

/* keybind menu */
int                     keybind_status;

struct _spell_list      spell_list[SPELL_LIST_MAX]; /* skill list entries */
struct _skill_list      skill_list[SKILL_LIST_MAX]; /* skill list entries */

struct _dialog_list_set spell_list_set;
struct _dialog_list_set skill_list_set;
struct _dialog_list_set option_list_set;
struct _dialog_list_set bindkey_list_set;
struct _dialog_list_set create_list_set;

int                     media_count;        /* buffered media files*/
int                     media_show; /* show this media file*/
int                     media_show_update;
int                     keybind_startoff    = 0;

_quickslot              quick_slots[MAX_QUICK_SLOTS];
int                     quickslots_pos[MAX_QUICK_SLOTS][2]  =
    {
        {17,1}, {50,1}, {83,1}, {116,1}, {149,1}, {182,1}, {215,1}, {248,1}
    };

void do_console(int x, int y)
{
    if (InputStringEscFlag == 1)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CONSOLE, 0, 0, 100);
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        WIDGET_SHOW(WIDGET_IN_CONSOLE_ID) = 0;
        map_udate_flag = 2;
    }
    /* if set, we got a finished input!*/
    if (InputStringFlag == 0 && InputStringEndFlag == 1)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CONSOLE, 0, 0, 100);
        if (InputString[0])
        {
//            textwin_show_string(0, NDI_COLR_OLIVE, ":%s", InputString);
            client_cmd_generic(InputString);
        }

        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
        WIDGET_SHOW(WIDGET_IN_CONSOLE_ID) = 0;
    }
    else
        WIDGET_SHOW(WIDGET_IN_CONSOLE_ID) = 1;
}

void widget_show_console(int x, int y)
{
    SDL_Rect box;

    sprite_blt(skin_sprites[SKIN_SPRITE_TEXTINPUT],x, y, NULL, NULL);
    box.x = x + 8;
    box.y = y + 6;
    box.w = skin_sprites[SKIN_SPRITE_TEXTINPUT]->bitmap->w - 22;
    box.h = font_small.line_height;
    strout_input(&font_small, &box, 0);
}

void do_number(int x, int y)
{
    if (InputStringEscFlag == 1)
    {
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        WIDGET_SHOW(WIDGET_IN_NUMBER_ID) = 0;
        map_udate_flag = 2;
    }
    /* if set, we got a finished input!*/
    if (InputStringFlag == 0 && InputStringEndFlag == 1)
    {
        if (InputString[0])
        {
            int     tmp;

            tmp = atoi(InputString);
            if (tmp > 0)
            {
                client_cmd_invmove(cpl.loc, cpl.tag, tmp);

                if (cpl.nummode == NUM_MODE_GET)
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100);
                else
                    sound_play_effect(SOUNDTYPE_NORMAL, SOUND_DROP, 0, 0, 100);

                textwin_show_string(0, skin_prefs.widget_info, "%s %d from %d %s",
                                   (cpl.nummode == NUM_MODE_GET) ? "get" :
                                   "drop", tmp, cpl.nrof, cpl.num_text);
            }
        }
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
        WIDGET_SHOW(WIDGET_IN_NUMBER_ID) = 0;
    }
    else
        WIDGET_SHOW(WIDGET_IN_NUMBER_ID) = 1;
}

void do_keybind_input(void)
{
    if (InputStringEscFlag == 1)
    {
        reset_keys();
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, 100);
        cpl.input_mode = INPUT_MODE_NO;
        keybind_status = KEYBIND_STATUS_NO;
        map_udate_flag = 2;
    }
    /* if set, we got a finished input!*/
    if (InputStringFlag == 0 && InputStringEndFlag == 1)
    {
        if (InputString[0])
        {
            strcpy(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text, InputString);
            keybind_status = KEYBIND_STATUS_EDITKEY; /* now get the key code */
        }
        else
        {
            /* cleared string - delete entry */
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text[0] = 0;
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].keyname[0] = 0;
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key = 0;
            keybind_status = KEYBIND_STATUS_NO;
        }
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
    }
}

void do_npcdialog_input(void)
{
    if (InputStringEscFlag == 1)
    {
        reset_keys();
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, 100);
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
        gui_npc->input_flag = 0;
    }

    /* if set, we got a finished input!*/
    if (InputStringFlag == 0 && InputStringEndFlag == 1)
    {
        if (InputString[0])
        {
            client_cmd_guitalk(GUI_NPC_MODE_NPC, InputString);
            textwin_add_history(InputString);
            reset_input_mode();
        }

        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        gui_npc->input_flag = 0;
        map_udate_flag = 2;
    }
}

void widget_number_event(int x, int y, SDL_Event event)
{
    int mx=0, my=0;
    mx = x - widget_data[WIDGET_IN_NUMBER_ID].x1;
    my = y - widget_data[WIDGET_IN_NUMBER_ID].y1;

    /* close number input */
    if (InputStringFlag && cpl.input_mode == INPUT_MODE_NUMBER)
    {
        if (mx > 239 && mx < 249 && my > 5 && my < 17)
        {
            SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
            InputStringFlag = 0;
            InputStringEndFlag = 1;
        }
    }
}

void widget_show_number(int x, int y)
{
    SDL_Rect box;
    char     buf[TINY_BUF];

    box.x = x + 8;
    box.y = y + 6;
    box.w = skin_sprites[SKIN_SPRITE_NUMBER]->bitmap->w - 22;
    box.h = font_small.line_height;
    sprite_blt(skin_sprites[SKIN_SPRITE_NUMBER], x, y, NULL, NULL);
    sprintf(buf, "%s how many from %d %s", cpl.nummode == NUM_MODE_GET ? "get" : "drop", cpl.nrof, cpl.num_text);
    strout_blt(ScreenSurface, &font_small, buf, x + 8, y + 6, skin_prefs.widget_title, NULL, NULL);
    box.y = y + 25;
    strout_input(&font_small, &box, 0);
}

void widget_show_resist(int x, int y)
{
    _BLTFX bltfx;
    SDL_Rect box;

    if (!widget_surface[WIDGET_RESIST_ID])
        widget_surface[WIDGET_RESIST_ID]=SDL_ConvertSurface(skin_sprites[SKIN_SPRITE_RESIST_BG]->bitmap,skin_sprites[SKIN_SPRITE_RESIST_BG]->bitmap->format,skin_sprites[SKIN_SPRITE_RESIST_BG]->bitmap->flags);

    if (widget_data[WIDGET_RESIST_ID].redraw)
    {
        widget_data[WIDGET_RESIST_ID].redraw=0;

        bltfx.surface=widget_surface[WIDGET_RESIST_ID];
        bltfx.flags = 0;
        bltfx.alpha=0;

        sprite_blt(skin_sprites[SKIN_SPRITE_RESIST_BG], 0, 0, NULL, &bltfx);
        strout_blt(widget_surface[WIDGET_RESIST_ID], &font_tiny, "Resistance Table", 4,  1, skin_prefs.widget_title, NULL, NULL);
        PrintResist("IM", 68, 3, ATNR_PHYSICAL);
        PrintResist("SL", 100, 3, ATNR_SLASH);
        PrintResist("CL", 132, 3, ATNR_CLEAVE);
        PrintResist("PI", 164, 3, ATNR_PIERCE);
        PrintResist("FI", 4, 15, ATNR_FIRE);
        PrintResist("CO", 36, 15, ATNR_COLD);
        PrintResist("EL", 68, 15, ATNR_ELECTRICITY);
        PrintResist("PO", 100, 15, ATNR_POISON);
        PrintResist("AC", 132, 15, ATNR_ACID);
        PrintResist("SO", 164, 15, ATNR_SONIC);
        PrintResist("CH", 4, 27, ATNR_CHANNELLING);
        PrintResist("CR", 36, 27, ATNR_CORRUPTION);
        PrintResist("PS", 68, 27, ATNR_PSIONIC);
        PrintResist("LI", 100, 27, ATNR_LIGHT);
        PrintResist("SH", 132, 27, ATNR_SHADOW);
        PrintResist("LS", 164, 27, ATNR_LIFESTEAL);
        PrintResist("AE", 4, 39, ATNR_AETHER);
        PrintResist("NE", 36, 39, ATNR_NETHER);
        PrintResist("CH", 68, 39, ATNR_CHAOS);
        PrintResist("DE", 100, 39, ATNR_DEATH);
        PrintResist("WE", 132, 39, ATNR_WEAPONMAGIC);
        PrintResist("GO", 164, 39, ATNR_GODPOWER);
        PrintResist("DR", 4, 51, ATNR_DRAIN);
        PrintResist("DE", 36, 51, ATNR_DEPLETION);
        PrintResist("CM", 68, 51, ATNR_COUNTERMAGIC);
        PrintResist("CA", 100, 51, ATNR_CANCELLATION);
        PrintResist("CF", 132, 51, ATNR_CONFUSION);
        PrintResist("FE", 164, 51, ATNR_FEAR);
        PrintResist("SL", 4, 63, ATNR_SLOW);
        PrintResist("PA", 36, 63, ATNR_PARALYZE);
        PrintResist("SN", 68, 63, ATNR_SNARE);
    }
    box.x=x;
    box.y=y;
    SDL_BlitSurface(widget_surface[WIDGET_RESIST_ID], NULL, ScreenSurface, &box);
}

static void PrintResist(char *name, int x, int y, int num)
{
    char   buf[16];
    sint8  resist = cpl.stats.protection[num];
    uint32 colr = (!resist)
                  ? NDI_COLR_GREY
                  : percentage_colr(((resist + 100) * 0.005 * 100));

    strout_blt(widget_surface[WIDGET_RESIST_ID], &font_small, name, x, y,
               skin_prefs.widget_key, NULL, NULL);
    sprintf(buf, "%02d", resist);
    strout_blt(widget_surface[WIDGET_RESIST_ID], &font_small, buf, x + 12, y,
               colr, NULL, NULL);
}

#define ICONDEFLEN 32
uint8 blt_face_centered(int face, int x, int y)
{
    register int temp;
    SDL_Rect    box;

    if (!face_list[face].sprite)
        return 0;

    if (face_list[face].sprite->status != SPRITE_STATUS_LOADED)
        return 0;

    box.x = face_list[face].sprite->border_left;
    box.w = face_list[face].sprite->bitmap->w;
    temp = box.w - face_list[face].sprite->border_left - face_list[face].sprite->border_right;
    if (temp > 32)
    {
        box.w = 32;
        temp -= 32;
        temp >>= 1;
        box.x += temp;
    }
    else if (temp < 32)
    {
        temp = 32 - temp;
        box.x -= (temp >> 1);
    }

    box.y = -face_list[face].sprite->border_up;
    box.h = face_list[face].sprite->bitmap->h;
    temp = box.h - face_list[face].sprite->border_up - face_list[face].sprite->border_down;
    if (temp > 32)
    {
        box.h = 32;
        temp -= 32;
        temp >>= 1;
        box.y += temp;
    }
    else if (temp < 32)
    {
        temp = 32 - temp;
        box.y = -(temp >> 1) + face_list[face].sprite->border_up;
    }
    sprite_blt(face_list[face].sprite, x, y, &box, NULL);

    return 1;
}

void widget_range_event(int x, int y, SDL_Event event, uint8 e)
{
    if (e == SDL_MOUSEBUTTONDOWN)
    {
        uint8 button = event.button.button;

        if (button == SDL_BUTTON_LEFT)
        {
            process_macro_keys(KEYFUNC_RANGE, 0);
        }
        else if (button == SDL_BUTTON_WHEELUP)
        {
            process_macro_keys(KEYFUNC_RANGE, 0);
        }
        else if (button == SDL_BUTTON_WHEELDOWN)
        {
            process_macro_keys(KEYFUNC_RANGE_BACK, 0);
        }
        else
        {
            process_macro_keys(KEYFUNC_RANGE_BACK, 0);
        }
    }
    else if (e == SDL_MOUSEBUTTONUP)
    {
        sint8  drag = draggingInvItem(DRAG_GET_STATUS);
        int    tag = (drag == DRAG_IWIN_INV)
                     ? cpl.win_inv_tag : cpl.win_quick_tag;
        item  *ip;

        switch (drag)
        {
            case DRAG_IWIN_INV:
            case DRAG_QUICKSLOT:
                if ((ip = (tag == -1) ? NULL : locate_item(tag)) &&
                    (ip->itype == TYPE_ARROW ||
                     ip->itype == TYPE_BOW ||
                     ip->itype == TYPE_WAND ||
                     ip->itype == TYPE_ROD ||
                     ip->itype == TYPE_HORN) &&
                    !ip->applied)
                {
                    client_cmd_apply(tag);
                }

                break;

            case DRAG_QUICKSLOT_SPELL:
                fire_mode.spell = &spell_list[quick_slots[tag].spell.groupNr].entry[quick_slots[tag].spell.classNr][quick_slots[tag].spell.spellNr];
                fire_mode.mode = FIRE_MODE_SPELL_ID;

                break;
        }
    }
}

void widget_show_range(int x, int y)
{
    char        buf[256];
    SDL_Rect    rec_range;
    SDL_Rect    rec_item;
    item       *tmp, *tmp2;

    rec_range.w = 160;
    rec_range.h = font_small.line_height;
    rec_item.w = 185;
    rec_item.h = font_small.line_height;
    examine_range_inv();

    sprite_blt(skin_sprites[SKIN_SPRITE_RANGE], x, y, NULL, NULL);

    switch (fire_mode.mode)
    {
        case FIRE_MODE_ARCHERY_ID:
            if (fire_mode.weapon != FIRE_ITEM_NO)
            {
                tmp2=locate_item(fire_mode.weapon);
                if (!tmp2)
                {
                    LOG(LOG_DEBUG,"BUG: applied range weapon don't exist\n");
                    strout_blt(ScreenSurface, &font_small, "using Nothing", x + 5, y + 36, skin_prefs.widget_info, &rec_range, NULL);
                }
                else
                {
                    item  *ip;
                    uint8  quacon;

                    sprintf(buf, "using %s", tmp2->s_name);

                    if ((ip = locate_item(fire_mode.weapon)))
                    {
                        quacon = (ip->item_qua == 255)
                                 ? 255 : (float)ip->item_con / (float)ip->item_qua * 100;
                        sprite_blt_as_icon(face_list[ip->face].sprite, x + 5, y + 2,
                                           SPRITE_ICON_TYPE_ACTIVE, 0,
                                           (ip->flagsval & ~(F_LOCKED | F_APPLIED)),
                                           (quacon == 100) ? 0 : quacon,
                                           (ip->nrof == 1) ? 0 : ip->nrof, NULL);
                    }

                    strout_blt(ScreenSurface, &font_small, buf, x + 5, y + 36, skin_prefs.widget_valueEq, &rec_range, NULL);
                }

                if (fire_mode.ammo != FIRE_ITEM_NO)
                {
                    item  *ip;
                    uint8  quacon;

                    if ((tmp = locate_item_from_item(cpl.ob, fire_mode.ammo)))
                    {
                        if (tmp->itype == TYPE_ARROW)
                            sprintf(buf, "ammo %s (%d)", tmp->s_name, tmp->nrof);
                        else
                            sprintf(buf, "ammo %s", tmp->s_name);
                    }
                    else
                        strcpy(buf, "ammo not selected");

                    if ((ip = locate_item(fire_mode.ammo)))
                    {
                        quacon = (ip->item_qua == 255)
                                 ? 255 : (float)ip->item_con / (float)ip->item_qua * 100;
                        sprite_blt_as_icon(face_list[ip->face].sprite, x + 45, y + 2,
                                           SPRITE_ICON_TYPE_ACTIVE, 0,
                                           (ip->flagsval & ~(F_LOCKED | F_APPLIED)),
                                           (quacon == 100) ? 0 : quacon,
                                           (ip->nrof == 1) ? 0 : ip->nrof, NULL);
                    }
                }
                else if (tmp2->itype==TYPE_BOW)
                {
                    sprintf(buf, "ammo not selected");
                }
                else if (tmp2->itype==TYPE_ARROW)
                {
                    sprintf(buf, "amount: %d",tmp2->nrof);
                }
                else
//                    sprintf(buf, "Type: %d",tmp2->itype);
                    buf[0]=0;

                strout_blt(ScreenSurface, &font_small, buf, x + 5, y + 47, skin_prefs.widget_valueEq, &rec_item, NULL);
            }
            else
            {
                sprintf(buf, "no range weapon applied");
                strout_blt(ScreenSurface, &font_small, buf, x + 5, y + 36, skin_prefs.widget_info, &rec_range, NULL);
            }

            sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_MARKER], x + 5, y + 2, NULL, NULL);
            break;

        case FIRE_MODE_SPELL_ID:
            if (fire_mode.spell)
            {
                /* we use wiz spells as default */
                sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_WIZARD], x + 5, y + 2, NULL, NULL);
                if (fire_mode.spell->flag != -1)
                {
                    sprite_blt(fire_mode.spell->icon, x + 45, y + 2, NULL, NULL);
                    strout_blt(ScreenSurface, &font_small, fire_mode.spell->name, x + 5, y + 47,
                              skin_prefs.widget_valueEq, &rec_item, NULL);
                }
                else
                    fire_mode.spell = NULL;
            }
            else
            {
                sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_WIZARD_NO], x + 5, y + 2, NULL, NULL);
                sprintf(buf, "no spell selected");
                strout_blt(ScreenSurface, &font_small, buf, x + 5, y + 47, skin_prefs.widget_info, &rec_item, NULL);
            }
            sprintf(buf, "cast spell");
            strout_blt(ScreenSurface, &font_small, buf, x + 5, y + 36, skin_prefs.widget_info, &rec_range, NULL);

            break;

        case FIRE_MODE_SKILL_ID:
            if (fire_mode.skill)
            {
                sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_SKILL], x + 5, y + 2, NULL, NULL);
                if (fire_mode.skill->flag != -1)
                {
                    sprite_blt(fire_mode.skill->icon, x + 45, y + 2, NULL, NULL);
                    strout_blt(ScreenSurface, &font_small, fire_mode.skill->name, x + 5, y + 47,
                              skin_prefs.widget_valueEq, &rec_item, NULL);
                }
                else
                    fire_mode.skill = NULL;
            }
            else
            {
                sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_SKILL_NO], x + 5, y + 2, NULL, NULL);
                sprintf(buf, "no skill selected");
                strout_blt(ScreenSurface, &font_small, buf, x + 5, y + 47, skin_prefs.widget_info, &rec_item, NULL);
            }
            sprintf(buf, "use skill");
            strout_blt(ScreenSurface, &font_small, buf, x + 5, y + 36, skin_prefs.widget_info, &rec_range, NULL);

            break;

        default:
            LOG(LOG_ERROR, "Unknown fire mode %u\n", fire_mode.mode);
    }
}

void show_menu(void)
{
    SDL_Rect box;

    switch (cpl.menustatus)
    {
        case MENU_KEYBIND:
            show_keybind();

            break;

        case MENU_BOOK:
            show_book(400 - skin_sprites[SKIN_SPRITE_JOURNAL]->bitmap->w / 2,
                      300 - skin_sprites[SKIN_SPRITE_JOURNAL]->bitmap->h / 2);

            break;

        case MENU_NPC:
            if (!gui_npc)
            {
                cpl.menustatus = MENU_NO;

                return;
            }
            else
            {
                int x,
                    y;

                gui_npc_show();

                /* Force selection of element under pointer. */
                SDL_PumpEvents();
                SDL_GetMouseState(&x, &y);
                gui_npc_mousemove(x, y);
            }

            break;

       case MENU_STATUS:
            show_status();

            break;

       case MENU_SPELL:
            show_spelllist();
            box.x = Screensize.x / 2 -
                    skin_sprites[SKIN_SPRITE_DIALOG_BG]->bitmap->w / 2;
            box.y = Screensize.y / 2 -
                    skin_sprites[SKIN_SPRITE_DIALOG_BG]->bitmap->h / 2 - 42;
            box.h = 42;
            box.w = skin_sprites[SKIN_SPRITE_DIALOG_BG]->bitmap->w;
            SDL_FillRect(ScreenSurface, &box, 0);
            show_quickslots(box.x + 120, box.y + 3);

            break;

       case MENU_SKILL:
            show_skilllist();

            break;

       case MENU_OPTION:
            show_optwin();

            break;

       case MENU_CREATE:
            show_newplayer_server();

            break;

       default:
           return;
    }
}

void show_media(int x, int y)
{
    _Sprite *sprite;
    int      xtemp;

    if (media_show != MEDIA_SHOW_NO)
    {
        /* we show a png*/
        if (media_file[media_show].type == MEDIA_TYPE_PNG)
        {
            sprite = (_Sprite *)media_file[media_show].data;

            if (sprite)
            {
                xtemp = x - sprite->bitmap->w;
                sprite_blt(sprite, xtemp, y, NULL, NULL);
            }
        }
    }
}

void widget_show_mapname(int x, int y)
{
    strout_blt(ScreenSurface, &font_large, MapData.name, x, y, skin_prefs.widget_title, NULL, NULL);
}


void show_status(void)
{
    /*
            int y, x;
            x= Screensize.x/2-skin_sprites[SKIN_SPRITE_STATUS]->bitmap->w/2;
            y= Screensize.y/2-skin_sprites[SKIN_SPRITE_STATUS]->bitmap->h/2;
            sprite_blt(skin_sprites[SKIN_SPRITE_STATUS],x, y, NULL, NULL);
    */
}

int blt_window_slider(_Sprite *slider, int maxlen, int winlen, int startoff, int len, int x, int y)
{
    SDL_Rect    box;
    double      temp;
    int         startpos, len_h;

    if (len != -1)
        len_h = len;
    else
        len_h = slider->bitmap->h;

    if (maxlen < winlen)
        maxlen = winlen;
    if (startoff + winlen > maxlen)
        maxlen = startoff + winlen;

    box.x = 0;
    box.y = 0;
    box.w = slider->bitmap->w;

    /* now we have 100% = 1.0 to 0% = 0.0 of the length */
    temp = (double) winlen / (double) maxlen; /* between 0.0 <-> 1.0 */
    startpos = (int) ((double) startoff * ((double) len_h / (double) maxlen)); /* startpixel */
    temp = (double) len_h * temp;
    box.h = (Uint16) temp;
    if (!box.h)
        box.h = 1;

    if (startoff + winlen >= maxlen && startpos + box.h < len_h)
        startpos ++;

    sprite_blt(slider, x, y + startpos, &box, NULL);
    return 0;
}

int get_quickslot(int x, int y)
{
    int i;
    int qsx, qsy, xoff;
    if (widget_data[WIDGET_QUICKSLOT_ID].ht > 34)
    {
        qsx = 1;
        qsy = 0;
        xoff = 0;
    }
    else
    {
        qsx = 0;
        qsy = 1;
        xoff= -17;
    }

    for (i = 0; i < MAX_QUICK_SLOTS; i++)
    {
        if (x >= widget_data[WIDGET_QUICKSLOT_ID].x1 + quickslots_pos[i][qsx]+xoff
                && x <= widget_data[WIDGET_QUICKSLOT_ID].x1 + quickslots_pos[i][qsx]+xoff + 32
                && y >= widget_data[WIDGET_QUICKSLOT_ID].y1 + quickslots_pos[i][qsy]
                && y <= widget_data[WIDGET_QUICKSLOT_ID].y1 + quickslots_pos[i][qsy] + 32)
            return i;
    }
    return -1;
}
void show_quickslots(int x, int y)
{
    int     i, mx, my;
    char    buf[512];
    int     qsx, qsy, xoff;

        qsx = 0;
        qsy = 1;
        xoff = -17;
        sprite_blt(skin_sprites[SKIN_SPRITE_QUICKSLOTS], x, y, NULL, NULL);

    SDL_GetMouseState(&mx, &my);
    update_quickslots(-1);

    for (i = MAX_QUICK_SLOTS - 1; i >= 0; i--)
    {
        if (quick_slots[i].shared.tag != -1)
        {
            /* spell in quickslot */
            if (quick_slots[i].shared.is_spell == 1)
            {
                sprite_blt(spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].icon,
                           x + quickslots_pos[i][qsx]+xoff, y + quickslots_pos[i][qsy], NULL, NULL);
                if (mx >= x + quickslots_pos[i][qsx]+xoff
                        && mx < x + quickslots_pos[i][qsx]+xoff + 33
                        && my >= y + quickslots_pos[i][qsy]
                        && my < y + quickslots_pos[i][qsy] + 33
                        && widget_get_mouse_state(&mx,&my,WIDGET_QUICKSLOT_ID))
                    strout_tooltip(mx, my,
                                 spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].name);
            }
            /* item in quickslot */
            else
            {
                item *ip;

                if ((ip = locate_item_from_item(cpl.ob, quick_slots[i].shared.tag)))
                {
                    uint8 quacon = (ip->item_qua == 255) ? 255
                                   : (float)ip->item_con /
                                     (float)ip->item_qua * 100;

                    sprite_blt_as_icon(face_list[ip->face].sprite,
                                       x + quickslots_pos[i][qsx] + xoff,
                                       y + quickslots_pos[i][qsy],
                                       SPRITE_ICON_TYPE_ACTIVE, 0, ip->flagsval,
                                       (quacon == 100) ? 0 : quacon,
                                       (ip->nrof == 1) ? 0 : ip->nrof, NULL);

                    /* show tooltip */
                    if (mx >= x + quickslots_pos[i][qsx]+xoff
                            && mx < x + quickslots_pos[i][qsx]+xoff + 33
                            && my >= y + quickslots_pos[i][qsy]
                            && my < y + quickslots_pos[i][qsy] + 33
                            && widget_get_mouse_state(&mx,&my,WIDGET_QUICKSLOT_ID))
                    {
                        sprintf(buf,"%s (q/c: %d/%d)",ip->s_name, ip->item_qua, ip->item_con);
                        strout_tooltip(mx, my, buf);
                    }
                }
            }
        }
        sprintf(buf, "F%d", i + 1);
        strout_blt(ScreenSurface, &font_tiny, buf, x + quickslots_pos[i][qsx]+xoff + 12, y + quickslots_pos[i][qsy] - 6,
                  skin_prefs.widget_title, NULL, NULL);
    }
}
void widget_quickslots(int x, int y)
{
    int     i, mx, my;
    char    buf[512];
    int     qsx, qsy, xoff;

    if (widget_data[WIDGET_QUICKSLOT_ID].ht > 34)
    {
        qsx = 1;
        qsy = 0;
        xoff = 0;
        sprite_blt(skin_sprites[SKIN_SPRITE_QUICKSLOTSV], x, y, NULL, NULL);
    }
    else
    {
        qsx = 0;
        qsy = 1;
        xoff = -17;
        sprite_blt(skin_sprites[SKIN_SPRITE_QUICKSLOTS], x, y, NULL, NULL);
    }

    SDL_GetMouseState(&mx, &my);
    update_quickslots(-1);

    for (i = MAX_QUICK_SLOTS - 1; i >= 0; i--)
    {
        if (quick_slots[i].shared.tag != -1)
        {
            /* spell in quickslot */
            if (quick_slots[i].shared.is_spell == 1)
            {
                sprite_blt(spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].icon,
                           x + quickslots_pos[i][qsx]+xoff, y + quickslots_pos[i][qsy], NULL, NULL);
                if (mx >= x + quickslots_pos[i][qsx]+xoff
                        && mx < x + quickslots_pos[i][qsx]+xoff + 33
                        && my >= y + quickslots_pos[i][qsy]
                        && my < y + quickslots_pos[i][qsy] + 33
                        && widget_get_mouse_state(&mx,&my,WIDGET_QUICKSLOT_ID))
                    strout_tooltip(mx, my,
                                 spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].name);
            }
            /* item in quickslot */
            else
            {
                item *ip;

                if ((ip = locate_item_from_item(cpl.ob, quick_slots[i].shared.tag)))
                {
                    uint8 quacon = (ip->item_qua == 255) ? 255
                                   : (float)ip->item_con /
                                     (float)ip->item_qua * 100;

                    sprite_blt_as_icon(face_list[ip->face].sprite,
                                       x + quickslots_pos[i][qsx] + xoff,
                                       y + quickslots_pos[i][qsy],
                                       SPRITE_ICON_TYPE_ACTIVE, 0, ip->flagsval,
                                       (quacon == 100) ? 0 : quacon,
                                       (ip->nrof == 1) ? 0 : ip->nrof, NULL);

                    /* show tooltip */
                    if (mx >= x + quickslots_pos[i][qsx]+xoff
                            && mx < x + quickslots_pos[i][qsx]+xoff + 33
                            && my >= y + quickslots_pos[i][qsy]
                            && my < y + quickslots_pos[i][qsy] + 33
                            && widget_get_mouse_state(&mx,&my,WIDGET_QUICKSLOT_ID))
                    {
                        sprintf(buf,"%s (QC: %d/%d)",ip->s_name, ip->item_qua, ip->item_con);
                        strout_tooltip(mx, my, buf);
                    }
                }
            }
        }
        sprintf(buf, "F%d", i + 1);
        strout_blt(ScreenSurface, &font_tiny, buf, x + quickslots_pos[i][qsx]+xoff + 12, y + quickslots_pos[i][qsy] - 6,
                  skin_prefs.widget_title, NULL, NULL);
    }
}
void widget_quickslots_mouse_event(int x, int y, uint8 e)
{
    if (e == SDL_MOUSEBUTTONUP)
    {
        if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
        {
            int ind = get_quickslot(x, y);
            if (ind != -1) /* valid slot */
            {
                if (draggingInvItem(DRAG_GET_STATUS) == DRAG_QUICKSLOT_SPELL)
                {
                    quick_slots[ind].shared.is_spell = 1;
                    quick_slots[ind].spell.groupNr = quick_slots[cpl.win_quick_tag].spell.groupNr;
                    quick_slots[ind].spell.classNr = quick_slots[cpl.win_quick_tag].spell.classNr;
                    quick_slots[ind].shared.tag = quick_slots[cpl.win_quick_tag].spell.spellNr;
                    quick_slots[cpl.win_quick_tag].shared.tag = -1;
                    cpl.win_quick_tag = -1;
                }
                else
                {
                    if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_INV)
                        cpl.win_quick_tag = cpl.win_inv_tag;
                    else if (draggingInvItem(DRAG_GET_STATUS) == DRAG_PDOLL)
                        cpl.win_quick_tag = cpl.win_pdoll_tag;
                    quick_slots[ind].shared.tag = cpl.win_quick_tag;
                    quick_slots[ind].item.invSlot = ind;
                    quick_slots[ind].shared.is_spell = 0;
                    /* now we do some tests... first, ensure this item can fit */
                    update_quickslots(-1);
                    /* now: if this is null, item is *not* in the main inventory
                                       * of the player - then we can't put it in quickbar!
                                       * Server will not allow apply of items in containers!
                                       */
                    if (!locate_item_from_inv(cpl.ob->inv, cpl.win_quick_tag))
                    {
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, 100);
                        textwin_show_string(0, skin_prefs.widget_info, "Only items from main inventory allowed in quickbar!");
                    }
                    else
                    {
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100); /* no bug - we 'get' it in quickslots */
                        textwin_show_string(0, skin_prefs.widget_info, "set F%d to %s",
                                           ind + 1,
                                           locate_item(cpl.win_quick_tag)->s_name);
                    }
                }
            }
            draggingInvItem(DRAG_NONE);
            itemExamined = 0; /* ready for next item */
        }
    }
    else /*Mousedown Event */
    {
        /* drag from quickslots */
        int   ind = get_quickslot(x, y);
        if (ind != -1 && quick_slots[ind].shared.tag != -1) /* valid slot */
        {
            cpl.win_quick_tag = quick_slots[ind].shared.tag;
            if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
            {
                if (quick_slots[ind].shared.is_spell == 1)
                {
                    draggingInvItem(DRAG_QUICKSLOT_SPELL);
                    quick_slots[ind].spell.spellNr = quick_slots[ind].shared.tag;
                    cpl.win_quick_tag = ind;
                }
                else
                {
                    draggingInvItem(DRAG_QUICKSLOT);
                    quick_slots[ind].shared.tag = -1;
                }
            }
            else
            {
                inventory_win_t stemp = cpl.inventory_win;
                int             itemp = cpl.win_inv_tag;

                cpl.inventory_win = IWIN_INV;
                cpl.win_inv_tag = quick_slots[ind].shared.tag;
                process_macro_keys(KEYFUNC_APPLY, 0);
                cpl.inventory_win = stemp;
                cpl.win_inv_tag = itemp;
            }
        }
        else if (x >= widget_data[WIDGET_QUICKSLOT_ID].x1+266
                 && x <= widget_data[WIDGET_QUICKSLOT_ID].x1 + 282
                 && y >= widget_data[WIDGET_QUICKSLOT_ID].y1
                 && y <= widget_data[WIDGET_QUICKSLOT_ID].y1 + 34
                 && (widget_data[WIDGET_QUICKSLOT_ID].ht <= 34))
        {
            widget_data[WIDGET_QUICKSLOT_ID].wd = 34;
            widget_data[WIDGET_QUICKSLOT_ID].ht = 282;
            widget_data[WIDGET_QUICKSLOT_ID].x1 +=266;
        }
        else if (x >= widget_data[WIDGET_QUICKSLOT_ID].x1
                 && x <= widget_data[WIDGET_QUICKSLOT_ID].x1 + 34
                 && y >= widget_data[WIDGET_QUICKSLOT_ID].y1
                 && y <= widget_data[WIDGET_QUICKSLOT_ID].y1 + 15
                 && (widget_data[WIDGET_QUICKSLOT_ID].ht > 34))
        {
            widget_data[WIDGET_QUICKSLOT_ID].wd = 282;
            widget_data[WIDGET_QUICKSLOT_ID].ht = 34;
            widget_data[WIDGET_QUICKSLOT_ID].x1 -=266;
        }
    }


    return;
}


void update_quickslots(int del_item)
{
    int i;

    for (i = 0; i < MAX_QUICK_SLOTS; i++)
    {
        if (quick_slots[i].shared.tag == del_item)
            quick_slots[i].shared.tag = -1;
        if (quick_slots[i].shared.tag == -1)
            continue;
        /* only items in the *main* inventory can used with quickslot! */
        if (quick_slots[i].shared.is_spell == 0)
        {
            if (!locate_item_from_inv(cpl.ob->inv, quick_slots[i].shared.tag))
                quick_slots[i].shared.tag = -1;
            if (quick_slots[i].shared.tag != -1)
                quick_slots[i].item.nr = locate_item_nr_from_tag(cpl.ob->inv, quick_slots[i].shared.tag);
        }
    }
}

static void freeQuickSlots(_quickslot *quickslots, int size)
{
    int i;

    for (i = 0; i != size; ++i)
    {
        if (quickslots[i].shared.is_spell == 0)
            FREE(quickslots[i].name.name);
    }
}

static int readNextQuickSlots(FILE *fp, char *server, int *port, char *name, _quickslot *quickslots)
{
    int     ch, i, r;

    for (ch = 1, i = 0; ch;)
    {
        if (i == 2048)
            return 0;
        ch = fgetc(fp);
        if (ch == EOF)
            return 0;
        server[i++] = ch;
    }
    if (!fread(port, sizeof(int), 1, fp))
        return 0;
    for (ch = 1, i = 0; ch;)
    {
        if (i == 40)
            return 0;
        ch = fgetc(fp);
        if (ch == EOF)
            return 0;
        name[i++] = ch;
    }
    for (i = r = 0; i != MAX_QUICK_SLOTS; ++i)
    {
        if (!fread(&quickslots[i].shared.is_spell, sizeof(uint8), 1, fp))
        {
            freeQuickSlots(quickslots, i);
            return 0;
        }
        r += sizeof(uint8);
        if (quickslots[i].shared.is_spell == 0)
        {
            int j;

            if (!fread(&quickslots[i].item.nr, sizeof(int), 1, fp))
            {
                freeQuickSlots(quickslots, i);
                return 0;
            }
            r += sizeof(int);
            MALLOC(quickslots[i].name.name, sizeof(char) * 128);
            for (ch = 1, j = 0; ch; ++r)
            {
                if (j == 128)
                {
                    freeQuickSlots(quickslots, i + 1);
                    return 0;
                }
                ch = fgetc(fp);
                if (ch == EOF)
                {
                    freeQuickSlots(quickslots, i + 1);
                    return 0;
                }
                quickslots[i].name.name[j++] = ch;
            }
        }
        else
        {
            if (!fread(&quickslots[i].shared.tag, sizeof(int), 1, fp))
            {
                freeQuickSlots(quickslots, i);
                return 0;
            }
            if (!fread(&quickslots[i].spell.groupNr, sizeof(int), 1, fp))
            {
                freeQuickSlots(quickslots, i);
                return 0;
            }
            if (!fread(&quickslots[i].spell.classNr, sizeof(int), 1, fp))
            {
                freeQuickSlots(quickslots, i);
                return 0;
            }
            r += sizeof(int) * 3;
        }
    }
    return r;
}

/******************************************************************
 Restore quickslots from last game.
******************************************************************/
#define QUICKSLOT_FILE "settings/quick.dat"
#define QUICKSLOT_FILE_VERSION 2
#define QUICKSLOT_FILE_HEADER ((QUICKSLOT_FILE_VERSION << 24) | 0x53 << 16 | 0x51 << 8 | 0x44)
void load_quickslots_entrys()
{
    long        header;
    int         i, port;
    char        name[40], server[2048];
    _quickslot  quickslots[MAX_QUICK_SLOTS];
    FILE       *stream;
    size_t      dummy; // purely to suppress GCC's warn_unused_result warning

    if (!(stream = fopen_wrapper(QUICKSLOT_FILE, "rb")))
        return;
    dummy = fread(&header, sizeof(header), 1, stream);
    if (header != QUICKSLOT_FILE_HEADER)
    {
        fclose(stream);
        remove(file_path(QUICKSLOT_FILE, ""));
        return;
    }
    while (readNextQuickSlots(stream, server, &port, name, quickslots))
    {
        if (!strcmp(gameserver_sel->address, server) && gameserver_sel->port == port)
        {
            uint8 cont = 0;

            port = strlen(cpl.name) + 1;
            for (i = 0; i != port; ++i)
            {
                if (tolower(cpl.name[i]) != tolower(name[i]))
                {
                    cont = 1;
                    break;
                }
            }

            if (cont == 1)
                continue;

            for (i = 0; i != MAX_QUICK_SLOTS; ++i)
            {
                if (quick_slots[i].shared.is_spell == 0)
                {
                    int      j;
                    uint8  match = 0;
                    item    *ob = cpl.ob->inv;

                    for (j = 0; ob != NULL; ++j, ob = ob->next)
                    {
                        if (j == quick_slots[i].item.nr)
                        {
                            if (!strcmp(ob->s_name, quick_slots[i].name.name))
                            {
                                quick_slots[i].item.tag = ob->tag;
                                match = 1;
                            }
                            break;
                        }
                    }
                    if (match == 0)
                    {
                        for (ob = cpl.ob->inv; ob; ob = ob->next)
                        {
                            if (!strcmp(ob->s_name, quick_slots[i].name.name))
                            {
                                quick_slots[i].item.tag = ob->tag;
                                match = 1;
                                break;
                            }
                        }
                        if (match == 0)
                        {
                            cont = 1;
                            quick_slots[i].item.tag = -1;
                        }
                    }
                    FREE(quick_slots[i].name.name);
                }
                else
                {
                    memcpy(&quick_slots[i], &quickslots[i], sizeof(_quickslot));
                    if (quick_slots[i].shared.tag == -1)
                        cont = 1;
                }
                if (cont == 1)
                    continue;
                if (quick_slots[i].shared.is_spell == 0)
                    cpl.win_inv_slot = quick_slots[i].item.invSlot;
            }
            break;
        }
    }
    fclose(stream);
    update_quickslots(-1);
}

/******************************************************************
 Save the current quickslots.
******************************************************************/
void save_quickslots_entrys()
{
    long        header;
    char        name[40], server[2048];
    int         n, size, w;
    _quickslot  quickslots[MAX_QUICK_SLOTS];
    FILE       *stream;

    if (!(stream = fopen_wrapper(QUICKSLOT_FILE, "rb+")))
    {
        if (!(stream = fopen_wrapper(QUICKSLOT_FILE, "wb+")))
            return;
    }
    header = QUICKSLOT_FILE_HEADER;
    fwrite(&header, sizeof(header), 1, stream);
    for (n = w = 0; n != MAX_QUICK_SLOTS; ++n)
    {
        w += sizeof(uint8);
        if (quick_slots[n].shared.is_spell == 0)
        {
            item *ob = locate_item_from_inv(cpl.ob->inv, quick_slots[n].item.tag);

            w += sizeof(int);
            MALLOC(quick_slots[n].name.name, sizeof(char) * 128);
            if (ob != NULL)
            {
                int i = strlen(ob->s_name) + 1;

                strncpy(quick_slots[n].name.name, ob->s_name, i);
                w += i;
            }
            else
                w += sizeof(char);
        }
        else
            w += sizeof(int) * 3;
    }
    /* readNextQuickSlots has problems with wb+ and rb+ opened files */
    n = ftell(stream);

    if (!(freopen(file_path(QUICKSLOT_FILE, "rb"),"rb",stream)))
        return;

    fseek(stream,n,SEEK_SET);
    while ((size = readNextQuickSlots(stream, server, &n, name, quickslots)) != 0)
    {
        if (!strcmp(gameserver_sel->address, server) && n == gameserver_sel->port && !strcmp(cpl.name, name))
        {
            if ((n = w - size) != 0)
            {
                char  *buf;
                long   pos = ftell(stream);
                size_t dummy; // purely to suppress GCC's warn_unused_result warning

                if (!(freopen(file_path(QUICKSLOT_FILE, "rb+"),"rb+",stream)))
                    return;

                fseek(stream, 0, SEEK_END);
                w = ftell(stream) - pos;
                MALLOC(buf, w);
                fseek(stream, pos, SEEK_SET);
                dummy = fread(buf, 1, w, stream);
                fseek(stream, pos + n, SEEK_SET);
                fwrite(buf, 1, w, stream);
                if (n < 0)
                {
                    w = ftell(stream);
                    rewind(stream);
                    buf = (char *)realloc(buf, w);
                    dummy = fread(buf, 1, w, stream);

                    if (!(freopen(file_path(QUICKSLOT_FILE, "wb+"), "wb+", stream)))
                        return;

                    fwrite(buf, 1, w, stream);
                }
                FREE(buf);
                fseek(stream, pos, SEEK_SET);
            }
            fseek(stream, -size, SEEK_CUR);
            for (n = 0; n != MAX_QUICK_SLOTS; ++n)
            {
                fwrite(&quick_slots[n].shared.is_spell, sizeof(uint8), 1, stream);
                if (quick_slots[n].shared.is_spell == 0)
                {
                    fwrite(&quick_slots[n].item.nr, sizeof(int), 1, stream);
                    fwrite(quick_slots[n].name.name, sizeof(char), strlen(quick_slots[n].name.name) + 1, stream);
                }
                else
                {
                    fwrite(&quick_slots[n].shared.tag, sizeof(int), 1, stream);
                    fwrite(&quick_slots[n].spell.groupNr, sizeof(int), 1, stream);
                    fwrite(&quick_slots[n].spell.classNr, sizeof(int), 1, stream);
                }
            }
            fclose(stream);
            freeQuickSlots(quick_slots, MAX_QUICK_SLOTS);
            return;
        }
    }

    if (!(freopen(file_path(QUICKSLOT_FILE, "rb+"),"rb+",stream)))
        return;

    fseek(stream, 0, SEEK_END);
    fwrite(&gameserver_sel->address, sizeof(char), strlen(gameserver_sel->address) + 1, stream);
    fwrite(&gameserver_sel->port, sizeof(int), 1, stream);
    fwrite(&cpl.name, sizeof(char), strlen(cpl.name) + 1, stream);
    for (n = 0; n != MAX_QUICK_SLOTS; ++n)
    {
        fwrite(&quick_slots[n].shared.is_spell, sizeof(uint8), 1, stream);
        if (quick_slots[n].shared.is_spell == 0)
        {
            fwrite(&quick_slots[n].item.nr, sizeof(int), 1, stream);
            fwrite(quick_slots[n].name.name, sizeof(char), strlen(quick_slots[n].name.name) + 1, stream);
        }
        else
        {
            fwrite(&quick_slots[n].shared.tag, sizeof(int), 1, stream);
            fwrite(&quick_slots[n].spell.groupNr, sizeof(int), 1, stream);
            fwrite(&quick_slots[n].spell.classNr, sizeof(int), 1, stream);
        }
    }
    fclose(stream);
    freeQuickSlots(quick_slots, MAX_QUICK_SLOTS);
}
void widget_event_target(int x, int y, SDL_Event event)
{
    /* combat modus */
    if (y > widget_data[WIDGET_TARGET_ID].y1+3 &&
        y < widget_data[WIDGET_TARGET_ID].y1+38 &&
        x > widget_data[WIDGET_TARGET_ID].x1+3 &&
        x < widget_data[WIDGET_TARGET_ID].x1+30)
    {
        check_keys(SDLK_c);
    }
    /* talk button */
    if (y > widget_data[WIDGET_TARGET_ID].y1 + 7 &&
        y < widget_data[WIDGET_TARGET_ID].y1 + 25 &&
        x > widget_data[WIDGET_TARGET_ID].x1 + 223 &&
        x < widget_data[WIDGET_TARGET_ID].x1 + 259)
    {
        if (cpl.target_code)
        {
            char buf[6] = "hello";

            client_cmd_guitalk(GUI_NPC_MODE_NPC, buf);
        }
    }
}

void widget_show_target(int x, int y)
{
    char       *ptr = NULL;
    SDL_Rect    box;
    double      temp;
    int         hp_tmp;

    sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_BG], x, y, NULL, NULL);

    sprite_blt(skin_sprites[cpl.target_mode ? SKIN_SPRITE_TARGET_ATTACK : SKIN_SPRITE_TARGET_NORMAL], x+5, y+4, NULL, NULL);

    sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_HP_B], x+4, y+24, NULL, NULL);

    /* redirect target_hp to our hp - server don't send it
     * because we should now our hp exactly
     */
    hp_tmp = (int) cpl.target_hp ;
    if (cpl.target_code == 0)
        hp_tmp = (int) (((float) cpl.stats.hp / (float) cpl.stats.maxhp) * 100.0f);

    if (cpl.target_code == 0)
    {
        if (cpl.target_mode)
            ptr = "target self (hold attack)";
        else
            ptr = "target self";
    }
    else if (cpl.target_code == 1)
    {
        if (cpl.target_mode)
            ptr = "target and attack enemy";
        else
            ptr = "target enemy";
    }
    else if (cpl.target_code == 2)
    {
        if (cpl.target_mode)
            ptr = "target friend (hold attack)";
        else
            ptr = "target friend";
    }
    if (cpl.target_code)
        sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_TALK], x + 223, y + 7, NULL, NULL);

    if (options.show_target_self || cpl.target_code != 0)
    {
        if (hp_tmp)
        {
            temp = (double) hp_tmp * 0.01;
            box.x = 0;
            box.y = 0;
            box.h = skin_sprites[SKIN_SPRITE_TARGET_HP]->bitmap->h;
            box.w = (int) (skin_sprites[SKIN_SPRITE_TARGET_HP]->bitmap->w * temp);
            if (!box.w)
                box.w = 1;
            if (box.w > skin_sprites[SKIN_SPRITE_TARGET_HP]->bitmap->w)
                box.w = skin_sprites[SKIN_SPRITE_TARGET_HP]->bitmap->w;
            sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_HP], x+5, y + 25, &box, NULL);
        }

        if (ptr)
        {
            /* Draw the name of the target */
            strout_blt(ScreenSurface, &font_small, cpl.target_name, x + 35,
                       y + 3, cpl.target_colr, NULL, NULL);
            /* Either draw HP remaining percent and description... */
            if (hp_tmp)
            {
                char hp_text[9];

                sprintf(hp_text, "HP: %d%%", hp_tmp);
                strout_blt(ScreenSurface, &font_small, hp_text, x + 35, y + 14,
                           percentage_colr(cpl.target_hp), NULL, NULL);
                strout_blt(ScreenSurface, &font_small, ptr, x + 85, y + 14,
                           cpl.target_colr, NULL, NULL);
            }
            /* ...or draw just the description */
            else
            {
                strout_blt(ScreenSurface, &font_small, ptr, x + 35, y + 14,
                           cpl.target_colr, NULL, NULL);
            }
        }
    }
}


void reset_menu_status(void)
{
    if (cpl.menustatus != MENU_NO)
    {
        cpl.menustatus = MENU_NO;
    }

}
