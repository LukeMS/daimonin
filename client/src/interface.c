/* Daimonin SDL client, a client program for the Daimonin MMORPG.

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

_gui_npc *gui_npc;

static void BadInterfaceString(char *data, uint16 pos);
static uint8 ParseTag(_gui_npc_element *element, char *data, int *pos);
static char *StripCodes(char *s, const char *ct);
static void FormatBody(_font *font, uint16 width, uint8 lines, _gui_npc_element *element);
static void FineTuneButtons(void);
static uint16 PrecalcGUI(void);
static void ShowGUIBackground(uint16 x, uint16 y);
static void ShowGUIPanel(uint16 x, uint16 y);
static void ShowGUIFurniture(uint16 x, uint16 y);
static void ShowGUIContents(uint16 x, uint16 y);
static void ShowIcon(_gui_npc_element *this);
static _gui_npc_element *GetElement(int mx, int my);
static void SelectKeyword(_gui_npc_element *element);
static void SelectIcon(_gui_npc_element *element);
static void ChooseLink(uint8 n);
static void SelectLink(_gui_npc_element *element);
static void ChooseButton(char c);
static void SelectButton(_gui_npc_element *element);
static void SendCommand(void);
static uint8 ScrollGUI(sint16 dist);

/* called from commands.c after we got a interface command */
_gui_npc *gui_npc_create(int mode, char *data, int len, int pos)
{
    int  flag_start,
         flag_end;
    char c;

#ifdef DAI_DEVELOPMENT
    LOG(LOG_DEBUG, "Interface command: mode=%d len=%d, %s\n",
        mode, len, (char *)(data + pos));
#else
    LOG(LOG_DEBUG, "got gui-interface-cmd\n");
#endif
    MALLOC(gui_npc, sizeof *gui_npc);

    if (!gui_npc)
    {
        return NULL;
    }

    interface_mode = mode;

    for (flag_start = 0, flag_end = 0; len > pos; pos++)
    {
        c = *(data+pos);

        if (c == '<')
        {
            if (flag_end == 1)
            {
                /* our char before this was a '>' - now we get a '<' */
                flag_start = 0;
                flag_end = 0;
            }

            if (flag_start) /* double << ?*/
            {
                BadInterfaceString(data, pos);

                return NULL;
            }
            else
            {
                flag_start = 1;
            }
        }
        else if (c == '>')
        {
            if (flag_end)
            {
                BadInterfaceString(data, pos);

                return NULL;
            }
            else
            {
                flag_end = 1;
            }
        }
        else
        {
            /* we have a single '<' or '>'? */
            if (flag_start == 1)
            {
                _gui_npc_element *element;

                flag_start = 2;

                /* This char is a command marker */
                switch (c)
                {
                    case 'h': /* head with picture & name this interface comes from */
                        if (!gui_npc->head)
                        {
                            MALLOC(gui_npc->head, sizeof *gui_npc->head);

                            if (gui_npc->head)
                            {
                                gui_npc->head->type = GUI_NPC_HEAD;
                                gui_npc->head->last = gui_npc->head;
                            }
                        }
                        else
                        {
                            gui_npc->head->last = NULL;
                        }

                        if (!gui_npc->head ||
                            !gui_npc->head->last ||
                            !ParseTag(gui_npc->head, data, &pos))
                        {
                            gui_npc_reset();

                            return NULL;
                        }

                        break;

                    case 'm': /* title & text - what he has to say */
                        if (!gui_npc->message)
                        {
                            MALLOC(gui_npc->message, sizeof *gui_npc->message);

                            if (gui_npc->message)
                            {
                                gui_npc->message->type = GUI_NPC_MESSAGE;
                                gui_npc->message->last = gui_npc->message;
                            }
                        }
                        else
                        {
                            gui_npc->message->last = NULL;
                        }

                        if (!gui_npc->message ||
                            !gui_npc->message->last ||
                            !ParseTag(gui_npc->message, data, &pos))
                        {
                            gui_npc_reset();

                            return NULL;
                        }

                        break;

                    case 'r': /* reward info */
                        if (!gui_npc->reward)
                        {
                            MALLOC(gui_npc->reward, sizeof *gui_npc->reward);

                            if (gui_npc->reward)
                            {
                                gui_npc->reward->type = GUI_NPC_REWARD;
                                gui_npc->reward->last = gui_npc->reward;
                            }
                        }
                        else
                        {
                            gui_npc->reward->last = NULL;
                        }

                        if (!gui_npc->reward ||
                            !gui_npc->reward->last ||
                            !ParseTag(gui_npc->reward, data, &pos))
                        {
                            gui_npc_reset();

                            return NULL;
                        }

                        break;

                    case 'i': /* define a "icon" - graphical presentation of reward or message part */
                        MALLOC(element, sizeof *element);

                        if (element)
                        {
                            if (!gui_npc->icon)
                            {
                                gui_npc->icon = element;
                            }
                            else
                            {
                                element->prev = gui_npc->icon->last;
                                element->prev->next = element;
                            }

                            element->type = GUI_NPC_ICON;
                            gui_npc->icon->last = element;
                        }

                        if (!element ||
                            !ParseTag(element, data, &pos))
                        {
                            gui_npc_reset();

                            return NULL;
                        }

                        break;

                    case 'l': /* define a "link" string line */
                        MALLOC(element, sizeof *element);

                        if (element)
                        {
                            if (!gui_npc->link)
                            {
                                gui_npc->link = element;
                            }
                            else
                            {
                                element->prev = gui_npc->link->last;
                                element->prev->next = element;
                            }

                            element->type = GUI_NPC_LINK;
                            gui_npc->link->last = element;
                        }

                        if (!element ||
                            !ParseTag(element, data, &pos))
                        {
                            gui_npc_reset();

                            return NULL;
                        }

                        break;

                    case 'u': /* update (intended for qlist) */
                        MALLOC(element, sizeof *element);

                        if (element)
                        {
                            if (!gui_npc->update)
                            {
                                gui_npc->update = element;
                            }
                            else
                            {
                                element->prev = gui_npc->update->last;
                                element->prev->next = element;
                            }

                            element->type = GUI_NPC_UPDATE;
                            gui_npc->update->last = element;
                        }

                        if (!element ||
                            !ParseTag(element, data, &pos))
                        {
                            gui_npc_reset();

                            return NULL;
                        }

                        break;

                    case 'a': /* define lhsbutton button */
                        if (!gui_npc->lhsbutton)
                        {
                            MALLOC(gui_npc->lhsbutton, sizeof *gui_npc->lhsbutton);

                            if (gui_npc->lhsbutton)
                            {
                                gui_npc->lhsbutton->type = GUI_NPC_BUTTON;
                                gui_npc->lhsbutton->last = gui_npc->lhsbutton;
                            }
                        }
                        else
                        {
                            gui_npc->lhsbutton->last = NULL;
                        }

                        if (!gui_npc->lhsbutton ||
                            !gui_npc->lhsbutton->last ||
                            !ParseTag(gui_npc->lhsbutton, data, &pos))
                        {
                            gui_npc_reset();

                            return NULL;
                        }

                        break;

                    case 'd': /* define rhsbutton button */
                        if (!gui_npc->rhsbutton)
                        {
                            MALLOC(gui_npc->rhsbutton, sizeof *gui_npc->rhsbutton);

                            if (gui_npc->rhsbutton)
                            {
                                gui_npc->rhsbutton->type = GUI_NPC_BUTTON;
                                gui_npc->rhsbutton->last = gui_npc->rhsbutton;
                            }
                        }
                        else
                        {
                            gui_npc->rhsbutton->last = NULL;
                        }

                        if (!gui_npc->rhsbutton ||
                            !gui_npc->rhsbutton->last ||
                            !ParseTag(gui_npc->rhsbutton, data, &pos))
                        {
                            gui_npc_reset();

                            return NULL;
                        }

                        break;

                    case 't': /* textfield contents */
                        if (!gui_npc->textfield)
                        {
                            MALLOC(gui_npc->textfield, sizeof *gui_npc->textfield);

                            if (gui_npc->textfield)
                            {
                                gui_npc->textfield->type = GUI_NPC_TEXTFIELD;
                                gui_npc->textfield->last = gui_npc->textfield;
                            }
                        }
                        else
                        {
                            gui_npc->textfield->last = NULL;
                        }

                        if (!gui_npc->textfield ||
                            !gui_npc->textfield->last ||
                            !ParseTag(gui_npc->textfield, data, &pos))
                        {
                            gui_npc_reset();

                            return NULL;
                        }

                        break;

                    default:
                        BadInterfaceString(data, pos);

                        return NULL;
                }
            }
            else if (flag_end == 1)
            {
                flag_start = 0;
                flag_end = 0;
            }
        }
    }

    /* if we are here, we have a legal gui_npc structure.
     * Now lets create a legal formula and preprocess some structures.
     */
    if (gui_npc->head)
    {
        int i = get_bmap_id(gui_npc->head->image.name);

        /* If the image is not in FaceList, load it from the local icons
         * dir. */
        if (i == -1)
        {
            char     buf[MEDIUM_BUF];

            sprintf(buf, "%s%s.png",
                    GetIconDirectory(), gui_npc->head->image.name);

             /* Still can't find it? Bug. */
            if (!(gui_npc->head->image.sprite = sprite_load_file(buf,
                                                                 SURFACE_FLAG_DISPLAYFORMAT)))
            {
                i = 0;
                LOG(LOG_ERROR, "Can't find image '%s' in FacList or akin!\n",
                    gui_npc->head->image.name);
            }
        }

        if (i != -1)
        {
            request_face(i);
            gui_npc->head->image.sprite = FaceList[i].sprite;
        }

        gui_npc->head->image.face = i;

        /* If the interface string specifies no explicit title, default to the
         * name of the target (ie, who you are talking to). */
        if (!gui_npc->head->title &&
            cpl.target_name)
        {
            MALLOC_STRING(gui_npc->head->title, cpl.target_name);
        }
    }

    if (gui_npc->message)
    {
        FormatBody(&font_medium, GUI_NPC_WIDTH, GUI_NPC_MESSAGE_MAX_LINE,
                   gui_npc->message);
    }

    if (gui_npc->reward)
    {
        FormatBody(&font_medium, GUI_NPC_WIDTH, GUI_NPC_REWARD_MAX_LINE,
                   gui_npc->reward);
    }

    if (gui_npc->icon)
    {
        _gui_npc_element *this;

        for (this = gui_npc->icon; this; this = this->next)
        {
            int i = get_bmap_id(this->image.name);

            /* First split body_text into lines. */
            if (gui_npc->shop)
            {
                FormatBody(&font_small, GUI_NPC_WIDTH, GUI_NPC_ICON_MAX_LINE,
                           this);
            }
            else
            {
                FormatBody(&font_tiny_out,
                           GUI_NPC_WIDTH - GUI_NPC_ICONSIZE - 5,
                           GUI_NPC_ICON_MAX_LINE, this);
            }

            /* If the image is not in FaceList, load it from the local icons
             * dir. */
            if (i == -1)
            {
                char buf[MEDIUM_BUF];

                sprintf(buf, "%s%s.png", GetIconDirectory(), this->image.name);

                /* Still can't find it? Bug. */
                if (!(this->image.sprite = sprite_load_file(buf,
                                                            SURFACE_FLAG_DISPLAYFORMAT)))
                {
                    i = 0;
                    LOG(LOG_ERROR, "Can't find image '%s' in FacList or akin!\n",
                        this->image.name);
                }
            }

            if (i != -1)
            {
                request_face(i);
                this->image.sprite = FaceList[i].sprite;
            }

            this->image.face = i;

            /* if we have already come across a selectable icon and now find a
             * non-selectable one, reorder them */
            if (gui_npc->first_selectable)
            {
                if (this->mode == 'G' ||
                    this->mode == 'g')
                {
                    if ((this->prev->next = this->next))
                    {
                        this->next->prev = this->prev;
                    }
                    else
                    {
                       gui_npc->icon->last = this->prev;
                    }

                    this->prev = gui_npc->first_selectable->prev;
                    this->next = gui_npc->first_selectable;
                    gui_npc->first_selectable->prev->next = this;
                    gui_npc->first_selectable->prev = this;
                    this = this->next;
                }
            }
            /* do we have a selectable icon? */
            else if ((this->mode == 'S' ||
                      this->mode == 's'))
            {
                gui_npc->first_selectable = this;
            }
        }
    }

    if (gui_npc->update)
    {
        _gui_npc_element *this;

        for (this = gui_npc->update; this; this = this->next)
        {
            FormatBody(&font_medium, GUI_NPC_WIDTH, GUI_NPC_UPDATE_MAX_LINE,
                       this);
        }
    }

    FineTuneButtons();

    /* Prefilled (and focused) textfield */
    if (gui_npc->textfield)
    {
        gui_npc->input_flag = 1;
        reset_keys();
        open_input_mode(240);
        textwin_putstring(gui_npc->textfield->command);
        cpl.input_mode = INPUT_MODE_NPCDIALOG;
        HistoryPos = 0;
    }

    /* Turn any music playing down to 25% volume (relative). */
    if (music.data)
    {
        sound_play_music(music.name, music.vol / 4, music.fade, music.loop, 1,
                         0);
    }

    gui_npc->height = PrecalcGUI();
    cpl.menustatus = MENU_NPC;
    gui_npc->startx = (Screensize.x / 2) - (Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->w / 2);
    gui_npc->starty = (Screensize.y - 600) / 2 + 50;
    mb_clicked = 0;

    return gui_npc;
}

static void BadInterfaceString(char *data, uint16 pos)
{
    textwin_showstring(COLOR_RED, "ERROR: bad interface string (flag start error)");
    LOG(LOG_ERROR, "ERROR: bad command tag: %s\n", data + pos);
    gui_npc_reset();
}

static uint8 ParseTag(_gui_npc_element *element, char *data, int *pos)
{
    char  *cp,
           buf[HUGE_BUF],
           c;

    (*pos)++;

    while ((c = *(data + *pos)) != '\0' &&
           c)
    {
        /* c is legal string part - check it is '>' */
        if (c == '>')
        {
            if (*(data + (*pos) + 1) != '>') /* no double >>? then we return */
            {
                (*pos)--;

                return 1;
            }
        }

        (*pos)++;

        if (c <= ' ')
        {
            continue;
        }

        /* c is part of the head command inside the '<' - lets handle it
         * It must be a command. If it is unknown, return NULL
         */
        switch (c)
        {
            case 'b': /* body_text */
                if (element->type == GUI_NPC_HEAD ||
                    element->type == GUI_NPC_LINK ||
                    element->type == GUI_NPC_BUTTON ||
                    element->type == GUI_NPC_TEXTFIELD)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                MALLOC_STRING(element->body.text, cp);

                break;

            case 'c': /* command */
                if (element->type == GUI_NPC_HEAD ||
                    element->type == GUI_NPC_MESSAGE ||
                    element->type == GUI_NPC_REWARD ||
                    element->type == GUI_NPC_UPDATE)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                MALLOC_STRING(element->command, StripCodes(buf, cp));

                break;

            case 'f': /* face */
                if (element->type == GUI_NPC_MESSAGE ||
                    element->type == GUI_NPC_REWARD ||
                    element->type == GUI_NPC_UPDATE ||
                    element->type == GUI_NPC_LINK ||
                    element->type == GUI_NPC_BUTTON ||
                    element->type == GUI_NPC_TEXTFIELD)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                MALLOC_STRING(element->image.name, cp);

                break;

            case 'm': /* mode */
                if (element->type != GUI_NPC_ICON)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                element->mode = *cp;

                break;

            case 'q': /* quantity */
                if (element->type != GUI_NPC_ICON)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                element->quantity = atoi(cp);

                break;

            case 's': /* sound */
                if (element->type != GUI_NPC_HEAD)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                gui_npc->sound = atoi(cp);

                break;

            case 't': /* title */
                if (element->type == GUI_NPC_TEXTFIELD)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                if (element->type == GUI_NPC_MESSAGE ||
                    element->type == GUI_NPC_REWARD)
                {
                    sprintf(buf, "`");
                    sprintf(strchr(StripCodes(buf + 1, cp), '\0'), "`");
                    MALLOC_STRING(element->title, buf);
                }
                else
                {
                    MALLOC_STRING(element->title, StripCodes(buf, cp));
                }

                break;

            case '1': /* copper cash */
                if (element->type != GUI_NPC_REWARD)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                element->copper = atoi(cp);
                gui_npc->total_coins += ABS(element->copper);

                break;

            case '2': /* silver cash */
                if (element->type != GUI_NPC_REWARD)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                element->silver = atoi(cp);
                gui_npc->total_coins += ABS(element->silver);

                break;

            case '3': /* gold cash */
                if (element->type != GUI_NPC_REWARD)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                element->gold = atoi(cp);
                gui_npc->total_coins += ABS(element->gold);

                break;

            case '4': /* mithril cash */
                if (element->type != GUI_NPC_REWARD)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                element->mithril = atoi(cp);
                gui_npc->total_coins += ABS(element->mithril);

                break;

            case '$': /* shop */
                if (element->type != GUI_NPC_HEAD)
                {
                    return 0;
                }

                if (!(cp = get_parameter_string(data, pos, 0)))
                {
                    return 0;
                }

                gui_npc->shop = atoi(cp);

                break;

            default:
                return 0;
        }
    }

    return 0;
}

/* strip out embedded character codes */
static char *StripCodes(char *s, const char *ct)
{
    uint16 i,
           j;

    for (i = 0, j = 0; *(ct + i); i++)
    {
        if (*(ct + i) != ECC_STRONG &&
            *(ct + i) != ECC_EMPHASIS &&
            *(ct + i) != ECC_UNDERLINE &&
            *(ct + i) != ECC_HYPERTEXT)
        {
            s[j++] = *(ct + i);
        }
    }

    s[j] = '\0';

    return s;
}

static void FormatBody(_font *font, uint16 width, uint8 lines, _gui_npc_element *element)
{
    uint8  intertitle = 0,
           strong = 0,
           emphasis = 0,
           hyper = 0,
           endline = 0;
    uint16 yoff = element->box.y,
           bc = 0,
           lc = 0;
    char   buf[MEDIUM_BUF] = "";

    /* Sanity check */
    if (!element->body.text)
    {
        return;
    }

    /* Here we adjust yoff according to the element type. For message, reward,
     * and update blocks, account for the title, and for icons in a shop
     * interface, set to 0. */
    if ((element->type == GUI_NPC_MESSAGE ||
         element->type == GUI_NPC_REWARD ||
         element->type == GUI_NPC_UPDATE) &&
        element->title)
    {
        yoff += font_large_out.line_height + FONT_BLANKLINE;
    }
    else if (element->type == GUI_NPC_ICON &&
             gui_npc->shop)
    {
        yoff = USHRT_MAX;
    }

    /* Loop through body.text char-by-char. */
    for (; lc < sizeof(buf); bc++)
    {
        int len;

        switch (element->body.text[bc])
        {
            /* End of body_text? */
            case '\0':
                endline = 1;
                buf[lc] = '\0';

                break;

            /* Newline? */
            case '\n':
                endline = 1;
                buf[lc] = '\0';

                break;

            /* Carriage return? Ignore these. */
            case '\r':
                break;

            /* Intertitle (gold text) markup tag? Toggle intertitle and append
             * a tag to line. If intertitle has been switched on, force a line break. */
            case ECC_UNDERLINE:
                intertitle = !intertitle;

                if (intertitle)
                {
                    buf[lc++] = ECC_UNDERLINE;

                    if (strong)
                    {
                        strong = 0;
                        buf[lc++] = ECC_STRONG;
                    }

                    if (emphasis)
                    {
                        emphasis = 0;
                        buf[lc++] = ECC_EMPHASIS;
                    }
                }
                else
                {
                    buf[lc++] = ECC_UNDERLINE;

                    if (element->body.text[bc + 1] == '\n' ||
                        element->body.text[bc + 1] == '\0')
                    {
                        bc++;
                    }

                    endline = 1;
                    buf[lc] = '\0';
               }

                break;

            /* Strong (yellow text) markup tag? Toggle strong and append a tag
             * to line. */
            case ECC_STRONG:
                if (!intertitle)
                {
                    strong = !strong;
                    buf[lc++] = ECC_STRONG;
                }

                break;

            /* Emphasis (green text) markup tag? Toggle emphasis and append a
             * tag to line. */
            case ECC_EMPHASIS:
                if (!intertitle)
                {
                    emphasis = !emphasis;
                    buf[lc++] = ECC_EMPHASIS;
                }

                break;

            /* Hypertext (keyword) markup tag? Toggle hyper and, if this is the
             * opening tag, get the keyword. */
            case ECC_HYPERTEXT:
                hyper = !hyper;

                if (hyper)
                {
                    int  bcc,
                         kc;
                    char kbuf[MEDIUM_BUF];

                    for (bcc = bc + 1, kc = 0; ; bcc++)
                    {
                        if (element->body.text[bcc] == '\0' ||
                            element->body.text[bcc] == '\n' ||
                            element->body.text[bcc] == ECC_HYPERTEXT)
                        {
                            _gui_npc_element *this;

                            MALLOC(this, sizeof *this);

                            if (this)
                            {
                                if (!gui_npc->hypertext)
                                {
                                    gui_npc->hypertext = this;
                                }
                                else
                                {
                                    this->prev = gui_npc->hypertext->last;
                                    this->prev->next = this;
                                }

                                this->type = GUI_NPC_HYPERTEXT;
                                this->box.x = 0;
                                this->box.y = yoff;
                                this->box.w = 0;
                                this->box.h = font->line_height;
                                gui_npc->hypertext->last = this;
                                kbuf[kc] = '\0';
                                MALLOC_STRING(this->keyword, kbuf);
                            }

                            break;
                        }

                        switch (element->body.text[bcc])
                        {
                            case '\r':
                            case ECC_STRONG:
                            case ECC_EMPHASIS:
                            case ECC_UNDERLINE:
                                break;

                            default:
                                kbuf[kc++] = tolower(element->body.text[bcc]);
                        }
                    }
                }

                break;

            /* Any other character? Append it to line and check if line wants
             * to overflow the specified width. */
            default:
                /* If we're at the beginning of line and
                 * intertitle/strong/emphasis are on (ie, they were turned on
                 * in the previous line) append a tag to line. */
                if (lc == 0)
                {
                    if (intertitle)
                    {
                        buf[lc++] = ECC_UNDERLINE;
                    }

                    if (strong)
                    {
                        buf[lc++] = ECC_STRONG;
                    }

                    if (emphasis)
                    {
                        buf[lc++] = ECC_EMPHASIS; 
                    }
                }

                /* Append the character to line. */
                buf[lc++] = element->body.text[bc];
                buf[lc] = '\0';

                /* Check if line wants to overflow the specified width. */
                if (string_width_offset(font, buf, &len, width))
                {
                    int lcc;

                    /* Here we loop backwards through line, looking for a nice
                     * space at which to break the line. */
                    for (lcc = len; lcc >= 0; lcc--)
                    {
                        if (buf[lcc] == ' ')
                        {
                            break;
                        }
                        else if (buf[lcc] == ECC_UNDERLINE)
                        {
                            intertitle = !intertitle;
                        }
                        else if (buf[lcc] == ECC_STRONG)
                        {
                            strong = !strong;
                        }
                        else if (buf[lcc] == ECC_EMPHASIS)
                        {
                            emphasis = !emphasis;
                        }
                        else if (buf[lcc] == ECC_HYPERTEXT)
                        {
                            hyper = !hyper;
                        }
                    }

                    /* Must mean we couldn't find a space, so the line breaks
                     * at its end. */
                    if (lcc < 0)
                    {
                        lcc = len;
                    }

                    bc -= lc - lcc - 1;
                    lc = lcc;

                    /* If intertitle/strong/emphasis are on append a closing
                     * tag to line, but do NOT turn them off (we need the info
                     * for the next line). */
                    if (intertitle)
                    {
                        buf[lc++] = ECC_UNDERLINE;
                    }

                    if (strong)
                    {
                        buf[lc++] = ECC_STRONG;
                    }

                    if (emphasis)
                    {
                        buf[lc++] = ECC_EMPHASIS;
                    }

                    endline = 1;
                }

                buf[lc] = '\0';
        }

        if (endline)
        {
            MALLOC_STRING(element->body.line[element->body.line_count], buf);

            if (++element->body.line_count == lines ||
                !element->body.text[bc])
            {
                return;
            }

            yoff += (element->body.text[bc + 1] == '\n') ? FONT_BLANKLINE :
                    font->line_height;
            lc = 0;
            endline = 0;
        }
    }
}

/* Fiddles the buttons to guarantee a sensible layout. */
static void FineTuneButtons(void)
{
    switch (interface_mode)
    {
        case GUI_NPC_MODE_NPC:
            /* If there is no LHS button, make one. */
            if (!gui_npc->lhsbutton)
            {
                MALLOC(gui_npc->lhsbutton, sizeof *gui_npc->lhsbutton);

                if (gui_npc->lhsbutton)
                {
                    gui_npc->lhsbutton->type = GUI_NPC_BUTTON;
                    gui_npc->lhsbutton->last = gui_npc->lhsbutton;
                }
                else
                {
                    gui_npc_reset();

                    return;
                }
            }

            /* If the LHS button has no command or title, it defaults to a hello
             * button. */
            if (!gui_npc->lhsbutton->command &&
                !gui_npc->lhsbutton->title)
            {
                MALLOC_STRING(gui_npc->lhsbutton->command, "hello");
                MALLOC_STRING(gui_npc->lhsbutton->title, "Hello");
            }

            /* If the LHS button has no title, copy the command as title. */
            if (!gui_npc->lhsbutton->title &&
                gui_npc->lhsbutton->command)
            {
                if (*gui_npc->lhsbutton->command == '#')
                {
                    MALLOC_STRING(gui_npc->lhsbutton->title, gui_npc->lhsbutton->command + 1);
                }
                else
                {
                    MALLOC_STRING(gui_npc->lhsbutton->title, gui_npc->lhsbutton->command);
                }
            }

        case GUI_NPC_MODE_RHETORICAL:
            /* If there is no RHS button, make one. */
            if (!gui_npc->rhsbutton)
            {
                MALLOC(gui_npc->rhsbutton, sizeof *gui_npc->rhsbutton);

                if (gui_npc->rhsbutton)
                {
                    gui_npc->rhsbutton->type = GUI_NPC_BUTTON;
                    gui_npc->rhsbutton->last = gui_npc->rhsbutton;
                }
                else
                {
                    gui_npc_reset();

                    return;
                }
            }

            /* If the RHS button has no command or title, it defaults to a goodbye
             * button. */
            if (!gui_npc->rhsbutton->command &&
                !gui_npc->rhsbutton->title)
            {
                if (gui_npc->lhsbutton &&
                    gui_npc->lhsbutton->title &&
                    (*gui_npc->lhsbutton->title == 'G' ||
                     *gui_npc->lhsbutton->title == 'g'))
                {
                    MALLOC_STRING(gui_npc->rhsbutton->title, "Bye");
                }
                else
                {
                    MALLOC_STRING(gui_npc->rhsbutton->title, "Goodbye");
                }
            }

            /* If the RHS button has no title, copy the command as title. */
            if (!gui_npc->rhsbutton->title &&
                gui_npc->rhsbutton->command)
            {
                if (*gui_npc->rhsbutton->command == '#')
                {
                    MALLOC_STRING(gui_npc->rhsbutton->title, gui_npc->rhsbutton->command + 1);
                }
                else
                {
                    MALLOC_STRING(gui_npc->rhsbutton->title, gui_npc->rhsbutton->command);
                }
            }

            break;

        default:
            break;
    }

    /* Highlight the LHS hotkey if necessary. */
    if (gui_npc->lhsbutton)
    {
        if (isalpha(*gui_npc->lhsbutton->title))
        {
            *gui_npc->lhsbutton->title = toupper(*gui_npc->lhsbutton->title);

            /* use MALLOC and sprintf rather than MALLOC_STRING here because we
             * insert extra characters. */
            MALLOC(gui_npc->lhsbutton->title2,
                   strlen(gui_npc->lhsbutton->title) + 2 + 1);

            if (gui_npc->lhsbutton->title2)
            {
                sprintf(gui_npc->lhsbutton->title2, "~%c~%s",
                        *gui_npc->lhsbutton->title,
                        gui_npc->lhsbutton->title + 1);
            }
        }
        else
        {
            MALLOC_STRING(gui_npc->lhsbutton->title2, gui_npc->lhsbutton->title);
        }
    }

    /* Highlight the RHS hotkey if necessary. */
    if (gui_npc->rhsbutton)
    {
        if (isalpha(*gui_npc->rhsbutton->title))
        {
            *gui_npc->rhsbutton->title = toupper(*gui_npc->rhsbutton->title);

            /* use MALLOC and sprintf rather than MALLOC_STRING here because we
             * insert extra characters. */
            MALLOC(gui_npc->rhsbutton->title2,
                   strlen(gui_npc->rhsbutton->title) + 2 + 1);

            if (gui_npc->rhsbutton->title2)
            {
                sprintf(gui_npc->rhsbutton->title2, "~%c~%s",
                        *gui_npc->rhsbutton->title,
                        gui_npc->rhsbutton->title + 1);
            }
        }
        else
        {
            MALLOC_STRING(gui_npc->rhsbutton->title2, gui_npc->rhsbutton->title);
        }
    }
}

/* clear & reset the gui interface */
/* Alderan 2007-11-08: 'free'ing a sprite won't free all stuff, we need to free also the surfaces
 * with the function sprite_free_sprite
 */
void gui_npc_reset(void)
{
    map_udate_flag = 2;
    interface_mode = GUI_NPC_MODE_NO;

    if (gui_npc)
    {
        /* Hypertext. */
        if (gui_npc->hypertext)
        {
             _gui_npc_element *this = gui_npc->hypertext->last,
                              *prev;

            for (; this; this = prev)
            {
                prev = this->prev;
                FREE(this->keyword);
                FREE(this);
            }
        }

        /* Head. */
        if (gui_npc->head)
        {
            FREE(gui_npc->head->title);
            FREE(gui_npc->head->image.name);

            if (gui_npc->head->image.sprite)
            {
                /* Don't free a sprite from FaceList! */
                if (gui_npc->head->image.face == -1)
                {
                    sprite_free_sprite(gui_npc->head->image.sprite);
                }

                gui_npc->head->image.sprite = NULL;
            }

            FREE(gui_npc->head);
        }

        /* Message. */
        if (gui_npc->message)
        {
            FREE(gui_npc->message->title);

            while (gui_npc->message->body.line_count)
            {
                FREE(gui_npc->message->body.line[gui_npc->message->body.line_count]);
                gui_npc->message->body.line_count--;
            }

            FREE(gui_npc->message);
        }

        /* Reward. */
        if (gui_npc->reward)
        {
            FREE(gui_npc->reward->title);

            while (gui_npc->reward->body.line_count)
            {
                FREE(gui_npc->reward->body.line[gui_npc->reward->body.line_count]);
                gui_npc->reward->body.line_count--;
            }

            FREE(gui_npc->reward);
        }

        /* Icons. */
        if (gui_npc->icon)
        {
            _gui_npc_element *this = gui_npc->icon,
                             *next;

            for (; this; this = next)
            {
                next = this->next;
                FREE(this->title);
                FREE(this->command);

                while (this->body.line_count)
                {
                    FREE(this->body.line[this->body.line_count]);
                    this->body.line_count--;
                }

                if (this->image.sprite)
                {
                    /* Don't free a sprite from FaceList! */
                    if (this->image.face == -1)
                    {
                        sprite_free_sprite(this->image.sprite);
                    }

                    this->image.sprite = NULL;
                }

                FREE(this);
            }
        }

        /* Links. */
        if (gui_npc->link)
        {
            _gui_npc_element *this = gui_npc->link,
                             *next;

            for (; this; this = next)
            {
                next = this->next;
                FREE(this->title);
                FREE(this->command);
                FREE(this);
            }
        }

        /* Updates. */
        if (gui_npc->update)
        {
            _gui_npc_element *this = gui_npc->update,
                             *next;

            for (; this; this = next)
            {
                next = this->next;
                FREE(this->title);

                while (this->body.line_count)
                {
                    FREE(this->body.line[this->body.line_count]);
                    this->body.line_count--;
                }

                FREE(this);
            }
        }

        /* LHS Button. */
        if (gui_npc->lhsbutton)
        {
            FREE(gui_npc->lhsbutton->title);
            FREE(gui_npc->lhsbutton->title2);
            FREE(gui_npc->lhsbutton->command);
            FREE(gui_npc->lhsbutton);
        }

        /* RHS Button. */
        if (gui_npc->rhsbutton)
        {
            FREE(gui_npc->rhsbutton->title);
            FREE(gui_npc->rhsbutton->title2);
            FREE(gui_npc->rhsbutton->command);
            FREE(gui_npc->rhsbutton);
        }

        /* Textfield. */
        if (gui_npc->textfield)
        {
            FREE(gui_npc->textfield->command);
            FREE(gui_npc->textfield);
        }

        FREE(gui_npc);
    }

    reset_keys();
    cpl.input_mode = INPUT_MODE_NO;

    if (cpl.menustatus == MENU_NPC)
    {
        cpl.menustatus = MENU_NO;
    }

    /* Restore any music playing to full volume (according to options). */
    if (music.data)
    {
        sound_play_music(music.name, options.music_volume, music.fade,
                         music.loop, 1, 0);
    }
}

static uint16 PrecalcGUI(void)
{
    uint16 xoff = 0,
           yoff = 0;
    uint8  i;

    if (gui_npc->message)
    {
        gui_npc->message->box.x = 0;
        gui_npc->message->box.y = yoff;

        if (gui_npc->message->title)
        {
            yoff += font_large_out.line_height + FONT_BLANKLINE;
        }

        for (i = 0; i < gui_npc->message->body.line_count; i++)
        {
            yoff += (*gui_npc->message->body.line[i]) ?
                    font_medium.line_height : FONT_BLANKLINE;
        }

        gui_npc->message->box.w = GUI_NPC_WIDTH;
        gui_npc->message->box.h = yoff - gui_npc->message->box.y;
        yoff += FONT_BLANKLINE * 2;
    }

    if (gui_npc->reward)
    {
        gui_npc->reward->box.x = 0;
        gui_npc->reward->box.y = yoff;

        yoff += font_large_out.line_height + FONT_BLANKLINE;

        for (i = 0; i < gui_npc->reward->body.line_count; i++)
        {
            yoff += (*gui_npc->reward->body.line[i]) ?
                    font_medium.line_height : FONT_BLANKLINE;
        }

        if (gui_npc->reward->copper ||
            gui_npc->reward->gold ||
            gui_npc->reward->silver ||
            gui_npc->reward->mithril)
        {
            if (gui_npc->reward->body.line_count)
            {
                yoff += FONT_BLANKLINE;
            }

            yoff += GUI_NPC_ICONSIZE;
        }

        gui_npc->reward->box.w = GUI_NPC_WIDTH;
        gui_npc->reward->box.h = yoff - gui_npc->reward->box.y;
        yoff += FONT_BLANKLINE * 2;
    }

    if (gui_npc->icon)
    {
        _gui_npc_element *this = gui_npc->icon;

        for (; this && (this->mode == 'g' || this->mode == 'G');
             this = this->next)
        {
            this->box.x = 0;
            this->box.y = yoff;
            this->box.w = GUI_NPC_ICONSIZE;
            this->box.h = GUI_NPC_ICONSIZE;
            yoff += GUI_NPC_ICONSIZE + FONT_BLANKLINE;
        }

        if (this)
        {
            yoff += font_tiny_out.line_height + FONT_BLANKLINE;

            for (i = 0; this; this = this->next,
                 i += (gui_npc->shop) ? 1 : GUI_NPC_ICONSHOP)
            {
                if (i)
                {
                    if (i % GUI_NPC_ICONSHOP == 0)
                    {
                        xoff = 0;
                        yoff += GUI_NPC_ICONSIZE + FONT_BLANKLINE;
                    }
                    else
                    {
                        xoff += GUI_NPC_ICONSIZE + 6;
                    }
                }

                this->box.x = xoff;
                this->box.y = yoff;
                this->box.w = GUI_NPC_ICONSIZE;
                this->box.h = GUI_NPC_ICONSIZE;
            }

            yoff += font_tiny_out.line_height + GUI_NPC_ICONSIZE +
                    FONT_BLANKLINE;
        }

        yoff += FONT_BLANKLINE * 2;
    }

    if (gui_npc->link)
    {
        _gui_npc_element *this = gui_npc->link;

        for (; this; this = this->next)
        {
            this->box.x = 0;
            this->box.y = yoff;
            this->box.w = MIN(GUI_NPC_WIDTH, string_width(&font_medium,
                                                         this->title));
            this->box.h = font_medium.line_height;
            yoff += font_medium.line_height + FONT_BLANKLINE;
        }

        yoff += FONT_BLANKLINE * 2;
    }

    if (gui_npc->update)
    {
        _gui_npc_element *this = gui_npc->update;

        for (; this; this = this->next)
        {
            this->box.x = 0;
            this->box.y = yoff;

            if (this->title)
            {
                yoff += font_large_out.line_height + FONT_BLANKLINE;
            }

            for (i = 0; i < this->body.line_count; i++)
            {
                yoff += (*this->body.line[i]) ?
                        font_medium.line_height : FONT_BLANKLINE;
            }

            this->box.w = GUI_NPC_WIDTH;
            this->box.h = yoff - this->box.y;
            yoff += FONT_BLANKLINE * 2;
        }
    }

    return yoff;
}

/* show npc interface. ATM its included in the menu system, but
 * we need to crate a lower layer level for it. */
void gui_npc_show(void)
{
    ShowGUIBackground(gui_npc->startx, gui_npc->starty);

    if (gui_npc->hypertext)
    {
        if (options.keyword_panel == 1) /* left */
        {
            ShowGUIPanel(gui_npc->startx -
                         Bitmaps[BITMAP_GUI_NPC_PANEL]->bitmap->w + 23,
                         gui_npc->starty + GUI_NPC_TOPMARGIN - 6);
        }
        else if (options.keyword_panel == 2) /* right */
        {
            ShowGUIPanel(gui_npc->startx +
                         Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->w - 5,
                         gui_npc->starty +  GUI_NPC_TOPMARGIN - 6);
        }
    }

    add_close_button(gui_npc->startx - 116, gui_npc->starty + 5, MENU_NPC,
                     skindef.newclosebutton);


    /* When clicked, the close button obviously closes the GUI which frees the
     * structure, so only draw the rest if there is anything left to draw! */
    if (gui_npc)
    {
        ShowGUIFurniture(gui_npc->startx, gui_npc->starty);
        ShowGUIContents(gui_npc->startx + GUI_NPC_LEFTMARGIN,
                        gui_npc->starty + GUI_NPC_TOPMARGIN);
    }
}

static void ShowGUIBackground(uint16 x, uint16 y)
{
    sprite_blt(Bitmaps[BITMAP_GUI_NPC_TOP], x, y, NULL, NULL);
    y += Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->h;

    if (gui_npc->shop)
    {
        sprite_blt(Bitmaps[BITMAP_GUI_NPC_MIDDLE], x, y, NULL, NULL);
        y += Bitmaps[BITMAP_GUI_NPC_MIDDLE]->bitmap->h;
    }

    sprite_blt(Bitmaps[BITMAP_GUI_NPC_BOTTOM], x, y, NULL, NULL);
}

static void ShowGUIPanel(uint16 x, uint16 y)
{
    _gui_npc_element *this = gui_npc->hypertext;
    uint8             i = 0;

    sprite_blt(Bitmaps[BITMAP_GUI_NPC_PANEL], x, y, NULL, NULL);

    for (; this; this = this->next, i++)
    {
        char   buf[SMALL_BUF];
        int    len;
        uint16 ch = font_medium.line_height,
               cw = 16;

        sprintf(buf, "%s", this->keyword);

        if (string_width_offset(&font_medium, buf, &len,
            Bitmaps[BITMAP_GUI_NPC_PANEL]->bitmap->w - cw))
        {
            buf[len - 2] = '\0';
            strcat(buf, "...");
        }

        if (gui_npc->keyword_selected == this)
        {
            string_blt(ScreenSurface, &font_medium, buf, x + cw / 2,
                       y + ch / 2 + ch * i, COLOR_DK_NAVY, NULL, NULL);
        }
        else
        {
            string_blt(ScreenSurface, &font_medium, buf, x + cw / 2,
                      y + ch / 2 + ch * i, COLOR_TURQUOISE, NULL, NULL);
        }
    }
}

static void ShowGUIFurniture(uint16 x, uint16 y)
{
    SDL_Rect box;
    char     buf[SMALL_BUF];
    int      len;
    uint16   xoff,
             yoff;

    if (gui_npc->head)
    {
        if (gui_npc->head->title)
        {
            sprintf(buf, "%s", gui_npc->head->title);

            if (string_width_offset(&font_medium, buf, &len, 260))
            {
                buf[len - 2] = '\0';
                strcat(buf, "...");
            }

            string_blt(ScreenSurface, &font_medium, buf, x + 80, y + 50,
                      COLOR_WHITE, NULL, NULL);
        }

        if (gui_npc->head->image.sprite)
        {
            _Sprite  *sprite;

            box.x = x + 5;
            box.y = y + 5;
            box.w = 54;
            box.h = 54;
//            SDL_SetClipRect(ScreenSurface, &box);
            sprite = gui_npc->head->image.sprite;
            xoff = box.x + box.w / 2 -
                   (sprite->bitmap->w - sprite->border_left) / 2 -
                   sprite->border_left;
            yoff = box.y + box.h / 2 -
                   (sprite->bitmap->h - sprite->border_down) / 2;
            sprite_blt(sprite, xoff, yoff, NULL, NULL);
//            SDL_SetClipRect(ScreenSurface, NULL);
        }
    }

    blt_window_slider(Bitmaps[BITMAP_NPC_INT_SLIDER], gui_npc->height,
                      GUI_NPC_HEIGHT, gui_npc->yoff, -1, x + 341, y + 90);

    if (gui_npc->lhsbutton)
    {
        xoff = x + GUI_NPC_LEFTMARGIN;
        yoff = y + Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->h + ((gui_npc->shop) ?
               Bitmaps[BITMAP_GUI_NPC_MIDDLE]->bitmap->h : 0) - 1;

        /* Button title. */
        sprintf(buf, "%s", gui_npc->lhsbutton->title);

        if (string_width_offset(&font_small, buf, &len, GUI_NPC_BUTTONTEXT))
        {
            buf[len - 2] = '\0';
            strcat(buf, "...");
        }

        /* Button bg. */
        if (gui_npc->lhsbutton->command &&
            *gui_npc->lhsbutton->command == '#')
        {
            (void)add_button(xoff + 4, yoff + 4, 0,
                             BITMAP_DIALOG_BUTTON_UP_PREFIX, buf,
                             gui_npc->lhsbutton->title2);
        }
        else
        {
            (void)add_button(xoff + 4, yoff + 4, 0, BITMAP_DIALOG_BUTTON_UP,
                             buf, gui_npc->lhsbutton->title2);
        }

        /* Button fg (frame if selected). */
        if (gui_npc->button_selected == gui_npc->lhsbutton)
        {
            sprite_blt(Bitmaps[BITMAP_DIALOG_BUTTON_SELECTED], xoff, yoff,
                       NULL, NULL);
        }
    }

    if (gui_npc->rhsbutton)
    {
        xoff = x + GUI_NPC_LEFTMARGIN + GUI_NPC_WIDTH - GUI_NPC_BUTTONWIDTH;
        yoff = y + Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->h + ((gui_npc->shop) ?
               Bitmaps[BITMAP_GUI_NPC_MIDDLE]->bitmap->h : 0) - 1;

        /* Button title. */
        sprintf(buf, "%s", gui_npc->rhsbutton->title);

        if (string_width_offset(&font_small, buf, &len, GUI_NPC_BUTTONTEXT))
        {
            buf[len - 2] = '\0';
            strcat(buf, "...");
        }

        /* Button bg. */
        if (gui_npc->rhsbutton->command &&
            *gui_npc->rhsbutton->command == '#')
        {
            (void)add_button(xoff + 4, yoff + 4, 0,
                             BITMAP_DIALOG_BUTTON_UP_PREFIX, buf,
                             gui_npc->rhsbutton->title2);
        }
        else
        {
            (void)add_button(xoff + 4, yoff + 4, 0, BITMAP_DIALOG_BUTTON_UP,
                             buf, gui_npc->rhsbutton->title2);
        }

        /* Button fg (frame if selected). */
        if (gui_npc->button_selected == gui_npc->rhsbutton)
        {
            sprite_blt(Bitmaps[BITMAP_DIALOG_BUTTON_SELECTED], xoff, yoff,
                       NULL, NULL);
        }
    }

    if (interface_mode != GUI_NPC_MODE_RHETORICAL)
    {
        xoff = x + GUI_NPC_LEFTMARGIN + GUI_NPC_BUTTONWIDTH;
        yoff = y + Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->h + ((gui_npc->shop) ?
                   Bitmaps[BITMAP_GUI_NPC_MIDDLE]->bitmap->h : 0) - 1;
        box.x = xoff + 4;
        box.y = yoff - 2 + font_small.line_height;
        box.h = font_medium.line_height;
        box.w = GUI_NPC_TEXTFIELDWIDTH;

        if (gui_npc->input_flag)
        {
            sprintf(buf, "~RETURN~ to send, ~ESCAPE~ to cancel");
            string_blt(ScreenSurface, &font_small, buf,
                      xoff + 4 + (GUI_NPC_TEXTFIELDWIDTH -
                                  string_width(&font_small, buf)) / 2,
                      yoff - 2, COLOR_WHITE, NULL, NULL);
            SDL_FillRect(ScreenSurface, &box, 0);
            string_blt(ScreenSurface, &font_medium,
                      show_input_string(InputString, &font_medium, box.w - 1),
                      box.x, box.y, COLOR_WHITE, NULL, NULL);
        }
        else if (interface_mode == GUI_NPC_MODE_NPC &&
                 !gui_npc->keyword_selected &&
                 !gui_npc->icon_selected &&
                 !gui_npc->link_selected &&
                 !gui_npc->button_selected)
        {
            sprintf(buf, "~BACKSPACE~ to talk");
            string_blt(ScreenSurface, &font_small, buf,
                      xoff + 4 + (GUI_NPC_TEXTFIELDWIDTH -
                                  string_width(&font_small, buf)) / 2,
                      yoff - 2, COLOR_WHITE, NULL, NULL);
        }
        else
        {
            if (interface_mode == GUI_NPC_MODE_QUEST)
            {
                sprintf(buf, "~RETURN~ to send");
                string_blt(ScreenSurface, &font_small, buf,
                          xoff + 4 + (GUI_NPC_TEXTFIELDWIDTH -
                                      string_width(&font_small, buf)) / 2,
                          yoff - 2, COLOR_WHITE, NULL, NULL);
            }
            else
            {
                sprintf(buf, "~RETURN~ to send, ~BACKSPACE~ to overwrite");
                string_blt(ScreenSurface, &font_small, buf,
                          xoff + 4 + (GUI_NPC_TEXTFIELDWIDTH -
                                      string_width(&font_small, buf)) / 2,
                          yoff - 2, COLOR_WHITE, NULL, NULL);
            }

            /* TODO: the rest of this function is in severe need of tidying! */
            /* A selected keyword overrides everything else. */
            if (gui_npc->keyword_selected)
            {
                int  c;

                sprintf(buf, "%s", gui_npc->keyword_selected->keyword);
                strcpy(buf, normalize_string(buf));

                for (c = 0; *(buf + c); c++)
                {
                    *(buf + c) = tolower(*(buf + c));
                }

                if (string_width_offset(&font_medium, buf, &len,
                                      GUI_NPC_TEXTFIELDWIDTH))
                {
                    char buf_tmp[SMALL_BUF];

                    strncpy(buf_tmp, buf, len - 2);
                    buf_tmp[len - 2] = '\0';
                    strcat(buf_tmp, "...");
                    string_blt(ScreenSurface, &font_medium, buf_tmp, box.x,
                               box.y, COLOR_DK_NAVY, &box, NULL);
                }
                else
                {
                    string_blt(ScreenSurface, &font_medium, buf, box.x, box.y,
                               COLOR_DK_NAVY, &box, NULL);
                }
            }
            else
            {
                char  cmd[SMALL_BUF] = "",
                      btn[SMALL_BUF] = "";
                int   c;
                _gui_npc_element *button = NULL;

                /* Check for a selected icon or link-> */
                if (gui_npc->icon_selected)
                {
                    if (gui_npc->icon_selected->command)
                    {
                        int off;

                        switch (*gui_npc->icon_selected->command)
                        {
                            case '<': /* default to LHS button */
                                if (!gui_npc->button_selected)
                                {
                                    if (gui_npc->lhsbutton &&
                                        gui_npc->lhsbutton->command &&
                                        *gui_npc->lhsbutton->command == '#')
                                    {
                                        button = gui_npc->lhsbutton;
                                    }
                                    else if (gui_npc->rhsbutton &&
                                             gui_npc->rhsbutton->command &&
                                             *gui_npc->rhsbutton->command == '#')
                                    {
                                        button = gui_npc->rhsbutton;
                                    }
                                }

                                off = 1;

                                break;

                            case '-': /* default to no button */
                                off = 1;

                                break;

                            default: /* default to RHS button */
                                if (!gui_npc->button_selected)
                                {
                                    if (gui_npc->rhsbutton &&
                                        gui_npc->rhsbutton->command &&
                                        *gui_npc->rhsbutton->command == '#')
                                    {
                                        button = gui_npc->rhsbutton;
                                    }
                                    else if (gui_npc->lhsbutton &&
                                             gui_npc->lhsbutton->command &&
                                             *gui_npc->lhsbutton->command == '#')
                                    {
                                        button = gui_npc->lhsbutton;
                                    }
                                }

                                off = (gui_npc->icon_selected->command &&
                                       *gui_npc->icon_selected->command == '>') ?
                                      1 : 0;
                        }

                        sprintf(cmd, "%s", gui_npc->icon_selected->command + off);
                    }
                    else
                    {
                        _gui_npc_element *this = gui_npc->icon;
                        uint8             i = 1;

                        if (!gui_npc->button_selected)
                        {
                            if (gui_npc->rhsbutton &&
                                gui_npc->rhsbutton->command &&
                                *gui_npc->rhsbutton->command == '#')
                            {
                                button = gui_npc->rhsbutton;
                            }
                            else if (gui_npc->lhsbutton &&
                                     gui_npc->lhsbutton->command &&
                                     *gui_npc->lhsbutton->command == '#')
                            {
                                button = gui_npc->lhsbutton;
                            }
                        }

                        for (; this && this != gui_npc->icon_selected;
                             this = this->next, i++)
                        {
                            ;
                        }

                        if (this == gui_npc->icon_selected)
                        {
                            sprintf(cmd, "#%d", i);
                        }
                        else
                        {
                            // TODO
                        }
                    }
                }
                else if (gui_npc->link_selected)
                {
                    int off;

                    switch (*gui_npc->link_selected->command)
                    {
                        case '<': /* default to LHS button */
                            if (!gui_npc->button_selected)
                            {
                                if (gui_npc->lhsbutton &&
                                    gui_npc->lhsbutton->command &&
                                    *gui_npc->lhsbutton->command == '#')
                                {
                                    button = gui_npc->lhsbutton;
                                }
                                else if (gui_npc->rhsbutton &&
                                         gui_npc->rhsbutton->command &&
                                         *gui_npc->rhsbutton->command == '#')
                                {
                                    button = gui_npc->rhsbutton;
                                }
                            }

                            off = 1;

                            break;

                        case '-': /* default to no button */
                            off = 1;

                            break;

                        default: /* default to RHS button */
                            if (!gui_npc->button_selected)
                            {
                                if (gui_npc->rhsbutton &&
                                    gui_npc->rhsbutton->command &&
                                    *gui_npc->rhsbutton->command == '#')
                                {
                                    button = gui_npc->rhsbutton;
                                }
                                else if (gui_npc->lhsbutton &&
                                         gui_npc->lhsbutton->command &&
                                         *gui_npc->lhsbutton->command == '#')
                                {
                                    button = gui_npc->lhsbutton;
                                }
                            }

                            off = (gui_npc->link_selected &&
                                   gui_npc->link_selected->command &&
                                   *gui_npc->link_selected->command == '>') ?
                                  1 : 0;
                    }

                    sprintf(cmd, "%s", gui_npc->link_selected->command + off); // off might not be needed
                }

                /* Check for a selected button. */
                if (gui_npc->rhsbutton &&
                    (gui_npc->button_selected == gui_npc->rhsbutton ||
                     button == gui_npc->rhsbutton))
                {
                    if (!gui_npc->rhsbutton->command)
                    {
                        sprintf(btn, "%s", gui_npc->rhsbutton->title);
                    }
                    else
                    {
                        int off;

                        off = (*gui_npc->rhsbutton->command == '#') ? 1 : 0;

                        sprintf(btn, "%s", gui_npc->rhsbutton->command + off);
                    }
                }
                else if (gui_npc->lhsbutton &&
                         (gui_npc->button_selected == gui_npc->lhsbutton ||
                          button == gui_npc->lhsbutton))
                {
                    if (!gui_npc->lhsbutton->command)
                    {
                        sprintf(btn, "%s", gui_npc->lhsbutton->title);
                    }
                    else
                    {
                        int off;

                        off = (*gui_npc->lhsbutton->command == '#') ? 1 : 0;

                        sprintf(btn, "%s", gui_npc->lhsbutton->command + off);
                    }
                }

                strcpy(btn, normalize_string(btn));

                for (c = 0; *(btn + c); c++)
                {
                    *(btn + c) = tolower(*(btn + c));
                }

                if (string_width_offset(&font_medium, btn, &len,
                                      GUI_NPC_TEXTFIELDWIDTH))
                {
                    char buf_tmp[SMALL_BUF];

                    strncpy(buf_tmp, btn, len - 2);
                    buf_tmp[len - 2] = '\0';
                    strcat(buf_tmp, "...");

                    if (!gui_npc->button_selected &&
                        button)
                    {
                        string_blt(ScreenSurface, &font_medium, buf_tmp, box.x,
                                  box.y, COLOR_GREY, &box, NULL);
                    }
                    else if (gui_npc->button_selected ||
                             button)
                    {
                        string_blt(ScreenSurface, &font_medium, buf_tmp, box.x,
                                  box.y, COLOR_DK_NAVY, &box, NULL);
                    }
                }
                else
                {
                    uint16 xoff2;

                    xoff2 = string_width(&font_medium, btn) +
                            string_width(&font_medium, " ");

                    if (!gui_npc->button_selected &&
                        button)
                    {
                        string_blt(ScreenSurface, &font_medium, btn, box.x, box.y,
                                  COLOR_GREY, &box, NULL);
                    }
                    else if (gui_npc->button_selected ||
                             button)
                    {
                        string_blt(ScreenSurface, &font_medium, btn, box.x, box.y,
                                  COLOR_DK_NAVY, &box, NULL);
                    }

                    if (cmd[0])
                    {
                        strcpy(cmd, normalize_string(cmd));

                        for (c = 0; *(cmd + c); c++)
                        {
                            *(cmd + c) = tolower(*(cmd + c));
                        }

                        if (string_width_offset(&font_medium, cmd, &len, box.w - xoff2))
                        {
                            char buf_tmp[SMALL_BUF];

                            strncpy(buf_tmp, cmd, len - 2);
                            buf_tmp[len - 2] = '\0';
                            strcat(buf_tmp, "...");
                            string_blt(ScreenSurface, &font_medium, buf_tmp,
                                      box.x + xoff2, box.y, COLOR_DK_NAVY,
                                      &box, NULL);
                        }
                        else
                        {
                            string_blt(ScreenSurface, &font_medium, cmd,
                                      box.x + xoff2, box.y, COLOR_DK_NAVY,
                                      &box, NULL);
                        }
                    }
                }
            }
        }
    }
}

static void ShowGUIContents(uint16 x, uint16 y)
{
    SDL_Rect box;
    uint16   guitop = gui_npc->yoff,
             guibot = guitop + GUI_NPC_HEIGHT,
             xoff,
             yoff;
    char     buf[SMALL_BUF];
    int      len;

    box.x = x;
    box.y = y;
    box.w = GUI_NPC_WIDTH;
    box.h = GUI_NPC_HEIGHT;
    SDL_SetClipRect(ScreenSurface, &box);

    if (gui_npc->message)
    {
        if (gui_npc->message->box.y <= guibot &&
            gui_npc->message->box.y + gui_npc->message->box.h >= guitop)
        {
            uint8  i;

            xoff = x + gui_npc->message->box.x;
            yoff = y + gui_npc->message->box.y - guitop;

            if (gui_npc->message->title)
            {
                sprintf(buf, "%s", gui_npc->message->title);

                if (string_width_offset(&font_large_out, buf, &len, GUI_NPC_WIDTH))
                {
                    buf[len - 2] = '\0';
                    strcat(buf, "...");
                }

                string_blt(ScreenSurface, &font_large_out, buf, xoff, yoff,
                          COLOR_HGOLD, NULL, NULL);
                yoff += font_large_out.line_height + FONT_BLANKLINE;
            }

            for (i = 0; i < gui_npc->message->body.line_count; i++)
            {
                if (!*gui_npc->message->body.line[i])
                {
                    yoff += FONT_BLANKLINE;
                }
                else
                {
                    string_blt(ScreenSurface, &font_medium,
                              gui_npc->message->body.line[i], xoff, yoff,
                              COLOR_WHITE, NULL, NULL);
                    yoff += font_medium.line_height;
                }
            }
        }
    }

    if (gui_npc->reward)
    {
        if (gui_npc->reward->box.y <= guibot &&
            gui_npc->reward->box.y + gui_npc->reward->box.h >= guitop)
        {
            xoff = x + gui_npc->reward->box.x;
            yoff = y + gui_npc->reward->box.y - guitop;

            if (!gui_npc->reward->title)
            {
                sprintf(buf, "Description"); /* default title */
            }
            else
            {
                sprintf(buf, "%s", gui_npc->reward->title);
            }

            if (string_width_offset(&font_large_out, buf, &len, GUI_NPC_WIDTH))
            {
                buf[len - 2] = '\0';
                strcat(buf, "...");
            }

            string_blt(ScreenSurface, &font_large_out, buf, xoff, yoff,
                      COLOR_HGOLD, NULL, NULL);
            yoff += font_large_out.line_height + FONT_BLANKLINE;

            if (gui_npc->reward->body.text)
            {
                uint8 i;

                for (i = 0; i < gui_npc->reward->body.line_count; i++)
                {
                    if (!*gui_npc->reward->body.line[i])
                    {
                        yoff += FONT_BLANKLINE;
                    }
                    else
                    {
                        string_blt(ScreenSurface, &font_medium,
                                  gui_npc->reward->body.line[i], xoff, yoff,
                                  COLOR_WHITE, NULL, NULL);
                        yoff += font_medium.line_height;
                    }
                }
            }

            if (gui_npc->reward->copper ||
                gui_npc->reward->gold ||
                gui_npc->reward->silver ||
                gui_npc->reward->mithril)
            {
                uint8    i = 0;
                int      coins[DENOMINATIONS],
                         id;
                _Sprite *sprites[DENOMINATIONS];

                coins[i] = gui_npc->reward->copper;
                sprintf(buf, "coppercoin.101");
                id = get_bmap_id(buf);

                if (id == -1)
                {
                    id = 0;
                    LOG(LOG_ERROR, "Can't find image '%s'!\n", buf);
                }
                else
                {
                    request_face(id);
                }

                sprites[i++] = FaceList[id].sprite;
                coins[i] = gui_npc->reward->silver;
                sprintf(buf, "silvercoin.101");
                id = get_bmap_id(buf);

                if (id == -1)
                {
                    id = 0;
                    LOG(LOG_ERROR, "Can't find image '%s'!\n", buf);
                }
                else
                {
                    request_face(id);
                }

                sprites[i++] = FaceList[id].sprite;
                coins[i] = gui_npc->reward->gold;
                sprintf(buf, "goldcoin.101");
                id = get_bmap_id(buf);

                if (id == -1)
                {
                    id = 0;
                    LOG(LOG_ERROR, "Can't find image '%s'!\n", buf);
                }
                else
                {
                    request_face(id);
                }

                sprites[i++] = FaceList[id].sprite;
                coins[i] = gui_npc->reward->mithril;
                sprintf(buf, "mit_coin.101");
                id = get_bmap_id(buf);

                if (id == -1)
                {
                    id = 0;
                    LOG(LOG_ERROR, "Can't find image '%s'!\n", buf);
                }
                else
                {
                    request_face(id);
                }

                sprites[i++] = FaceList[id].sprite;

                if (gui_npc->reward->body.line_count)
                {
                    yoff += FONT_BLANKLINE;
                }

                for (i = 0; i < DENOMINATIONS; i++) // 4 denominations
                {
                    if (coins[i] != 0)
                    {
                        box.x = x + gui_npc->reward->box.x +
                               (GUI_NPC_WIDTH / 4) * i;
                        box.y = yoff + 9;
                        box.w = sprites[i]->bitmap->w;
                        box.h = sprites[i]->bitmap->h;
//                        SDL_SetClipRect(ScreenSurface, &box);
                        xoff = box.x + box.w / 2 -
                               (sprites[i]->bitmap->w - sprites[i]->border_left) / 2 -
                               sprites[i]->border_left;
                        sprite_blt(sprites[i], xoff, yoff + 9, NULL, NULL);
//                        SDL_SetClipRect(ScreenSurface, NULL);

                        if (gui_npc->shop)
                        {
                            if (coins[i] > 9999 ||
                                coins[i] < -9999)
                            {
                                sprintf(buf, "many");
                            }
                            else
                            {
                                sprintf(buf, "%d", coins[i]);
                            }
                        }
                        else
                        {
                            if (coins[i] > 9999)
                            {
                                sprintf(buf, "+many");
                            }
                            else if (coins[i] < -9999)
                            {
                                sprintf(buf, "-many");
                            }
                            else
                            {
                                sprintf(buf, "%+d", coins[i]);
                            }
                        }

                        if (coins[i] < 0)
                        {
                            uint16 w = string_width(&font_small_out, buf);

                            string_blt(ScreenSurface, &font_small_out, buf,
                                      xoff + 28 - w / 2, yoff + 18, COLOR_RED, NULL,
                                      NULL);
                        }
                        else
                        {
                            uint16 w = string_width(&font_small_out, buf);

                            string_blt(ScreenSurface, &font_small_out, buf,
                                      xoff + 28 - w / 2, yoff + 18, COLOR_GREEN, NULL,
                                      NULL);
                        }
                    }
                }

                /* Play a coins sound depending on gui_npc->total_coins.
                 * Reset head->sound to 0 afterwards to prevent a constant
                 * loop (this function is called repeatedly as long as the
                 * interface remains open. */
                if (gui_npc->sound)
                {
                    if (gui_npc->total_coins > 500)
                    {
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_COINS4, 0, 0,
                                          MENU_SOUND_VOL);
                    }
                    else if (gui_npc->total_coins > 100)
                    {
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_COINS3, 0, 0,
                                          MENU_SOUND_VOL);
                    }
                    else if (gui_npc->total_coins > 50)
                    {
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_COINS2, 0, 0,
                                          MENU_SOUND_VOL);
                    }
                    else if (gui_npc->total_coins > 0)
                    {
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_COINS1, 0, 0,
                                          MENU_SOUND_VOL);
                    }

                    gui_npc->sound = 0;
                }
            }
        }
    }

    if (gui_npc->icon)
    {
        _gui_npc_element *this = gui_npc->icon;

        for (; this && (this->mode == 'g' || this->mode == 'G');
             this = this->next)
        {
            if (this->box.y > guibot ||
                this->box.y + this->box.h < guitop)
            {
                continue;
            }

            ShowIcon(this);
        }

        if (this)
        {
            sprintf(buf, "--- Select an item below ---");
            xoff = x + string_width(&font_tiny_out, buf);
            yoff = y + this->box.y - guitop - font_tiny_out.line_height -
                   FONT_BLANKLINE;
            string_blt(ScreenSurface, &font_tiny_out, buf, xoff, yoff,
                      COLOR_GREEN, NULL, NULL);

            for (; this; this = this->next)
            {
                if (this->box.y > guibot ||
                    this->box.y + this->box.h < guitop)
                {
                    continue;
                }

                ShowIcon(this);
            }

            sprintf(buf, "--- Select an item above ---");
            xoff = x + string_width(&font_tiny_out, buf);
            yoff = y + gui_npc->icon->last->box.y - guitop + GUI_NPC_ICONSIZE +
                   FONT_BLANKLINE;
            string_blt(ScreenSurface, &font_tiny_out, buf, xoff, yoff,
                      COLOR_GREEN, NULL, NULL);
        }
    }

    if (gui_npc->link)
    {
        _gui_npc_element *this = gui_npc->link;

        for (; this; this = this->next)
        {
            if (this->box.y > guibot ||
                this->box.y + this->box.h < guitop)
            {
                continue;
            }

            xoff = x + this->box.x;
            yoff = y + this->box.y - guitop;
            sprintf(buf, "%s", this->title);

            if (string_width_offset(&font_medium, buf, &len, GUI_NPC_WIDTH))
            {
                buf[len - 2] = '\0';
                strcat(buf, "...");
            }

            if (gui_npc->link_selected == this &&
                !gui_npc->keyword_selected)
            {
                string_blt(ScreenSurface, &font_medium, buf, xoff, yoff,
                          COLOR_DK_NAVY, NULL, NULL);
            }
            else
            {
                string_blt(ScreenSurface, &font_medium, buf, xoff, yoff,
                          COLOR_TURQUOISE, NULL, NULL);
            }

            yoff += font_medium.line_height + FONT_BLANKLINE;
        }

        yoff += FONT_BLANKLINE * 2;
    }

    if (gui_npc->update)
    {
        _gui_npc_element *this = gui_npc->update;

        for (; this; this = this->next)
        {
            uint8 i;

            if (this->box.y > guibot ||
                this->box.y + this->box.h < guitop)
            {
                continue;
            }

            xoff = x + this->box.x;
            yoff = y + this->box.y - guitop;
            sprintf(buf, "%s", this->title);

            if (string_width_offset(&font_large_out, buf, &len, GUI_NPC_WIDTH))
            {
                buf[len - 2] = '\0';
                strcat(buf, "...");
            }

            string_blt(ScreenSurface, &font_large_out, buf, xoff, yoff,
                      COLOR_HGOLD, NULL, NULL);
            yoff += font_large_out.line_height + FONT_BLANKLINE;

            for (i = 0; i < this->body.line_count; i++)
            {
                if (!*this->body.line[i])
                {
                    yoff += FONT_BLANKLINE;
                }
                else
                {
                    string_blt(ScreenSurface, &font_medium, this->body.line[i],
                              xoff, yoff, COLOR_WHITE, NULL, NULL);
                    yoff += font_medium.line_height;
                }
            }

            yoff += FONT_BLANKLINE * 2;
        }
    }

    SDL_SetClipRect(ScreenSurface, NULL);

//    if (gui_npc->status == GUI_NPC_STATUS_WAIT)
//    {
//        return;
//    }
} 

static void ShowIcon(_gui_npc_element *this)
{
    const uint16 xoff = gui_npc->startx + GUI_NPC_LEFTMARGIN + this->box.x,
                 yoff = gui_npc->starty + GUI_NPC_TOPMARGIN + this->box.y -
                        gui_npc->yoff;
    char         buf[SMALL_BUF];
    int          len;
    uint8        i;

    /* Icon box bg. */
    if (this->mode == 's' ||
        this->mode == 'g')
    {
        sprite_blt(Bitmaps[BITMAP_DIALOG_ICON_BG_INACTIVE], xoff + 3, yoff + 3,
                   NULL, NULL);
    }
    else if (this->mode == 'S' ||
             this->mode == 'G')
    {
        if (this->quantity > 0)
        {
            sprite_blt(Bitmaps[BITMAP_DIALOG_ICON_BG_POSITIVE], xoff + 3,
                       yoff + 3, NULL, NULL);
        }
        else if (this->quantity < 0)
        {
            sprite_blt(Bitmaps[BITMAP_DIALOG_ICON_BG_NEGATIVE], xoff + 3,
                       yoff + 3, NULL, NULL);
        }
        else
        {
            sprite_blt(Bitmaps[BITMAP_DIALOG_ICON_BG_ACTIVE], xoff + 3,
                       yoff + 3, NULL, NULL);
        }
    }

    /* Icon face. */
    if (this->image.sprite)
    {
        SDL_Rect  box;
        _Sprite  *sprite = this->image.sprite;
        _BLTFX    bltfx;

        box.x = xoff;
        box.y = yoff;
        box.w = GUI_NPC_ICONSIZE;
        box.h = GUI_NPC_ICONSIZE;
        memset(&bltfx, 0, sizeof(_BLTFX));

        if (this->mode == 'g' ||
            this->mode == 's')
        {
            bltfx.flags |= BLTFX_FLAG_GREY;
        }

//        SDL_SetClipRect(ScreenSurface, &box);
        sprite_blt(sprite, box.x + box.w / 2 - (sprite->bitmap->w -
                   sprite->border_left) / 2 - sprite->border_left,
                   box.y + box.h / 2 - (sprite->bitmap->h -
                   sprite->border_down) / 2, NULL, &bltfx);
//        SDL_SetClipRect(ScreenSurface, &box);
    }

    /* Icon box fg. */
    if (this->mode == 's' ||
        this->mode == 'g')
    {
        sprite_blt(Bitmaps[BITMAP_DIALOG_ICON_FG_INACTIVE], xoff, yoff, NULL,
                   NULL);
    }
    else if (gui_npc->icon_selected == this &&
             !gui_npc->keyword_selected)
    {
        sprite_blt(Bitmaps[BITMAP_DIALOG_ICON_FG_SELECTED], xoff, yoff, NULL,
                   NULL);
    }
    else
    {
        sprite_blt(Bitmaps[BITMAP_DIALOG_ICON_FG_ACTIVE], xoff, yoff, NULL,
                   NULL);
    }

    /* Icon quantity. */
    if (this->quantity > 9999 ||
        this->quantity < -9999)
    {
        sprintf(buf, "many");
    }
    else
    {
        sprintf(buf, "%d", this->quantity);
    }

    if (this->quantity > 0)
    {
        uint8 w = string_width(&font_tiny_out, buf);

        string_blt(ScreenSurface, &font_tiny_out, buf, xoff + 28 - w / 2,
                  yoff + 18, COLOR_GREEN, NULL, NULL);
    }
    else if (this->quantity < 0)
    {
        uint8 w = string_width(&font_tiny_out, buf);

        string_blt(ScreenSurface, &font_tiny_out, buf, xoff + 28 - w / 2,
                  yoff + 18, COLOR_RED, NULL, NULL);
    }

    if (gui_npc->shop)
    {
        uint16 xoff2 = xoff,
               yoff2 = gui_npc->starty + Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->h;

        if (gui_npc->icon_selected == this)
        {
            /* Icon title. */
            sprintf(buf, "%s", this->title);

            if (string_width_offset(&font_medium, buf, &len, GUI_NPC_WIDTH))
            {
                buf[len - 2] = '\0';
                strcat(buf, "...");
            }

            string_blt(ScreenSurface, &font_small, buf, xoff2, yoff2, COLOR_HGOLD,
                      NULL, NULL);
            yoff2 += font_small.line_height;

            /* Icon body text. */
            for (i = 0; i < this->body.line_count; i++)
            {
                string_blt(ScreenSurface, &font_small, this->body.line[i], xoff2,
                          yoff2, COLOR_WHITE, NULL, NULL);
                yoff2 += font_small.line_height;
            }
        }
    }
    else
    {
        /* Icon title. */
        uint16 xoff2 = xoff + GUI_NPC_ICONSIZE,
               yoff2 = yoff;

        sprintf(buf, "%s", this->title);

        if (string_width_offset(&font_small, buf, &len,
                              GUI_NPC_WIDTH - GUI_NPC_ICONSIZE - 5))
        {
            buf[len - 2] = '\0';
            strcat(buf, "...");
        }

        string_blt(ScreenSurface, &font_small, buf, xoff2, yoff2, COLOR_HGOLD,
                  NULL, NULL);
        yoff2 += font_small.line_height;

        /* Icon body text. */
        for (i = 0; i < this->body.line_count; i++)
        {
            string_blt(ScreenSurface, &font_tiny_out, this->body.line[i], xoff2,
                      yoff2, COLOR_WHITE, NULL, NULL);
            yoff2 += font_small.line_height;
        }
    }
}

/* Returns the element under the pointer. */
static _gui_npc_element *GetElement(int mx, int my)
{
    uint16      x,
                y,
                xoff,
                yoff;
    uint8       i;

    /* Sanity. */
    if (!gui_npc)
    {
        return NULL;
    }

    x = gui_npc->startx;
    y = gui_npc->starty;

    /* Keyword panel. */
    if (options.keyword_panel &&
        gui_npc->hypertext)
    {
        _gui_npc_element *this = gui_npc->hypertext;
        uint8 cw = 16,
              ch = font_medium.line_height;

        xoff = (options.keyword_panel == 1) ? /* left */
               x - Bitmaps[BITMAP_GUI_NPC_PANEL]->bitmap->w + 23 +
               cw / 2 :
               x + Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->w - 5 + cw / 2;

        for (i = 1; this; this = this->next, i++)
        {
            uint16 kw = string_width(&font_medium, this->keyword);

            yoff = y + 73 + ch / 2 + ch * (i - 1);

            if (mx >= xoff &&
                mx <= xoff + kw &&
                my >= yoff &&
                my <= yoff + ch)
            {
                return this;
            }
        }
    }

    /* Buttons. */
    xoff = x + GUI_NPC_LEFTMARGIN;
    yoff = y + Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->h + ((gui_npc->shop) ?
           Bitmaps[BITMAP_GUI_NPC_MIDDLE]->bitmap->h : 0) - 1;
    if (gui_npc->lhsbutton &&
        mx >= xoff &&
        mx <= xoff + GUI_NPC_BUTTONWIDTH &&
        my >= yoff &&
        my <= yoff + Bitmaps[BITMAP_GUI_NPC_BOTTOM]->bitmap->h)
    {
        return gui_npc->lhsbutton;
    }

    xoff = x + GUI_NPC_LEFTMARGIN + GUI_NPC_WIDTH - GUI_NPC_BUTTONWIDTH;
    if (gui_npc->rhsbutton &&
        mx >= xoff &&
        mx <= xoff + GUI_NPC_BUTTONWIDTH &&
        my >= yoff &&
        my <= yoff + Bitmaps[BITMAP_GUI_NPC_BOTTOM]->bitmap->h)
    {
        return gui_npc->rhsbutton;
    }

    /* Now constrain ourselves to just the visible window contents. */
    xoff = x + GUI_NPC_LEFTMARGIN;
    yoff = y + GUI_NPC_TOPMARGIN;

    if (mx < xoff ||
        mx > xoff + GUI_NPC_WIDTH ||
        my < yoff ||
        ((!gui_npc->shop &&
          my > yoff + GUI_NPC_HEIGHT) ||
         (gui_npc->shop &&
          my > y + Bitmaps[BITMAP_GUI_NPC_TOP]->bitmap->h +
          Bitmaps[BITMAP_GUI_NPC_MIDDLE]->bitmap->h)))
    {
        return NULL;
    }

    /* Selectable icons. */
    if (gui_npc->first_selectable)
    {
        _gui_npc_element *this = gui_npc->first_selectable;

        for (; this; this = this->next)
        {
            xoff = x + GUI_NPC_LEFTMARGIN + this->box.x;
            yoff = y + GUI_NPC_TOPMARGIN + this->box.y - gui_npc->yoff;

            if (this->mode == 'S' &&
                mx >= xoff &&
                mx <= xoff + this->box.w &&
                my >= yoff &&
                my <= yoff + this->box.h)
            {
                return this;
            }
        }
    }

    /* Links. */
    if (gui_npc->link)
    {
        _gui_npc_element *this = gui_npc->link;

        for (; this; this = this->next)
        {
            yoff = y + GUI_NPC_TOPMARGIN + this->box.y - gui_npc->yoff;

            if (my >= yoff &&
                my <= yoff + this->box.h)
            {
                xoff = x + GUI_NPC_LEFTMARGIN + this->box.x;

                if (mx >= xoff &&
                    mx <= xoff + this->box.w)
                {
                   return this;
                }
            }
        }
    }

    return NULL;
}

/* Select the specified keyword. */
static void SelectKeyword(_gui_npc_element *element)
{
    /* Only click when a new keyword is selected. */
    if (element != gui_npc->keyword_selected)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0,
                          MENU_SOUND_VOL);
    }

    /* Reposition the window on the GUI so the keyword is near the top of the
     * visible window (plus a bit for context). USHRT_MAX means do not
     * reposition the window. */
    if (element &&
        element->box.y != SHRT_MAX &&
        gui_npc->height > GUI_NPC_HEIGHT)
    {
        gui_npc->yoff = MIN(element->box.y - element->box.h,
                            gui_npc->height - GUI_NPC_HEIGHT);
    }

    gui_npc->keyword_selected = element;
    gui_npc->link_selected = NULL;
    gui_npc->icon_selected = NULL;
    gui_npc->button_selected = NULL;
}

/* Select the specified icon. */
static void SelectIcon(_gui_npc_element *element)
{
    /* Only click when a new icon is selected. */
    if (element != gui_npc->icon_selected)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0,
                          MENU_SOUND_VOL);
    }

    if (element &&
        element->mode != 'S')
    {
        if ((!gui_npc->icon_selected &&
             element == gui_npc->first_selectable) ||
            (gui_npc->icon_selected &&
             element == gui_npc->icon_selected->next))
        {
            _gui_npc_element *this = element;

            for (; this; this = this->next)
            {
                if (this->mode == 'S')
                {
                    element = this;

                    break;
                }
            }

            if (!this)
            {
                element = NULL;
            }
        }
        else
        {
            _gui_npc_element *this = element;

            for (; this; this = this->prev)
            {
                if (this->mode == 'S')
                {
                    element = this;

                    break;
                }
                else if (this == gui_npc->first_selectable)
                {
                    element = NULL;

                    break;
                }
            }
        }
    }

    /* Reposition the window on the GUI so the element is visible. */
    if (element &&
        gui_npc->height > GUI_NPC_HEIGHT)
    {
        /* If the element is above the currently visible GUI, reposition so
         * that it is at the top. */
        if (element->box.y < gui_npc->yoff)
        {
            gui_npc->yoff = element->box.y;
        }
        /* If the element is (even partially) below the currently visible
         * GUI, reposition so that it is at the bottom. */
        else if (element->box.y + element->box.h + FONT_BLANKLINE >
                 gui_npc->yoff + GUI_NPC_HEIGHT)
        {
            gui_npc->yoff = MAX(0, MIN(element->box.y + element->box.h +
                                       FONT_BLANKLINE - GUI_NPC_HEIGHT,
                                       gui_npc->height - GUI_NPC_HEIGHT));
        }
    }

    gui_npc->keyword_selected = NULL;
    gui_npc->icon_selected = element;
    gui_npc->link_selected = NULL;
    gui_npc->button_selected = NULL;
}

/* Select and execute link #n. */
static void ChooseLink(uint8 n)
{
    _gui_npc_element *this = gui_npc->link;

    for (; this; this = this->next)
    {
        if (!--n)
        {
            SelectLink(this);
            SendCommand();

            break;
        }
    }
}

/* Select the specified link. */
static void SelectLink(_gui_npc_element *element)
{
    /* Only click when a new link is selected. */
    if (element != gui_npc->link_selected)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0,
                          MENU_SOUND_VOL);
    }

    /* Reposition the window on the GUI so the element is visible. */
    if (element &&
        gui_npc->height > GUI_NPC_HEIGHT)
    {
        /* If the element is above the currently visible GUI, reposition so
         * that it is at the top. */
        if (element->box.y < gui_npc->yoff)
        {
            gui_npc->yoff = element->box.y;
        }
        /* If the element is (even partially) below the currently visible
         * GUI, reposition so that it is at the bottom. */
        else if (element->box.y + element->box.h > gui_npc->yoff +
                                                   GUI_NPC_HEIGHT)
        {
            gui_npc->yoff = MAX(0, MIN(element->box.y + element->box.h -
                                       GUI_NPC_HEIGHT,
                                       gui_npc->height - GUI_NPC_HEIGHT));
        }
    }

    gui_npc->keyword_selected = NULL;
    gui_npc->icon_selected = NULL;
    gui_npc->link_selected = element;
    gui_npc->button_selected = NULL;
}

/* Select and execute the button whose hotkey is c. */
static void ChooseButton(char c)
{
    _gui_npc_element *this = NULL;

    if (gui_npc->lhsbutton &&
        *gui_npc->lhsbutton->title == c)
    {
        this = gui_npc->lhsbutton;
    }
    else if (gui_npc->rhsbutton &&
             *gui_npc->rhsbutton->title == c)
    {
        this = gui_npc->rhsbutton;
    }

    if (this)
    {
        SelectButton(this);
        SendCommand();
    }
}

/* Select the specified button. */
static void SelectButton(_gui_npc_element *element)
{
    if ((!gui_npc->lhsbutton ||
         element != gui_npc->lhsbutton) &&
        (!gui_npc->rhsbutton ||
         element != gui_npc->rhsbutton))
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                          MENU_SOUND_VOL);
    }

    if (gui_npc->button_selected != element)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0,
                          MENU_SOUND_VOL);
    }

    gui_npc->keyword_selected = NULL;

    if (!element->command ||
        *element->command != '#')
    {
        gui_npc->link_selected = NULL;
        gui_npc->icon_selected = NULL;
    }

    gui_npc->button_selected = element;
}

/* Mouse moves are used to select keywords, icons, links, and buttons.
 */
void gui_npc_mousemove(uint16 x, uint16 y)
{
    _gui_npc_element *element = GetElement(x, y);

    if (element)
    {
        switch (element->type)
        {
            case GUI_NPC_HYPERTEXT:
                SelectKeyword(element);

                break;

            case GUI_NPC_ICON:
                SelectIcon(element);

                break;

            case GUI_NPC_LINK:
                SelectLink(element);

                break;

            case GUI_NPC_BUTTON:
                SelectButton(element);

                break;

            default:
                LOG(LOG_ERROR, "ERROR:: %s/gui_npc_mousemove(): Unexpected NPC GUI element: %d!\n!",
                    __FILE__, element->type);
        }
    }
}

static void SendCommand(void)
{
    _gui_npc_element *keyword = gui_npc->keyword_selected,
                     *icon = gui_npc->icon_selected,
                     *link = gui_npc->link_selected,
                     *button = gui_npc->button_selected;
    char              buf[MEDIUM_BUF] = "";

    /* This function should never be called when nothing is selected. */
    if (!keyword &&
        !icon &&
        !link &&
        !button)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, MENU_SOUND_VOL);

        return;
    }

    /* A selected keyword overrides everything else. */
    if (keyword)
    {
        sprintf(buf, "%s", keyword->keyword);
    }
    else
    {
        char cmd[SMALL_BUF];

        cmd[0] = '\0';

        /* Check for a selected icon or link-> */
        if (icon)
        {
            if (icon->command)
            {
                int off;

                switch (*icon->command)
                {
                    case '<': /* default to LHS button */
                        if (!button)
                        {
                            if (gui_npc->lhsbutton &&
                                gui_npc->lhsbutton->command &&
                                *gui_npc->lhsbutton->command == '#')
                            {
                                button = gui_npc->lhsbutton;
                            }
                            else if (gui_npc->rhsbutton &&
                                     gui_npc->rhsbutton->command &&
                                     *gui_npc->rhsbutton->command == '#')
                            {
                                button = gui_npc->rhsbutton;
                            }
                        }

                        off = 1;

                        break;
                    case '-': /* default to no button */
                        off = 1;

                        break;

                    default: /* default to RHS button */
                        if (!button)
                        {
                            if (gui_npc->rhsbutton &&
                                gui_npc->rhsbutton->command &&
                                *gui_npc->rhsbutton->command == '#')
                            {
                                button = gui_npc->rhsbutton;
                            }
                            else if (gui_npc->lhsbutton &&
                                     gui_npc->lhsbutton->command &&
                                     *gui_npc->lhsbutton->command == '#')
                            {
                                button = gui_npc->lhsbutton;
                            }
                        }

                        off = (icon->command &&
                               *icon->command == '>') ? 1 : 0;
                }

                if (icon->command)
                {
                    sprintf(cmd, "%s", icon->command + off);
                }
            }
            else
            {
                _gui_npc_element *this = gui_npc->icon;
                uint8             i = 1;

                if (!button)
                {
                    if (gui_npc->rhsbutton &&
                        gui_npc->rhsbutton->command &&
                        *gui_npc->rhsbutton->command == '#')
                    {
                        button = gui_npc->rhsbutton;
                    }
                    else if (gui_npc->lhsbutton &&
                             gui_npc->lhsbutton->command &&
                             *gui_npc->lhsbutton->command == '#')
                    {
                        button = gui_npc->lhsbutton;
                    }
                }

                for (; this; this = this->next, i++)
                {
                    if (this == icon)
                    {
                        sprintf(cmd, "#%d", i);

                        break;
                    }
                }
            }
        }
        else if (link)
        {
            int off;

            switch (*link->command)
            {
                case '<': /* default to LHS button */
                    if (!button)
                    {
                        if (gui_npc->lhsbutton &&
                            gui_npc->lhsbutton->command &&
                            *gui_npc->lhsbutton->command == '#')
                        {
                            button = gui_npc->lhsbutton;
                        }
                        else if (gui_npc->rhsbutton &&
                                 gui_npc->rhsbutton->command &&
                                 *gui_npc->rhsbutton->command == '#')
                        {
                            button = gui_npc->rhsbutton;
                        }
                    }

                    off = 1;

                    break;

                case '-': /* default to no button */
                    off = 1;

                    break;

                default: /* default to RHS button */
                    if (!button)
                    {
                        if (gui_npc->rhsbutton &&
                            gui_npc->rhsbutton->command &&
                            *gui_npc->rhsbutton->command == '#')
                        {
                            button = gui_npc->rhsbutton;
                        }
                        else if (gui_npc->lhsbutton &&
                                 gui_npc->lhsbutton->command &&
                                 *gui_npc->lhsbutton->command == '#')
                        {
                            button = gui_npc->lhsbutton;
                        }
                    }

                    off = (link->command &&
                           *link->command == '>') ? 1 : 0;
            }

            if (link->command)
            {
                sprintf(cmd, "%s", link->command + off); // off might not be needed
            }
        }

        /* Check for a selected button. */
        if (gui_npc->rhsbutton &&
            button == gui_npc->rhsbutton)
        {
            if (gui_npc->rhsbutton->command)
            {
                if (cmd[0])
                {
                    sprintf(buf, "%s %s",
                            (*gui_npc->rhsbutton->command == '#') ?
                            gui_npc->rhsbutton->command + 1 :
                            gui_npc->rhsbutton->command, cmd);
                }
                else
                {
                    sprintf(buf, "%s",
                            (*gui_npc->rhsbutton->command == '#') ?
                            gui_npc->rhsbutton->command + 1 :
                            gui_npc->rhsbutton->command);
                }
            }
        }
        else if (gui_npc->lhsbutton &&
                 button == gui_npc->lhsbutton)
        {
            if (gui_npc->lhsbutton->command)
            {
                if (cmd[0])
                {
                    sprintf(buf, "%s %s",
                            (*gui_npc->lhsbutton->command == '#') ?
                            gui_npc->lhsbutton->command + 1 :
                            gui_npc->lhsbutton->command, cmd);
                }
                else
                {
                    sprintf(buf, "%s",
                            (*gui_npc->lhsbutton->command == '#') ?
                            gui_npc->lhsbutton->command + 1 :
                            gui_npc->lhsbutton->command);
                }
            }
        }
        else
        {
            sprintf(buf, "%s", cmd);
        }
    }

    /* Send the compound command that is in buf and tidy up. */
    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, MENU_SOUND_VOL);

    if (!buf[0])
    {
        gui_npc_reset();
    }
    else
    {
        if (gui_npc->status != GUI_NPC_STATUS_WAIT)
        {
            send_talk_command(interface_mode, buf);
            textwin_addhistory(buf);
            reset_keys();
            reset_input_mode();
            cpl.input_mode = INPUT_MODE_NO;
            gui_npc->status = GUI_NPC_STATUS_WAIT;
        }
    }
}

/* Mouse clicks are used to scroll the interface or
 * execute keywords, icon, links, and buttons.
 */
void gui_npc_mouseclick(SDL_Event *e)
{
    int mx = e->motion.x - gui_npc->startx,
        my = e->motion.y - gui_npc->starty;

    if (e->button.button == 4) /* mousewheel up */
    {
        if (!ScrollGUI(-GUI_NPC_SCROLL))
        {
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                              MENU_SOUND_VOL);
        }

        return;
    }
    else if (e->button.button == 5) /* mousewheel down */
    {
        if (!ScrollGUI(GUI_NPC_SCROLL))
        {
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                              MENU_SOUND_VOL);
        }

        return;
    }

    if (mx >= 349 &&
        mx <= 358 &&
        my >= 36 &&
        my <= 45) // close button
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
        gui_npc_reset();
    }

    if (mx >= 341 &&
        mx <= 352) // scroll buttons
    {
        if (my >= 79 &&
            my <= 89) // scroll up
        {
            if (!ScrollGUI(-GUI_NPC_SCROLL))
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);
            }
        }
        else if (my >= 432 &&
                 my <= 441) // scroll down
        {
            if (!ScrollGUI(GUI_NPC_SCROLL))
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);
            }
        }

        return;
    }
    else if (mx >= 95 &&
             mx <= 275 &&
             my >= 453 &&
             my <= 465) // textfield
    {
        if (!gui_npc->input_flag)
        {
            if (gui_npc->keyword_selected ||
                gui_npc->icon_selected ||
                gui_npc->link_selected ||
                gui_npc->button_selected)
            {
                check_menu_keys(MENU_NPC, SDLK_RETURN);
            }
            else
            {
                check_menu_keys(MENU_NPC, SDLK_BACKSPACE);
            }
        }
    }
    else
    {
        _gui_npc_element *element = GetElement(e->motion.x, e->motion.y);

        if (element)
        {
            switch (element->type)
            {
                case GUI_NPC_HYPERTEXT:
                case GUI_NPC_ICON:
                case GUI_NPC_LINK:
                case GUI_NPC_BUTTON:
                    SendCommand();

                    break;

                default:
                    LOG(LOG_ERROR, "ERROR:: %s/gui_npc_mouseclick(): Unexpected NPC GUI element: %d!\n!",
                        __FILE__, element->type);
            }
        }
    }
}

/* Keypresses perform a variety of tasks, depending on the key (see below),
 * including: selecting and/or executing keywords, icons, links, and buttons;
 * manipulating the textfield; and scrolling the interface.
 */
void gui_npc_keypress(int key)
{
//    if (gui_npc->status == GUI_NPC_STATUS_WAIT)
//    {
//        return;
//    }

    switch (key)
    {
        /* Selecting previous/next keyword. */
        case SDLK_KP_DIVIDE:
        case SDLK_KP_MULTIPLY:
        case SDLK_TAB:
            if (key == SDLK_KP_DIVIDE ||
                (key == SDLK_TAB &&
                 (SDL_GetModState() & KMOD_SHIFT)))
            {
                if (gui_npc->keyword_selected)
                {
                    SelectKeyword(gui_npc->keyword_selected->prev);
                }
                else if (gui_npc->hypertext)
                {
                    SelectKeyword(gui_npc->hypertext->last);
                }
            }
            else
            {
                if (gui_npc->keyword_selected)
                {
                    SelectKeyword(gui_npc->keyword_selected->next);
                }
                else if (gui_npc->hypertext)
                {
                    SelectKeyword(gui_npc->hypertext);
                }
            }

            break;

        /* Selecting previous/next selectable icon and/or link. */
        case SDLK_UP:
        case SDLK_LEFT:
        case SDLK_KP_MINUS:
            if (!gui_npc->first_selectable &&
                !gui_npc->link)
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);

                break;
            }

            if (!gui_npc->icon_selected &&
                !gui_npc->link_selected)
            {
                if (gui_npc->link)
                {
                    SelectLink(gui_npc->link->last);
                }
                else
                {
                    SelectIcon(gui_npc->icon->last);
                }
            }
            else if (gui_npc->icon_selected)
            {
                if (gui_npc->shop &&
                    key == SDLK_UP)
                {
                    _gui_npc_element *this = gui_npc->icon_selected;
                    uint8             i = 0;

                    for (; this && i < GUI_NPC_ICONSHOP; i++)
                    {
                        this = this->prev;
                    }

                    SelectIcon(this);
                }
                else
                {
                    SelectIcon(gui_npc->icon_selected->prev);
                }
            }
            else if (gui_npc->link_selected)
            {
                SelectLink(gui_npc->link_selected->prev);

                if (!gui_npc->link_selected &&
                    gui_npc->first_selectable)
                {
                    SelectIcon(gui_npc->icon->last);
                }
            }

            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);

            break;

        case SDLK_DOWN:
        case SDLK_RIGHT:
        case SDLK_KP_PLUS:
            if (!gui_npc->first_selectable &&
                !gui_npc->link)
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);

                break;
            }

            if (!gui_npc->icon_selected &&
                !gui_npc->link_selected)
            {
                if (gui_npc->first_selectable)
                {
                    SelectIcon(gui_npc->first_selectable);
                }
                else
                {
                    SelectLink(gui_npc->link);
                }
            }
            else if (gui_npc->icon_selected)
            {
                if (gui_npc->shop &&
                    key == SDLK_DOWN)
                {
                    _gui_npc_element *this = gui_npc->icon_selected;
                    uint8             i = 0;

                    for (; this && i < GUI_NPC_ICONSHOP; i++)
                    {
                        this = this->next;
                    }

                    SelectIcon(this);
                }
                else
                {
                    SelectIcon(gui_npc->icon_selected->next);
                }

                if (!gui_npc->icon_selected)
                {
                    if (gui_npc->link)
                    {
                        SelectLink(gui_npc->link);
                    }
                }
            }
            else if (gui_npc->link_selected)
            {
                SelectLink(gui_npc->link_selected->next);
            }

            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);

            break;

        /* Selecting and executing one of the first 9 links. */
        case SDLK_1:
        case SDLK_KP1:
            ChooseLink(1);

            break;

        case SDLK_2:
        case SDLK_KP2:
            ChooseLink(2);

            break;

        case SDLK_3:
        case SDLK_KP3:
            ChooseLink(3);

            break;

        case SDLK_4:
        case SDLK_KP4:
            ChooseLink(4);

            break;

        case SDLK_5:
        case SDLK_KP5:
            ChooseLink(5);

            break;

        case SDLK_6:
        case SDLK_KP6:
            ChooseLink(6);

            break;

        case SDLK_7:
        case SDLK_KP7:
            ChooseLink(7);

            break;

        case SDLK_8:
        case SDLK_KP8:
            ChooseLink(8);

            break;

        case SDLK_9:
        case SDLK_KP9:
            ChooseLink(9);

            break;

        /* Selecting and executing the LHS or RHS buttons. */
        case SDLK_MINUS:
        case SDLK_KP0:
            if (gui_npc->lhsbutton)
            {
                SelectButton(gui_npc->lhsbutton);
                SendCommand();
            }

            break;

        case SDLK_EQUALS:
        case SDLK_KP_PERIOD:
            if (gui_npc->rhsbutton)
            {
                SelectButton(gui_npc->rhsbutton);
                SendCommand();
            }

            break;

        /* Selecting and executing either button according to its hot key. */
        case SDLK_a:
            ChooseButton('A');

            break;

        case SDLK_b:
            ChooseButton('B');

            break;

        case SDLK_c:
            ChooseButton('C');

            break;

        case SDLK_d:
            ChooseButton('D');

            break;

        case SDLK_e:
            ChooseButton('E');

            break;

        case SDLK_f:
            ChooseButton('F');

            break;

        case SDLK_g:
            ChooseButton('G');

            break;

        case SDLK_h:
            ChooseButton('H');

            break;

        case SDLK_i:
            ChooseButton('I');

            break;

        case SDLK_j:
            ChooseButton('J');

            break;

        case SDLK_k:
            ChooseButton('K');

            break;

        case SDLK_l:
            ChooseButton('L');

            break;

        case SDLK_m:
            ChooseButton('M');

            break;

        case SDLK_n:
            ChooseButton('N');

            break;

        case SDLK_o:
            ChooseButton('O');

            break;

        case SDLK_p:
            ChooseButton('P');

            break;

        case SDLK_q:
            ChooseButton('Q');

            break;

        case SDLK_r:
            ChooseButton('R');

            break;

        case SDLK_s:
            ChooseButton('S');

            break;

        case SDLK_t:
            ChooseButton('T');

            break;

        case SDLK_u:
            ChooseButton('U');

            break;

        case SDLK_v:
            ChooseButton('V');

            break;

        case SDLK_w:
            ChooseButton('W');

            break;

        case SDLK_x:
            ChooseButton('X');

            break;

        case SDLK_y:
            ChooseButton('Y');

            break;

        case SDLK_z:
            ChooseButton('Z');

            break;

        /* Sending commands. */
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            SendCommand();

            break;

        /* Overwriting commands in the textfield-> */
        case SDLK_BACKSPACE:
            if (interface_mode == GUI_NPC_MODE_NPC)
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0,
                                  MENU_SOUND_VOL);
                reset_keys();
                reset_input_mode();
                open_input_mode(240);
                textwin_putstring("");
                cpl.input_mode = INPUT_MODE_NPCDIALOG;
                gui_npc->input_flag = 1;
                HistoryPos = 0;
            }

            break;

        /* Scrolling the visible window. */
        case SDLK_INSERT:
            if (!ScrollGUI(-GUI_NPC_SCROLL))
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);
            }

            break;

        case SDLK_DELETE:
            if (!ScrollGUI(GUI_NPC_SCROLL))
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);
            }

            break;

        case SDLK_HOME:
            if (!ScrollGUI(0))
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);
            }

            break;

        case SDLK_END:
            if (!ScrollGUI(gui_npc->height))
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);
            }

            break;

        case SDLK_PAGEUP:
            if (!ScrollGUI(-GUI_NPC_HEIGHT))
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);
            }

            break;

        case SDLK_PAGEDOWN:
            if (!ScrollGUI(GUI_NPC_HEIGHT))
            {
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0,
                                  MENU_SOUND_VOL);
            }

            break;

        case SDLK_ESCAPE:
            gui_npc_reset();

            break;
    }
}

/* Scroll the visible window. 'Scroll' is not really right -- we don't scroll,
 * we reposition. Maybe in future we can do real scrolling? */
static uint8 ScrollGUI(sint16 dist)
{
    /* No scrolling small windows. */
    if (gui_npc->height <= GUI_NPC_HEIGHT)
    {
        return 0;
    }

    /* Scroll to top. */
    if (dist == 0)
    {
        /* Not if we're already at the top. */
        if (gui_npc->yoff == 0)
        {
            return 0;
        }

        gui_npc->yoff = 0;

        return 1;
    }
    /* Scroll to bottom. */
    else if (dist >= gui_npc->height)
    {
        /* Not if we're already at the bottom. */
        if (gui_npc->yoff == gui_npc->height - GUI_NPC_HEIGHT)
        {
            return 0;
        }

        gui_npc->yoff = gui_npc->height - GUI_NPC_HEIGHT;

        return 1;
    }
    /* Scroll up. */
    if (dist < 0)
    {
        /* Not if we're already at the top. */
        if (gui_npc->yoff == 0)
        {
            return 0;
        }

        if (gui_npc->yoff > ABS(dist))
        {
            gui_npc->yoff += dist;
        }
        else
        {
            gui_npc->yoff = 0;
        }

        return 1;
    }
    /* Scroll down. */
    else if (dist > 0)
    {
        /* Not if we're already at the bottom. */
        if (gui_npc->yoff == gui_npc->height - GUI_NPC_HEIGHT)
        {
            return 0;
        }

        if (gui_npc->yoff < gui_npc->height - GUI_NPC_HEIGHT - dist)
        {
            gui_npc->yoff += dist;
        }
        else
        {
            gui_npc->yoff = gui_npc->height - GUI_NPC_HEIGHT;
        }

        return 1;
    }

    /* I don't think it is possible to reach here, but JIC... */
    return 0;
}
