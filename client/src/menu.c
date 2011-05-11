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
static char            *get_range_item_name(int id);

_quickslot              quick_slots[MAX_QUICK_SLOTS];
int                     quickslots_pos[MAX_QUICK_SLOTS][2]  =
    {
        {17,1}, {50,1}, {83,1}, {116,1}, {149,1}, {182,1}, {215,1}, {248,1}
    };

static void GetLocalBmaps(void);

void do_console(int x, int y)
{
    if (InputStringEscFlag == 1)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CONSOLE, 0, 0, 100);
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        cur_widget[IN_CONSOLE_ID].show = 0;
        map_udate_flag = 2;
    }
    /* if set, we got a finished input!*/
    if (InputStringFlag == 0 && InputStringEndFlag == 1)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CONSOLE, 0, 0, 100);
        if (InputString[0])
        {
//            textwin_showstring(COLOR_DGOLD, ":%s", InputString);
            send_game_command(InputString);
        }

        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
        cur_widget[IN_CONSOLE_ID].show = 0;
    }
    else
        cur_widget[IN_CONSOLE_ID].show = 1;
}

void widget_show_console(int x, int y)
{
    sprite_blt(Bitmaps[BITMAP_TEXTINPUT],x, y, NULL, NULL);
    string_blt(ScreenSurface, &font_small, show_input_string(InputString, &font_small, 239), x+9, y+7, COLOR_WHITE, NULL,
              NULL);
}

void do_number(int x, int y)
{
    if (InputStringEscFlag == 1)
    {
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        cur_widget[IN_NUMBER_ID].show = 0;
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
                send_inv_move(cpl.loc, cpl.tag, tmp);

                if (cpl.nummode == NUM_MODE_GET)
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100);
                else
                    sound_play_effect(SOUNDTYPE_NORMAL, SOUND_DROP, 0, 0, 100);

                textwin_showstring(COLOR_DGOLD, "%s %d from %d %s",
                                   (cpl.nummode == NUM_MODE_GET) ? "get" :
                                   "drop", tmp, cpl.nrof, cpl.num_text);
            }
        }
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
        cur_widget[IN_NUMBER_ID].show = 0;
    }
    else
        cur_widget[IN_NUMBER_ID].show = 1;
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
            send_talk_command(GUI_NPC_MODE_NPC, InputString);
            textwin_addhistory(InputString);
            reset_input_mode();
            gui_npc->status = GUI_NPC_STATUS_WAIT;
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
    mx = x - cur_widget[IN_NUMBER_ID].x1;
    my = y - cur_widget[IN_NUMBER_ID].y1;

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
    SDL_Rect    tmp;
    char        buf[512];

    tmp.w = 238;

    sprite_blt(Bitmaps[BITMAP_NUMBER], x, y, NULL, NULL);
    sprintf(buf, "%s how many from %d %s", cpl.nummode == NUM_MODE_GET ? "get" : "drop", cpl.nrof, cpl.num_text);
    string_blt(ScreenSurface, &font_small, buf, x + 8, y + 6, COLOR_HGOLD, &tmp, NULL);
    string_blt(ScreenSurface, &font_small,
              show_input_string(InputString, &font_small, Bitmaps[BITMAP_NUMBER]->bitmap->w - 22), x + 8, y + 25,
              COLOR_WHITE, &tmp, NULL);
}

static inline void print_resist(char *name, int x, int y, int num)
{
    char    buf[16];

    string_blt(widgetSF[RESIST_ID], &font_small, name, x+5, y, (num>ATNR_GODPOWER)?COLOR_DGOLD:COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.protection[num]);
    string_blt(widgetSF[RESIST_ID], &font_small, buf, x + 17, y, cpl.stats.protection[num] ? (cpl.stats.protection[num]<0?COLOR_RED:(cpl.stats.protection[num]>=100?COLOR_ORANGE:COLOR_WHITE)) : COLOR_GREY,NULL, NULL);
}

void widget_show_resist(int x, int y)
{
    _BLTFX bltfx;
    SDL_Rect box;

    if (!widgetSF[RESIST_ID])
        widgetSF[RESIST_ID]=SDL_ConvertSurface(Bitmaps[BITMAP_RESIST_BG]->bitmap,Bitmaps[BITMAP_RESIST_BG]->bitmap->format,Bitmaps[BITMAP_RESIST_BG]->bitmap->flags);

    if (cur_widget[RESIST_ID].redraw)
    {
        cur_widget[RESIST_ID].redraw=0;

        bltfx.surface=widgetSF[RESIST_ID];
        bltfx.flags = 0;
        bltfx.alpha=0;

        sprite_blt(Bitmaps[BITMAP_RESIST_BG], 0, 0, NULL, &bltfx);
        string_blt(widgetSF[RESIST_ID], &font_tiny_out, "Resistance Table", 5,  1, COLOR_HGOLD, NULL, NULL);

        print_resist("IM", 68, 3, ATNR_PHYSICAL);
        print_resist("SL", 98, 3, ATNR_SLASH);
        print_resist("CL", 128, 3, ATNR_CLEAVE);
        print_resist("PI", 158, 3, ATNR_PIERCE);

        print_resist("FI", 8, 15, ATNR_FIRE);
        print_resist("CO", 38, 15, ATNR_COLD);
        print_resist("EL", 68, 15, ATNR_ELECTRICITY);
        print_resist("PO", 98, 15, ATNR_POISON);
        print_resist("AC", 128, 15, ATNR_ACID);
        print_resist("SO", 158, 15, ATNR_SONIC);

        print_resist("CH", 8, 27, ATNR_CHANNELLING);
        print_resist("CR", 38, 27, ATNR_CORRUPTION);
        print_resist("PS", 68, 27, ATNR_PSIONIC);
        print_resist("LI", 98, 27, ATNR_LIGHT);
        print_resist("SH", 128, 27, ATNR_SHADOW);
        print_resist("LS", 158, 27, ATNR_LIFESTEAL);

        print_resist("AE", 8, 39, ATNR_AETHER);
        print_resist("NE", 38, 39, ATNR_NETHER);
        print_resist("CH", 68, 39, ATNR_CHAOS);
        print_resist("DE", 98, 39, ATNR_DEATH);
        print_resist("WE", 128, 39, ATNR_WEAPONMAGIC);
        print_resist("GO", 158, 39, ATNR_GODPOWER);

        print_resist("DR", 8, 51, ATNR_DRAIN);
        print_resist("DE", 38, 51, ATNR_DEPLETION);
        print_resist("CM", 68, 51, ATNR_COUNTERMAGIC);
        print_resist("CA", 98, 51, ATNR_CANCELLATION);
        print_resist("CF", 128, 51, ATNR_CONFUSION);
        print_resist("FE", 158, 51, ATNR_FEAR);

        print_resist("SL", 8, 63, ATNR_SLOW);
        print_resist("PA", 38, 63, ATNR_PARALYZE);
        print_resist("SN", 68, 63, ATNR_SNARE);
    }
    box.x=x;
    box.y=y;
    SDL_BlitSurface(widgetSF[RESIST_ID], NULL, ScreenSurface, &box);
}

#define ICONDEFLEN 32
uint8 blt_face_centered(int face, int x, int y)
{
    register int temp;
    SDL_Rect    box;

    if (!FaceList[face].sprite)
        return 0;

    if (FaceList[face].sprite->status != SPRITE_STATUS_LOADED)
        return 0;

    box.x = FaceList[face].sprite->border_left;
    box.w = FaceList[face].sprite->bitmap->w;
    temp = box.w - FaceList[face].sprite->border_left - FaceList[face].sprite->border_right;
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

    box.y = -FaceList[face].sprite->border_up;
    box.h = FaceList[face].sprite->bitmap->h;
    temp = box.h - FaceList[face].sprite->border_up - FaceList[face].sprite->border_down;
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
        box.y = -(temp >> 1) + FaceList[face].sprite->border_up;
    }
    sprite_blt(FaceList[face].sprite, x, y, &box, NULL);

    return 1;
}

void widget_range_event(int x, int y, SDL_Event event, int MEvent)
{
    if (x > cur_widget[RANGE_ID].x1 + 5 &&
        x < cur_widget[RANGE_ID].x1 + 38 &&
        y >= cur_widget[RANGE_ID].y1 + 3 &&
        y <= cur_widget[RANGE_ID].y1 + 33)
    {
        if (MEvent==MOUSE_DOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
                process_macro_keys(KEYFUNC_RANGE, 0);
            else if (event.button.button == 4) /* mousewheel up */
                process_macro_keys(KEYFUNC_RANGE, 0);
            else if (event.button.button == 5) /* mousewheel down */
                process_macro_keys(KEYFUNC_RANGE_BACK, 0);
            else
                process_macro_keys(KEYFUNC_RANGE_BACK, 0);
        }
        else if (MEvent==MOUSE_UP)
        {
            if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
            {
                /* KEYFUNC_APPLY and KEYFUNC_DROP works only if cpl.inventory_win = IWIN_INV. The tag must
                            be placed in cpl.win_inv_tag. So we do this and after DnD we restore the old values. */
                int   old_inv_win = cpl.inventory_win;
                int   old_inv_tag = cpl.win_inv_tag;
                cpl.inventory_win = IWIN_INV;

                /* range field */
                if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_INV &&
                    x >= cur_widget[RANGE_ID].x1 &&
                    x <= cur_widget[RANGE_ID].x1 + 78 &&
                    y >= cur_widget[RANGE_ID].y1 &&
                    y <= cur_widget[RANGE_ID].y1 + 35)
                {
                    RangeFireMode = 4;
                    process_macro_keys(KEYFUNC_APPLY, 0); /* drop to player-doll */
                }
                cpl.inventory_win = old_inv_win;
                cpl.win_inv_tag = old_inv_tag;
            }
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
    rec_item.w = 185;
    examine_range_inv();

    sprite_blt(Bitmaps[BITMAP_RANGE], x, y, NULL, NULL);

    switch (RangeFireMode)
    {
        case FIRE_MODE_BOW:
            if (fire_mode_tab[FIRE_MODE_BOW].item != FIRE_ITEM_NO)
            {
                tmp2=locate_item(fire_mode_tab[FIRE_MODE_BOW].item);
                if (!tmp2)
                {
                    LOG(LOG_DEBUG,"BUG: applied range weapon don't exist\n");
                    string_blt(ScreenSurface, &font_small, "using Nothing", x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);
                }
                else
                {
                    sprintf(buf, "using %s", tmp2->s_name);
                    blt_inventory_face_from_tag(fire_mode_tab[FIRE_MODE_BOW].item, x + 5, y + 2);

                    string_blt(ScreenSurface, &font_small, buf, x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);
                }

                if (fire_mode_tab[FIRE_MODE_BOW].amun != FIRE_ITEM_NO)
                {
                    tmp = locate_item_from_item(cpl.ob, fire_mode_tab[FIRE_MODE_BOW].amun);
                    if (tmp)
                    {
                        if (tmp->itype == TYPE_ARROW)
                            sprintf(buf, "ammo %s (%d)", get_range_item_name(fire_mode_tab[FIRE_MODE_BOW].amun), tmp->nrof);
                        else
                            sprintf(buf, "ammo %s", get_range_item_name(fire_mode_tab[FIRE_MODE_BOW].amun));
                    }
                    else
                        strcpy(buf, "ammo not selected");
                    blt_inventory_face_from_tag(fire_mode_tab[FIRE_MODE_BOW].amun, x + 45, y + 2);
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

                string_blt(ScreenSurface, &font_small, buf, x + 5, y + 47, COLOR_WHITE, &rec_item, NULL);
            }
            else
            {
                sprintf(buf, "no range weapon applied");
                string_blt(ScreenSurface, &font_small, buf, x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);
            }

            sprite_blt(Bitmaps[BITMAP_RANGE_MARKER], x + 5, y + 2, NULL, NULL);
            break;

            /* these are client only, no server signal needed */
        case FIRE_MODE_SKILL:
            if (fire_mode_tab[FIRE_MODE_SKILL].skill)
            {
                sprite_blt(Bitmaps[BITMAP_RANGE_SKILL], x + 5, y + 2, NULL, NULL);
                if (fire_mode_tab[FIRE_MODE_SKILL].skill->flag != -1)
                {
                    sprite_blt(fire_mode_tab[FIRE_MODE_SKILL].skill->icon, x + 45, y + 2, NULL, NULL);
                    string_blt(ScreenSurface, &font_small, fire_mode_tab[FIRE_MODE_SKILL].skill->name, x + 5, y + 47,
                              COLOR_WHITE, &rec_item, NULL);
                }
                else
                    fire_mode_tab[FIRE_MODE_SKILL].skill = NULL;
            }
            else
            {
                sprite_blt(Bitmaps[BITMAP_RANGE_SKILL_NO], x + 5, y + 2, NULL, NULL);
                sprintf(buf, "no skill selected");
                string_blt(ScreenSurface, &font_small, buf, x + 5, y + 47, COLOR_WHITE, &rec_item, NULL);
            }
            sprintf(buf, "use skill");
            string_blt(ScreenSurface, &font_small, buf, x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);

            break;
        case FIRE_MODE_SPELL:
            if (fire_mode_tab[FIRE_MODE_SPELL].spell)
            {
                /* we use wiz spells as default */
                sprite_blt(Bitmaps[BITMAP_RANGE_WIZARD], x + 5, y + 2, NULL, NULL);
                if (fire_mode_tab[FIRE_MODE_SPELL].spell->flag != -1)
                {
                    sprite_blt(fire_mode_tab[FIRE_MODE_SPELL].spell->icon, x + 45, y + 2, NULL, NULL);
                    string_blt(ScreenSurface, &font_small, fire_mode_tab[FIRE_MODE_SPELL].spell->name, x + 5, y + 47,
                              COLOR_WHITE, &rec_item, NULL);
                }
                else
                    fire_mode_tab[FIRE_MODE_SPELL].spell = NULL;
            }
            else
            {
                sprite_blt(Bitmaps[BITMAP_RANGE_WIZARD_NO], x + 5, y + 2, NULL, NULL);
                sprintf(buf, "no spell selected");
                string_blt(ScreenSurface, &font_small, buf, x + 5, y + 47, COLOR_WHITE, &rec_item, NULL);
            }
            sprintf(buf, "cast spell");
            string_blt(ScreenSurface, &font_small, buf, x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);

            break;
    };
}

static char * get_range_item_name(int tag)
{
    item   *tmp;

    if (tag != FIRE_ITEM_NO)
    {
        tmp = locate_item(tag);
        if (tmp)
            return tmp->s_name;
    }
    return("Nothing");
}

void blt_inventory_face_from_tag(int tag, int x, int y)
{
    item   *tmp;

    /* check item is in inventory and faces are loaded, etc */
    tmp = locate_item(tag);
    if (!tmp)
        return;
    blt_inv_item_centered(tmp, x, y);
}

void show_menu(void)
{
    SDL_Rect    box;

    if (!cpl.menustatus)
        return;
    if (cpl.menustatus == MENU_KEYBIND)
        show_keybind();
    else if (cpl.menustatus == MENU_BOOK)
        show_book(400-Bitmaps[BITMAP_JOURNAL]->bitmap->w/2,300-Bitmaps[BITMAP_JOURNAL]->bitmap->h/2);
    else if (cpl.menustatus == MENU_NPC)
    {
        int x,
            y;

        gui_npc_show();

        /* Force selection of element under pointer. */
        SDL_PumpEvents();
        SDL_GetMouseState(&x, &y);
        gui_npc_mousemove(x, y);
    }
    else if (cpl.menustatus == MENU_STATUS)
        show_status();
    else if (cpl.menustatus == MENU_SPELL)
    {
        show_spelllist();
        box.x = Screensize.x / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->w / 2;
        box.y = Screensize.y / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->h / 2 - 42;
        box.h = 42;
        box.w = Bitmaps[BITMAP_DIALOG_BG]->bitmap->w;
        SDL_FillRect(ScreenSurface, &box, 0);
        show_quickslots(box.x + 120, box.y + 3);
    }
    else if (cpl.menustatus == MENU_SKILL)
        show_skilllist();
    else if (cpl.menustatus == MENU_OPTION)
        show_optwin();
    else if (cpl.menustatus == MENU_CREATE)
        show_newplayer_server();
}

void show_media(int x, int y)
{
    _Sprite    *bmap;
    int         xtemp;

    if (media_show != MEDIA_SHOW_NO)
    {
        /* we show a png*/
        if (media_file[media_show].type == MEDIA_TYPE_PNG)
        {
            bmap = (_Sprite *) media_file[media_show].data;
            if (bmap)
            {
                xtemp = x - bmap->bitmap->w;
                sprite_blt(bmap, xtemp, y, NULL, NULL);
            }
        }
    }
}

void widget_show_mapname(int x, int y)
{
    string_blt(ScreenSurface, &font_large_out, MapData.name, x, y, COLOR_HGOLD, NULL, NULL);
}


void show_status(void)
{
    /*
            int y, x;
            x= Screensize.x/2-Bitmaps[BITMAP_STATUS]->bitmap->w/2;
            y= Screensize.y/2-Bitmaps[BITMAP_STATUS]->bitmap->h/2;
            sprite_blt(Bitmaps[BITMAP_STATUS],x, y, NULL, NULL);
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

int read_anim_tmp(void)
{
    FILE        *stream;
    struct stat  stat_anim,
                 stat_tmp;

    if ((stream = fopen_wrapper(FILE_CLIENT_ANIMS, "rb")) == NULL)
    {
        LOG(LOG_FATAL, "read_anim_tmp:Error reading bmap.tmp for anim.tmp!\n");
    }
    fstat(fileno(stream), &stat_anim);
    fclose(stream);

    if ((stream = fopen_wrapper(FILE_ANIMS_TMP, "rb")) != NULL)
    {
        fstat(fileno(stream), &stat_tmp);
        fclose(stream);

        /* our anim file must be newer as our default anim file */
        if (difftime(stat_tmp.st_mtime, stat_anim.st_mtime) > 0.0f)
        {
            return load_anim_tmp(); /* all fine - load file */
        }
    }

    create_anim_tmp();

    return load_anim_tmp(); /* all fine - load file */
}

void load_bmaps(void)
{
    PHYSFS_File *handle;
    char         buf[MEDIUM_BUF];
    int          i = 0;

    if (!PHYSFS_exists(FILE_CLIENT_BMAPS))
    {
       LOG(LOG_SYSTEM, "Could not find '%s'. This means that the server will not send images so all images must be sourced locally!\n",
           FILE_CLIENT_BMAPS);
       GetLocalBmaps();

       return;
    }

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_CLIENT_BMAPS);

    /* Open the file for reading. */
    if (!(handle = PHYSFS_openRead(FILE_CLIENT_BMAPS)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (PHYSFS_readString(handle, buf, sizeof(buf)) > 0)
    {
        char         name[TINY_BUF];
        int          len;
        unsigned int crc;

        if (sscanf(buf, "%x %x %s", &len, &crc, name) != 3)
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "FAILED (Malformed string: >%s<)!\n", buf);
        }

        MALLOC_STRING(bmaptype_table[i].name, name);
        bmaptype_table[i].pos = -1; // not local, updated in GetLocalBmaps()
        bmaptype_table[i].len = len;
        bmaptype_table[i].crc = crc;
        bmaptype_table_size = ++i;
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
    GetLocalBmaps();
}

static void GetLocalBmaps(void)
{
    PHYSFS_File *handle;
    char         buf[MEDIUM_BUF];
    int          i = 0;

    /* Check for existance of local images file. */
    if (!PHYSFS_exists(FILE_DAIMONIN_P0))
    {
       /* If it doesn't exist and the server is not sending us images, give up.
        * TODO: We should print a client message and go back to server select
        * rather than exit the client. */
       if (bmaptype_table_size == 0)
       {
           LOG(LOG_FATAL, "Could not find '%s'!\n", FILE_DAIMONIN_P0);
       }

       LOG(LOG_SYSTEM, "Could not find '%s'. This means all images will need to be requested from the server!\n",
           FILE_DAIMONIN_P0);

       return;
    }

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_DAIMONIN_P0);

    /* Open the file for reading. */
    if (!(handle = PHYSFS_openRead(FILE_DAIMONIN_P0)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (PHYSFS_readString(handle, buf, sizeof(buf)) > 0)
    {
        char          *cp = buf + 6,
                      *name;
        unsigned char *buf_tmp;
        int            pos,
                       len;
        unsigned int   crc;

        if (strncmp(buf, "IMAGE ", 6))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "FAILED (Bad image line: >%s<)!\n", buf);
        }

        /* Skip accross the number data, which we don't even use any more. */
        while (isdigit(*cp))
        {
            cp++;
        }

        pos = (int)PHYSFS_tell(handle);
        len = atoi(++cp);
        MALLOC(buf_tmp, len);

        if (PHYSFS_read(handle, buf_tmp, 1, len) < len)
        {
            PHYSFS_close(handle);
            FREE(buf_tmp);
            LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
        }

        crc = crc32(1L, buf_tmp, len);
        FREE(buf_tmp);

        /* Skip accross the length data. */
        while (isdigit(*cp))
        {
            cp++;
        }

        name = cp + 1;

        /* If we have an image which the server doesn't have, ignore it. */
        if ((i = get_bmap_id(name)) == -1)
        {
            continue;
        }
        /* Only if our image and the server's are identical, update pos to
         * point to the local one. */
        else if (bmaptype_table[i].len == len &&
                 bmaptype_table[i].crc == crc)
        {
            bmaptype_table[i].pos = pos;
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}

/* find a face ID by name,
 * request the face (find it, load it or request it)
 * and return the ID
 */
int get_bmap_id(char *name)
{
    int l = 0,
        r = bmaptype_table_size - 1,
        x = r / 2;

    for (; r >= l; x = (l + r) / 2)
    {
        int diff = strcmp(name, bmaptype_table[x].name);

        if (diff < 0)
        {
            r = x - 1;
        }
        else if (diff > 0)
        {
            l = x + 1;
        }
        else
        {
            return x;
        }
    }

    return -1;
}

/* TODO: This maintains 0.10 compatibility. The file format will be reworked for 0.11.0. */
void load_settings(void)
{
    PHYSFS_File *handle;
    uint8        defn = 0;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_CLIENT_SETTINGS);

    /* Open the file for reading.*/
    if (!(handle = PHYSFS_openRead(FILE_CLIENT_SETTINGS)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (++defn)
    {
        char buf[MEDIUM_BUF],
             key[TINY_BUF],
             value[TINY_BUF];

        do
        {
            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }
        }
        while (buf[0] == '#');

        if (sscanf(buf, "%s %s", key, value) != 2)
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed key/value line for definition %u: %s!\n", defn, buf);
        }

        if (!strcmp(key, "char"))
        {
            char          face[TINY_BUF],
                          arch[TINY_BUF];
            _server_char *sc;
            uint8         i;

            MALLOC(sc, sizeof(_server_char));
            MALLOC_STRING(sc->name, value);

            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }

            if (sscanf(buf, "%s %d %d %d %d %d %d",
                       face, &sc->bar[0], &sc->bar[1], &sc->bar[2],
                       &sc->bar_add[0], &sc->bar_add[1], &sc->bar_add[2]) != 7)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed hi line for definition %u: %s!\n", defn, buf);
            }

            if ((sc->pic_id = get_bmap_id(face)) != -1)
            {
                request_face(sc->pic_id);
            }

            /* 4 genders: male, female, hermaphrodite, neuter. */
            for (i = 0; i <= 3; i++)
            {
                if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Unexpected EOF!\n");
                }

                if (sscanf(buf, "%d %s %s", &sc->gender[i], arch, face) != 3)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Malformed gender %u line for definition %u: %s!\n",
                        i, defn, buf);
                }

                MALLOC_STRING(sc->char_arch[i], arch);

                if ((sc->face_id[i] = get_bmap_id(face)) != -1)
                {
                    request_face(sc->face_id[i]);
                }
            }

            /* Str Dex Con Int Wis Pow Cha */
            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }

            if (sscanf(buf, "%d %d %d %d %d %d %d",
                       &sc->stats[0], &sc->stats[1], &sc->stats[2],
                       &sc->stats[3], &sc->stats[4], &sc->stats[5],
                       &sc->stats[6]) != 7)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed stat line for definition %u: %s!\n", defn, buf);
            }

            /* 4 lines of description. */
            for (i = 0; i <= 3; i++)
            {
                if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Unexpected EOF!\n");
                }

                MALLOC_STRING(sc->desc[i], buf);
            }

            /* add this char template to list */
            if (!first_server_char)
            {
                first_server_char = sc;
            }
            else
            {
                _server_char *sc_tmp = first_server_char;

                while (sc_tmp->next)
                {
                     sc_tmp = sc_tmp->next;
                }

                sc_tmp->next = sc;
                sc->prev = sc_tmp;
            }
        }
        else if (!strcmp(key, "level"))
        {
            uint8 i;

            server_level.level = atoi(value);

            for (i = server_level.level; i > 0; i--)
            {
                if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Unexpected EOF!\n");
                }

                if ((server_level.exp[i] = strtoul(buf, NULL, 16)) == ULONG_MAX)
                {
                    PHYSFS_close(handle);
                    LOG(LOG_FATAL, "Malformed exp line for level %u: %s!\n", i, buf);
                }
            }

            break;
        }
        else /* we close here... better we include later a fallback to login */
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Unrecognised key: %s!\n", buf);
        }
    }

    /* TODO: Remove, just ugly. */
    if (first_server_char)
    {
        int g;

        memcpy(&new_character, first_server_char, sizeof(_server_char));
        new_character.skill_selected = 0;
        /* adjust gender */
        for (g = 0; g < 4; g++)
        {
            if (new_character.gender[g])
            {
                new_character.gender_selected = g;
                break;
            }
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}

/* TODO: This maintains 0.10 compatibility. The file format will be reworked for 0.11.0. */
void load_spells(void)
{
    PHYSFS_File  *handle;
    uint8         defn = 0;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_CLIENT_SPELLS);

    /* Open the file for reading.*/
    if (!(handle = PHYSFS_openRead(FILE_CLIENT_SPELLS)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (++defn)
    {
        char   buf[MEDIUM_BUF],
              *start,
              *end,
               name[TINY_BUF],
               type,
               nchar,
               icon[TINY_BUF],
               desc[4][TINY_BUF];
        int    panel;
        uint8  i;
        _spell_list_entry *sle;

        /* Name */
        if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
        {
            /* EOF here is OK. */
            break;
        }

        if (!(start = strchr(buf, '"')))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        if (!(end = strchr(start + 1, '"')))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        start++;
        *end = '\0';
        sprintf(name, "%s", start);

        /* Type Entry Path Icon */
        if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Unexpected EOF!\n");
        }

        if (sscanf(buf, "%c %c %d %s", &type, &nchar, &panel, icon) != 4 ||
            (type != 'w' &&
             type != 'p') ||
            (panel <= 0 ||
             panel > SPELL_LIST_MAX))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        /* Desc */
        for (i = 0; i <= 3; i++)
        {
            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }

            if (!(start = strchr(buf, '"')))
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
            }

            if (!(end = strchr(start + 1, '"')))
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
            }

            start++;
            *end = '\0';
            sprintf(desc[i], "%s", start);
        }

        sle = &spell_list[panel - 1].entry[(type == 'w') ? 0 : 1][nchar - 'a'];
        sprintf(sle->name, "%s", name);
        sle->flag = LIST_ENTRY_USED;
        sprintf(sle->icon_name, "%s", icon);
        sprintf(buf, "%s%s", GetIconDirectory(), icon);
        sle->icon = sprite_load_file(buf, SURFACE_FLAG_DISPLAYFORMAT);

        for (i = 0; i <= 3; i++)
        {
            sprintf(sle->desc[i], "%s", desc[i]);
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}

/* TODO: This maintains 0.10 compatibility. The file format will be reworked for 0.11.0. */
void load_skills(void)
{
    PHYSFS_File  *handle;
    uint8         defn = 0;

    /* Log what we're doing. */
    LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_CLIENT_SKILLS);

    /* Open the file for reading.*/
    if (!(handle = PHYSFS_openRead(FILE_CLIENT_SKILLS)))
    {
        LOG(LOG_FATAL, "FAILED (%s)!\n", PHYSFS_getLastError());
    }

    while (++defn)
    {
        char   buf[MEDIUM_BUF],
              *start,
              *end,
               name[TINY_BUF],
               nchar,
               icon[TINY_BUF],
               desc[4][TINY_BUF];
        int    panel;
        uint8  i;
        _skill_list_entry *sle;

        /* Name */
        if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
        {
            /* EOF here is OK. */
            break;
        }

        if (!(start = strchr(buf, '"')))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        if (!(end = strchr(start + 1, '"')))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        start++;
        *end = '\0';
        sprintf(name, "%s", start);

        /* Type Entry Path Icon */
        if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Unexpected EOF!\n");
        }

        if (sscanf(buf, "%d %c %s", &panel, &nchar, icon) != 3 ||
            (panel < 0 ||
             panel >= SPELL_LIST_MAX))
        {
            PHYSFS_close(handle);
            LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
        }

        /* Desc */
        for (i = 0; i <= 3; i++)
        {
            if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Unexpected EOF!\n");
            }

            if (!(start = strchr(buf, '"')))
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
            }

            if (!(end = strchr(start + 1, '"')))
            {
                PHYSFS_close(handle);
                LOG(LOG_FATAL, "Malformed line for definition %u: %s!\n", defn, buf);
            }

            start++;
            *end = '\0';
            sprintf(desc[i], "%s", start);
        }

        sle = &skill_list[panel].entry[nchar - 'a'];
        sprintf(sle->name, "%s", name);
        sle->flag = LIST_ENTRY_USED;
        sprintf(sle->icon_name, "%s", icon);
        sprintf(buf, "%s%s", GetIconDirectory(), icon);
        sle->icon = sprite_load_file(buf, SURFACE_FLAG_DISPLAYFORMAT);

        for (i = 0; i <= 3; i++)
        {
            sprintf(sle->desc[i], "%s", desc[i]);
        }
    }

    /* Cleanup. */
    PHYSFS_close(handle);
    LOG(LOG_SYSTEM, "OK!\n");
}

int get_quickslot(int x, int y)
{
    int i;
    int qsx, qsy, xoff;
    if (cur_widget[QUICKSLOT_ID].ht > 34)
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
        if (x >= cur_widget[QUICKSLOT_ID].x1 + quickslots_pos[i][qsx]+xoff
                && x <= cur_widget[QUICKSLOT_ID].x1 + quickslots_pos[i][qsx]+xoff + 32
                && y >= cur_widget[QUICKSLOT_ID].y1 + quickslots_pos[i][qsy]
                && y <= cur_widget[QUICKSLOT_ID].y1 + quickslots_pos[i][qsy] + 32)
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
        sprite_blt(Bitmaps[BITMAP_QUICKSLOTS], x, y, NULL, NULL);

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
                        && GetMouseState(&mx,&my,QUICKSLOT_ID))
                    show_tooltip(mx, my,
                                 spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].name);
            }
            /* item in quickslot */
            else
            {
                item   *tmp;
                tmp = locate_item_from_item(cpl.ob, quick_slots[i].shared.tag);
                if (tmp)
                {
                    blt_inv_item(tmp, x + quickslots_pos[i][qsx]+xoff, y + quickslots_pos[i][qsy]);
                    /* show tooltip */
                    if (mx >= x + quickslots_pos[i][qsx]+xoff
                            && mx < x + quickslots_pos[i][qsx]+xoff + 33
                            && my >= y + quickslots_pos[i][qsy]
                            && my < y + quickslots_pos[i][qsy] + 33
                            && GetMouseState(&mx,&my,QUICKSLOT_ID))
                    {
                        sprintf(buf,"%s (q/c: %d/%d)",tmp->s_name, tmp->item_qua, tmp->item_con);
                        show_tooltip(mx, my, buf);
                    }
                }
            }
        }
        sprintf(buf, "F%d", i + 1);
        string_blt(ScreenSurface, &font_tiny_out, buf, x + quickslots_pos[i][qsx]+xoff + 12, y + quickslots_pos[i][qsy] - 6,
                  COLOR_DEFAULT, NULL, NULL);
    }
}
void widget_quickslots(int x, int y)
{
    int     i, mx, my;
    char    buf[512];
    int     qsx, qsy, xoff;

    if (cur_widget[QUICKSLOT_ID].ht > 34)
    {
        qsx = 1;
        qsy = 0;
        xoff = 0;
        sprite_blt(Bitmaps[BITMAP_QUICKSLOTSV], x, y, NULL, NULL);
    }
    else
    {
        qsx = 0;
        qsy = 1;
        xoff = -17;
        sprite_blt(Bitmaps[BITMAP_QUICKSLOTS], x, y, NULL, NULL);
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
                        && GetMouseState(&mx,&my,QUICKSLOT_ID))
                    show_tooltip(mx, my,
                                 spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].name);
            }
            /* item in quickslot */
            else
            {
                item   *tmp;
                tmp = locate_item_from_item(cpl.ob, quick_slots[i].shared.tag);
                if (tmp)
                {
                    blt_inv_item(tmp, x + quickslots_pos[i][qsx]+xoff, y + quickslots_pos[i][qsy]);
                    /* show tooltip */
                    if (mx >= x + quickslots_pos[i][qsx]+xoff
                            && mx < x + quickslots_pos[i][qsx]+xoff + 33
                            && my >= y + quickslots_pos[i][qsy]
                            && my < y + quickslots_pos[i][qsy] + 33
                            && GetMouseState(&mx,&my,QUICKSLOT_ID))
                    {
                        sprintf(buf,"%s (QC: %d/%d)",tmp->s_name, tmp->item_qua, tmp->item_con);
                        show_tooltip(mx, my, buf);
                    }
                }
            }
        }
        sprintf(buf, "F%d", i + 1);
        string_blt(ScreenSurface, &font_tiny_out, buf, x + quickslots_pos[i][qsx]+xoff + 12, y + quickslots_pos[i][qsy] - 6,
                  COLOR_DEFAULT, NULL, NULL);
    }
}
void widget_quickslots_mouse_event(int x, int y, int MEvent)
{
    if (MEvent==1) /* Mouseup Event */
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
                        textwin_showstring(COLOR_WHITE, "Only items from main inventory allowed in quickbar!");
                    }
                    else
                    {
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100); /* no bug - we 'get' it in quickslots */
                        textwin_showstring(COLOR_DGOLD, "set F%d to %s",
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
                }
                quick_slots[ind].shared.tag = -1;
            }
            else
            {
                int stemp = cpl.      inventory_win, itemp = cpl.win_inv_tag;
                cpl.inventory_win = IWIN_INV;
                cpl.win_inv_tag = quick_slots[ind].shared.tag;
                process_macro_keys(KEYFUNC_APPLY, 0);
                cpl.inventory_win = stemp;
                cpl.win_inv_tag = itemp;
            }
        }
        else if (x >= cur_widget[QUICKSLOT_ID].x1+266
                 && x <= cur_widget[QUICKSLOT_ID].x1 + 282
                 && y >= cur_widget[QUICKSLOT_ID].y1
                 && y <= cur_widget[QUICKSLOT_ID].y1 + 34
                 && (cur_widget[QUICKSLOT_ID].ht <= 34))
        {
            cur_widget[QUICKSLOT_ID].wd = 34;
            cur_widget[QUICKSLOT_ID].ht = 282;
            cur_widget[QUICKSLOT_ID].x1 +=266;
        }
        else if (x >= cur_widget[QUICKSLOT_ID].x1
                 && x <= cur_widget[QUICKSLOT_ID].x1 + 34
                 && y >= cur_widget[QUICKSLOT_ID].y1
                 && y <= cur_widget[QUICKSLOT_ID].y1 + 15
                 && (cur_widget[QUICKSLOT_ID].ht > 34))
        {
            cur_widget[QUICKSLOT_ID].wd = 282;
            cur_widget[QUICKSLOT_ID].ht = 34;
            cur_widget[QUICKSLOT_ID].x1 -=266;
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
        if (!strcmp(ServerName, server) && ServerPort == port)
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
        if (!strcmp(ServerName, server) && n == ServerPort && !strcmp(cpl.name, name))
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
    fwrite(&ServerName, sizeof(char), strlen(ServerName) + 1, stream);
    fwrite(&ServerPort, sizeof(int), 1, stream);
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
    if (y > cur_widget[TARGET_ID].y1+3 &&
        y < cur_widget[TARGET_ID].y1+38 &&
        x > cur_widget[TARGET_ID].x1+3 &&
        x < cur_widget[TARGET_ID].x1+30)
    {
        check_keys(SDLK_c);
    }
    /* talk button */
    if (y > cur_widget[TARGET_ID].y1 + 7 &&
        y < cur_widget[TARGET_ID].y1 + 25 &&
        x > cur_widget[TARGET_ID].x1 + 223 &&
        x < cur_widget[TARGET_ID].x1 + 259)
    {
        if (cpl.target_code)
        {
            char buf[6] = "hello";

            send_talk_command(GUI_NPC_MODE_NPC, buf);
        }
    }
}

void widget_show_target(int x, int y)
{
    char       *ptr = NULL;
    SDL_Rect    box;
    double      temp;
    int         hp_tmp;

    sprite_blt(Bitmaps[BITMAP_TARGET_BG], x, y, NULL, NULL);

    sprite_blt(Bitmaps[cpl.target_mode ? BITMAP_TARGET_ATTACK : BITMAP_TARGET_NORMAL], x+5, y+4, NULL, NULL);

    sprite_blt(Bitmaps[BITMAP_TARGET_HP_B], x+4, y+24, NULL, NULL);

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
        sprite_blt(Bitmaps[BITMAP_TARGET_TALK], x + 223, y + 7, NULL, NULL);

    if (options.show_target_self || cpl.target_code != 0)
    {
        if (hp_tmp)
        {
            temp = (double) hp_tmp * 0.01;
            box.x = 0;
            box.y = 0;
            box.h = Bitmaps[BITMAP_TARGET_HP]->bitmap->h;
            box.w = (int) (Bitmaps[BITMAP_TARGET_HP]->bitmap->w * temp);
            if (!box.w)
                box.w = 1;
            if (box.w > Bitmaps[BITMAP_TARGET_HP]->bitmap->w)
                box.w = Bitmaps[BITMAP_TARGET_HP]->bitmap->w;
            sprite_blt(Bitmaps[BITMAP_TARGET_HP], x+5, y + 25, &box, NULL);
        }

        if (ptr)
        {
            /* BEGIN modified robed's HP-%-patch */
            /* Draw the name of the target */
             string_blt(ScreenSurface, &font_small, cpl.target_name, x + 35, y+3, cpl.target_color, NULL, NULL);
            /* Either draw HP remaining percent and description... */
            if (hp_tmp)
            {
                char hp_text[9];
                int hp_color;
                int xhpoffset=0;
                sprintf((char *)hp_text, "HP: %d%%", hp_tmp);
                     if (hp_tmp > 90) hp_color = COLOR_GREEN;
                else if (hp_tmp > 75) hp_color = COLOR_DGOLD;
                else if (hp_tmp > 50) hp_color = COLOR_HGOLD;
                else if (hp_tmp > 25) hp_color = COLOR_ORANGE;
                else if (hp_tmp > 10) hp_color = COLOR_YELLOW;
                else                  hp_color = COLOR_RED;

               string_blt(ScreenSurface, &font_small, hp_text, x + 35, y + 14, hp_color, NULL, NULL);
               xhpoffset=50;

                string_blt(ScreenSurface, &font_small, ptr, x + 35 + xhpoffset, y + 14, cpl.target_color, NULL, NULL);
            }
            /* ...or draw just the description */
            else
                string_blt(ScreenSurface, &font_small, ptr, x + 35, y + 14, cpl.target_color, NULL, NULL);
            /* END modified robed's HP-%-patch */
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

void reload_icons(void)
{
    int i, ii;
    char    buf[512];

    for (i = 0; i < SPELL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            if ((spell_list[i].entry[0][ii].flag != LIST_ENTRY_UNUSED) && (spell_list[i].entry[0][ii].icon_name))
            {
                if (spell_list[i].entry[0][ii].icon)
                    sprite_free_sprite(spell_list[i].entry[0][ii].icon);
               sprintf(buf,"%s%s",GetIconDirectory(),spell_list[i].entry[0][ii].icon_name);
               spell_list[i].entry[0][ii].icon=sprite_load_file(buf, SURFACE_FLAG_DISPLAYFORMAT);
            }
            if ((spell_list[i].entry[1][ii].flag != LIST_ENTRY_UNUSED) && (spell_list[i].entry[1][ii].icon_name))
            {
                if (spell_list[i].entry[1][ii].icon)
                    sprite_free_sprite(spell_list[i].entry[1][ii].icon);
               sprintf(buf,"%s%s",GetIconDirectory(),spell_list[i].entry[1][ii].icon_name);
               spell_list[i].entry[1][ii].icon=sprite_load_file(buf, SURFACE_FLAG_DISPLAYFORMAT);
            }
        }
    }

    for (i = 0; i < SKILL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            if ((skill_list[i].entry[ii].flag != LIST_ENTRY_UNUSED) && (skill_list[i].entry[ii].icon_name))
            {
                if (skill_list[i].entry[ii].icon)
                    sprite_free_sprite(skill_list[i].entry[ii].icon);
               sprintf(buf,"%s%s",GetIconDirectory(),skill_list[i].entry[ii].icon_name);
               skill_list[i].entry[ii].icon=sprite_load_file(buf, SURFACE_FLAG_DISPLAYFORMAT);
            }
        }
    }

}


