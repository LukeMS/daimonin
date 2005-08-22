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

    The author can be reached via e-mail to info@daimonin.net
*/
#include <include.h>

#define INTERFACE_CMD_NO    0
#define INTERFACE_CMD_HEAD  1
#define INTERFACE_CMD_MESSAGE 2
#define INTERFACE_CMD_REWARD 4
#define INTERFACE_CMD_ICON 8
#define INTERFACE_CMD_ACCEPT 16
#define INTERFACE_CMD_DECLINE 32
#define INTERFACE_CMD_LINK 64


static _gui_interface_head *interface_cmd_head(char *data, int *pos)
{
    char *buf, c;
    _gui_interface_head *head=malloc(sizeof(_gui_interface_head));

    memset(head, 0, sizeof(_gui_interface_head));

    (*pos)++;
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        /* c is legal string part - check it is '<' */
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') /* no double >>? then we return */
            {
                (*pos)--;
                return head;
            }
        }

        (*pos)++;
        if(c<=' ')
            continue;

        /* c is part of the head command inside the '<' - lets handle it
         * It must be a command. If it is unknown, return NULL
         */
        switch(c)
        {
            case 'f': /* face for this head */
                 if(!(buf = get_parameter_string(data, pos)))
                     return NULL;
                 strcpy(head->name, buf);
            break;

            case 'b': /* test body */
                if(!(buf = get_parameter_string(data, pos)))
                    return NULL;
                strcpy(head->body_text, buf);
                break;

            default:
                return NULL; /* error */
            break;
        }
    }
    return NULL;
}

static _gui_interface_link *interface_cmd_link(char *data, int *pos)
{
    char *buf, c;
    _gui_interface_link *head=malloc(sizeof(_gui_interface_link));

    memset(head, 0, sizeof(_gui_interface_link));

    (*pos)++;
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        /* c is legal string part - check it is '<' */
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') /* no double >>? then we return */
            {
                (*pos)--;
                return head;
            }
        }

        (*pos)++;
        if(c<=' ')
            continue;

        /* c is part of the head command inside the '<' - lets handle it
         * It must be a command. If it is unknown, return NULL
         */
        switch(c)
        {
            case 't': /* link title/text */
                 if(!(buf = get_parameter_string(data, pos)))
                     return NULL;
                 strcpy(head->link, buf);
            break;

            case 'c': /* link command */
                if(!(buf = get_parameter_string(data, pos)))
                    return NULL;
                strcpy(head->cmd, buf);
                break;

            default:
                return NULL; /* error */
            break;
        }
    }
    return NULL;
}

static _gui_interface_reward *interface_cmd_reward(char *data, int *pos)
{
    char *buf, c;
    _gui_interface_reward *head=malloc(sizeof(_gui_interface_reward));

    memset(head, 0, sizeof(_gui_interface_reward));

    (*pos)++;
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        /* c is legal string part - check it is '<' */
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') /* no double >>? then we return */
            {
                (*pos)--;
                return head;
            }
        }

        (*pos)++;
        if(c<=' ')
            continue;

        switch(c)
        {

        case 't': /* title of the reward */
            if(!(buf = get_parameter_string(data, pos)))
                return NULL;
            strcpy(head->title, buf);
            break;

        case 'b': /* reward body */
            if(!(buf = get_parameter_string(data, pos)))
                return NULL;
            strcpy(head->body_text, buf);
            break;

        case 'c': /* copper cash */
                 if(!(buf = get_parameter_string(data, pos)))
                     return NULL;
                 head->copper =atoi(buf);
            break;

        case 's': /* silver cash */
            if(!(buf = get_parameter_string(data, pos)))
                return NULL;
            head->silver =atoi(buf);
            break;

        case 'g': /* gold cash */
            if(!(buf = get_parameter_string(data, pos)))
                return NULL;
            head->gold =atoi(buf);
            break;

        case 'm': /* mithril cash */
            if(!(buf = get_parameter_string(data, pos)))
                return NULL;
            head->mithril =atoi(buf);
            break;

            default:
                return NULL; /* error */
            break;
        }
    }
    return NULL;
}


static _gui_interface_message *interface_cmd_message(char *data, int *pos)
{
    char *buf, c;
    _gui_interface_message *msg=malloc(sizeof(_gui_interface_message));

    memset(msg, 0, sizeof(_gui_interface_message));

    (*pos)++;
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        /* c is legal string part - check it is '<' */
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') /* no double >>? then we return */
            {
                (*pos)--;
                return msg;
            }
        }

        (*pos)++;
        if(c<=' ')
            continue;

        switch(c)
        {
            case 't': /* title of the message */
                 if(!(buf = get_parameter_string(data, pos)))
                     return NULL;
                 strcpy(msg->title, buf);
            break;

            case 'b': /* message body */
                if(!(buf = get_parameter_string(data, pos)))
                    return NULL;
                strcpy(msg->body_text, buf);
                break;

            default:
                return NULL; /* error */
            break;
        }
    }
    return NULL;
}


static _gui_interface_icon *interface_cmd_icon(char *data, int *pos)
{
    char *buf, c;
    _gui_interface_icon *head=malloc(sizeof(_gui_interface_icon));

    memset(head, 0, sizeof(_gui_interface_icon));

    (*pos)++;
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        /* c is legal string part - check it is '<' */
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') /* no double >>? then we return */
            {
                (*pos)--;
                return head;
            }
        }

        (*pos)++;
        if(c<=' ')
            continue;
        switch(c)
        {
            case 'f': /* face for this icon */
                 if(!(buf = get_parameter_string(data, pos)))
                     return NULL;
                 strcpy(head->name, buf);
            break;

            case 't': /* title of the icon */
                if(!(buf = get_parameter_string(data, pos)))
                    return NULL;
                strcpy(head->title, buf);
                break;

            case 'm': /* mode for this icon */
                if(!(buf = get_parameter_string(data, pos)))
                    return NULL;
                head->mode = buf[0];
                break;

            case 'b': /* test body */
                if(!(buf = get_parameter_string(data, pos)))
                    return NULL;
                strcpy(head->body_text, buf);
                break;

            default:
                return NULL; /* error */
            break;
        }
    }
    return NULL;
}


static _gui_interface_button *interface_cmd_button(char *data, int *pos)
{
    char *buf, c;
    _gui_interface_button *head=malloc(sizeof(_gui_interface_button));

    memset(head, 0, sizeof(_gui_interface_button));

    (*pos)++;
    while((c= *(data+*pos)) != '\0' && c  != 0)
    {
        /* c is legal string part - check it is '<' */
        if(c == '>')
        {
            if(*(data+(*pos)+1) != '>') /* no double >>? then we return */
            {
                (*pos)--;
                return head;
            }
        }

        (*pos)++;
        if(c<=' ')
            continue;

        /* c is part of the head command inside the '<' - lets handle it
         * It must be a command. If it is unknown, return NULL
         */
        switch(c)
        {
            case 't': /* button title */
                if(!(buf = get_parameter_string(data, pos)))
                    return NULL;
                strcpy(head->title, buf);
                break;

            case 'c': /* button command */
                if(!(buf = get_parameter_string(data, pos)))
                    return NULL;
                strcpy(head->command, buf);
                break;

            default:
                return NULL; /* error */
            break;
        }
    }
    return NULL;
}


/* clear & reset the gui interface */
void reset_gui_interface(void)
{
    map_udate_flag = 2;
    interface_mode = INTERFACE_MODE_NO;
    if(gui_interface_npc)
    {
        int s;
        for(s=0;s<gui_interface_npc->icon_count;s++)
            free(gui_interface_npc->icon[s].picture);
        free(gui_interface_npc);
    }
    gui_interface_npc = NULL;
    if (cpl.menustatus == MENU_NPC)
        cpl.menustatus = MENU_NO;

}

static _gui_interface_struct *format_gui_interface(_gui_interface_struct *gui_int)
{
    int s;


    if(gui_int->used_flag&GUI_INTERFACE_HEAD)
    {
        gui_int->head.face = get_bmap_id(gui_int->head.name);

        if(gui_int->head.body_text[0]=='\0')
            strcpy(gui_int->head.body_text, cpl.target_name?cpl.target_name:"");
    }

    /* sort out the message text body to sigle lines */
    if(gui_int->used_flag&GUI_INTERFACE_MESSAGE)
    {
        int i, c=0;

        gui_int->message.line_count=0;
        for(i=0;;i++)
        {
            if(gui_int->message.body_text[i]==0x0d)
                continue;
            if(gui_int->message.body_text[i]==0x0a || gui_int->message.body_text[i]=='\0')
            {
                gui_int->message.lines[gui_int->message.line_count][c]='\0';
        // draw_info(gui_int->message.lines[gui_int->message.line_count], COLOR_YELLOW);
                gui_int->message.line_count++;
                if(gui_int->message.body_text[i]=='\0')
                    break;
                c=0;
            }
            else
            {
                gui_int->message.lines[gui_int->message.line_count][c++]=gui_int->message.body_text[i];
            }

            if(gui_int->message.line_count>=INTERFACE_MAX_LINE || c>=INTERFACE_MAX_CHAR )
            {
                LOG(LOG_ERROR, "ERROR: interface call out of borders: %s\n", gui_int->message.body_text);
                break;
            }
        }
    }

    if(gui_int->used_flag&GUI_INTERFACE_REWARD)
    {
        int i, c=0;

        gui_int->reward.line_count=0;
        for(i=0;;i++)
        {
            if(gui_int->reward.body_text[i]==0x0d)
                continue;
            if(gui_int->reward.body_text[i]==0x0a || gui_int->reward.body_text[i]=='\0')
            {
                gui_int->reward.lines[gui_int->reward.line_count][c]='\0';
                // draw_info(gui_int->message.lines[gui_int->message.line_count], COLOR_YELLOW);
                gui_int->reward.line_count++;
                if(gui_int->reward.body_text[i]=='\0')
                    break;
                c=0;
            }
            else
            {
                gui_int->reward.lines[gui_int->reward.line_count][c++]=gui_int->reward.body_text[i];
            }

            if(gui_int->reward.line_count>=INTERFACE_MAX_LINE || c>=INTERFACE_MAX_CHAR )
            {
                LOG(LOG_ERROR, "ERROR: interface call out of borders: %s\n", gui_int->reward.body_text);
                break;
            }
        }
    }

    /* icons */
    /* search for the bmap num id's and load/request them if possible */
    for(s=0;s<gui_int->icon_count;s++)
    {

        if(gui_int->icon[s].mode == 'S')
            gui_int->icon_select = TRUE;
        gui_int->icon[s].element.face = get_bmap_id(gui_int->icon[s].name);
        if(gui_int->icon[s].element.face==-1)
        {
            char line[256];
            sprintf(line, "%s%s.png", GetIconDirectory(), gui_int->icon[s].name);
            gui_int->icon[s].picture = sprite_load_file(line, 0);
        }
    }

    /* prepare the buttons (titles) */
    if(gui_int->used_flag&GUI_INTERFACE_DECLINE)
    {
        if(gui_int->decline.title[0] != '\0')
        {
            sprintf(gui_int->decline.title2,"~%c~%s", gui_int->decline.title[0],gui_int->decline.title+1);
        }
        else
        {
            strcpy(gui_int->decline.title,"Decline");
            strcpy(gui_int->decline.title2,"~D~ecline");
        }
    }

    if(gui_int->used_flag&GUI_INTERFACE_ACCEPT)
    {
        if(gui_int->accept.title[0] != '\0')
        {
            sprintf(gui_int->accept.title2,"~%c~%s", gui_int->accept.title[0],gui_int->accept.title+1);
        }
        else
        {
            strcpy(gui_int->accept.title,"Accept");
            strcpy(gui_int->accept.title2,"~A~ccept");
        }
    }

    return gui_int;
}

/* called from commands.c after we got a interface command */
_gui_interface_struct *load_gui_interface(int mode, char *data, int len, int pos)
{
    int flag_start=0, flag_end=0;
    char c;
    /*buf[256];*/
    _gui_interface_head *head_tmp;
    _gui_interface_message *message_tmp;
    _gui_interface_reward *reward_tmp;
    _gui_interface_link *link_tmp;
    _gui_interface_icon *icon_tmp;
    _gui_interface_button *button_tmp;
    int cmd = INTERFACE_CMD_NO;      /* we have a open '<' and a command is active */
    int cmd_mode = INTERFACE_CMD_NO; /* when we collect outside a cmd tag strings,
                                      * the string is related to this cmd
                                      */

    _gui_interface_struct *gui_int = malloc(sizeof(_gui_interface_struct));
    memset(gui_int,0,sizeof(_gui_interface_struct));

    for(;len>pos;pos++)
    {
        c = *(data+pos);

        if(c == '<')
        {
            if(flag_end==1)
            {
                if(flag_end == 2) /* bug */
                {
                    /*draw_info("ERROR: bad interface string (flag end error)", COLOR_RED);*/
                    LOG(LOG_ERROR, "ERROR: bad interface string (flag end error): %s\n", data);
                    free(gui_int);
                    return NULL;
                }

                /* our char before this was a '>' - now we get a '<' */
                flag_start=0;
                flag_end=0;
                cmd_mode = cmd;
                cmd = INTERFACE_CMD_NO;
            }

            if(flag_start) /* double << ?*/
            {
                if(flag_start == 2) /* bug */
                {
                    /*draw_info("ERROR: bad interface string (flag start error)", COLOR_RED);*/
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
            /* we have a single '<' or '>'? */
            if(flag_start==1)
            {
                flag_start=2;
                /* This char is a command marker */
                /*
                sprintf(buf, "found cmd: %c", c);
                draw_info(buf, COLOR_GREEN);
                */

                cmd_mode = INTERFACE_CMD_NO;

                switch(c)
                {
                    case 'h': /* head with picture & name this interface comes from */
                        cmd = INTERFACE_CMD_HEAD;
                        head_tmp = interface_cmd_head(data, &pos);
                        if(!head_tmp)
                        {
                            free(gui_int);
                            return NULL;
                        }
                        memcpy(&gui_int->head, head_tmp,sizeof(_gui_interface_head));
                        gui_int->used_flag |=GUI_INTERFACE_HEAD;
                        break;

                    case 'm': /* title & text - what he has to say */
                        cmd = INTERFACE_CMD_MESSAGE;
                        message_tmp = interface_cmd_message(data, &pos);
                        if(!message_tmp)
                        {
                            free(gui_int);
                            return NULL;
                        }
                        memcpy(&gui_int->message, message_tmp,sizeof(_gui_interface_message));
                        gui_int->used_flag |=GUI_INTERFACE_MESSAGE;
                        break;

                    case 'r': /* title & text - what he has to say */
                        cmd = INTERFACE_CMD_REWARD;
                        reward_tmp = interface_cmd_reward(data, &pos);
                        if(!reward_tmp)
                        {
                            free(gui_int);
                            return NULL;
                        }
                        memcpy(&gui_int->reward, reward_tmp,sizeof(_gui_interface_reward));
                        gui_int->used_flag |=GUI_INTERFACE_REWARD;
                        break;

                    case 'l': /* define a "link" string line */
                        cmd = INTERFACE_CMD_LINK;
                        link_tmp = interface_cmd_link(data, &pos);
                        if(!link_tmp)
                        {
                            free(gui_int);
                            return NULL;
                        }
                        memcpy(&gui_int->link[gui_int->link_count++], link_tmp,sizeof(_gui_interface_link));
                        break;


                    case 'i': /* define a "icon" - graphical presentation of reward or message part */
                        cmd = INTERFACE_CMD_ICON;
                        icon_tmp = interface_cmd_icon(data, &pos);
                        if(!icon_tmp)
                        {
                            free(gui_int);
                            return NULL;
                        }
                        memcpy(&gui_int->icon[gui_int->icon_count++], icon_tmp,sizeof(_gui_interface_icon));
                        break;

                    case 'a': /* define accept button */
                        cmd = INTERFACE_CMD_ACCEPT;
                        button_tmp = interface_cmd_button(data, &pos);
                        if(!button_tmp)
                        {
                            free(gui_int);
                            return NULL;
                        }
                        memcpy(&gui_int->accept, button_tmp,sizeof(_gui_interface_button));
                        gui_int->used_flag |=GUI_INTERFACE_ACCEPT;
                        break;

                    case 'd': /* define decline button */
                        cmd = INTERFACE_CMD_DECLINE;
                        button_tmp = interface_cmd_button(data, &pos);
                        if(!button_tmp)
                        {
                            free(gui_int);
                            return NULL;
                        }
                        memcpy(&gui_int->decline, button_tmp,sizeof(_gui_interface_button));
                        gui_int->used_flag |=GUI_INTERFACE_DECLINE;
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
                /* close this command - perhaps we stay string collect mode for it */
                /*
                sprintf(buf, "close cmd");
                draw_info(buf, COLOR_GREEN);
                */
            }
        normal_char:;
            /* we don't have "text" between the tags (<> <>) atm */
        }
    }

    /* if we are here, we have a legal gui_int structure.
     * Now lets create a legal formular and preprocess some structures.
     */

     gui_int = format_gui_interface(gui_int);
     return gui_int;
}


/* send a command from the gui to server.
 * if mode is 1, its a real command.
 * mode 0 means to add /talk first.
 */
void gui_interface_send_command(int mode, char *cmd)
{
    char msg[1024];

    if(mode)
    {
        send_command(cmd, -1, SC_NORMAL);
        /*
        sprintf(msg,"You talk to %s", cpl.target_name?cpl.target_name:"");
        draw_info(msg,COLOR_WHITE);*/
    }
    else
    {
        char buf[1024];
        sprintf(buf,"/talk %s", cmd);
        send_command(buf, -1, SC_NORMAL);
        sprintf(msg,"Talking to %s: %s", cpl.target_name?cpl.target_name:"", cmd);
        draw_info(msg,COLOR_WHITE);
    }

    gui_interface_npc->status = GUI_INTERFACE_STATUS_WAIT;
}
