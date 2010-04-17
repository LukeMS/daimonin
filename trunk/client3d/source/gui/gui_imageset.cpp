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

#include <OgreVector3.h>
#include <OgreResourceGroupManager.h>
#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif
#include "logger.h"
#include "gui/gui_imageset.h"

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

const int UNKNOWN_ITEM_GFX = 0;

//================================================================================================
// .
//================================================================================================
GuiImageset::~GuiImageset()
{
    for (std::vector<gfxSrcEntry*>::iterator i = mvSrcEntry.begin(); i < mvSrcEntry.end(); ++i)
    {
#ifdef D_DEBUG
        if (!(*i)->isUsed)
            Logger::log().info() << "Element '" << (*i)->name << "' is defined in " << GuiManager::FILE_TXT_IMAGESET << " but is not used by the GUI.";
#endif
        delete (*i);
    }
    mvSrcEntry.clear();
    delete mSrcEntryMouse;
    mvAtlasGfxName.clear();
}

//================================================================================================
// Parse the gfx data from the imageset.
//================================================================================================
void GuiImageset::parseXML(const char *fileImageSet, bool createItemAtlas)
{
    // ////////////////////////////////////////////////////////////////////
    // Parse the imageset.
    // ////////////////////////////////////////////////////////////////////
    Logger::log().headline() << "Parsing the imageset";
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
    // Parse the gfx coordinates.
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
    Logger::log().list() << (int) mvSrcEntry.size() +1 << " Image Entries were parsed."; // +1 because of mouseCursor.
    static Image image;
    image.load(mStrImageSetGfxFile, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mGuiGfxPixelBox = image.getPixelBox();
    // ////////////////////////////////////////////////////////////////////
    // Parse the items.
    // ////////////////////////////////////////////////////////////////////
    parseItems(createItemAtlas);
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

//================================================================================================
// Parse the Items
//================================================================================================
void GuiImageset::parseItems(bool createItemAtlas)
{
    String path = GuiManager::getSingleton().getPathItems();
    String fileAtlasGfx = GuiManager::FILE_ITEM_ATLAS;
    String fileAtlasTxt = path + GuiManager::FILE_ITEM_ATLAS;
    fileAtlasTxt.replace(fileAtlasTxt.size()-3, fileAtlasTxt.size(), "txt");
    // ////////////////////////////////////////////////////////////////////
    // Create the item texture atlas.
    // ////////////////////////////////////////////////////////////////////
    if (createItemAtlas)
    {
        bool unknownGfxFound = false;
        std::vector<std::string> itemFilename;
#ifdef WIN32
        String filename = path + "\\*.png";
        BOOL found = true;
        WIN32_FIND_DATA FindFileData;
        HANDLE handle=FindFirstFile(filename.c_str(), &FindFileData);
        while (handle && found)
        {
            // Force the unknown item gfx to be the first gfx.
            if (!strcmp(FindFileData.cFileName, GuiManager::FILE_ITEM_UNKNOWN))
            {
                itemFilename.insert(itemFilename.begin(), FindFileData.cFileName);
                unknownGfxFound = true;
            }
            else
                itemFilename.push_back(FindFileData.cFileName);
            found = FindNextFile(handle, &FindFileData);
        }
#else
        struct dirent *dir_entry;
        DIR *dir = opendir(path.c_str()); // Open the current directory
        while ((dir_entry = readdir(dir)))
        {
            if (strstr(dir_entry->d_name, ".png"))
            {
                // Force the unknown item gfx to be the first gfx.
                if (!strcmp(dir_entry->d_name, GuiManager::FILE_ITEM_UNKNOWN))
                {
                    itemFilename.insert(itemFilename.begin(), dir_entry->d_name);
                    unknownGfxFound = true;
                }
                else
                    itemFilename.push_back(dir_entry->d_name);
            }
        }
        closedir(dir);
#endif
        if (itemFilename.empty())
        {
            Logger::log().error() << "Could not find any item graphics in " << path;
            return;
        }
        if (!unknownGfxFound)
        {
            Logger::log().error() << "Could not find the gfx for an unknown item (filename: " << GuiManager::FILE_ITEM_UNKNOWN
            << " in folder " << path << ").";
        }
        Image itemImage, itemAtlas;
        uint32 *itemBuffer = new uint32[ITEM_SIZE * ITEM_SIZE * itemFilename.size()];
        uint32 *nextPos = itemBuffer;
        itemAtlas = itemAtlas.loadDynamicImage((uchar*)itemBuffer, ITEM_SIZE, ITEM_SIZE * itemFilename.size(), 1, PF_A8R8G8B8);
        for (unsigned int i = 0; i < itemFilename.size(); ++i)
        {
            itemImage.load(itemFilename[i], ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            if (itemImage.getHeight() != ITEM_SIZE || itemImage.getWidth() != ITEM_SIZE)
            {
                Logger::log().warning() << "CreateItemAtlas: Unsupported image size. Only Items of "
                << (int)ITEM_SIZE << " * " << (int)ITEM_SIZE << " pixel are allowed "<< "[" << itemFilename[i] << "].";
                break;
            }
            if (itemImage.getFormat() != PF_A8R8G8B8)
            {
                Logger::log().warning() << "CreateItemAtlas: Unsupported image format ("<< itemImage.getFormat() <<")."
                << " Only 32bit [ARGB] png format is allowed "<< "[" << itemFilename[i] << "].";
                break;
            }
            memcpy(nextPos, itemImage.getData(), ITEM_SIZE * ITEM_SIZE * sizeof(uint32));
            nextPos+=ITEM_SIZE * ITEM_SIZE;
        }
        // ////////////////////////////////////////////////////////////////////
        // Write the data to disc.
        // ////////////////////////////////////////////////////////////////////
        itemAtlas.save(GuiManager::getSingleton().getPathTextures() + fileAtlasGfx);
        std::ofstream txtFile(fileAtlasTxt.c_str(), std::ios::out | std::ios::binary);
        txtFile << "# This file holds the content of the image-texture-atlas." << std::endl;
        txtFile << "# The filename for an undefined/missing gfx must be: " << GuiManager::FILE_ITEM_UNKNOWN << std::endl;
        if (txtFile)
        {
            for (unsigned int i = 0; i < itemFilename.size(); ++i)
                txtFile << itemFilename[i] << std::endl;
        }
        txtFile.close();
        itemFilename.clear();
        delete[] itemBuffer;
    }
    // ////////////////////////////////////////////////////////////////////
    // Read in the gfxpos of the items.
    // ////////////////////////////////////////////////////////////////////
    std::ifstream txtFile;
    txtFile.open(fileAtlasTxt.c_str(), std::ios::in | std::ios::binary);
    if (!txtFile)
    {
        Logger::log().error() << "Error on file " << fileAtlasTxt;
        return;
    }
    getline(txtFile, fileAtlasTxt); // skip the comment.
    getline(txtFile, fileAtlasTxt); // skip the comment.
    while (getline(txtFile, fileAtlasTxt))
        mvAtlasGfxName.push_back(fileAtlasTxt);
    txtFile.close();
    // ////////////////////////////////////////////////////////////////////
    // Read in the texture atlas.
    // ////////////////////////////////////////////////////////////////////
    static Image image;
    image.load(fileAtlasGfx, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mItemPixelBox = image.getPixelBox();
}

//================================================================================================
// Get the item Id.
//================================================================================================
int GuiImageset::getItemId(const char *gfxName)
{
    if (gfxName)
    {
        for (unsigned int i = 0; i < mvAtlasGfxName.size(); ++i)
        {
            if (mvAtlasGfxName[i] == gfxName)
                return i;
        }
    }
    return UNKNOWN_ITEM_GFX;
}

//================================================================================================
// Get the pixelbox of an item.
//================================================================================================
const PixelBox &GuiImageset::getItemPB(int itemNr)
{
    static PixelBox pb;
    pb = mItemPixelBox.getSubVolume(Box(0, ITEM_SIZE * itemNr, ITEM_SIZE, ITEM_SIZE *(itemNr+1)));
    return (pb);
}
