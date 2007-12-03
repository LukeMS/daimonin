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
    mActItemID[ITEMLIST_GROUND]    = 0; // Ground tile container ID is always 0.
    mActItemID[ITEMLIST_BACKPACK]  = CONTAINER_UNKNOWN;
    mActItemID[ITEMLIST_CONTAINER] = CONTAINER_UNKNOWN;
    // Link the item lists to the windows containing the slots where
    // the items are displayed.
    mWindowID[ITEMLIST_GROUND]     = GuiManager::GUI_WIN_TILEGROUND;
    mWindowID[ITEMLIST_BACKPACK]   = GuiManager::GUI_WIN_INVENTORY;
    mWindowID[ITEMLIST_CONTAINER]  = GuiManager::GUI_WIN_CONTAINER;
}

//================================================================================================
// Destructor.
//================================================================================================
Item::~Item()
{
    for (int i =0; i < ITEMLIST_SUM; ++i)
        clearContainer(mActItemID[i]);
}

//================================================================================================
// Clear the whole container.
//================================================================================================
void Item::clearContainer(int container)
{
    for (int i =0; i < ITEMLIST_SUM; ++i)
    {
        if (container != mActItemID[i]) continue;
        for (std::list<sItem*>::const_iterator iter = mItemList[i].begin(); iter!= mItemList[i].end(); ++iter)
            delete (*iter);
        mItemList[i].clear();
        GuiManager::getSingleton().clrItem(mWindowID[i]);
        return;
    }
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Item::clearContainer fails!");
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
    if (mode == MODE_TOGGLE_OPEN)
    {
        if (container == CONTAINER_UNKNOWN)
        {   // Close container.
            clearContainer(mActItemID[ITEMLIST_CONTAINER]);
            mActItemID[ITEMLIST_CONTAINER] = CONTAINER_UNKNOWN;
            return;
        }
        mActItemID[ITEMLIST_CONTAINER] = container;
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
        if (container != mActItemID[ITEMLIST_GROUND])
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
        Logger::log().warning() << "Add item: " << tmpItem->d_name << "  gfx-id: " << tmpItem->face;
        update(tmpItem, container, bflag);
    }
    // map_udate_flag = 2;
}

//================================================================================================
//
//================================================================================================
int Item::getContainerID(unsigned int ItemID)
{
    std::list<sItem*>::iterator iter;
    for (int i =0; i < ITEMLIST_SUM; ++i)
    {
        for (iter = mItemList[i].begin(); iter!= mItemList[i].end(); ++iter)
        {
            if (ItemID == (*iter)->tag)
                return mActItemID[i];
        }
    }
    return CONTAINER_UNKNOWN; // Should not happen.
}

//================================================================================================
//  Returns pointer to the item which tag is given.
//  returns 0 if item was not found.
//================================================================================================
Item::sItem *Item::locateItem(int container, unsigned int tag)
{
    std::list<sItem*>::iterator iter;
    for (int i =0; i < ITEMLIST_SUM; ++i)
    {
        if (mActItemID[i] == container || mActItemID[i] == CONTAINER_UNKNOWN)
        {
            for (iter = mItemList[i].begin(); iter!= mItemList[i].end(); ++iter)
                if (tag == (*iter)->tag) return (*iter);
        }
    }
    return 0;
}

//================================================================================================
// Deletes an Item.
// If container == CONTAINER_UNKNOWN, all containers will be searched.
//================================================================================================
void Item::delItem(unsigned int item, int container)
{
    //GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "-- del-- del Item");
    std::list<sItem*>::iterator iter;
    for (int i =0; i < ITEMLIST_SUM; ++i)
    {
        if (mActItemID[i] == container || mActItemID[i] == CONTAINER_UNKNOWN)
        {
            for (iter = mItemList[i].begin(); iter!= mItemList[i].end(); ++iter)
            {
                if ((*iter)->tag != item) continue;
                GuiManager::getSingleton().delItem(mWindowID[i], *iter);
                delete (*iter);
                mItemList[i].erase(iter);
            }
        }
    }
}

//================================================================================================
// Add an Item to a given container.
//================================================================================================
void Item::addItem(sItem *tmpItem, int container)
{
    //GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "-- add Item");
    for (int i =0; i < ITEMLIST_SUM; ++i)
    {
        if (mActItemID[i] == container)
        {
            GuiManager::getSingleton().addItem(mWindowID[i], tmpItem);
            mItemList[i].push_back(tmpItem);
            return;
        }
    }
    Logger::log().error() << "<Item::addItem> Unknown container ID: " << container;
}

//================================================================================================
// .
//================================================================================================
void Item::getInventoryItemFromFloor(int slotNr)
{
    if (slotNr >= (int)mItemList[ITEMLIST_GROUND].size()) return;

    std::list<sItem*>::iterator iter;
    for (iter = mItemList[ITEMLIST_GROUND].begin(); slotNr-- && iter != mItemList[ITEMLIST_GROUND].end(); )  ++iter;
    char buf[256];
    sprintf(buf, "drop %s", (*iter)->d_name.c_str());
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);

    // move item to Backpack.
    sprintf(buf, "mv %d %d %d", mActItemID[ITEMLIST_BACKPACK], (*iter)->tag, (*iter)->nrof);
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
    Network::getSingleton().cs_write_string(buf);
    delete (*iter);
    mItemList[ITEMLIST_GROUND].erase(iter);
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
    return false;
}

//================================================================================================
// .
//================================================================================================
void Item::dropInventoryItemToFloor(int slotNr)
{
    //int nrof = 1;
    //sound_play_effect(SOUND_DROP, 0, 0, 100);

    std::list<sItem*>::iterator iter;
    for (iter = mItemList[ITEMLIST_BACKPACK].begin(); slotNr-- && iter != mItemList[ITEMLIST_BACKPACK].end(); )  ++iter;
    char buf[256];
    sprintf(buf, "drop %s", (*iter)->d_name.c_str());
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);

    // move item TO Ground.
    sprintf(buf, "mv %d %d %d", mActItemID[ITEMLIST_GROUND], (*iter)->tag, (*iter)->nrof);
    Network::getSingleton().cs_write_string(buf);

    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);

    GuiManager::getSingleton().delItem(GuiManager::GUI_WIN_INVENTORY, *iter);
    delete (*iter);
    mItemList[ITEMLIST_BACKPACK].erase(iter);
}

//================================================================================================
// End of Drag'n'Drop.
//================================================================================================
void Item::dropItem(int srcWindow, int srcItemSlot, int dstWindow, int dstItemSlot)
{
    // ////////////////////////////////////////////////////////////////////
    // Drop outside a window -> drop this item to the floor.
    // ////////////////////////////////////////////////////////////////////
    if (dstWindow <0)
    {
        dropInventoryItemToFloor(srcItemSlot);
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Drop inside a window.
    // ////////////////////////////////////////////////////////////////////

// TODO

    char buf[256];
    sprintf(buf, "drag and drop src: %d, %d dest: %d, %d", srcWindow, srcItemSlot, dstWindow, dstItemSlot);
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
    return;
}

//================================================================================================
// Just for debug.
//================================================================================================
void Item::printAllItems()
{
    String strTmp;
    std::list<sItem*>::iterator iter;
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Backpack:", 0x00ff0000);
    if (mItemList[ITEMLIST_BACKPACK].empty())
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "<empty>");
    else
        for (iter = mItemList[ITEMLIST_BACKPACK].begin(); iter!= mItemList[ITEMLIST_BACKPACK].end(); ++iter)
        {
            strTmp = (*iter)->d_name +
                     " [" + StringConverter::toString(mActItemID[ITEMLIST_BACKPACK]) + "]"+
                     " [" + StringConverter::toString((*iter)->tag) + "]"+
                     " [" + ObjectWrapper::getSingleton().getMeshName((*iter)->face & ~0x8000) + "]";
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, strTmp.c_str());
        }
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Container:", 0x00ff0000);
    if (mItemList[ITEMLIST_CONTAINER].empty())
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "<empty>");
    else
        for (iter = mItemList[ITEMLIST_CONTAINER].begin(); iter!= mItemList[ITEMLIST_CONTAINER].end(); ++iter)
        {
            strTmp = (*iter)->d_name +
                     " [" + StringConverter::toString(mActItemID[ITEMLIST_CONTAINER]) + "]"+
                     " [" + StringConverter::toString((*iter)->tag) + "]"+
                     " [" + ObjectWrapper::getSingleton().getMeshName((*iter)->face & ~0x8000) + "]";
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, strTmp.c_str());
        }
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Ground:", 0x00ff0000);
    if (mItemList[ITEMLIST_GROUND].empty())
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "<empty>");
    else
        for (iter = mItemList[ITEMLIST_GROUND].begin(); iter!= mItemList[ITEMLIST_GROUND].end(); ++iter)
        {
            strTmp = (*iter)->d_name +
                     " [" + StringConverter::toString(mActItemID[ITEMLIST_GROUND]) + "]"+
                     " [" + StringConverter::toString((*iter)->tag) + "]"+
                     " [" + ObjectWrapper::getSingleton().getMeshName((*iter)->face & ~0x8000) + "]";
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, strTmp.c_str());
        }
}

/*
    char buf[256];
    sprintf(buf, "%d", mActHeroContainerID);
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, buf);
*/
