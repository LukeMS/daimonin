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

#include <Ogre.h>
#include <tinyxml.h>
#include <OgreFontManager.h>
#include "define.h"
#include "gui_imageset.h"
#include "gui_manager.h"
#include "gui_window.h"
#include "gui_cursor.h"
#include "resourceloader.h"
#include "option.h"
#include "logger.h"

using namespace Ogre;

GuiImageset::GuiElementNames GuiImageset::mGuiElementNames[GUI_ELEMENTS_SUM]=
{
    // Standard Buttons (Handled inside of gui_windows).
    { "But_Close",          GUI_BUTTON_CLOSE    },
    { "But_OK",             GUI_BUTTON_OK       },
    { "But_Cancel",         GUI_BUTTON_CANCEL   },
    { "But_Min",            GUI_BUTTON_MINIMIZE },
    { "But_Max",            GUI_BUTTON_MAXIMIZE },
    { "But_Resize",         GUI_BUTTON_RESIZE   },
    // Unique Buttons (Handled outside of gui_windows).
    { "But_NPC_Accept",     GUI_BUTTON_NPC_ACCEPT },
    { "But_NPC_Decline",    GUI_BUTTON_NPC_DECLINE},
    // Listboxes.
    { "List_Msg",           GUI_LIST_MSGWIN    },
    { "List_Chat",          GUI_LIST_CHATWIN   },
    { "List_NPC",           GUI_LIST_NPC       },
    // Statusbar.
    { "Bar_Health",         GUI_STATUSBAR_NPC_HEALTH    },
    { "Bar_Mana",           GUI_STATUSBAR_PLAYER_MANA   },
    { "Bar_Grace",          GUI_STATUSBAR_PLAYER_GRACE  },
    { "Bar_PlayerHealth",   GUI_STATUSBAR_PLAYER_HEALTH },
    { "Bar_PlayerMana",     GUI_STATUSBAR_NPC_MANA      },
    { "Bar_PlayerGrace",    GUI_STATUSBAR_NPC_GRACE     },
    // TextValues.
    { "Engine_CurrentFPS",  GUI_TEXTVALUE_STAT_CUR_FPS   },
    { "Engine_BestFPS",     GUI_TEXTVALUE_STAT_BEST_FPS  },
    { "Engine_WorstFPS",    GUI_TEXTVALUE_STAT_WORST_FPS },
    { "Engine_SumTris",     GUI_TEXTVALUE_STAT_SUM_TRIS  },
    { "Login_ServerInfo1",  GUI_TEXTBOX_SERVER_INFO1     },
    { "Login_ServerInfo2",  GUI_TEXTBOX_SERVER_INFO2     },
    { "Login_ServerInfo3",  GUI_TEXTBOX_SERVER_INFO3     },
    { "Login_LoginWarn",    GUI_TEXTBOX_LOGIN_WARN       },
    { "Login_LoginInfo1",   GUI_TEXTBOX_LOGIN_INFO1      },
    { "Login_LoginInfo2",   GUI_TEXTBOX_LOGIN_INFO2      },
    { "Login_LoginInfo3",   GUI_TEXTBOX_LOGIN_INFO3      },
    { "NPC_Headline",       GUI_TEXTBOX_NPC_HEADLINE     },
    { "Inv_Equipment",      GUI_TEXTBOX_INV_EQUIP        },
    { "Inv_Equip_Weight",   GUI_TEXTBOX_INV_EQUIP_WEIGHT },
    // TextInput.
    { "Input_Login_Name",   GUI_TEXTINPUT_LOGIN_NAME   },
    { "Input_Login_Passwd", GUI_TEXTINPUT_LOGIN_PASSWD },
    { "Input_Login_Verify", GUI_TEXTINPUT_LOGIN_VERIFY },
    { "Input_NPC_Dialog",   GUI_TEXTINPUT_NPC_DIALOG   },
    // Table
    { "Table_Server",       GUI_TABLE },
    // Combobox.
    { "ComboBoxTest",       GUI_COMBOBOX_TEST  },
    // Gadget_Slot
    { "Slot_Quickslot",     GUI_SLOT_QUICKSLOT    },
    { "Slot_Equipment",     GUI_SLOT_EQUIPMENT    },
    { "Slot_Inventory",     GUI_SLOT_INVENTORY    },
    { "Slot_Container",     GUI_SLOT_CONTAINER    },
    { "Slot_TradeOffer",    GUI_SLOT_TRADE_OFFER  },
    { "Slot_TradeReturn",   GUI_SLOT_TRADE_RETURN },
    { "Slot_Shop",          GUI_SLOT_SHOP         },
};

// Mouse states.
GuiImageset::GuiElementNames GuiImageset::mMouseState[STATE_MOUSE_SUM]=
{
    { "Default",           STATE_MOUSE_DEFAULT            },
    { "Pushed",            STATE_MOUSE_PUSHED             },
    { "Talk",              STATE_MOUSE_TALK               },
    { "Attack-ShortRange", STATE_MOUSE_SHORT_RANGE_ATTACK },
    { "Attack-LongRange",  STATE_MOUSE_LONG_RANGE_ATTACK  },
    { "Open",              STATE_MOUSE_OPEN               },
    { "Cast",              STATE_MOUSE_CAST               },
    { "Dragging",          STATE_MOUSE_DRAGGING           },
    { "Resizing",          STATE_MOUSE_RESIZING           },
    { "PickUp",            STATE_MOUSE_PICKUP             },
    { "Stop",              STATE_MOUSE_STOP               },
};

// GuiElement states.
GuiImageset::GuiElementNames GuiImageset::mElementState[STATE_ELEMENT_SUM]=
{
    { "Default",   STATE_ELEMENT_DEFAULT },
    { "Pressed",   STATE_ELEMENT_PUSHED  },
    { "MouseOver", STATE_ELEMENT_M_OVER  },
    { "Passive",   STATE_ELEMENT_PASSIVE },
};

//================================================================================================
// .
//================================================================================================
GuiImageset::GuiImageset()
{}

//================================================================================================
// .
//================================================================================================
GuiImageset::~GuiImageset()
{
    for (std::vector<gfxSrcEntry*>::iterator i = mvSrcEntry.begin(); i < mvSrcEntry.end(); ++i)
    {
        delete (*i);
    }
    mvSrcEntry.clear();
    delete mSrcEntryMouse;
}

//================================================================================================
// Parse the gfx datas from the imageset.
//================================================================================================
void GuiImageset::parseXML(const char *fileImageSet)
{
    Logger::log().headline() << "Parsing the imageset";
    // ////////////////////////////////////////////////////////////////////
    // Check for a working description file.
    // ////////////////////////////////////////////////////////////////////
    TiXmlElement *xmlRoot, *xmlElem;
    TiXmlDocument doc(fileImageSet);
    const char *strTemp;
    if (!doc.LoadFile() || !(xmlRoot = doc.RootElement()) || !(strTemp = xmlRoot->Attribute("file")))
    {
        Logger::log().error() << "XML-File '" << fileImageSet << "' is broken or missing.";
        return;
    }
    mStrImageSetGfxFile = strTemp;
    Logger::log().info() << "Parsing the ImageSet file '" << mStrImageSetGfxFile << "'.";
    // ////////////////////////////////////////////////////////////////////
    // Parse the gfx coordinates.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Image"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Image"))
    {
        if (!(strTemp = xmlElem->Attribute("name"))) continue;
        if (!stricmp(strTemp, "MouseCursor"))
        {
            gfxSrcMouse *Entry = new gfxSrcMouse;
            if ((strTemp = xmlElem->Attribute("width" ))) Entry->w  = atoi(strTemp);
            if ((strTemp = xmlElem->Attribute("height"))) Entry->h = atoi(strTemp);
            if (parseStates(xmlElem, Entry->state, STATE_MOUSE_SUM, true))
                mSrcEntryMouse = Entry;
            else
            {
                mSrcEntryMouse =0;
                Logger::log().error() << "MouseCursor has no default state and will be ignored.";
                delete Entry;
            }
        }
        else // A gui Element.
        {
            gfxSrcEntry *Entry = new gfxSrcEntry;
            Entry->name = strTemp;
            if ((strTemp = xmlElem->Attribute("width" ))) Entry->w = atoi(strTemp);
            if ((strTemp = xmlElem->Attribute("height"))) Entry->h = atoi(strTemp);
            if (parseStates(xmlElem, Entry->state, STATE_ELEMENT_SUM, false))
                mvSrcEntry.push_back(Entry);
            else
            {
                Logger::log().warning() << "Element '" << Entry->name << "' has no default state and will be ignored.";
                delete Entry;
            }
        }
    }
    Logger::log().info() << (int) mvSrcEntry.size() +1 << " Image Entries were parsed."; // +1 because of mouseCursor.
    mImageSetImg.load(mStrImageSetGfxFile, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mSrcPixelBox = mImageSetImg.getPixelBox();

    // ////////////////////////////////////////////////////////////////////
    // If requested (by cmd-line) print all element names.
    // ////////////////////////////////////////////////////////////////////
    if (Option::getSingleton().getIntValue(Option::CMDLINE_LOG_GUI_ELEMENTS))
    {
        Logger::log().info() << "These elements are currently known and can be used in " << FILE_GUI_WINDOWS<< ":";
        for (int i =0; i < GUI_ELEMENTS_SUM; ++i) Logger::log().warning() << mGuiElementNames[i].name;
    }
}

//================================================================================================
// Parse the Position entries.
//================================================================================================
bool GuiImageset::parseStates(TiXmlElement *xmlElem, gfxPos *stateNr, int sum_state, bool mouseStates)
{
    // By setting the xpos to -1 we declare this state to empty.
    for (int i=0; i < sum_state; ++i) stateNr[i].x = -1;
    // Read in the positions for all the states.
    const char *strTemp;
    int state;
    for (TiXmlElement *xmlState = xmlElem->FirstChildElement("State"); xmlState; xmlState = xmlState->NextSiblingElement("State"))
    {
        if (!(strTemp= xmlState->Attribute("name"))) continue;
        // Compare the found state with the names of all supported states.
        state = -1;
        for (int i=0; i < sum_state; ++i)
        {
            if (!stricmp(strTemp, mouseStates?mMouseState[i].name:mElementState[i].name))
            {
                state = i;
                break;
            }
        }
        if (state < 0)
        {
            Logger::log().error() << "Unknown state: " << strTemp;
            continue;
        }
        if ((strTemp= xmlState->Attribute("posX"))) stateNr[state].x = atoi(strTemp);
        if ((strTemp= xmlState->Attribute("posY"))) stateNr[state].y = atoi(strTemp);
    }
    // ////////////////////////////////////////////////////////////////////
    // Every element MUST have at least the default state.
    // ////////////////////////////////////////////////////////////////////
    if (stateNr[STATE_ELEMENT_DEFAULT].x < 0) return false;
    // Set all empty states to the default state.
    for (int i=1; i < sum_state; ++i)
    {
        if (stateNr[i].x  < 0)
        {
            stateNr[i].x = stateNr[STATE_ELEMENT_DEFAULT].x;
            stateNr[i].y = stateNr[STATE_ELEMENT_DEFAULT].y;
        }
    }
    return true;
}

//================================================================================================
// Returns the array of the gfx positions for an element.
//================================================================================================
GuiImageset::gfxSrcEntry *GuiImageset::getStateGfxPositions(const char* guiImage)
{
    if (guiImage)
    {
        for (unsigned int j = 0; j < mvSrcEntry.size(); ++j)
        {
            if (!stricmp(guiImage, mvSrcEntry[j]->name.c_str()))
                return mvSrcEntry[j];
        }
    }
    return 0;
}

//================================================================================================
// .
//================================================================================================
const char *GuiImageset::getElementName(int i)
{
    if (i < GUI_ELEMENTS_SUM && mGuiElementNames[i].name)
        return mGuiElementNames[i].name;
    return "ERROR";
}

//================================================================================================
// .
//================================================================================================
int GuiImageset::getElementIndex(int i)
{
    if (i < GUI_ELEMENTS_SUM)
        return mGuiElementNames[i].index;
    return -1;
}
