/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#include <OISKeyboard.h>
#include <OgreStringConverter.h>
#include "logger.h"
#include "profiler.h"
#include "gui/gui_manager.h"
#include "network.h"
#include "network_cmd_interface.h"
#include "sound.h"

using namespace Ogre;

//================================================================================================
// .
//================================================================================================
void Network::InterfaceCmd(uchar *data, int len)
{
    PROFILE()
    //TileManager::getSingleton().map_udate_flag = 2;
    /*
        if ((gui_interface_npc && gui_interface_npc->status != INTERFACE_STATUS_WAIT) &&
                ((!len && cpl.menustatus == MENU_NPC) || (len && cpl.menustatus != MENU_NPC)))
        {
            //sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        }
    */

    /*
    String str = "len: " + StringConverter::toString(len);
    GuiManager::getSingleton().print(GuiManager::GUI_LIST_CHATWIN, str.c_str());
    */
    CmdInterface::getSingleton().reset();
    //if (len)
    {
        int mode = *data;
        int pos =1;
        if (!CmdInterface::getSingleton().load(mode, (char*)data, len, pos))
        {
            Logger::log().error() << Logger::ICON_CLIENT << "INVALID GUI CMD";
            return;
        }
        CmdInterface::getSingleton().show();


        /*
                gui_interface_npc->win_length = precalc_interface_npc();
                interface_mode = mode;
                cpl.menustatus = MENU_NPC;
                gui_interface_npc->startx = 400-(Bitmaps[BITMAP_NPC_INTERFACE]->bitmap->w / 2);
                gui_interface_npc->starty = 50;
                // Prefilled (and focused) textfield
                if (gui_interface_npc->used_flag&INTERFACE_TEXTFIELD)
                {
                    gui_interface_npc->input_flag = true;
                    open_input_mode(240);
                    textwin_putstring(gui_interface_npc->textfield.text);
                    cpl.input_mode = INPUT_MODE_NPCDIALOG;
                    HistoryPos = 0;
                }
        */
    }
}

//================================================================================================
//
//================================================================================================
CmdInterface::CmdInterface()
{
    PROFILE()
}

//================================================================================================
//
//================================================================================================
CmdInterface::~CmdInterface()
{
    PROFILE()
}

//================================================================================================
//
//================================================================================================
char CmdInterface::parseParameter(char *data, int &pos)
{
    PROFILE()
    char c;
    while ((c= *(data + pos)))
    {
        // c is legal string part - check it is '>'
        if (c == '>')
        {
            if (*(data + pos +1) != '>') // no double >>? then we return
            {
                --pos;
                return -1;
            }
        }
        ++pos;
        if (c > ' ')
            return c;
    }
    return 0;
}

//================================================================================================
// this function gets a ="xxxxxxx" string from a line.
// It removes the =" and the last " and returns the string in a static buffer.
//================================================================================================
char *CmdInterface::get_parameter_string(char *data, int &pos)
{
    PROFILE()
    static char buf[4024];
    char *start_ptr = strchr(data+pos,'"');
    if (!start_ptr) return (char*)""; // error
    char *end_ptr = strchr(++start_ptr,'"');
    if (!end_ptr) return (char*)""; // error
    strncpy(buf, start_ptr, end_ptr-start_ptr);
    buf[end_ptr-start_ptr]=0;
    pos+= ++end_ptr-(data+pos);
    return buf;
}

//================================================================================================
//
//================================================================================================
bool CmdInterface::cmd_head(char *data, int &pos)
{
    PROFILE()
    mHead.name = "";
    mHead.body_text = "";
    char c;
    while ((c = parseParameter(data, pos)))
    {
        if (c < 0) return true;
        switch (c)
        {
            case 'f': // face for this head
                mHead.name = get_parameter_string(data, pos);
                break;
            case 'b': // test body
                mHead.body_text = get_parameter_string(data, pos);
                break;
            default:
                return false; // error
        }
    }
    return false;
}

//================================================================================================
//
//================================================================================================
bool CmdInterface::cmd_link(char *data, int &pos)
{
    PROFILE()
    mLink[mLink_count].link ="";
    mLink[mLink_count].cmd  ="";
    char c;
    while ((c = parseParameter(data, pos)))
    {
        if (c < 0) return true;
        switch (c)
        {
            case 't': // link title/text
                mLink[mLink_count].link = get_parameter_string(data, pos);
                break;
            case 'c': // link command
                mLink[mLink_count].cmd = get_parameter_string(data, pos);
                if (!mLink[mLink_count].cmd.empty() && mLink[mLink_count].cmd[0] != '/')
                    mLink[mLink_count].cmd = "/talk " + mLink[mLink_count].cmd;
                break;
            default:
                return false; // error
        }
    }
    return false;
}

//================================================================================================
// internal server string - the client use it as hint to use /tx instead of /talk
//================================================================================================
bool CmdInterface::cmd_who(char *data, int &pos)
{
    PROFILE()
    mWho.body = "";
    char c;
    while ((c = parseParameter(data, pos)))
    {
        if (c < 0) return true;
        switch (c)
        {
            case 'b': // link title/text
                mWho.body = get_parameter_string(data, pos);
                break;
            default:
                return false; // error
        }
    }
    return false;
}

//================================================================================================
//
//================================================================================================
bool CmdInterface::cmd_reward(char *data, int &pos)
{
    PROFILE()
    char *buf, c;
    mReward.title = "Description"; // default title
    mReward.body_text = "";
    mReward.gold =0;
    mReward.copper =0;
    mReward.silver =0;
    mReward.mithril =0;
    while ((c = parseParameter(data, pos)))
    {
        if (c < 0) return true;
        switch (c)
        {
            case 't': // title of the reward
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mReward.title = buf;
                break;
            case 'b': // reward body
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mReward.body_text = buf;
                break;
            case 'c': // copper cash
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mReward.copper =atoi(buf);
                break;
            case 's': // silver cash
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mReward.silver =atoi(buf);
                break;
            case 'g': // gold cash
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mReward.gold =atoi(buf);
                break;
            case 'm': // mithril cash
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mReward.mithril =atoi(buf);
                break;
            default:
                return false; // error
        }
    }
    return false;
}

//================================================================================================
//
//================================================================================================
bool CmdInterface::cmd_message(char *data, int &pos)
{
    PROFILE()
    char c;
    mMessage.title = "";
    mMessage.body_text = "";
    while ((c = parseParameter(data, pos)))
    {
        if (c < 0) return true;
        switch (c)
        {
            case 't': // title of the message
                mMessage.title = get_parameter_string(data, pos);
                break;
            case 'b': // message body
                mMessage.body_text = get_parameter_string(data, pos);
                break;
            default:
                return false; // error
        }
    }
    return false;
}

//================================================================================================
//
//================================================================================================
bool CmdInterface::cmd_xtended(char *data, int &pos)
{
    PROFILE()
    mXtended.title = "";
    mXtended.body_text = "";
    char c;
    while ((c = parseParameter(data, pos)))
    {
        if (c < 0) return true;
        switch (c)
        {
            case 't': // title of the message
                mXtended.title = get_parameter_string(data, pos);
                break;
            case 'b': // message body
                mXtended.body_text = get_parameter_string(data, pos);
                break;
            default:
                return false; // error
        }
    }
    return false;
}

//================================================================================================
//
//================================================================================================
bool CmdInterface::cmd_icon(char *data, int &pos)
{
    PROFILE()
    mIcon[mIcon_count].name = "";
    mIcon[mIcon_count].title = "";
    mIcon[mIcon_count].body_text = "";
    mIcon[mIcon_count].mode = 0;
    char *buf, c;
    while ((c = parseParameter(data, pos)))
    {
        if (c < 0) return true;
        switch (c)
        {
            case 'f': // face for this icon
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mIcon[mIcon_count].name = buf;
                break;
            case 't': // title of the icon
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mIcon[mIcon_count].title = buf;
                break;
            case 'm': // mode for this icon
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mIcon[mIcon_count].mode = buf[0];
                break;
            case 'b': // test body
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                mIcon[mIcon_count].body_text = buf;
                break;
            default:
                return false; // error
        }
    }
    return false;
}

//================================================================================================
// Parse a button.
//================================================================================================
bool CmdInterface::cmd_button(Button &button, char *data, int &pos)
{
    PROFILE()
    char *buf, c;
    button.label = "";
    button.command = "";
    while ((c = parseParameter(data, pos)))
    {
        if (c < 0) return true;
        switch (c)
        {
            case 't': // label.
                if (!(buf = get_parameter_string(data, pos)))
                    return false;
                button.label = "~";
                button.label+= (char)toupper(buf[0]);
                button.label+= "~";
                button.label+= buf+1;
                break;
            case 'c': // command.
                button.command = get_parameter_string(data, pos);
                if (!button.command.empty() && button.command[0] != '/')
                    button.command = "/talk " + button.command;
                break;
            default:
                return false; // error
        }
    }
    return false;
}

//================================================================================================
// Parse a <t b=""> textfield command
//================================================================================================
bool CmdInterface::cmd_textfield(char *data, int &pos)
{
    PROFILE()
    mTextfield.text = "";
    char c;
    while ((c = parseParameter(data, pos)))
    {
        if (c < 0) return true;
        switch (c)
        {
            case 'b': // Textfield text
                mTextfield.text = get_parameter_string(data, pos);
                break;
            default:
                return false; // error
        }
    }
    return false;
}

//================================================================================================
//
//================================================================================================
void CmdInterface::format_gui_interface()
{
    PROFILE()
    mMode= INTERFACE_MODE_NPC;
    if (mUsed_flag & INTERFACE_WHO)
    {
        if (mWho.body[0] == 'Q') mMode= INTERFACE_MODE_QLIST;
    }
    if (mUsed_flag & INTERFACE_ICON)
    {
        /*
        char *tmp;
        for (int s=0; s< mIcon_count; ++s)
        {
            mIcon[s].second_line = 0;
            if ((tmp = strchr(mIcon[s].body_text, '\n')))
            {
                mIcon[s].second_line = tmp+1;
                *tmp = 0;
            }
        }
        */
    }
    if (mUsed_flag & INTERFACE_HEAD)
    {
        /*
        mHead.face = get_bmap_id(mHead.name);
        if (mHead.face == -1)
        {
                   char line[256];
                   (line, "%s%s.png", GetIconDirectory(), head.name);
                   head.picture = sprite_load_file(line, 0);
        }
        */
        if (!mHead.body_text.empty() && mHead.body_text[0]== '\0')
        {
            // strcpy(head.body_text, cpl.target_name?cpl.target_name:"");
        }
    }
    // overrule/extend the message block
    if (mUsed_flag & INTERFACE_XTENDED)
    {
        mMessage.title = mXtended.title;
        mMessage.body_text += mXtended.body_text;
        mUsed_flag&=~INTERFACE_XTENDED;
    }
    // sort out the message text body to single lines
    if (mUsed_flag & INTERFACE_MESSAGE)
    {
        // done by gui_listbox.
    }
    if (mUsed_flag&INTERFACE_REWARD)
    {
        // done by gui_listbox.
    }
    // icons
    // search for the bmap num id's and load/request them if possible
    for (int s=0; s < mIcon_count; ++s)
    {
        mIcon_select = (mIcon[s].mode == 'S')?true:false;
        //mIcon[s].element.face = get_bmap_id(mIcon[s].name);
        //if (mIcon[s].element.face==-1)
        {
            // char line[256];
            // sprintf(line, "%s%s.png", GetIconDirectory(), mIcon[s].name);
            // mIcon[s].picture = sprite_load_file(line, 0);
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Buttons.
    // ////////////////////////////////////////////////////////////////////
    if (mUsed_flag & INTERFACE_DECLINE && !(mUsed_flag & INTERFACE_ACCEPT))
    {
        Logger::log().error() << Logger::ICON_CLIENT << " if (mUsed_flag & INTERFACE_DECLINE && !(mUsed_flag & INTERFACE_ACCEPT))";

        if (butDecline.label.empty())
            butDecline.label = "~D~ecline";
        mUsed_flag |= INTERFACE_ACCEPT;
        butAccept.command ="";
        butAccept.label ="~A~ccept";
    }
    else if (mUsed_flag & INTERFACE_ACCEPT)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "else if (mUsed_flag & INTERFACE_ACCEPT)";
        if (butAccept.label.empty())
            butAccept.label = "~A~ccept";
        if (mUsed_flag & INTERFACE_DECLINE)
        {
            if (butDecline.label.empty())
                butDecline.label = "~D~ecline";
        }
        else
        {
            mUsed_flag |=INTERFACE_DECLINE;
            butDecline.command= "";
            butDecline.label  = "";
        }
    }
    else if (mUsed_flag & INTERFACE_BUTTON) // means: single button
    {
        Logger::log().error() << Logger::ICON_CLIENT << "else if (mUsed_flag & INTERFACE_BUTTON) // means: single button";
        mUsed_flag |=INTERFACE_ACCEPT; // yes, thats right! we fake the accept button
        if (butAccept.label.empty())
        {
            butAccept.label= "~B~ye";
            butAccept.command="";
        }
        butDecline.label="";
        butDecline.command="";
    }
    else /* no accept/decline and no button? set it to 'Bye' default button */
    {
        mUsed_flag |=INTERFACE_ACCEPT;
        butDecline.label  = "";
        butDecline.command= "";
        butAccept.label= "~B~ye";
        butAccept.command="";
    }
}

//================================================================================================
// clear & reset the gui interface
//================================================================================================
void CmdInterface::reset()
{
    PROFILE()
    mMessage.line_count =0;
    mXtended.line_count =0;
    mReward.line_count  =0;
    mLink_count =0;
    mUsed_flag =0;
    mStatus = 0;
    GuiManager::getSingleton().clear(GuiManager::LIST_NPC);
}

//================================================================================================
// called from commands.c after we got a interface command
//================================================================================================
bool CmdInterface::load(int /*mode*/, char *data, int len, int pos)
{
    PROFILE()
    int cmd      = INTERFACE_CMD_NO; // we have a open '<' and a command is active the string is related to this cmd.
    //int cmd_mode = INTERFACE_CMD_NO; // when we collect outside a cmd tag strings,
    int flag_start=0, flag_end=0;
    for (char c; len > pos; ++pos)
    {
        c = *(data + pos);
        if (c == '<')
        {
            if (flag_end==1)
            {
                if (flag_end == 2) // bug
                {
                    Logger::log().error() << Logger::ICON_CLIENT << "Bad interface string (flag end error): " << data;
                    return false;
                }
                // our char before this was a '>' - now we get a '<'
                flag_start=0;
                flag_end=0;
                //cmd_mode = cmd;
                cmd = INTERFACE_CMD_NO;
            }
            if (flag_start) // double << ?
            {
                if (flag_start == 2) // bug
                {
                    Logger::log().error() << Logger::ICON_CLIENT << "Bad interface string (flag end error): " << data;
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
                Logger::log().info() << Logger::ICON_CLIENT << "found cmd: "<< c;
                //cmd_mode = INTERFACE_CMD_NO;
                ++pos;
                switch (c)
                {
                    case 'h': // head with picture & name this interface comes from
                        cmd = INTERFACE_CMD_HEAD;
                        if (!cmd_head(data, pos)) return false;
                        mUsed_flag |=INTERFACE_HEAD;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_HEAD";
                        break;
                    case 'm': // title & text - what he has to say
                        cmd = INTERFACE_CMD_MESSAGE;
                        if (!cmd_message(data, pos)) return false;
                        mUsed_flag |=INTERFACE_MESSAGE;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_MESSAGE";
                        break;
                    case 'r': // reward info
                        cmd = INTERFACE_CMD_REWARD;
                        if (!cmd_reward(data, pos)) return false;
                        mUsed_flag |=INTERFACE_REWARD;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_REWARD";
                        break;
                    case 'w': // who info
                        cmd = INTERFACE_CMD_WHO;
                        if (!cmd_who(data, pos)) return false;
                        mUsed_flag |=INTERFACE_WHO;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_WHO";
                        break;
                    case 'x': // xtended info
                        cmd = INTERFACE_CMD_XTENDED;
                        if (!cmd_xtended(data, pos)) return false;
                        mUsed_flag |=INTERFACE_XTENDED;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_XTENDED";
                        break;
                    case 'l': // define a "link" string line
                        cmd = INTERFACE_CMD_LINK;
                        if (!cmd_link(data, pos)) return false;
                        ++mLink_count;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_LINK";
                        break;
                    case 'i': // define a "icon" - graphical presentation of reward or message part
                        cmd = INTERFACE_CMD_ICON;
                        if (!cmd_icon(data, pos)) return false;
                        ++mIcon_count;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_ICON";
                        break;
                    case 'a': // define accept button
                        cmd = INTERFACE_CMD_ACCEPT;
                        if (!cmd_button(butAccept, data, pos)) return false;
                        mUsed_flag |=INTERFACE_ACCEPT;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_ACCEPT";
                        break;
                    case 'b': // define single button
                        cmd = INTERFACE_CMD_BUTTON;
                        if (!cmd_button(butAccept, data, pos)) return false;
                        // we use the accept button struct for single buttons too
                        mUsed_flag |=INTERFACE_BUTTON;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_BUTTON";
                        break;
                    case 'd': // define decline button
                        cmd = INTERFACE_CMD_DECLINE;
                        if (!cmd_button(butDecline, data, pos)) return false;
                        mUsed_flag |=INTERFACE_DECLINE;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_DECLINE";
                        break;
                    case 't': // textfield contents
                        cmd = INTERFACE_CMD_TEXTFIELD;
                        if (!cmd_textfield(data, pos)) return false;
                        mUsed_flag |=INTERFACE_TEXTFIELD;
                        Logger::log().error() << Logger::ICON_CLIENT << "INTERFACE_CMD_TEXTFIELD";
                        break;
                    default:
                        Logger::log().error() << Logger::ICON_CLIENT << "NPC Dialog -> Bad command tag: " << data;
                        return false;
                }
            }
            else if (flag_end==1)
            {
                flag_end=0;
                flag_start=0;
                //cmd_mode = cmd;
                cmd = INTERFACE_CMD_NO;
                // close this command - perhaps we stay string collect mode for it
            }
normal_char:
            ; // we don't have "text" between the tags (<> <>) atm
        }
    }
    // if we are here, we have a legal structure.
    // Now create a legal formular and preprocess some structures.
    format_gui_interface();


    if (butAccept.label.empty())
        GuiManager::getSingleton().setVisible(GuiManager::BUTTON_NPC_ACCEPT, false);
    else
    {
        GuiManager::getSingleton().setText(GuiManager::BUTTON_NPC_ACCEPT, butAccept.label.c_str());
        GuiManager::getSingleton().setVisible(GuiManager::BUTTON_NPC_ACCEPT, true);
    }
    if (butDecline.label.empty())
        GuiManager::getSingleton().setVisible(GuiManager::BUTTON_NPC_DECLINE, false);
    else
    {
        GuiManager::getSingleton().setText(GuiManager::BUTTON_NPC_DECLINE, butDecline.label.c_str());
        GuiManager::getSingleton().setVisible(GuiManager::BUTTON_NPC_DECLINE, true);
    }
    GuiManager::getSingleton().showWindow(GuiManager::WIN_NPCDIALOG, true);
    return true;
}

//================================================================================================
// send a command from the gui to server.
// if mode is 1, its a real command.
// mode 0 or 2 means to add /talk first.
// mode 2 means manual input / add to history
//================================================================================================
void CmdInterface::sendCommand(int mode, char *cmd)
{
    PROFILE()
    /*
        if (mStatus == INTERFACE_STATUS_WAIT) return;
        if (mUsed_flag & INTERFACE_WHO)
        {
            if (!strncmp(cmd,"/talk ",6))
                cmd +=6;
            char buf[MAX_BUF];
            (buf, "tx %s %s", who.body, cmd);
            Network::getSingleton().cs_write_string(buf, (int)strlen(buf));
        }
        else
        {
            if (mode == 1)
            {
                Network::getSingleton().send_command(cmd, -1, Network::SC_NORMAL);
                // if(!strncmp(cmd, "/talk ", 6)) textwin_addhistory(cmd);
            }
            else
            {
                char buf[1024];
                (buf,"/talk %s", cmd);
                Network::getSingleton().send_command(buf, -1, Network::SC_NORMAL);
                //if (mode == 2) textwin_addhistory(buf);
                (buf, "Talking about: %s", cmd);
                GuiManager::getSingleton().setElementText(WIN_NPCDIALOG, GUI_TEXTBOX_NPC_HEADLINE, head.body_text);
            }
        }
        mStatus = INTERFACE_STATUS_WAIT;
    */

    if (mUsed_flag & INTERFACE_WHO)
    {
        if (!strncmp(cmd,"/talk ",6))
            cmd +=6;
        // client_send_tell_extended(who.body, cmd);
    }
    else
    {
        if (mode == 1)
        {
            //Network::getSingleton().send_command(cmd, -1, Network::SC_NORMAL);
            /* if(strncmp(cmd, "/talk ", 6) == 0)
             textwin_addhistory(cmd); */
        }
        else
        {
            String strBuf = "/talk "; strBuf+= cmd;
            //Network::getSingleton().send_command(strBuf.c_str(), -1, Network::SC_NORMAL);
            String msgBuf = "Talking about: "; msgBuf+=cmd;
            //draw_info(msg,COLOR_WHITE);
            //if(mode == 2) textwin_addhistory(buf);
        }
    }
//    reset_keys();
// reset_input_mode();
//    cpl.input_mode = INPUT_MODE_NO;
    mStatus = INTERFACE_STATUS_WAIT;
}

//================================================================================================
// Display the npc interface.
//================================================================================================
void CmdInterface::show()
{
    PROFILE()
    if (mUsed_flag & INTERFACE_HEAD)
    {
        // print head
        GuiManager::getSingleton().setText(GuiManager::TEXTBOX_NPC_HEADLINE, mHead.body_text.c_str());
    }
    if (mUsed_flag & INTERFACE_MESSAGE)
    {
        mMessage.line_count = GuiManager::getSingleton().print(GuiManager::LIST_NPC, mMessage.title.c_str(), GuiManager::COLOR_YELLOW);
        mMessage.line_count+= GuiManager::getSingleton().print(GuiManager::LIST_NPC, "");
        mMessage.line_count+= GuiManager::getSingleton().print(GuiManager::LIST_NPC, mMessage.body_text.c_str());
    }
    if (mLink_count)
    {
        mReward.line_count = GuiManager::getSingleton().print(GuiManager::LIST_NPC, "");
        for (int i=0; i< mLink_count; ++i)
        {
            String link = "^" + mLink[i].link + "�" + mLink[i].cmd + "^";
            mReward.line_count = GuiManager::getSingleton().print(GuiManager::LIST_NPC, link.c_str(), GuiManager::COLOR_GREEN);
        }
    }
    // reward is also used as "objective"
    if (mUsed_flag & INTERFACE_REWARD)
    {
        mReward.line_count = GuiManager::getSingleton().print(GuiManager::LIST_NPC, "");
        mReward.line_count+= GuiManager::getSingleton().print(GuiManager::LIST_NPC, mReward.title.c_str(), GuiManager::COLOR_YELLOW);
        mReward.line_count+= GuiManager::getSingleton().print(GuiManager::LIST_NPC, "");
        mReward.line_count+= GuiManager::getSingleton().print(GuiManager::LIST_NPC, mReward.body_text.c_str());
        // only print the "Your rewards:" message when there is one
        if (mReward.copper || mReward.gold || mReward.silver || mReward.mithril || mIcon_count)
        {
            String strMsg = "Your rewards: ";
            if (mReward.copper)
            {
                //sprite_blt(Bitmaps[BITMAP_COIN_COPPER], x + 110, y + yoff, NULL, NULL);
                strMsg+= StringConverter::toString(mReward.copper);
            }
            if (mReward.silver)
            {
                //sprite_blt(Bitmaps[BITMAP_COIN_SILVER], x + 140, y + yoff+6, NULL, NULL);
                strMsg+= StringConverter::toString(mReward.silver);
            }
            if (mReward.gold)
            {
                //sprite_blt(Bitmaps[BITMAP_COIN_GOLD], x + 170, y + yoff+6, NULL, NULL);
                strMsg+= StringConverter::toString(mReward.gold);
            }
            if (mReward.mithril)
            {
                //sprite_blt(Bitmaps[BITMAP_COIN_MITHRIL], x + 200, y + yoff+9, NULL, NULL);
                strMsg+= StringConverter::toString(mReward.mithril);
            }
            GuiManager::getSingleton().print(GuiManager::LIST_NPC, strMsg.c_str());
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


    // Select link || enter text.
    if (mInput_flag)
    {
        // StringBlt(ScreenSurface, &MediumFont, show_input_string(InputString, &MediumFont,box.w-10),box.x+5 ,box.y, COLOR_WHITE, NULL, NULL);
    }
    else
    {
        if (mLink_selected)
        {
            StringBlt(ScreenSurface, &MediumFont, gui_interface_npc->link[gui_interface_npc->link_selected-1].link, box.x+5, box.y-1, COLOR_DK_NAVY, NULL, NULL);
        }
    }
    */
}

//================================================================================================
//
//================================================================================================
bool CmdInterface::keyEvent(const char /*keyChar*/, const uchar /*key*/)
{
    PROFILE()
    return false;
}

//================================================================================================
// Accept or Decline button was pressed.
//================================================================================================
void CmdInterface::buttonEvent(int index)
{
    PROFILE()
    // Accept button pressed.
    if (index)
    {
        if (!butDecline.command.empty())
            sendCommand(1, (char*)butDecline.command.c_str());
    }
    // Decline button pressed.
    else
    {
        if (mIcon_select && !mSelected)
        {
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "select an item first.");
            Sound::getSingleton().playStream(Sound::BUTTON_CLICK); // SOUND_CLICKFAIL
            return;
        }
        if (!butAccept.label.empty())
        {
            Sound::getSingleton().playStream(Sound::BUTTON_CLICK); // SOUND_SCROLL
            if (!butAccept.command.empty())
            {
                // if we have accepted, we must check selected for possible slot selection.
                if (mIcon_select)
                {
                    //char cmd[1024];
                    //(cmd,"%s #%d", butAccept.command, mSelected);
                }
                else
                    sendCommand(1, (char*)butAccept.command.c_str());
            }
        }
    }
}

//================================================================================================
// we have a left click inside the interface -> check it
//================================================================================================
void CmdInterface::mouseEvent(int line)
{
    PROFILE()
    if (line <0) return;
    int element, index;
    String keyword = "";
    if (getElement(line, &element, &index, &keyword))
    {
        //Logger::log().error() << Logger::ICON_CLIENT <<  "keyword: " << keyword;
        if (element == INTERFACE_ICON)
        {
            Sound::getSingleton().playStream(Sound::BUTTON_CLICK); // SOUND_GET
            mSelected = index;
            //GuiManager::getSingleton().addTextline(WIN_TEXTWINDOW, LIST_MSGWIN, "Icon pressed");
            return;
        }
        else if (element == INTERFACE_MESSAGE)
        {
            Sound::getSingleton().playStream(Sound::BUTTON_CLICK); // SOUND_GET
            sendCommand(0, (char*)keyword.c_str());
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Message pressed");
            return;
        }
        else if (element == INTERFACE_LINK)
        {
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, keyword.c_str());
            Sound::getSingleton().playStream(Sound::BUTTON_CLICK); // SOUND_GET
            sendCommand(keyword[0]!='/'?0:1, (char*)keyword.c_str());
            //GuiManager::getSingleton().addTextline(WIN_TEXTWINDOW, LIST_MSGWIN, "Link pressed");
            return;
        }
    }
}

//================================================================================================
// If we click on something in the window, this functions returns the element and/or keyword.
// returned in *element is the gui element (message, body, icon).
// returned in *index is the indicator for the element.
// If there is a keyword or command, we have a pointer to it in keyword.
// return: true = we hit something.
//================================================================================================
bool CmdInterface::getElement(int line, int *element, int *index, String *keyword)
{
    PROFILE()
    // static char key[256]; // used to get keyword string parts for keyword and save it statically
    if ((mUsed_flag & INTERFACE_MESSAGE) && line < mMessage.line_count)
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
                          xt += MediumFont.c[(uchar)message.lines[i][s]].w + MediumFont.char_offset;
                      if (flag && mx>=xs && mx <=xt) // only when we have a active keyword part
                      {
                          char *ptr = strchr(&message.lines[i][s], '^');
                          *element = INTERFACE_MESSAGE;
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
    line-= mMessage.line_count;

    if (mLink_count && line)
    {
        *element = INTERFACE_LINK;
        *index = --line;
        if (mLink[line].cmd.c_str())
            *keyword = mLink[line].cmd;
        else
            *keyword = mLink[line].link;
        return true;
    }
    //    for (int i=0;i < mLink_count; ++i)
    //        Logger::log().error() << Logger::ICON_CLIENT <<  "Link_count: " << link[i].cmd << " - " << link[i].link;
    /*
        // reward is also used as "objective"
        if (mUsed_flag & INTERFACE_REWARD)
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
                    *element = INTERFACE_ICON;
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
