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

#include "define.h"
#include "gui_imageset.h"
#include "option.h"
#include "logger.h"

using namespace Ogre;

// Mouse states.
GuiImageset::StateNames mMouseState[GuiManager::STATE_MOUSE_SUM]=
{
    { "Default",           GuiManager::STATE_MOUSE_DEFAULT            },
    { "Pushed",            GuiManager::STATE_MOUSE_PUSHED             },
    { "Talk",              GuiManager::STATE_MOUSE_TALK               },
    { "Attack-ShortRange", GuiManager::STATE_MOUSE_SHORT_RANGE_ATTACK },
    { "Attack-LongRange",  GuiManager::STATE_MOUSE_LONG_RANGE_ATTACK  },
    { "Open",              GuiManager::STATE_MOUSE_OPEN               },
    { "Cast",              GuiManager::STATE_MOUSE_CAST               },
    { "Dragging",          GuiManager::STATE_MOUSE_DRAGGING           },
    { "Resizing",          GuiManager::STATE_MOUSE_RESIZING           },
    { "PickUp",            GuiManager::STATE_MOUSE_PICKUP             },
    { "Stop",              GuiManager::STATE_MOUSE_STOP               },
};

// GuiElement states.
GuiImageset::StateNames GuiImageset::mElementState[STATE_ELEMENT_SUM]=
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
#ifdef D_DEBUG
        if (!(*i)->isUsed)
            Logger::log().info() << "Element '" << (*i)->name << "' is defined in " << FILE_GUI_IMAGESET << " but is not used by the GUI.";
#endif
        delete (*i);
    }
    mvSrcEntry.clear();
    delete mSrcEntryMouse;
}

//================================================================================================
// Parse the gfx data from the imageset.
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
            if (parseStates(xmlElem, Entry->state, GuiManager::STATE_MOUSE_SUM, true))
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
#ifdef D_DEBUG
            Entry->isUsed = false;
#endif
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
            {
#ifdef D_DEBUG
                mvSrcEntry[j]->isUsed = true;
#endif
                return mvSrcEntry[j];
            }
        }
    }
    return 0;
}
