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

//================================================================================================
///
//================================================================================================
GuiDialog::GuiDialog()
{
}

//================================================================================================
///
//================================================================================================
GuiDialog::~GuiDialog()
{
}

//================================================================================================
// this function gets a ="xxxxxxx" string from a line.
// It removes the =" and the last " and returns the string in a static buffer.
//================================================================================================
char *GuiDialog::get_parameter_string(char *data, int *pos)
{
    char *start_ptr, *end_ptr;
    static char buf[4024];

    // we assume a " after the =... don't be to shy, we search for a '"'
    start_ptr = strchr(data+*pos,'"');
    if(!start_ptr)
        return ""; // error

    end_ptr = strchr(++start_ptr,'"');
    if(!end_ptr)
        return ""; // error

    strncpy(buf, start_ptr, end_ptr-start_ptr);
    buf[end_ptr-start_ptr]=0;

    // ahh... ptr arithmetic... eat that, high level language fans ;)
    *pos += ++end_ptr-(data+*pos);

    return buf;
}

//================================================================================================
///
//================================================================================================
int GuiDialog::interface_cmd_head(_gui_interface_head *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_head));

    ++(*pos);
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if (c <=' ') continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL
        switch(c)
        {
            case 'f': // face for this head
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->name, buf);
                break;

            case 'b': // test body
                if(!(buf = get_parameter_string(data, pos)))
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
///
//================================================================================================
int GuiDialog::interface_cmd_link(_gui_interface_link *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_link));

    ++(*pos);
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if(c<=' ') continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL
        switch(c)
        {
            case 't': // link title/text
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->link, buf);
                break;

            case 'c': // link command
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->cmd[0]=0;
                if(buf[0] != '/')
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
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if(c<=' ') continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL
        switch(c)
        {
            case 'b': // link title/text
                if(!(buf = get_parameter_string(data, pos)))
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
///
//================================================================================================
int GuiDialog::interface_cmd_reward(_gui_interface_reward *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_reward));
    strcpy(head->title, "Description"); // default title

    ++(*pos);
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if(c<=' ') continue;

        switch(c)
        {

            case 't': // title of the reward
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->title, buf);
                break;

            case 'b': // reward body
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->body_text, buf);
                break;

            case 'c': // copper cash
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->copper =atoi(buf);
                break;

            case 's': // silver cash
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->silver =atoi(buf);
                break;

            case 'g': // gold cash
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->gold =atoi(buf);
                break;

            case 'm': // mithril cash
                if(!(buf = get_parameter_string(data, pos)))
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
///
//================================================================================================
int GuiDialog::interface_cmd_message(_gui_interface_message *msg, char *data, int *pos)
{
    char *buf, c;
    memset(msg, 0, sizeof(_gui_interface_message));

    ++(*pos);
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if(c<=' ') continue;

        switch(c)
        {
            case 't': // title of the message
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(msg->title, buf);
                break;

            case 'b': // message body
                if(!(buf = get_parameter_string(data, pos)))
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
///
//================================================================================================
int GuiDialog::interface_cmd_xtended(_gui_interface_xtended *msg, char *data, int *pos)
{
    char *buf, c;
    memset(msg, 0, sizeof(_gui_interface_xtended));

    ++(*pos);
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if(c<=' ')
            continue;

        switch(c)
        {
            case 't': // title of the message
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(msg->title, buf);
                break;

            case 'b': // message body
                if(!(buf = get_parameter_string(data, pos)))
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
///
//================================================================================================
int GuiDialog::interface_cmd_icon(_gui_interface_icon *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_icon));

    ++(*pos);
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if(c<=' ')
            continue;
        switch(c)
        {
            case 'f': // face for this icon
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->name, buf);
                break;

            case 't': // title of the icon
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->title, buf);
                break;

            case 'm': // mode for this icon
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                head->mode = buf[0];
                break;

            case 'b': // test body
                if(!(buf = get_parameter_string(data, pos)))
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
///
//================================================================================================
int GuiDialog::interface_cmd_button(_gui_interface_button *head, char *data, int *pos)
{
    char *buf, c;
    memset(head, 0, sizeof(_gui_interface_button));

    ++(*pos);
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if(c<=' ')
            continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL

        switch(c)
        {
            case 't': // button title
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;
                strcpy(head->title, buf);
                break;

            case 'c': // button command
                if(!(buf = get_parameter_string(data, pos)))
                    return -1;

                head->command[0]=0;
                if(buf[0] != '/')
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
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        // c is legal string part - check it is '<'
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') // no double >>? then we return
            {
                --(*pos);
                return 0;
            }
        }

        ++(*pos);
        if(c<=' ')
            continue;

        // c is part of the head command inside the '<' - lets handle it
        // It must be a command. If it is unknown, return NULL

        switch(c)
        {
            case 'b': // Textfield text
                if(!(buf = get_parameter_string(data, pos)))
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
// clear & reset the gui interface
//================================================================================================
void GuiDialog::reset_gui_interface(void)
{
    /*
      map_udate_flag = 2;
      mInterfaceMode= INTERFACE_MODE_NO;
      if(gui_interface_npc)
      {
        int s;
        for(s=0;s<gui_interface_npc->icon_count;s++)
          free(gui_interface_npc->icon[s].picture);
        free(gui_interface_npc);
      }
     reset_keys();
     cpl.input_mode = INPUT_MODE_NO;
      gui_interface_npc = NULL;
      if (cpl.menustatus == MENU_NPC)
        cpl.menustatus = MENU_NO;
    */
}

//================================================================================================
///
//================================================================================================
void GuiDialog::format_gui_interface(_gui_interface_struct *gui_int)
{
    mInterfaceMode= INTERFACE_MODE_NPC;
    if(gui_int->used_flag & GUI_INTERFACE_WHO)
    {
        if(*gui_int->who.body == 'Q')
            mInterfaceMode= INTERFACE_MODE_QLIST;
    }

    if(gui_int->used_flag & GUI_INTERFACE_ICON)
    {
        char *tmp;
        for(int s=0;s<gui_int->icon_count; ++s)
        {
            gui_int->icon[s].second_line = NULL;
            tmp = strchr(gui_int->icon[s].body_text, '\n');
            if(tmp)
            {
                gui_int->icon[s].second_line = tmp+1;
                *tmp = 0;
            }
        }
    }

    if(gui_int->used_flag & GUI_INTERFACE_HEAD)
    {
//     gui_int->head.face = get_bmap_id(gui_int->head.name);
        if(gui_int->head.face==-1)
        {
            /*
                   char line[256];
                   sprintf(line, "%s%s.png", GetIconDirectory(), gui_int->head.name);
                   gui_int->head.picture = sprite_load_file(line, 0);
            */
        }
        if(gui_int->head.body_text[0]=='\0')
        {
//        strcpy(gui_int->head.body_text, cpl.target_name?cpl.target_name:"");
        }
    }

    // overrule/extend the message block
    if(gui_int->used_flag & GUI_INTERFACE_XTENDED)
    {
        strcpy(gui_int->message.title, gui_int->xtended.title);
        strcat(gui_int->message.body_text, gui_int->xtended.body_text);
        gui_int->used_flag&=~GUI_INTERFACE_XTENDED;
    }

    // sort out the message text body to single lines
    if(gui_int->used_flag & GUI_INTERFACE_MESSAGE)
    {
        int i, len, c=0;

        gui_int->message.line_count=0;
        for(i=0;;i++)
        {
            if(gui_int->message.body_text[i]==0x0d)
                continue;
            if(gui_int->message.body_text[i]==0x0a || gui_int->message.body_text[i]=='\0')
            {
                gui_int->message.lines[gui_int->message.line_count][c]='\0';
                // draw_info(gui_int->message.lines[gui_int->message.line_count], COLOR_YELLOW);
                ++gui_int->message.line_count;
                if(gui_int->message.body_text[i]=='\0')
                    break;
                c=0;
            }
            else
            {
                // lets do automatic line breaks
                gui_int->message.lines[gui_int->message.line_count][c]=gui_int->message.body_text[i];

//    if(StringWidthOffset(&MediumFont, gui_int->message.lines[gui_int->message.line_count], &len, 270))
                {
                    char tmp_line[INTERFACE_MAX_CHAR];
                    int ii;

                    strcpy(tmp_line, gui_int->message.lines[gui_int->message.line_count]); // safe the line
                    gui_int->message.lines[gui_int->message.line_count][len]=0;
                    for(ii=len;ii>=0;ii--)
                    {
                        if(gui_int->message.lines[gui_int->message.line_count][ii] == ' ')
                        {
                            gui_int->message.lines[gui_int->message.line_count][ii]=0;
                            break;
                        }
                    }
                    if(ii<0) // we have not find any usable whitespace
                        ii = len;

                    // we don't eliminate leading whitespaces because we can't know its a format issue or not
                    // better to live with this little glitch as to destroy perhaps the text format.

                    strcpy(gui_int->message.lines[++gui_int->message.line_count], &tmp_line[ii+1]);
                    c = strlen(gui_int->message.lines[gui_int->message.line_count]);
                }
//    else
                {
                    ++c;
                }
            }
            if(gui_int->message.line_count>=INTERFACE_MAX_LINE || c>=INTERFACE_MAX_CHAR )
            {
                Logger::log().error() << "Interface call out of borders: " << gui_int->message.body_text;
                break;
            }
        }
    }

    if(gui_int->used_flag&GUI_INTERFACE_REWARD)
    {
        int i, len, c=0;

        gui_int->reward.line_count=0;
        for(i=0;;i++)
        {
            if(gui_int->reward.body_text[i]==0x0d)
                continue;
            if(gui_int->reward.body_text[i]==0x0a || gui_int->reward.body_text[i]=='\0')
            {
                gui_int->reward.lines[gui_int->reward.line_count][c]='\0';
                // draw_info(gui_int->reward.lines[gui_int->message.line_count], COLOR_YELLOW);
                ++gui_int->reward.line_count;
                if(gui_int->reward.body_text[i]=='\0') break;
                c=0;
            }
            else
            {
                // lets do automatic line breaks
                gui_int->reward.lines[gui_int->reward.line_count][c]=gui_int->reward.body_text[i];

//    if(StringWidthOffset(&MediumFont, gui_int->reward.lines[gui_int->reward.line_count], &len, 270))
                {
                    char tmp_line[INTERFACE_MAX_CHAR];
                    int ii;

                    strcpy(tmp_line, gui_int->reward.lines[gui_int->reward.line_count]); // safe the line
                    gui_int->reward.lines[gui_int->reward.line_count][len]=0;
                    for(ii=len;ii>=0;ii--)
                    {
                        if(gui_int->reward.lines[gui_int->reward.line_count][ii] == ' ')
                        {
                            gui_int->reward.lines[gui_int->reward.line_count][ii]=0;
                            break;
                        }
                    }
                    if(ii<0) // we have not find any usable whitespace
                        ii = len;

                    // we don't eliminate leading whitespaces because we can't know its a format issue or not
                    // better to live with this little glitch as to destroy perhaps the text format.

                    strcpy(gui_int->reward.lines[++gui_int->reward.line_count], &tmp_line[ii+1]);
                    c = strlen(gui_int->reward.lines[gui_int->reward.line_count]);
                }
//                else
                {
                    ++c;
                }

            }

            if(gui_int->reward.line_count>=INTERFACE_MAX_LINE || c>=INTERFACE_MAX_CHAR )
            {
                Logger::log().error() << "Interface call out of borders: %s\n" << gui_int->reward.body_text;
                break;
            }
        }
    }

    // icons
    // search for the bmap num id's and load/request them if possible
    for(int s=0; s<gui_int->icon_count; ++s)
    {

        if(gui_int->icon[s].mode == 'S')
            gui_int->icon_select = true;
//      gui_int->icon[s].element.face = get_bmap_id(gui_int->icon[s].name);
//        if(gui_int->icon[s].element.face==-1)
        {
//            char line[256];
//        sprintf(line, "%s%s.png", GetIconDirectory(), gui_int->icon[s].name);
//        gui_int->icon[s].picture = sprite_load_file(line, 0);
        }
    }

    if(gui_int->used_flag&GUI_INTERFACE_DECLINE && !(gui_int->used_flag&GUI_INTERFACE_ACCEPT))
    {
        if(gui_int->decline.title[0] != '\0')
        {
            gui_int->decline.title[0] = toupper(gui_int->decline.title[0]);
            sprintf(gui_int->decline.title2,"~%c~%s", gui_int->decline.title[0],gui_int->decline.title+1);
        }
        else
        {
            strcpy(gui_int->decline.title,"Decline");
            strcpy(gui_int->decline.title2,"~D~ecline");
        }

        gui_int->used_flag |=GUI_INTERFACE_ACCEPT;
        gui_int->accept.command[0]='\0';
        strcpy(gui_int->accept.title,"Accept");
        strcpy(gui_int->accept.title2,"~A~ccept");
    }
    else if(gui_int->used_flag&GUI_INTERFACE_ACCEPT)
    {
        if(gui_int->accept.title[0] != '\0')
        {
            gui_int->accept.title[0] = toupper(gui_int->accept.title[0]);
            sprintf(gui_int->accept.title2,"~%c~%s", gui_int->accept.title[0],gui_int->accept.title+1);
        }
        else
        {
            strcpy(gui_int->accept.title,"Accept");
            strcpy(gui_int->accept.title2,"~A~ccept");
        }

        // prepare the buttons (titles)
        if(gui_int->used_flag&GUI_INTERFACE_DECLINE)
        {
            if(gui_int->decline.title[0] != '\0')
            {
                gui_int->decline.title[0] = toupper(gui_int->decline.title[0]);
                sprintf(gui_int->decline.title2,"~%c~%s", gui_int->decline.title[0],gui_int->decline.title+1);
            }
            else
            {
                strcpy(gui_int->decline.title,"Decline");
                strcpy(gui_int->decline.title2,"~D~ecline");
            }
        }
        else // if we have a accept button but no decline one - we set it without command = close gui
        {
            gui_int->used_flag |=GUI_INTERFACE_DECLINE;
            gui_int->decline.command[0]='\0';
            strcpy(gui_int->decline.title,"Decline");
            strcpy(gui_int->decline.title2,"~D~ecline");
        }
    }
    else if(gui_int->used_flag&GUI_INTERFACE_BUTTON) // means: single button
    {
        gui_int->used_flag |=GUI_INTERFACE_ACCEPT; // yes, thats right! we fake the accept button
        if(gui_int->accept.title[0] != '\0')
        {
            gui_int->accept.title[0] = toupper(gui_int->accept.title[0]);
            sprintf(gui_int->accept.title2,"~%c~%s", gui_int->accept.title[0],gui_int->accept.title+1);
        }
        else
        {
            strcpy(gui_int->accept.title,"Bye");
            strcpy(gui_int->accept.title2,"~B~ye");
        }
    }
    else // no accept/decline and no button? set it to 'Bye' default button
    {
        gui_int->used_flag |=GUI_INTERFACE_ACCEPT; // yes, thats right! we fake the accept button
        gui_int->accept.command[0]='\0';
        strcpy(gui_int->accept.title,"Bye");
        strcpy(gui_int->accept.title2,"~B~ye");
    }
}

//================================================================================================
// called from commands.c after we got a interface command
//================================================================================================
void GuiDialog::load_gui_interface(int mode, char *data, int len, int pos)
{
    /*
      int flag_start=0, flag_end=0;
      char c;
      //buf[256];
      _gui_interface_head    head_tmp;
      _gui_interface_message   message_tmp;
     _gui_interface_reward  reward_tmp;
     _gui_interface_who   who_tmp;
     _gui_interface_xtended   xtended_tmp;
      _gui_interface_link    link_tmp;
      _gui_interface_icon    icon_tmp;
      _gui_interface_button  button_tmp;
      _gui_interface_textfield textfield_tmp;
      int cmd = INTERFACE_CMD_NO;    // we have a open '<' and a command is active
      int cmd_mode = INTERFACE_CMD_NO; // when we collect outside a cmd tag strings,
                        * the string is related to this cmd


      _gui_interface_struct *gui_int = malloc(sizeof(_gui_interface_struct));
      memset(gui_int,0,sizeof(_gui_interface_struct));

      for(;len>pos;pos++)
      {
        c = *(data+pos);

        if(c == '<')
        {
          if(flag_end==1)
          {
            if(flag_end == 2) // bug
            {
              //draw_info("ERROR: bad interface string (flag end error)", COLOR_RED);
              LOG(LOG_ERROR, "ERROR: bad interface string (flag end error): %s\n", data);
              free(gui_int);
              return NULL;
            }

            // our char before this was a '>' - now we get a '<'
            flag_start=0;
            flag_end=0;
            cmd_mode = cmd;
            cmd = INTERFACE_CMD_NO;
          }

          if(flag_start) // double << ?
          {
            if(flag_start == 2) // bug
            {
              //draw_info("ERROR: bad interface string (flag start error)", COLOR_RED);
              LOG(LOG_ERROR, "ERROR: bad interface string (flag start error): %s\n", data);
              free(gui_int);
              return NULL;
            }

            flag_start=0;
            goto normal_char;
          }
          else
            flag_start=1;

        }
        else if(c == '>')
        {
          if(flag_end)
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
          if(flag_start==1)
          {
            flag_start=2;
            // This char is a command marker
            //
            sprintf(buf, "found cmd: %c", c);
            draw_info(buf, COLOR_GREEN);


            cmd_mode = INTERFACE_CMD_NO;

            switch(c)
            {
              case 'h': // head with picture & name this interface comes from
                cmd = INTERFACE_CMD_HEAD;
                if(interface_cmd_head(&head_tmp, data, &pos))
                {
                  free(gui_int);
                  return NULL;
                }
                memcpy(&gui_int->head, &head_tmp,sizeof(_gui_interface_head));
                gui_int->used_flag |=GUI_INTERFACE_HEAD;
                break;

              case 'm': // title & text - what he has to say
                cmd = INTERFACE_CMD_MESSAGE;
                if(interface_cmd_message(&message_tmp, data, &pos))
                {
                  free(gui_int);
                  return NULL;
                }
                memcpy(&gui_int->message, &message_tmp,sizeof(_gui_interface_message));
                gui_int->used_flag |=GUI_INTERFACE_MESSAGE;
                break;

              case 'r': // reward info
                cmd = INTERFACE_CMD_REWARD;
                if(interface_cmd_reward(&reward_tmp, data, &pos))
                {
                  free(gui_int);
                  return NULL;
                }
                memcpy(&gui_int->reward, &reward_tmp,sizeof(_gui_interface_reward));
                gui_int->used_flag |=GUI_INTERFACE_REWARD;
                break;

       case 'w': // who info
        cmd = INTERFACE_CMD_WHO;
        if(interface_cmd_who(&who_tmp, data, &pos))
        {
         free(gui_int);
         return NULL;
        }
        memcpy(&gui_int->who, &who_tmp,sizeof(_gui_interface_who));
        gui_int->used_flag |=GUI_INTERFACE_WHO;
        break;

       case 'x': // xtended info
        cmd = INTERFACE_CMD_XTENDED;
        if(interface_cmd_xtended(&xtended_tmp, data, &pos))
        {
         free(gui_int);
         return NULL;
        }
        memcpy(&gui_int->xtended, &xtended_tmp,sizeof(_gui_interface_xtended));
        gui_int->used_flag |=GUI_INTERFACE_XTENDED;
        break;

              case 'l': // define a "link" string line
                cmd = INTERFACE_CMD_LINK;
                if(interface_cmd_link(&link_tmp, data, &pos))
                {
                  free(gui_int);
                  return NULL;
                }
                memcpy(&gui_int->link[gui_int->link_count++], &link_tmp,sizeof(_gui_interface_link));
                break;


              case 'i': // define a "icon" - graphical presentation of reward or message part
                cmd = INTERFACE_CMD_ICON;
                if(interface_cmd_icon(&icon_tmp, data, &pos))
                {
                  free(gui_int);
                  return NULL;
                }
                memcpy(&gui_int->icon[gui_int->icon_count++], &icon_tmp,sizeof(_gui_interface_icon));
                break;

              case 'a': // define accept button
                cmd = INTERFACE_CMD_ACCEPT;
                if(interface_cmd_button(&button_tmp, data, &pos))
                {
                  free(gui_int);
                  return NULL;
                }
                memcpy(&gui_int->accept, &button_tmp,sizeof(_gui_interface_button));
                gui_int->used_flag |=GUI_INTERFACE_ACCEPT;
                break;

              case 'b': // define single button
                cmd = INTERFACE_CMD_BUTTON;
                if(interface_cmd_button(&button_tmp, data, &pos))
                {
                  free(gui_int);
                  return NULL;
                }
        // we use the accept button struct for single buttons too
                memcpy(&gui_int->accept, &button_tmp,sizeof(_gui_interface_button));
                gui_int->used_flag |=GUI_INTERFACE_BUTTON;
                break;

              case 'd': // define decline button
                cmd = INTERFACE_CMD_DECLINE;
                if(interface_cmd_button(&button_tmp, data, &pos))
                {
                  free(gui_int);
                  return NULL;
                }
                memcpy(&gui_int->decline, &button_tmp,sizeof(_gui_interface_button));
                gui_int->used_flag |=GUI_INTERFACE_DECLINE;
                break;

              case 't': // textfield contents
                cmd = INTERFACE_CMD_TEXTFIELD;
                if(interface_cmd_textfield(&textfield_tmp, data, &pos))
                {
                  free(gui_int);
                  return NULL;
                }
                memcpy(&gui_int->textfield, &textfield_tmp,sizeof(_gui_interface_textfield));
                gui_int->used_flag |=GUI_INTERFACE_TEXTFIELD;
                break;

              default:

                draw_info("ERROR: bad interface string (flag start error)", COLOR_RED);
                LOG(LOG_ERROR, "ERROR: bad command tag: %s\n", data);
                return FALSE;
              break;
            }
          }
          else if(flag_end==1)
          {
            flag_end=0;
            flag_start=0;
            cmd_mode = cmd;
            cmd = INTERFACE_CMD_NO;
            // close this command - perhaps we stay string collect mode for it
            //
            sprintf(buf, "close cmd");
            draw_info(buf, COLOR_GREEN);

          }
        normal_char:;
          // we don't have "text" between the tags (<> <>) atm
        }
      }

      // if we are here, we have a legal gui_int structure.
       * Now lets create a legal formular and preprocess some structures.

    */
//     gui_int = format_gui_interface(gui_int);
//     return gui_int;
}

//================================================================================================
// send a command from the gui to server.
// if mode is 1, its a real command.
// mode 0 or 2 means to add /talk first.
// mode 2 means manual input / add to history
//================================================================================================
void GuiDialog::gui_interface_send_command(int mode, char *cmd)
{
    /*
      char msg[1024];

     if(gui_interface_npc->status == GUI_INTERFACE_STATUS_WAIT)
      return;

     if(gui_interface_npc->used_flag & GUI_INTERFACE_WHO)
     {
      if(!strncmp(cmd,"/talk ",6))
       cmd +=6;
      client_send_tell_extended(gui_interface_npc->who.body, cmd);
     }
     else
     {
       if(mode == 1)
      {
       send_command(cmd, -1, SC_NORMAL);
       // if(strncmp(cmd, "/talk ", 6) == 0)
      textwin_addhistory(cmd);
      }
      else
      {
       char buf[1024];
       sprintf(buf,"/talk %s", cmd);
       send_command(buf, -1, SC_NORMAL);
       sprintf(msg,"Talking about: %s", cmd);
       draw_info(msg,COLOR_WHITE);
       if(mode == 2)
      textwin_addhistory(buf);
      }
     }
      reset_keys();
     reset_input_mode();
      cpl.input_mode = INPUT_MODE_NO;
      gui_interface_npc->status = GUI_INTERFACE_STATUS_WAIT;
    */
}

//================================================================================================
// if we click on something in a gui interface, this functions
// * returns us the element and/or keyword.
// * we return in *element the gui element (message, body, icon)
// * and in *index what element. If there is a keyword or command,
// * we have a pointer to it in keyword. The pointer is to a static
// * buffer here in get_interface_line or to a buffer in gui_interface.
// * return: TRUE = we hit something.
//================================================================================================
int GuiDialog::get_interface_line(int *element, int *index, char **keyword, int x, int y, int mx, int my)
{
    /*
      static char key[256]; // used to get keyword string parts for keyword and save it statically
      int i,yoff = y+85+gui_interface_npc->yoff;

      if(my <= y+85 || my >= y+85+INTERFACE_WINLEN_NPC)
        return FALSE;

      if(gui_interface_npc->used_flag&GUI_INTERFACE_MESSAGE)
      {
        yoff+=26;

        for(i=0;i<gui_interface_npc->message.line_count;i++,yoff+=15)
        {
          if(my >= yoff && my <=yoff+15)
          {
            int st=0, xt, xs=x+40, s, flag=FALSE;

            xt=xs;
            for(s=0;s<(int)strlen(gui_interface_npc->message.lines[i]);s++)
            {
              if(gui_interface_npc->message.lines[i][s]=='^')
              {
                flag?(flag=FALSE):(flag=TRUE);
                xs = xt;
                st =s+1;
              }
              else
              {
                if(gui_interface_npc->message.lines[i][s] != '~' &&
                    gui_interface_npc->message.lines[i][s] != '�')
                  xt += MediumFont.c[(unsigned char)gui_interface_npc->message.lines[i][s]].w + MediumFont.char_offset;

                if(flag && mx>=xs && mx <=xt) // only when we have a active keyword part
                {
                  char *ptr = strchr(&gui_interface_npc->message.lines[i][s], '^');

                  *element = GUI_INTERFACE_MESSAGE;
                  *index = i;
                  if(!ptr)
                    strcpy(key, &gui_interface_npc->message.lines[i][st]);
                  else
                  {
                    // eat that, mueslifresser ;)=
                    strncpy(key, &gui_interface_npc->message.lines[i][st],ptr-&gui_interface_npc->message.lines[i][st]);
                    key[ptr-&gui_interface_npc->message.lines[i][st]]='\0';
                  }
                  *keyword = key;
                  return TRUE;
                }
              }

            }
            return FALSE;
          }
        }
      }

      if(gui_interface_npc->link_count)
      {
        yoff+=15;
        for(i=0;i<gui_interface_npc->link_count;i++,yoff+=15)
          if(my >= yoff && my <=yoff+15)
          {
            int len =  get_string_pixel_length(gui_interface_npc->link[i].link, &MediumFont);

            if(mx>=x+40 && mx<=x+40+len)
            {
              *element = GUI_INTERFACE_LINK;
              *index = i;
              if(gui_interface_npc->link[i].cmd)
                *keyword = gui_interface_npc->link[i].cmd;
              else
                *keyword = gui_interface_npc->link[i].link;
              return TRUE;
            }
            return FALSE;
          }
      }

      // reward is also used as "objective"
      if(gui_interface_npc->used_flag&GUI_INTERFACE_REWARD)
      {
        yoff +=51;

        for(i=0;i<gui_interface_npc->reward.line_count;i++)
          yoff+=15;

        if(gui_interface_npc->reward.copper || gui_interface_npc->reward.gold ||
            gui_interface_npc->reward.silver || gui_interface_npc->reward.mithril ||
            gui_interface_npc->icon_count)
        {
          if(gui_interface_npc->reward.line_count)
            yoff+=15;
          yoff+=15;
        }
      }

      yoff+=5;
      if(gui_interface_npc->icon_count)
      {
        int flag_s=FALSE;
        yoff+=25;
        for(i=0;i<gui_interface_npc->icon_count;i++)
        {
          if(gui_interface_npc->icon[i].mode == 's' )
            flag_s=TRUE;
          else if(gui_interface_npc->icon[i].mode == 'G' )
            yoff+=44;
        }

        if(flag_s)
        {
          yoff+=20;
          for(i=0;i<gui_interface_npc->icon_count;i++)
          {
            if(gui_interface_npc->icon[i].mode == 's' )
              yoff+=44;
          }
        }
      }

      if(gui_interface_npc->icon_select)
      {
        int t;

        yoff+=20;
        for(t=1,i=0;i<gui_interface_npc->icon_count;i++)
        {

          if(gui_interface_npc->icon[i].mode == 'S' )
          {
            if(my >= yoff && my <=yoff+32 && mx >=x+40 && mx<=x+72)
            {
              *element = GUI_INTERFACE_ICON;
              *index = t;
              *keyword = gui_interface_npc->icon[i].title;
              return TRUE;
            }
            yoff+=44;
            ++t;
          }
        }
      }
    */
    return false;
}

//================================================================================================
///
//================================================================================================
int GuiDialog::precalc_interface_npc(void)
{
      int yoff = 5;
	  /*
      if(gui_interface_npc->used_flag&GUI_INTERFACE_MESSAGE)
      {
        yoff+=26;

        for(int i=0;i<gui_interface_npc->message.line_count;i++)
          yoff+=15;
      }

      if(gui_interface_npc->link_count)
      {
        yoff+=15;
        for(int i=0;i<gui_interface_npc->link_count;i++)
          yoff+=15;
      }

      // reward is also used as "objective"
      if(gui_interface_npc->used_flag&GUI_INTERFACE_REWARD)
      {
        yoff +=51;

        for(int i=0;i<gui_interface_npc->reward.line_count;i++,yoff+=15)
          ;

        yoff+=15;

        if(gui_interface_npc->reward.copper || gui_interface_npc->reward.gold ||
            gui_interface_npc->reward.silver || gui_interface_npc->reward.mithril ||
            gui_interface_npc->icon_count)
        {
          if(gui_interface_npc->reward.line_count)
            yoff+=15;
          yoff+=15;
        }
      }

      yoff+=5;
      if(gui_interface_npc->icon_count)
      {
        int flag_s=FALSE;
        yoff+=25;
        for(int i=0;i<gui_interface_npc->icon_count;i++)
        {
          if(gui_interface_npc->icon[i].mode == 's' )
            flag_s=TRUE;
          else if(gui_interface_npc->icon[i].mode == 'G' )
            yoff+=44;
        }

        if(flag_s)
        {
          yoff+=20;
          for(int i=0;i<gui_interface_npc->icon_count;i++)
          {
            if(gui_interface_npc->icon[i].mode == 's' )
              yoff+=44;
          }
        }
      }

      if(gui_interface_npc->icon_select)
      {
        yoff+=20;
        for(int i=0;i<gui_interface_npc->icon_count;i++)
        {

          if(gui_interface_npc->icon[i].mode == 'S' )
            yoff+=44;
        }
      }
    */
	  return yoff;
}

//================================================================================================
// show npc interface. ATM its included in the menu system, but
// we need to crate a lower layer level for it.
//================================================================================================
void GuiDialog::show_interface_npc(int mark)
{
    /*
      SDL_Rect  box;
      int x=gui_interface_npc->startx, y=gui_interface_npc->starty, numButton=0,yoff, i;

      sprite_blt(Bitmaps[BITMAP_NPC_INTERFACE], x, y, NULL, NULL);
      add_close_button(x-113, y+4, MENU_NPC);

      if(gui_interface_npc->used_flag&GUI_INTERFACE_HEAD)
      {
        // print head
        //sprintf(xxbuf, "%s (%d,%d)", keyword?keyword:"--", mx, my);
        StringBlt(ScreenSurface,&MediumFont , xxbuf, x+75, y+48, COLOR_WHITE, NULL, NULL);

        StringBlt(ScreenSurface,&MediumFont , gui_interface_npc->head.body_text, x+75, y+48, COLOR_WHITE, NULL, NULL);

        if(gui_interface_npc->head.face>=0 && FaceList[gui_interface_npc->head.face].sprite != NULL)
        {
          int xp, yp;

          box.x=x+4;
          box.y=y+4;
          box.w=54;
          box.h=54;

          SDL_SetClipRect(ScreenSurface, &box);
          xp = box.x+(box.w/2)-((FaceList[gui_interface_npc->head.face].sprite->bitmap->w-FaceList[gui_interface_npc->head.face].sprite->border_left)/2);
          yp = box.y+(box.h/2)-((FaceList[gui_interface_npc->head.face].sprite->bitmap->h-FaceList[gui_interface_npc->head.face].sprite->border_down)/2);
          sprite_blt(FaceList[gui_interface_npc->head.face].sprite, xp-FaceList[gui_interface_npc->head.face].sprite->border_left,yp, NULL, NULL);
          SDL_SetClipRect(ScreenSurface, NULL);
        }
        else if(gui_interface_npc->head.picture)
        {
          int xp, yp;

          box.x=x+4;
          box.y=y+4;
          box.w=54;
          box.h=54;

          SDL_SetClipRect(ScreenSurface, &box);
          xp = box.x+(box.w/2)-(gui_interface_npc->head.picture->bitmap->w/2);
          yp = box.y+(box.h/2)-(gui_interface_npc->head.picture->bitmap->h/2);
          sprite_blt(gui_interface_npc->head.picture, xp, yp, NULL, NULL);
          SDL_SetClipRect(ScreenSurface, NULL);

        }
      }

      yoff = 79;

      box.x=x+35;
      box.y=y+yoff;
      box.w=295;
      box.h=INTERFACE_WINLEN_NPC;

      blt_window_slider(Bitmaps[BITMAP_NPC_INT_SLIDER], gui_interface_npc->win_length, INTERFACE_WINLEN_NPC,
                gui_interface_npc->yoff*-1, -1, x + 340, y + 90);

      SDL_SetClipRect(ScreenSurface, &box);
      //SDL_FillRect(ScreenSurface, &box, 3);

      yoff+=gui_interface_npc->yoff;
      yoff+=5;


      if(gui_interface_npc->used_flag&GUI_INTERFACE_MESSAGE)
      {
        //len =  get_string_pixel_length(gui_interface_npc->message.title, &BigFont);
        StringBlt(ScreenSurface, &BigFont, gui_interface_npc->message.title, x+width2-len/2, y+yoff, COLOR_WHITE, NULL, NULL);

        StringBlt(ScreenSurface, &BigFont, gui_interface_npc->message.title, x+40, y+yoff, COLOR_HGOLD, NULL, NULL);
        yoff+=26;

        for(i=0;i<gui_interface_npc->message.line_count;i++,yoff+=15)
          StringBlt(ScreenSurface, &MediumFont, gui_interface_npc->message.lines[i], x+40, y+yoff, COLOR_WHITE, NULL, NULL);
      }

      if(gui_interface_npc->link_count)
      {
        yoff+=15;
        for(i=0;i<gui_interface_npc->link_count;i++,yoff+=15)
        {
          if(gui_interface_npc->link_selected == i+1)
            StringBlt(ScreenSurface, &MediumFont, gui_interface_npc->link[i].link, x+40, y+yoff, COLOR_DK_NAVY, NULL, NULL);
          else
            StringBlt(ScreenSurface, &MediumFont, gui_interface_npc->link[i].link, x+40, y+yoff, COLOR_GREEN, NULL, NULL);
        }
      }

      // reward is also used as "objective"
      if(gui_interface_npc->used_flag&GUI_INTERFACE_REWARD)
      {
        //char xbuf[256];
        sprintf(xbuf, "len: %d yoff: %d (%d)", gui_interface_npc->win_length,gui_interface_npc->yoff,INTERFACE_WINLEN_NPC-gui_interface_npc->win_length);

        yoff +=25;
        StringBlt(ScreenSurface, &BigFont, gui_interface_npc->reward.title, x+40, y+yoff, COLOR_HGOLD, NULL, NULL);
        //StringBlt(ScreenSurface, &BigFont, xbuf, x+40, y+yoff, COLOR_WHITE, NULL, NULL);
        yoff+=26;


        for(i=0;i<gui_interface_npc->reward.line_count;i++,yoff+=15)
          StringBlt(ScreenSurface, &MediumFont, gui_interface_npc->reward.lines[i], x+40, y+yoff, COLOR_WHITE, NULL, NULL);

        // only print the "Your rewards:" message when there is one
        if(gui_interface_npc->reward.copper || gui_interface_npc->reward.gold ||
            gui_interface_npc->reward.silver || gui_interface_npc->reward.mithril ||
            gui_interface_npc->icon_count)
        {
          char buf[64];

          if(gui_interface_npc->reward.line_count)
            yoff+=15;

          StringBlt(ScreenSurface, &MediumFont, "Your rewards:", x+40, y+yoff+5, COLOR_WHITE, NULL, NULL);

          if(gui_interface_npc->reward.copper)
          {
            sprite_blt(Bitmaps[BITMAP_COIN_COPPER], x + 110, y + yoff, NULL, NULL);
            sprintf(buf, "%d", gui_interface_npc->reward.copper);
            StringBlt(ScreenSurface, &SystemFont, buf, x+128, y+yoff+18, COLOR_WHITE, NULL, NULL);
          }
          if(gui_interface_npc->reward.silver)
          {
            sprite_blt(Bitmaps[BITMAP_COIN_SILVER], x + 140, y + yoff+6, NULL, NULL);
            sprintf(buf, "%d", gui_interface_npc->reward.silver);
            StringBlt(ScreenSurface, &SystemFont, buf, x+160, y+yoff+18, COLOR_WHITE, NULL, NULL);
          }
          if(gui_interface_npc->reward.gold)
          {
            sprite_blt(Bitmaps[BITMAP_COIN_GOLD], x + 170, y + yoff+6, NULL, NULL);
            sprintf(buf, "%d", gui_interface_npc->reward.gold);
            StringBlt(ScreenSurface, &SystemFont, buf, x+190, y+yoff+18, COLOR_WHITE, NULL, NULL);
          }
          if(gui_interface_npc->reward.mithril)
          {
            sprite_blt(Bitmaps[BITMAP_COIN_MITHRIL], x + 200, y + yoff+9, NULL, NULL);
            sprintf(buf, "%d", gui_interface_npc->reward.mithril);
            StringBlt(ScreenSurface, &SystemFont, buf, x+220, y+yoff+18, COLOR_WHITE, NULL, NULL);
          }

          yoff+=15;
        }
      }

      yoff+=5;
      // present now the icons for rewards or whats searched
      if(gui_interface_npc->icon_count)
      {
        int flag_s = FALSE;

        yoff+=25;
        for(i=0;i<gui_interface_npc->icon_count;i++)
        {
          // we have a 's' to announce a 'S' selection for real rewards?
          if(gui_interface_npc->icon[i].mode == 's' )
          {
            flag_s = TRUE;
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
          for(i=0;i<gui_interface_npc->icon_count;i++)
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

        if(gui_interface_npc->icon_select)
        {
          int t;

          StringBlt(ScreenSurface, &MediumFont, "And one of these (select one):", x+40, y+yoff, COLOR_WHITE, NULL, NULL);
          yoff+=20;
          for(t=1,i=0;i<gui_interface_npc->icon_count;i++)
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

      SDL_SetClipRect(ScreenSurface, NULL);

      if(gui_interface_npc->status == GUI_INTERFACE_STATUS_WAIT)
        return;

      if(gui_interface_npc->used_flag&GUI_INTERFACE_ACCEPT)
      {

        if (add_button(x + 35, y + 443, numButton++, BITMAP_DIALOG_BUTTON_UP,
                 gui_interface_npc->accept.title, gui_interface_npc->accept.title2))
        {
          int ekey=-1;

          if(gui_interface_npc->icon_select && !gui_interface_npc->selected)
          {
            draw_info("select a item first.", COLOR_GREEN);
            sound_play_effect(SOUND_CLICKFAIL, 0, 0, 100);
            return;

          }

          switch(gui_interface_npc->accept.title[0])
          {
            case 'A':
            ekey = SDLK_a;
            break;
            case 'B':
            ekey = SDLK_b;
            break;
            case 'C':
            ekey = SDLK_c;
            break;
            case 'D':
            ekey = SDLK_d;
            break;
            case 'E':
            ekey = SDLK_e;
            break;
            case 'F':
            ekey = SDLK_f;
            break;
            case 'G':
            ekey = SDLK_g;
            break;
            case 'H':
            ekey = SDLK_h;
            break;
            case 'I':
            ekey = SDLK_i;
            break;
            case 'J':
            ekey = SDLK_j;
            break;
            case 'K':
            ekey = SDLK_k;
            break;
            case 'L':
            ekey = SDLK_l;
            break;
            case 'M':
            ekey = SDLK_m;
            break;
            case 'N':
            ekey = SDLK_n;
            break;
            case 'O':
            ekey = SDLK_o;
            break;
            case 'P':
            ekey = SDLK_p;
            break;
            case 'Q':
            ekey = SDLK_q;
            break;
            case 'R':
            ekey = SDLK_r;
            break;
            case 'S':
            ekey = SDLK_s;
            break;
            case 'T':
            ekey = SDLK_t;
            break;
            case 'U':
            ekey = SDLK_u;
            break;
            case 'V':
            ekey = SDLK_v;
            break;
            case 'W':
            ekey = SDLK_w;
            break;
            case 'X':
            ekey = SDLK_x;
            break;
            case 'Y':
            ekey = SDLK_y;
            break;
            case 'Z':
            ekey = SDLK_z;
            break;
          }
          if(ekey != -1)
            check_menu_keys(MENU_NPC, ekey);

          return;
        }

        if(gui_interface_npc->used_flag&GUI_INTERFACE_DECLINE)
        {
          if (add_button(x + 285, y + 443, numButton++, BITMAP_DIALOG_BUTTON_UP,
                   gui_interface_npc->decline.title, gui_interface_npc->decline.title2))
          {
            int ekey=-1;
            switch(gui_interface_npc->decline.title[0])
            {
              case 'A':
              ekey = SDLK_a;
              break;
              case 'B':
              ekey = SDLK_b;
              break;
              case 'C':
              ekey = SDLK_c;
              break;
              case 'D':
              ekey = SDLK_d;
              break;
              case 'E':
              ekey = SDLK_e;
              break;
              case 'F':
              ekey = SDLK_f;
              break;
              case 'G':
              ekey = SDLK_g;
              break;
              case 'H':
              ekey = SDLK_h;
              break;
              case 'I':
              ekey = SDLK_i;
              break;
              case 'J':
              ekey = SDLK_j;
              break;
              case 'K':
              ekey = SDLK_k;
              break;
              case 'L':
              ekey = SDLK_l;
              break;
              case 'M':
              ekey = SDLK_m;
              break;
              case 'N':
              ekey = SDLK_n;
              break;
              case 'O':
              ekey = SDLK_o;
              break;
              case 'P':
              ekey = SDLK_p;
              break;
              case 'Q':
              ekey = SDLK_q;
              break;
              case 'R':
              ekey = SDLK_r;
              break;
              case 'S':
              ekey = SDLK_s;
              break;
              case 'T':
              ekey = SDLK_t;
              break;
              case 'U':
              ekey = SDLK_u;
              break;
              case 'V':
              ekey = SDLK_v;
              break;
              case 'W':
              ekey = SDLK_w;
              break;
              case 'X':
              ekey = SDLK_x;
              break;
              case 'Y':
              ekey = SDLK_y;
              break;
              case 'Z':
              ekey = SDLK_z;
              break;
            }
            if(ekey != -1)
              check_menu_keys(MENU_NPC, ekey);

            //check_menu_keys(MENU_NPC, SDLK_d);
            return;
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
// we have a left click inside the interface -> check it
//================================================================================================
void GuiDialog::gui_interface_mouse(int mouseAction, int mX, int mY)
{
/*
  int mx, my, mxr=e->motion.x,myr=e->motion.y;

  if(!gui_interface_npc)
    return;

  if (e->button.button == 4 || e->button.button == 5) // mousewheel up/down
  {
    if (e->button.button == 4)
      gui_interface_npc->yoff +=6;
    else
      gui_interface_npc->yoff -=6;

    if(gui_interface_npc->yoff < INTERFACE_WINLEN_NPC-gui_interface_npc->win_length)
    {
      gui_interface_npc->yoff = INTERFACE_WINLEN_NPC-gui_interface_npc->win_length;
    }
    if(gui_interface_npc->yoff >0)
    {
      gui_interface_npc->yoff=0;
    }

    return;
  }

  mx = mxr-gui_interface_npc->startx;
  my = myr-gui_interface_npc->starty;

  if(mx >= 345 && mx <= 354 && my >=32 && my <= 41)
  {
    sound_play_effect(SOUND_SCROLL, 0, 0, 100);
    reset_gui_interface();
  }
  else if(mx >= 339 && mx <= 350)
  {
    if(my >=73 && my <= 84)
    {
      gui_interface_npc->yoff +=12;
      if(gui_interface_npc->yoff < INTERFACE_WINLEN_NPC-gui_interface_npc->win_length)
      {
        gui_interface_npc->yoff = INTERFACE_WINLEN_NPC-gui_interface_npc->win_length;
      }
      if(gui_interface_npc->yoff >0)
      {
        gui_interface_npc->yoff=0;
      }
    }
    else if(my >=428 && my <= 437)
    {
      gui_interface_npc->yoff -=12;
      if(gui_interface_npc->yoff < INTERFACE_WINLEN_NPC-gui_interface_npc->win_length)
      {
        gui_interface_npc->yoff = INTERFACE_WINLEN_NPC-gui_interface_npc->win_length;
      }
      if(gui_interface_npc->yoff >0)
      {
        gui_interface_npc->yoff=0;
      }
    }
    return;
  }
  else
  {
    int element, index;
    char *keyword=NULL;

    if(get_interface_line(&element, &index, &keyword, gui_interface_npc->startx, gui_interface_npc->starty, mxr, myr))
    {
      LOG(-1,"%s\n",keyword);
      if(element == GUI_INTERFACE_ICON)
      {
        sound_play_effect(SOUND_GET, 0, 0, 100);
        gui_interface_npc->selected = index;
      }
      else if(element == GUI_INTERFACE_MESSAGE)
      {
        sound_play_effect(SOUND_GET, 0, 0, 100);
        gui_interface_send_command(0, keyword);
      }
      else if(element == GUI_INTERFACE_LINK)
      {
        sound_play_effect(SOUND_GET, 0, 0, 100);
        if(keyword[0]!='/')
          gui_interface_send_command(0, keyword);
        else
          gui_interface_send_command(1, keyword);
      }
    }
  }
*/
}

