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


#include <Ogre.h>
#include <tinyxml.h>
#include "define.h"
#include "gui_imageset.h"
#include "gui_manager.h"
#include "gui_window.h"
#include "gui_cursor.h"
#include "option.h"
#include "logger.h"
#include <OgreFontManager.h>

using namespace Ogre;

GuiElementNames GuiImageset::mGuiElementNames[GUI_ELEMENTS_SUM]=
    {
        { "But_Close",      GUI_BUTTON_CLOSE    },
        { "But_OK",         GUI_BUTTON_OK       },
        { "But_Cancel",     GUI_BUTTON_CANCEL   },
        { "But_Min",        GUI_BUTTON_MINIMIZE },
        { "But_Max",        GUI_BUTTON_MAXIMIZE },
        { "But_Resize",     GUI_BUTTON_RESIZE   },
        // Listboxes.
        { "List_Msg",       GUI_LIST_MSGWIN     },
        { "List_Chat",      GUI_LIST_CHATWIN    },
        // Statusbar.
        { "Bar_Health",     GUI_STATUSBAR_NPC_HEALTH     },
        { "Bar_Mana",       GUI_STATUSBAR_PLAYER_MANA    },
        { "Bar_Grace",      GUI_STATUSBAR_PLAYER_GRACE   },
        { "Bar_PlayerHealth",GUI_STATUSBAR_PLAYER_HEALTH },
        { "Bar_PlayerMana",GUI_STATUSBAR_NPC_MANA        },
        { "Bar_PlayerGrace",GUI_STATUSBAR_NPC_GRACE      },
        // TextValues.
        { "Engine_CurrentFPS", GUI_TEXTVALUE_STAT_CUR_FPS   },
        { "Engine_BestFPS",    GUI_TEXTVALUE_STAT_BEST_FPS  },
        { "Engine_WorstFPS",   GUI_TEXTVALUE_STAT_WORST_FPS },
        { "Engine_SumTris",    GUI_TEXTVALUE_STAT_SUM_TRIS  },
        // TextInput.
        { "Input_Login_Name",  GUI_TEXTINPUT_LOGIN_NAME     },
        { "Input_Login_Passwd", GUI_TEXTINPUT_LOGIN_PASSWD  },
        { "Input_Login_Verify", GUI_TEXTINPUT_LOGIN_VERIFY  },

        // Combobox.
        { "ComboBoxTest"      , GUI_COMBOBOX_TEST  },
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
    for (std::vector<GuiSrcEntry*>::iterator i = mvSrcEntry.begin(); i < mvSrcEntry.end(); ++i)
    {
        for (std::vector<GuiElementState*>::iterator j = (*i)->state.begin(); j < (*i)->state.end(); ++j)
        {
            delete (*j);
        }
        (*i)->state.clear();
        delete (*i);
    }
    mvSrcEntry.clear();
}

//================================================================================================
// Parse the gfx datas from the imageset.
//================================================================================================
void GuiImageset::parseXML(const char *fileImageSet)
{
    Logger::log().headline("Parsing the imageset");
    // ////////////////////////////////////////////////////////////////////
    // Check for a working description file.
    // ////////////////////////////////////////////////////////////////////
    TiXmlElement *xmlRoot, *xmlElem, *xmlState;
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
        GuiSrcEntry *Entry = new GuiSrcEntry;
        Entry->name   = strTemp;
        if ((strTemp = xmlElem->Attribute("width" ))) Entry->width  = atoi(strTemp);
        if ((strTemp = xmlElem->Attribute("height"))) Entry->height = atoi(strTemp);
        // ////////////////////////////////////////////////////////////////////
        // Parse the Position entries.
        // ////////////////////////////////////////////////////////////////////
        for (xmlState = xmlElem->FirstChildElement("State"); xmlState; xmlState = xmlState->NextSiblingElement("State"))
        {
            if (!(strTemp= xmlState->Attribute("name")))  continue;
            GuiElementState *s = new GuiElementState;
            s->name = strTemp;
            s->x = s->y = 0;
            if ((strTemp= xmlState->Attribute("posX"))) s->x = atoi(strTemp);
            if ((strTemp= xmlState->Attribute("posY"))) s->y = atoi(strTemp);
            Entry->state.push_back(s);
        }
        mvSrcEntry.push_back(Entry);
    }
    Logger::log().info() << (int) mvSrcEntry.size() << " Entries were parsed.";
    if (Option::getSingleton().getIntValue(Option::CMDLINE_LOG_GUI_ELEMENTS))
    {
        Logger::log().info() << "These elements are currently known and can be used in " << FILE_GUI_WINDOWS<< ":";
        for (int i =0; i < GUI_ELEMENTS_SUM; ++i)
            Logger::log().warning() << mGuiElementNames[i].name;
    }
    mImageSetImg.load(mStrImageSetGfxFile, "General");
    mSrcPixelBox = mImageSetImg.getPixelBox();
}

//================================================================================================
// .
//================================================================================================
GuiSrcEntry *GuiImageset::getStateGfxPositions(const char* guiImage)
{
    if (guiImage)
    {
        for (unsigned int j = 0; j < mvSrcEntry.size(); ++j)
        {
            if (!stricmp(guiImage, mvSrcEntry[j]->name.c_str())) return mvSrcEntry[j];
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
    else
        return "ERROR";
}

//================================================================================================
// .
//================================================================================================
int GuiImageset::getElementIndex(int i)
{
    if (i < GUI_ELEMENTS_SUM)
        return mGuiElementNames[i].index;
    else
        return -1;
}
