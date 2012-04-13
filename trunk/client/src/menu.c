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
        WIDGET_SHOW(WIDGET_CONSOLE_ID) = 0;
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
        WIDGET_SHOW(WIDGET_CONSOLE_ID) = 0;
    }
    else
        WIDGET_SHOW(WIDGET_CONSOLE_ID) = 1;
}

void do_number(int x, int y)
{
    if (InputStringEscFlag == 1)
    {
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        WIDGET_SHOW(WIDGET_NUMBER_ID) = 0;
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
        WIDGET_SHOW(WIDGET_NUMBER_ID) = 0;
    }
    else
        WIDGET_SHOW(WIDGET_NUMBER_ID) = 1;
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
//            wdh_process_quickslots(box.x + 120, box.y + 3);

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
    if (widget_data[WIDGET_QSLOTS_ID].ht > 34)
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
        if (x >= widget_data[WIDGET_QSLOTS_ID].x1 + quickslots_pos[i][qsx]+xoff
                && x <= widget_data[WIDGET_QSLOTS_ID].x1 + quickslots_pos[i][qsx]+xoff + 32
                && y >= widget_data[WIDGET_QSLOTS_ID].y1 + quickslots_pos[i][qsy]
                && y <= widget_data[WIDGET_QSLOTS_ID].y1 + quickslots_pos[i][qsy] + 32)
            return i;
    }
    return -1;
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

void reset_menu_status(void)
{
    if (cpl.menustatus != MENU_NO)
    {
        cpl.menustatus = MENU_NO;
    }

}
