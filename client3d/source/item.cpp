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

#include "define.h"
#include "item.h"
#include "logger.h"
#include "network.h"
#include "gui_manager.h"
#include "object_hero.h"
#include "gui_gadget_slot.h"
#include "tile_map_wrapper.h"

using namespace Ogre;

//================================================================================================
// Init all static Elemnts.
//================================================================================================

//================================================================================================
// Constructor.
//================================================================================================
Item::Item()
{
    mActGrndContainerID = 0; // Ground tile container ID is always 0.
    mActHeroContainerID = CONTAINER_UNKNOWN;
    mActOpenContainerID = CONTAINER_UNKNOWN;
    GuiManager::getSingleton().setItemReference(GuiManager::GUI_WIN_INVENTORY, GuiImageset::GUI_SLOT_CONTAINER, &HeroBackpack);
}

//================================================================================================
// Destructor.
//================================================================================================
Item::~Item()
{
    clearContainer(mActOpenContainerID);
    clearContainer(mActGrndContainerID);
    clearContainer(mActHeroContainerID);
}

//================================================================================================
// Clear the whole container.
//================================================================================================
void Item::clearContainer(int container)
{
    String strTmp = "-- del container";
    strTmp += " [" + StringConverter::toString(container) + "]";
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN,strTmp.c_str() );
    std::list<sItem*>::const_iterator iter;
    if (container == mActOpenContainerID)
    {
        for (iter = OpenContainer.begin(); iter!= OpenContainer.end(); ++iter)
            delete (*iter);
        OpenContainer.clear();
    }
    else if (container == mActGrndContainerID)
    {
        for (iter = HeroTileGround.begin(); iter!= HeroTileGround.end(); ++iter)
            delete (*iter);
        HeroTileGround.clear();
    }
    else if (container == mActHeroContainerID)
    {
        for (iter = HeroBackpack.begin(); iter!= HeroBackpack.end(); ++iter)
            delete (*iter);
        HeroBackpack.clear();
    }
    else
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "ClearContainer fails!");
}

//================================================================================================
// Add all Items to Inventory, Backpack, Ground or open Container.
//================================================================================================
void Item::ItemXYCmd(unsigned char *data, int len, bool bflag)
{
    int pos = 4;
    int mode= Network::getSingleton().GetInt_String(data);
    int container = Network::getSingleton().GetInt_String(data + pos);
    pos+= 4;
    /*
        char buf[400];
        sprintf(buf, "%d %d", container, mode);
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
    */
    if (mode == MODE_TOGGLE_OPEN)
    {
        if (container == CONTAINER_UNKNOWN)
        {   // Close container.
            clearContainer(mActOpenContainerID);
            mActOpenContainerID = CONTAINER_UNKNOWN;
            return;
        }
        mActOpenContainerID = container;
    }
    else if (mode != MODE_KEEP_ITEMS) clearContainer(container);

    while (pos < len)
    {
        sItem *tmpItem = new sItem;
        tmpItem->tag      = Network::getSingleton().GetInt_String(data + pos);
        pos += 4;
        tmpItem->flagsval = Network::getSingleton().GetInt_String(data + pos);
        pos += 4;
        tmpItem->weight   = Network::getSingleton().GetInt_String(data + pos);
        pos += 4;
        tmpItem->face     = Network::getSingleton().GetInt_String(data + pos);
        pos += 4;
        //request_face(face, 0);
        tmpItem->direction = data[pos++];
        if (container != mActGrndContainerID)
        {
            tmpItem->itype     = data[pos++];
            tmpItem->stype     = data[pos++];
            tmpItem->item_qua  = data[pos++];
            tmpItem->item_con  = data[pos++];
            tmpItem->item_level= data[pos++];
            tmpItem->item_skill= data[pos++];
        }
        else
        {
            tmpItem->itype     = 0;
            tmpItem->stype     = 0;
            tmpItem->item_qua  = 0;
            tmpItem->item_con  = 0;
            tmpItem->item_level= 0;
            tmpItem->item_skill= 0;
        }
        int nlen = data[pos++];
        char *name = new char[nlen+1];
        memcpy(name, (char *) data + pos, nlen);
        name[nlen] = '\0';
        tmpItem->d_name = name;
        delete[] name;
        pos += nlen;
        tmpItem->animation_id = Network::getSingleton().GetShort_String(data + pos);
        pos += 2;
        tmpItem->anim_speed = data[pos++];
        tmpItem->nrof = Network::getSingleton().GetInt_String(data + pos);
        pos += 4;

        char buf[256];
        sprintf(buf, "upd: %d %d %s", container, mActHeroContainerID, tmpItem->d_name.c_str());
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);

        update(tmpItem, container, bflag);
    }
    GuiManager::getSingleton().updateItemSlot(GuiManager::GUI_WIN_INVENTORY, GuiImageset::GUI_SLOT_CONTAINER, GuiGadgetSlot::UPDATE_ALL_SLOTS);
    //    map_udate_flag = 2;
}

//================================================================================================
//
//================================================================================================
Item::sItem *Item::getBackpackItem(int slotPosition)
{
    std::list<sItem*>::iterator iter = HeroBackpack.begin();
    while (iter != HeroBackpack.end())
    {
        if (!slotPosition) return (*iter);
        --slotPosition;
        ++iter;
    }
    return 0;
}

//================================================================================================
//  Returns pointer to the item which tag is given.
//  returns 0 if item was not found.
//================================================================================================
Item::sItem *Item::locateItem(int container, unsigned int tag)
{
    std::list<sItem*>::iterator iter;
    if (container == mActHeroContainerID || mActOpenContainerID == CONTAINER_UNKNOWN)
    {
        for (iter = HeroBackpack.begin(); iter!= HeroBackpack.end(); ++iter)
            if (tag == (*iter)->tag) return (*iter);
    }
    if (container == mActOpenContainerID || mActOpenContainerID == CONTAINER_UNKNOWN)
    {
        for (iter = OpenContainer.begin(); iter!= OpenContainer.end(); ++iter)
            if (tag == (*iter)->tag) return (*iter);
    }
    if (container == mActGrndContainerID || mActOpenContainerID == CONTAINER_UNKNOWN)
    {
        for (iter = HeroTileGround.begin(); iter!= HeroTileGround.end(); ++iter)
            if (tag == (*iter)->tag) return (*iter);
    }
    return 0;
}

//================================================================================================
//
//================================================================================================
int Item::getContainerID(unsigned int ItemID)
{
    std::list<sItem*>::iterator iter;
    for (iter = HeroBackpack.begin(); iter!= HeroBackpack.end(); ++iter)
    {
        if (ItemID == (*iter)->tag)
            return mActHeroContainerID;
    }
    for (iter = OpenContainer.begin(); iter!= OpenContainer.end(); ++iter)
    {
        if (ItemID == (*iter)->tag)
            return mActOpenContainerID;
    }
    for (iter = HeroTileGround.begin(); iter!= HeroTileGround.end(); ++iter)
    {
        if (ItemID == (*iter)->tag)
            return mActGrndContainerID;
    }
    return CONTAINER_UNKNOWN; // Should not happen.
}

//================================================================================================
// Deletes an Item.
// If container == CONTAINER_UNKNOWN, all containers will be searched.
//================================================================================================
void Item::delItem(unsigned int item, int container)
{
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "-- del-- del Item");
    /*
        char buf[256];
        sprintf(buf, "%d", mActHeroContainerID);
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
        sprintf(buf, "%d", container);
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
    */
    std::list<sItem*>::iterator iter;
    if (container == mActHeroContainerID || mActOpenContainerID == CONTAINER_UNKNOWN)
    {
        for (iter = HeroBackpack.begin(); iter!= HeroBackpack.end(); ++iter)
        {
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN,(*iter)->d_name.c_str() );
            if ((*iter)->tag != item) continue;
            delete (*iter);
            HeroBackpack.erase(iter);
        }
    }
    if (container == mActOpenContainerID || mActOpenContainerID == CONTAINER_UNKNOWN)
    {
        for (iter = OpenContainer.begin(); iter!= OpenContainer.end(); ++iter)
        {
            if ((*iter)->tag != item) continue;
            delete (*iter);
            OpenContainer.erase(iter);
        }
    }
    if (container == mActGrndContainerID || mActOpenContainerID == CONTAINER_UNKNOWN)
    {
        for (iter = HeroTileGround.begin(); iter!= HeroTileGround.end(); ++iter)
        {
            if ((*iter)->tag != item) continue;
            delete (*iter);
            HeroTileGround.erase(iter);
        }
    }
}

//================================================================================================
// Add an Item to a given container.
//================================================================================================
void Item::addItem(sItem *tmpItem, int container)
{
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "-- add Item");
    /*
        char buf[256];
        sprintf(buf, "%d", mActHeroContainerID);
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
        sprintf(buf, "%d", container);
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
    */
    if (container == mActHeroContainerID)
    {
        HeroBackpack.push_back(tmpItem);
        return;
    }
    if (container == mActGrndContainerID)
    {
        HeroTileGround.push_back(tmpItem);
        return;
    }
    if (container == mActOpenContainerID)
    {
        OpenContainer.push_back(tmpItem);
        return;
    }
    Logger::log().error() << "Unknown item container ID: " << container;
}

//================================================================================================
// .
//================================================================================================
void Item::getInventoryItemFromFloor(int slotNr)
{
    if (slotNr >= (int)HeroTileGround.size()) return;

    std::list<sItem*>::iterator iter;
    for (iter = HeroTileGround.begin(); slotNr-- && iter != HeroTileGround.end(); )  ++iter;
    char buf[256];
    sprintf(buf, "drop %s", (*iter)->d_name.c_str());
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);

    // move item TO Backpack.
    sprintf(buf, "mv %d %d %d", mActHeroContainerID, (*iter)->tag, (*iter)->nrof);
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
    Network::getSingleton().cs_write_string(buf);
    delete (*iter);
    HeroTileGround.erase(iter);
    return;
}

//================================================================================================
// .
//================================================================================================
void Item::dropInventoryItemToFloor(int slotNr)
{
    //int nrof = 1;
    //sound_play_effect(SOUND_DROP, 0, 0, 100);

    std::list<sItem*>::iterator iter;
    for (iter = HeroBackpack.begin(); slotNr-- && iter != HeroBackpack.end(); )  ++iter;
    char buf[256];
    sprintf(buf, "drop %s", (*iter)->d_name.c_str());
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);

    // move item TO Ground.
    sprintf(buf, "mv %d %d %d", mActGrndContainerID, (*iter)->tag, (*iter)->nrof);
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
    Network::getSingleton().cs_write_string(buf);
    delete (*iter);
    HeroBackpack.erase(iter);
    return;
}

//================================================================================================
// Updates an item with new attributes.
//================================================================================================
bool Item::update(sItem *tmpItem, int newContainerID, bool bflag)
{
    int actContainerID = getContainerID(tmpItem->tag);
    // This is a new Item.
    if (actContainerID == CONTAINER_UNKNOWN)
    {
        addItem(tmpItem, newContainerID);
    }
    // Move the item into a new container.
    else if (actContainerID != newContainerID)
    {
        delItem(tmpItem->tag, actContainerID);
        addItem(tmpItem, newContainerID);
    }
    // Update all attributes.

//GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, (*iter)->d_name.c_str());

    return false;
}

//================================================================================================
// Just for debug.
//================================================================================================
void Item::printAllItems()
{
    return;
    String strTmp;
    std::list<sItem*>::iterator iter;
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Backpack:", 0x00ff0000);
    if (HeroBackpack.empty())
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "<empty>");
    else
        for (iter = HeroBackpack.begin(); iter!= HeroBackpack.end(); ++iter)
        {
            strTmp = (*iter)->d_name +
                     " [" + StringConverter::toString(mActHeroContainerID) + "]"+
                     " [" + StringConverter::toString((*iter)->tag) + "]"+
                     " [" + ObjectWrapper::getSingleton().getMeshName((*iter)->face & ~0x8000) + "]";
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, strTmp.c_str());
        }
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Container:", 0x00ff0000);
    if (OpenContainer.empty())
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "<empty>");
    else
        for (iter = OpenContainer.begin(); iter!= OpenContainer.end(); ++iter)
        {
            strTmp = (*iter)->d_name +
                     " [" + StringConverter::toString(mActOpenContainerID) + "]"+
                     " [" + StringConverter::toString((*iter)->tag) + "]"+
                     " [" + ObjectWrapper::getSingleton().getMeshName((*iter)->face & ~0x8000) + "]";
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, strTmp.c_str());
        }
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Ground:", 0x00ff0000);
    if (HeroTileGround.empty())
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "<empty>");
    else
        for (iter = HeroTileGround.begin(); iter!= HeroTileGround.end(); ++iter)
        {
            strTmp = (*iter)->d_name +
                     " [" + StringConverter::toString(mActGrndContainerID) + "]"+
                     " [" + StringConverter::toString((*iter)->tag) + "]"+
                     " [" + ObjectWrapper::getSingleton().getMeshName((*iter)->face & ~0x8000) + "]";
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, strTmp.c_str());
        }
}
