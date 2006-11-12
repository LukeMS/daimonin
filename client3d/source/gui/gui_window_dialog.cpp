/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "logger.h"
#include "gui_window_dialog.h"
#include "gui_manager.h"
#include "network.h"



//================================================================================================
// Find a face ID by name, request the face (find it, load it or request it) and return the ID.
//================================================================================================
int get_bmap_id(char *name)
{
    /*
    for (int = 0; i < bmaptype_table_size; i++)
    {
        if (bmaptype_table[i].name[0] && !strcmp(bmaptype_table[i].name, name))
        {
            request_face(i, 0);
            return i;
        }
    }
    */
    return -1;
}






//================================================================================================
//
//================================================================================================
GuiDialog::GuiDialog()
{
    mVisible = false;
}

//================================================================================================
//
//================================================================================================
GuiDialog::~GuiDialog()
{}

//================================================================================================
// this function gets a ="xxxxxxx" string from a line.
// It removes the =" and the last " and returns the string in a static buffer.
//================================================================================================
char *GuiDialog::get_parameter_string(char *data, int *pos)
{
    static char buf[4024];

    // we assume a " after the =... don't be to shy, we search for a '"'
    char *start_ptr = strchr(data+*pos,'"');
    if (!start_ptr) return ""; // error
    char *end_ptr = strchr(++start_ptr,'"');
    if (!end_ptr) return ""; // error
    strncpy(buf, start_ptr, end_ptr-start_ptr);
    buf[end_ptr-start_ptr]=0;
    *pos += ++end_ptr-(data+*pos);
    return buf;
}

//================================================================================================
//
//================================================================================================
int GuiDialog::interface_cmd_head(_gui_interface_head *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_head));

    ++(*pos);
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if (c == '>')
        {
            if (*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }
        ++(*pos);
        if (c <=' ') continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL
        switch (c)
        {
            case 'f': // face for this head
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->name, buf);
                break;

            case 'b': // test body
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->body_text, buf);
                break;

            default:
                return -1; // error
        }
    }
    return -1;
}

//================================================================================================
//
//================================================================================================
int GuiDialog::interface_cmd_link(_gui_interface_link *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_link));

    ++(*pos);
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if (c == '>')
        {
            if (*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if (c<=' ') continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL
        switch (c)
        {
            case 't': // link title/text
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->link, buf);
                break;

            case 'c': // link command
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->cmd[0]=0;
                if (buf[0] != '/')
                    strcpy(head->cmd, "/talk ");
                strcat(head->cmd, buf);
                break;

            default:
                return -1; // error
        }
    }
    return -1;
}

//================================================================================================
// internal server string - the client use it as hint to use /tx instead of /talk
//================================================================================================
int GuiDialog::interface_cmd_who(_gui_interface_who *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_who));

    ++(*pos);
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if (c == '>')
        {
            if (*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if (c<=' ') continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL
        switch (c)
        {
            case 'b': // link title/text
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->body, buf);
                break;

            default:
                return -1; // error
        }
    }
    return -1;
}

//================================================================================================
//
//================================================================================================
int GuiDialog::interface_cmd_reward(_gui_interface_reward *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_reward));
    strcpy(head->title, "Description"); // default title

    ++(*pos);
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if (c == '>')
        {
            if (*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if (c<=' ') continue;

        switch (c)
        {
            case 't': // title of the reward
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->title, buf);
                break;

            case 'b': // reward body
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->body_text, buf);
                break;

            case 'c': // copper cash
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->copper =atoi(buf);
                break;

            case 's': // silver cash
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->silver =atoi(buf);
                break;

            case 'g': // gold cash
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->gold =atoi(buf);
                break;

            case 'm': // mithril cash
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->mithril =atoi(buf);
                break;

            default:
                return -1; // error
        }
    }
    return -1;
}

//================================================================================================
//
//================================================================================================
int GuiDialog::interface_cmd_message(_gui_interface_message *msg, char *data, int *pos)
{
    char *buf, c;
    memset(msg, 0, sizeof(_gui_interface_message));

    ++(*pos);
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if (c == '>')
        {
            if (*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if (c<=' ') continue;

        switch (c)
        {
            case 't': // title of the message
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(msg->title, buf);
                break;

            case 'b': // message body
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(msg->body_text, buf);
                break;

            default:
                return -1; // error
        }
    }
    return -1;
}

//================================================================================================
//
//================================================================================================
int GuiDialog::interface_cmd_xtended(_gui_interface_xtended *msg, char *data, int *pos)
{
    char *buf, c;
    memset(msg, 0, sizeof(_gui_interface_xtended));

    ++(*pos);
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if (c == '>')
        {
            if (*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if (c<=' ')
            continue;

        switch (c)
        {
            case 't': // title of the message
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(msg->title, buf);
                break;

            case 'b': // message body
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(msg->body_text, buf);
                break;

            default:
                return -1; // error
        }
    }
    return -1;
}

//================================================================================================
//
//================================================================================================
int GuiDialog::interface_cmd_icon(_gui_interface_icon *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_icon));

    ++(*pos);
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if (c == '>')
        {
            if (*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if (c<=' ')
            continue;
        switch (c)
        {
            case 'f': // face for this icon
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->name, buf);
                break;

            case 't': // title of the icon
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->title, buf);
                break;

            case 'm': // mode for this icon
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->mode = buf[0];
                break;

            case 'b': // test body
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->body_text, buf);
                break;

            default:
                return -1; // error
        }
    }
    return -1;
}

//================================================================================================
//
//================================================================================================
int GuiDialog::interface_cmd_button(_gui_interface_button *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_button));

    ++(*pos);
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if (c == '>')
        {
            if (*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if (c<=' ')
            continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL
        switch (c)
        {
            case 't': // button title
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->title, buf);
                break;

            case 'c': // button command
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;

                head->command[0]=0;
                if (buf[0] != '/')
                    strcpy(head->command, "/talk ");
                strcat(head->command, buf);
                break;

            default:
                return -1; // error
        }
    }
    return -1;
}

//================================================================================================
// Parse a <t b=""> textfield command
//================================================================================================
int GuiDialog::interface_cmd_textfield(_gui_interface_textfield *textfield, char *data, int *pos)
{
    char *buf, c;
    memset(textfield, 0, sizeof(_gui_interface_textfield));

    ++(*pos);
    while ((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if (c == '>')
        {
            if (*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if (c<=' ')
            continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL
        switch (c)
        {
            case 'b': // Textfield text
                if (!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(textfield->text, buf);
                break;

            default:
                return -1; // error
        }
    }
    return -1;
}

//================================================================================================
//
//================================================================================================
void GuiDialog::format_gui_interface()
{
    mMode= INTERFACE_MODE_NPC;
    if (mUsed_flag & GUI_INTERFACE_WHO)
    {
        if (*who.body == 'Q') mMode= INTERFACE_MODE_QLIST;
    }

    if (mUsed_flag & GUI_INTERFACE_ICON)
    {
        char *tmp;
        for (int s=0; s< mIcon_count; ++s)
        {
            icon[s].second_line = NULL;
            if ((tmp = strchr(icon[s].body_text, '\n')))
            {
                icon[s].second_line = tmp+1;
                *tmp = 0;
            }
        }
    }

    if (mUsed_flag & GUI_INTERFACE_HEAD)
    {
        head.face = get_bmap_id(head.name);
        if (head.face == -1)
        {
            /*
                   char line[256];
                   sprintf(line, "%s%s.png", GetIconDirectory(), head.name);
                   head.picture = sprite_load_file(line, 0);
            */
        }
        if (head.body_text[0]=='\0')
        {
            // strcpy(head.body_text, cpl.target_name?cpl.target_name:"");
        }
    }

    // overrule/extend the message block
    if (mUsed_flag & GUI_INTERFACE_XTENDED)
    {
        strcpy(message.title, xtended.title);
        strcat(message.body_text, xtended.body_text);
        mUsed_flag&=~GUI_INTERFACE_XTENDED;
    }

    // sort out the message text body to single lines
    if (mUsed_flag & GUI_INTERFACE_MESSAGE)
    {
        // done by gui_listbox.
    }
    if (mUsed_flag&GUI_INTERFACE_REWARD)
    {
        // done by gui_listbox.
    }

    // icons
    // search for the bmap num id's and load/request them if possible
    for (int s=0; s < mIcon_count; ++s)
    {

        if (icon[s].mode == 'S')
            mIcon_select = true;
//      gui_int->icon[s].element.face = get_bmap_id(gui_int->icon[s].name);
//        if(gui_int->icon[s].element.face==-1)
        {
//            char line[256];
//        sprintf(line, "%s%s.png", GetIconDirectory(), gui_int->icon[s].name);
//        gui_int->icon[s].picture = sprite_load_file(line, 0);
        }
    }
    Logger::log().error() << "6";
    if (mUsed_flag&GUI_INTERFACE_DECLINE && !(mUsed_flag&GUI_INTERFACE_ACCEPT))
    {
        if (decline.title[0] != '\0')
        {
            decline.title[0] = toupper(decline.title[0]);
            sprintf(decline.title2,"~%c~%s", decline.title[0],decline.title+1);
        }
        else
        {
            strcpy(decline.title,"Decline");
            strcpy(decline.title2,"~D~ecline");
        }

        mUsed_flag |=GUI_INTERFACE_ACCEPT;
        accept.command[0]='\0';
        strcpy(accept.title,"Accept");
        strcpy(accept.title2,"~A~ccept");
    }
    else if (mUsed_flag&GUI_INTERFACE_ACCEPT)
    {
        if (accept.title[0] != '\0')
        {
            accept.title[0] = toupper(accept.title[0]);
            sprintf(accept.title2,"~%c~%s", accept.title[0],accept.title+1);
        }
        else
        {
            strcpy(accept.title,"Accept");
            strcpy(accept.title2,"~A~ccept");
        }

        // prepare the buttons (titles)
        if (mUsed_flag&GUI_INTERFACE_DECLINE)
        {
            if (decline.title[0] != '\0')
            {
                decline.title[0] = toupper(decline.title[0]);
                sprintf(decline.title2,"~%c~%s", decline.title[0],decline.title+1);
            }
            else
            {
                strcpy(decline.title,"Decline");
                strcpy(decline.title2,"~D~ecline");
            }
        }
        else // if we have a accept button but no decline one - we set it without command = close gui
        {
            mUsed_flag |=GUI_INTERFACE_DECLINE;
            decline.command[0]='\0';
            strcpy(decline.title,"Decline");
            strcpy(decline.title2,"~D~ecline");
        }
    }
    else if (mUsed_flag&GUI_INTERFACE_BUTTON) // means: single button
    {
        mUsed_flag |=GUI_INTERFACE_ACCEPT; // yes, thats right! we fake the accept button
        if (accept.title[0] != '\0')
        {
            accept.title[0] = toupper(accept.title[0]);
            sprintf(accept.title2,"~%c~%s", accept.title[0],accept.title+1);
        }
        else
        {
            strcpy(accept.title,"Bye");
            strcpy(accept.title2,"~B~ye");
        }
    }
    else // no accept/decline and no button? set it to 'Bye' default button
    {
        mUsed_flag |=GUI_INTERFACE_ACCEPT; // yes, thats right! we fake the accept button
        accept.command[0]='\0';
        strcpy(accept.title,"Bye");
        strcpy(accept.title2,"~B~ye");
    }
    Logger::log().error() << "7";
}

//================================================================================================
// clear & reset the gui interface
//================================================================================================
void GuiDialog::reset_gui_interface(void)
{
    message.line_count =0;
    reward.line_count =0;
    mLink_count =0;
    GuiManager::getSingleton().clearListbox(GUI_WIN_NPCDIALOG, GUI_LIST_NPC);
}

//================================================================================================
// called from commands.c after we got a interface command
//================================================================================================
bool GuiDialog::load_gui_interface(int mode, char *data, int len, int pos)
{
    _gui_interface_head    head_tmp;
    _gui_interface_message   message_tmp;
    _gui_interface_reward  reward_tmp;
    _gui_interface_who   who_tmp;
    _gui_interface_xtended   xtended_tmp;
    _gui_interface_link    link_tmp;
    _gui_interface_icon    icon_tmp;
    _gui_interface_button  button_tmp;
    _gui_interface_textfield textfield_tmp;

    int cmd      = INTERFACE_CMD_NO; // we have a open '<' and a command is active the string is related to this cmd.
    int cmd_mode = INTERFACE_CMD_NO; // when we collect outside a cmd tag strings,
    int flag_start=0, flag_end=0;
    char c;
    for (; len > pos; ++pos)
    {
        c = *(data + pos);
        if (c == '<')
        {
            if (flag_end==1)
            {
                if (flag_end == 2) // bug
                {
                    Logger::log().error() << "Bad interface string (flag end error): " << data;
                    return false;
                }
                // our char before this was a '>' - now we get a '<'
                flag_start=0;
                flag_end=0;
                cmd_mode = cmd;
                cmd = INTERFACE_CMD_NO;
            }

            if (flag_start) // double << ?
            {
                if (flag_start == 2) // bug
                {
                    Logger::log().error() << "Bad interface string (flag end error): " << data;
                    return false;
                }
                flag_start=0;
                goto normal_char;
            }
            else
                flag_start=1;
        }
        else if (c == '>')
        {
            if (flag_end)
            {
                flag_end=0;
                goto normal_char;
            }
            else
                flag_end=1;
        }
        else
        {
            // we have a single '<' or '>'?
            if (flag_start==1)
            {
                flag_start=2;
                // This char is a command marker
                Logger::log().info() << "found cmd: "<< c;
                cmd_mode = INTERFACE_CMD_NO;

                switch (c)
                {
                    case 'h': // head with picture & name this interface comes from
                        cmd = INTERFACE_CMD_HEAD;
                        if (interface_cmd_head(&head_tmp, data, &pos)) return false;
                        memcpy(&head, &head_tmp,sizeof(_gui_interface_head));
                        mUsed_flag |=GUI_INTERFACE_HEAD;
                        break;

                    case 'm': // title & text - what he has to say
                        cmd = INTERFACE_CMD_MESSAGE;
                        if (interface_cmd_message(&message_tmp, data, &pos)) return false;
                        memcpy(&message, &message_tmp,sizeof(_gui_interface_message));
                        mUsed_flag |=GUI_INTERFACE_MESSAGE;
                        break;

                    case 'r': // reward info
                        cmd = INTERFACE_CMD_REWARD;
                        if (interface_cmd_reward(&reward_tmp, data, &pos)) return false;
                        memcpy(&reward, &reward_tmp,sizeof(_gui_interface_reward));
                        mUsed_flag |=GUI_INTERFACE_REWARD;
                        break;

                    case 'w': // who info
                        cmd = INTERFACE_CMD_WHO;
                        if (interface_cmd_who(&who_tmp, data, &pos)) return false;
                        memcpy(&who, &who_tmp,sizeof(_gui_interface_who));
                        mUsed_flag |=GUI_INTERFACE_WHO;
                        break;

                    case 'x': // xtended info
                        cmd = INTERFACE_CMD_XTENDED;
                        if (interface_cmd_xtended(&xtended_tmp, data, &pos)) return false;
                        memcpy(&xtended, &xtended_tmp, sizeof(_gui_interface_xtended));
                        mUsed_flag |=GUI_INTERFACE_XTENDED;
                        break;

                    case 'l': // define a "link" string line
                        cmd = INTERFACE_CMD_LINK;
                        if (interface_cmd_link(&link_tmp, data, &pos)) return false;
                        memcpy(&link[mLink_count++], &link_tmp, sizeof(_gui_interface_link));
                        break;


                    case 'i': // define a "icon" - graphical presentation of reward or message part
                        cmd = INTERFACE_CMD_ICON;
                        if (interface_cmd_icon(&icon_tmp, data, &pos)) return false;
                        memcpy(&icon[mIcon_count++], &icon_tmp,sizeof(_gui_interface_icon));
                        break;

                    case 'a': // define accept button
                        cmd = INTERFACE_CMD_ACCEPT;
                        if (interface_cmd_button(&button_tmp, data, &pos)) return false;
                        memcpy(&accept, &button_tmp,sizeof(_gui_interface_button));
                        mUsed_flag |=GUI_INTERFACE_ACCEPT;
                        break;

                    case 'b': // define single button
                        cmd = INTERFACE_CMD_BUTTON;
                        if (interface_cmd_button(&button_tmp, data, &pos)) return false;
                        // we use the accept button struct for single buttons too
                        memcpy(&accept, &button_tmp,sizeof(_gui_interface_button));
                        mUsed_flag |=GUI_INTERFACE_BUTTON;
                        break;

                    case 'd': // define decline button
                        cmd = INTERFACE_CMD_DECLINE;
                        if (interface_cmd_button(&button_tmp, data, &pos)) return false;
                        memcpy(&decline, &button_tmp,sizeof(_gui_interface_button));
                        mUsed_flag |=GUI_INTERFACE_DECLINE;
                        break;

                    case 't': // textfield contents
                        cmd = INTERFACE_CMD_TEXTFIELD;
                        if (interface_cmd_textfield(&textfield_tmp, data, &pos)) return false;
                        memcpy(&textfield, &textfield_tmp, sizeof(_gui_interface_textfield));
                        mUsed_flag |=GUI_INTERFACE_TEXTFIELD;
                        break;

                    default:
                        Logger::log().error() <<  "Bad command tag: " << data;
                        return false;
                }
            }
            else if (flag_end==1)
            {
                flag_end=0;
                flag_start=0;
                cmd_mode = cmd;
                cmd = INTERFACE_CMD_NO;
                // close this command - perhaps we stay string collect mode for it
                Logger::log().error() <<  "Interface close cmd";
            }
normal_char:
            ; // we don't have "text" between the tags (<> <>) atm
        }
    }
    // if we are here, we have a legal gui_int structure.
    // Now lets create a legal formular and preprocess some structures.
    format_gui_interface();
    return true;
}


void client_send_tell_extended(char *body, char *tail)
{}


//================================================================================================
// send a command from the gui to server.
// if mode is 1, its a real command.
// mode 0 or 2 means to add /talk first.
// mode 2 means manual input / add to history
//================================================================================================
void GuiDialog::gui_interface_send_command(int mode, char *cmd)
{
    char msg[1024];
    if (mStatus == GUI_INTERFACE_STATUS_WAIT)
        return;
    if (mUsed_flag& GUI_INTERFACE_WHO)
    {
        if (!strncmp(cmd,"/talk ",6))
            cmd +=6;
        char    buf[MAX_BUF];
        sprintf(buf, "tx %s %s", who.body, cmd);
        Network::getSingleton().cs_write_string(buf, (int)strlen(buf));
    }
    else
    {
        if (mode == 1)
        {
            Network::getSingleton().send_command(cmd, -1, Network::SC_NORMAL);
            // if(strncmp(cmd, "/talk ", 6) == 0)
            // textwin_addhistory(cmd);
        }
        else
        {
            char buf[1024];
            sprintf(buf,"/talk %s", cmd);
            Network::getSingleton().send_command(buf, -1, Network::SC_NORMAL);
            sprintf(msg,"Talking about: %s", cmd);
            //draw_info(msg,COLOR_WHITE);
            //if (mode == 2) textwin_addhistory(buf);
        }
    }
    mStatus = GUI_INTERFACE_STATUS_WAIT;
}

//================================================================================================
// Display the npc interface.
//================================================================================================
void GuiDialog::show_interface_npc(int mark)
{
    GuiManager::getSingleton().showWindow(GUI_WIN_NPCDIALOG, true);
    mVisible = true;

    // add_close_button(x-113, y+4, MENU_NPC);

    if (mUsed_flag & GUI_INTERFACE_HEAD)
    {
        // print head
        GuiManager::getSingleton().sendMessage(GUI_WIN_NPCDIALOG, GUI_MSG_TXT_CHANGED, GUI_TEXTBOX_NPC_HEADLINE, (void*)head.body_text);
    }

    if (mUsed_flag & GUI_INTERFACE_MESSAGE)
    {
        message.line_count = GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, message.title, COLOR_YELLOW);
        message.line_count+= GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, "");
        message.line_count+= GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, message.body_text);
    }

    if (mLink_count)
    {
        GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, " ");
        for (int i=0; i< mLink_count; ++i)
            GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, link[i].link, COLOR_GREEN);
    }

    // reward is also used as "objective"
    if (mUsed_flag & GUI_INTERFACE_REWARD)
    {
        reward.line_count = GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, "");
        reward.line_count+= GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, reward.title, COLOR_YELLOW);
        reward.line_count+= GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, "");
        reward.line_count+= GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, reward.body_text);
        // only print the "Your rewards:" message when there is one
        if (reward.copper || reward.gold || reward.silver || reward.mithril || mIcon_count)
        {
            char buffer[100];
            if (reward.copper)
            {
                //sprite_blt(Bitmaps[BITMAP_COIN_COPPER], x + 110, y + yoff, NULL, NULL);
                sprintf(buffer, "Your rewards: %d", reward.copper);
            }
            if (reward.silver)
            {
                //sprite_blt(Bitmaps[BITMAP_COIN_SILVER], x + 140, y + yoff+6, NULL, NULL);
                sprintf(buffer, "Your rewards: %d", reward.silver);
            }
            if (reward.gold)
            {
                //sprite_blt(Bitmaps[BITMAP_COIN_GOLD], x + 170, y + yoff+6, NULL, NULL);
                sprintf(buffer, "Your rewards: %d", reward.gold);
            }
            if (reward.mithril)
            {
                //sprite_blt(Bitmaps[BITMAP_COIN_MITHRIL], x + 200, y + yoff+9, NULL, NULL);
                sprintf(buffer, "Your rewards: %d", reward.mithril);
            }
            GuiManager::getSingleton().addTextline(GUI_WIN_NPCDIALOG, GUI_LIST_NPC, buffer);
        }
    }
    /*
              yoff+=5;
              // present now the icons for rewards or whats searched
              if(gui_interface_npc->mIcon_count)
              {
                int flag_s = FALSE;

                yoff+=25;
                for(i=0;i<gui_interface_npc->mIcon_count;i++)
                {
                  // we have a 's' to announce a 'S' selection for real rewards?
                  if(gui_interface_npc->icon[i].mode == 's' )
                  {
                    flag_s = true;
                    continue;
                  }

                  if(gui_interface_npc->icon[i].mode == 'G' )
                  {
                    sprite_blt(Bitmaps[BITMAP_INVSLOT], x + 40, y + yoff, NULL, NULL);

                    if(gui_interface_npc->icon[i].element.face>0)
                      blt_inv_item_centered(&gui_interface_npc->icon[i].element, x + 40, y + yoff);
                    else if(gui_interface_npc->icon[i].picture)
                      sprite_blt(gui_interface_npc->icon[i].picture, x + 40, y + yoff, NULL, NULL);

                    StringBlt(ScreenSurface, &MediumFont, gui_interface_npc->icon[i].title, x+80, y+yoff-3, COLOR_WHITE, NULL, NULL);
                    yoff+=10;
                    StringBlt(ScreenSurface, &SystemFont, gui_interface_npc->icon[i].body_text, x+80, y+yoff, COLOR_WHITE, NULL, NULL);
                    yoff+=10;
                    if(gui_interface_npc->icon[i].second_line)
                      StringBlt(ScreenSurface, &SystemFont, gui_interface_npc->icon[i].second_line, x+80, y+yoff+1, COLOR_WHITE, NULL, NULL);
                    yoff+=24;
                  }
                }

                if(flag_s)
                {
                  StringBlt(ScreenSurface, &MediumFont, "And one of these:", x+40, y+yoff, COLOR_WHITE, NULL, NULL);
                  yoff+=20;
                  for(i=0;i<gui_interface_npc->mIcon_count;i++)
                  {
                    if(gui_interface_npc->icon[i].mode == 's' )
                    {
                      sprite_blt(Bitmaps[BITMAP_INVSLOT], x + 40, y + yoff, NULL, NULL);

                      if(gui_interface_npc->icon[i].element.face>0)
                        blt_inv_item_centered(&gui_interface_npc->icon[i].element, x + 40, y + yoff);
                      else if(gui_interface_npc->icon[i].picture)
                        sprite_blt(gui_interface_npc->icon[i].picture, x + 40, y + yoff, NULL, NULL);

                      StringBlt(ScreenSurface, &MediumFont, gui_interface_npc->icon[i].title, x+80, y+yoff-3, COLOR_WHITE, NULL, NULL);
                      yoff+=10;
                      StringBlt(ScreenSurface, &SystemFont, gui_interface_npc->icon[i].body_text, x+80, y+yoff, COLOR_WHITE, NULL, NULL);
                      yoff+=10;
                      if(gui_interface_npc->icon[i].second_line)
                        StringBlt(ScreenSurface, &SystemFont, gui_interface_npc->icon[i].second_line, x+80, y+yoff+1, COLOR_WHITE, NULL, NULL);
                      yoff+=24;
                    }
                  }
                }

                if(gui_interface_npc->mIcon_select)
                {
                  int t;

                  StringBlt(ScreenSurface, &MediumFont, "And one of these (select one):", x+40, y+yoff, COLOR_WHITE, NULL, NULL);
                  yoff+=20;
                  for(t=1,i=0;i<gui_interface_npc->mIcon_count;i++)
                  {
                    if(gui_interface_npc->icon[i].mode == 'S' )
                    {
                      if(gui_interface_npc->selected == t)
                      {
                        box.x=x+38;
                        box.y=y+yoff-2;
                        box.w=36;
                        box.h=36;
                        SDL_FillRect(ScreenSurface, &box, sdl_dgreen);
                      }

                      sprite_blt(Bitmaps[BITMAP_INVSLOT], x + 40, y + yoff, NULL, NULL);

                      if(gui_interface_npc->icon[i].element.face>0)
                        blt_inv_item_centered(&gui_interface_npc->icon[i].element, x + 40, y + yoff);
                      else if(gui_interface_npc->icon[i].picture)
                        sprite_blt(gui_interface_npc->icon[i].picture, x + 40, y + yoff, NULL, NULL);

                      StringBlt(ScreenSurface, &MediumFont, gui_interface_npc->icon[i].title, x+80, y+yoff-3, COLOR_WHITE, NULL, NULL);
                      yoff+=10;
                      StringBlt(ScreenSurface, &SystemFont, gui_interface_npc->icon[i].body_text, x+78, y+yoff-1, COLOR_WHITE, NULL, NULL);
                      yoff+=10;
                      if(gui_interface_npc->icon[i].second_line)
                        StringBlt(ScreenSurface, &SystemFont, gui_interface_npc->icon[i].second_line, x+78, y+yoff, COLOR_WHITE, NULL, NULL);
                      yoff+=24;
                      ++t;
                    }
                  }
                }
              }

              box.x = x + 95;
              box.y = y+449;
              box.h = 12;
              box.w = 180;
              if(gui_interface_npc->input_flag)
              {
                SDL_FillRect(ScreenSurface, &box, 0);
                StringBlt(ScreenSurface, &MediumFont, show_input_string(InputString, &MediumFont,box.w-10),box.x+5 ,box.y, COLOR_WHITE, NULL, NULL);
              }
              else
              {
                StringBlt(ScreenSurface, &SystemFont, "~Return~ to talk", x+155, y+437, COLOR_WHITE, NULL, NULL);
                SDL_FillRect(ScreenSurface, &box, COLOR_GREY);

                if(gui_interface_npc->link_selected)
                {
                  StringBlt(ScreenSurface, &MediumFont, gui_interface_npc->link[gui_interface_npc->link_selected-1].link, box.x+5, box.y-1, COLOR_DK_NAVY, NULL, NULL);
                }

              }

            */
}

//================================================================================================
//
//================================================================================================
bool GuiDialog::keyEvent(const char keyChar, const unsigned char key)
{
    if (!mVisible || mStatus == GUI_INTERFACE_STATUS_WAIT) return false;
    if (key == KC_ESCAPE)
    {
        GuiManager::getSingleton().showWindow(GUI_WIN_NPCDIALOG, false);
        mVisible = false;
        return true;
    }
    //if (mUsed_flag & GUI_INTERFACE_ACCEPT) return false;



    mSelected =1;



    //    if (add_button(x + 35, y + 443, numButton++, BITMAP_DIALOG_BUTTON_UP, accept.title, accept.title2))
    {
        if (mIcon_select && !mSelected)
        {
            GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "select an item first.");
            //sound_play_effect(SOUND_CLICKFAIL, 0, 0, 100);
            return true;
        }

        if (accept.title[0])
        {
            //sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            if (accept.command[0]!='\0')
            {
                char cmd[1024];
                // if we have accept, we must check selected for possible slot selection.
                if (mIcon_select)
                    sprintf(cmd,"%s #%d", accept.command, mSelected);
                else
                    strcpy(cmd, accept.command);
                gui_interface_send_command(1, cmd);
            }
            else
                reset_gui_interface();

        }
        return true;
    }

    if (mUsed_flag & GUI_INTERFACE_DECLINE)
    {
        //if (add_button(x + 285, y + 443, numButton++, BITMAP_DIALOG_BUTTON_UP, decline.title, decline.title2))
        {
            // sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            if (decline.command[0]!='\0')
                gui_interface_send_command(1, decline.command);
            else
                reset_gui_interface();

            return true;
        }
    }
    return false;
}

//================================================================================================
// we have a left click inside the interface -> check it
//================================================================================================
bool GuiDialog::mouseEvent(int MouseAction, int x, int y)
{
    int line = GuiManager::getSingleton().getSelectedListboxLine(GUI_WIN_NPCDIALOG, GUI_LIST_NPC);
    if (line <0) return false;

    char buffer[300];
    sprintf(buffer, "%d", line);

    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, buffer);

    int element, index;
    char *keyword = 0;

    if (get_interface_line(line, &element, &index, &keyword))
    {
        Logger::log().error() <<  "keyword: " << keyword;
        if (element == GUI_INTERFACE_ICON)
        {
            //sound_play_effect(SOUND_GET, 0, 0, 100);
            mSelected = index;
        }
        else if (element == GUI_INTERFACE_MESSAGE)
        {
            //sound_play_effect(SOUND_GET, 0, 0, 100);
            gui_interface_send_command(0, keyword);
        }
        else if (element == GUI_INTERFACE_LINK)
        {
            GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, keyword);
            //sound_play_effect(SOUND_GET, 0, 0, 100);
            gui_interface_send_command(keyword[0]!='/'?0:1, keyword);
        }
    }
    return false;
}

//================================================================================================
// if we click on something in a gui interface, this functions
// * returns us the element and/or keyword.
// * we return in *element the gui element (message, body, icon)
// * and in *index what element. If there is a keyword or command,
// * we have a pointer to it in keyword. The pointer is to a static
// * buffer here in get_interface_line or to a buffer in gui_interface.
// * return: true = we hit something.
//================================================================================================
bool GuiDialog::get_interface_line(int line, int *element, int *index, char **keyword)
{
    //    static char key[256]; // used to get keyword string parts for keyword and save it statically
    if ((mUsed_flag & GUI_INTERFACE_MESSAGE) && line < message.line_count)
    {
        /*
          for (int i=0; i < message.line_count; ++i)
          {
              int st=0, xt, xs=x+40, s, flag= false;
              xt=xs;
              for (s=0; s< (int)strlen(message.lines[i]); ++s)
              {
                  if (message.lines[i][s]=='^')
                  {
                      flag?(flag=false):(flag=true);
                      xs = xt;
                      st =s+1;
                  }
                  else
                  {
                      if (message.lines[i][s] != '~' && message.lines[i][s] != '�')
                          xt += MediumFont.c[(unsigned char)message.lines[i][s]].w + MediumFont.char_offset;
                      if (flag && mx>=xs && mx <=xt) // only when we have a active keyword part
                      {
                          char *ptr = strchr(&message.lines[i][s], '^');

                          *element = GUI_INTERFACE_MESSAGE;
                          *index = i;
                          if (!ptr)
                              strcpy(key, &message.lines[i][st]);
                          else
                          {
                              strncpy(key, &message.lines[i][st],ptr - &message.lines[i][st]);
                              key[ptr-&message.lines[i][st]]='\0';
                          }
                          *keyword = key;
                          return true;
                      }
                  }
              }
              return false;
          }
          */
          return false;
    }
    line -= message.line_count;
    //    char buffer[300];
    //    sprintf(buffer, "Link: %d  %d  %d", line, message.line_count, mLink_count);
    //    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, buffer);
    if (mLink_count)
    {
        *element = GUI_INTERFACE_LINK;
        *index = --line;
        if (link[line].cmd)
            *keyword = link[line].cmd;
        else
            *keyword = link[line].link;
        return true;
    }

    //    for (int i=0;i < mLink_count; ++i)
    //        Logger::log().error() <<  "Link_count: " << link[i].cmd << " - " << link[i].link;

    /*
        // reward is also used as "objective"
        if (mUsed_flag & GUI_INTERFACE_REWARD)
        {
            if (reward.copper || reward.gold || reward.silver || reward.mithril || mIcon_count)
            {
                // if (reward.line_count) yoff+=15;
            }
        }

        if (mIcon_count)
        {
            int flag_s= false;
            for (i=0; i < mIcon_count; ++i)
            {
                if (icon[i].mode == 's' )   flag_s=true;
                //else if (icon[i].mode == 'G') yoff+=44;
            }
            if (flag_s)
            {
                for (i=0;i < mIcon_count; ++i)
                {
                    //if (icon[i].mode == 's' ) yoff+=44;
                }
            }
        }

        if (mIcon_select)
        {
            for (int t=1, i=0; i< mIcon_count; ++i)
            {
                if (icon[i].mode == 'S' )
                {
                    *element = GUI_INTERFACE_ICON;
                    *index = t;
                    *keyword = gui_interface_npc->icon[i].title;
                    return true;
                }
                ++t;
            }
        }
        */
    return false;
}
