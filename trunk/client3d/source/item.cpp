/*---------------------------------------------7--------------------------------
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

#include <OgreMath.h>  // Without it VC reports an error in OgreMath.h.
#include <OgreStringConverter.h>
#include "item.h"
#include "logger.h"
#include "profiler.h"
#include "network.h"
#include "gui/gui_manager.h"
#include "tile/tile_map_wrapper.h"

using namespace Ogre;


// todo put this into the object class

#define D_DEBUG

//================================================================================================
// Init all static Elements.
//================================================================================================

//================================================================================================
// Constructor.
//================================================================================================
Item::Item()
{
    PROFILE()
    mActItemID[ITEMLIST_GROUND]    = 0; // Ground tile container ID is always 0.
    mActItemID[ITEMLIST_BACKPACK]  = CONTAINER_UNKNOWN;
    mActItemID[ITEMLIST_CONTAINER] = CONTAINER_UNKNOWN;
    // Link the item lists to the gui-slots-elements where the items are displayed.
    mSlotID[ITEMLIST_GROUND]     = GuiManager::SLOT_EQUIPMENT;
    mSlotID[ITEMLIST_BACKPACK]   = GuiManager::SLOTGROUP_INVENTORY;
    mSlotID[ITEMLIST_CONTAINER]  = GuiManager::SLOT_CONTAINER;
}

//================================================================================================
// Destructor.
//================================================================================================
Item::~Item()
{
    PROFILE()
    for (int i =0; i < ITEMLIST_SUM; ++i)
        clearContainer(mActItemID[i]);
}

//================================================================================================
// Clear the whole container.
//================================================================================================
void Item::clearContainer(int container)
{
    PROFILE()
    for (int i =0; i < ITEMLIST_SUM; ++i)
    {
        if (container != mActItemID[i]) continue;
        for (std::list<sItem*>::const_iterator iter = mItemList[i].begin(); iter!= mItemList[i].end(); ++iter)
            delete *iter;
        mItemList[i].clear();
        //GuiManager::getSingleton().clrItem(mSlotID[i]);
        return;
    }
    GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, "Item::clearContainer failed!");
}

//================================================================================================
// Add all Items to Inventory, Backpack, Ground or open Container.
//================================================================================================
void Item::ItemXYCmd(uchar *data, int len, bool bflag)
{
    PROFILE()
    int pos = 0;
    int mode      = Network::getSingleton().GetInt_String(data + pos); pos+= 4;
    int container = Network::getSingleton().GetInt_String(data + pos); pos+= 4;
    if (mode == MODE_TOGGLE_OPEN)
    {
        if (container == CONTAINER_UNKNOWN)
        {
            // Close container.
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
        tmpItem->tag      = Network::getSingleton().GetInt_String(data + pos); pos += 4;
        tmpItem->flagsval = Network::getSingleton().GetInt_String(data + pos); pos += 4;
        tmpItem->weight   = Network::getSingleton().GetInt_String(data + pos); pos += 4;
        tmpItem->face     = Network::getSingleton().GetInt_String(data + pos); pos += 4;
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
        tmpItem->d_name = "";
        for (int i = data[pos++]; i>1; --i)
            tmpItem->d_name+= data[pos++];
        ++pos; // skip '\0'
        tmpItem->animation_id = Network::getSingleton().GetShort_String(data + pos);  pos += 2;
        tmpItem->anim_speed = data[pos++];
        tmpItem->sumItems = Network::getSingleton().GetInt_String(data + pos); pos += 4;
#ifdef D_DEBUG
        String strDebug = "Add item: " + tmpItem->d_name + "  gfx-id: " + StringConverter::toString(tmpItem->face);
        Logger::log().debug() << Logger::ICON_CLIENT << strDebug;
        GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, strDebug.c_str());
#endif
        if (!update(tmpItem, container, bflag))
        {
            Logger::log().error() << Logger::ICON_CLIENT << "<Item::ItemXYCmd> Unknown container ID: " << container << " for item " << tmpItem->d_name;
            delete tmpItem;
        }
    }
    // map_udate_flag = 2;
}

//================================================================================================
// Returns the container in which the item is located.
//================================================================================================
int Item::getContainerID(unsigned int ItemID)
{
    PROFILE()
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
const Item::sItem *Item::locateItem(int container, unsigned int tag)
{
    PROFILE()
    std::list<sItem*>::iterator iter;
    for (int i =0; i < ITEMLIST_SUM; ++i)
    {
        if (mActItemID[i] == container || mActItemID[i] == CONTAINER_UNKNOWN)
        {
            for (iter = mItemList[i].begin(); iter!= mItemList[i].end(); ++iter)
                if (tag == (*iter)->tag) return *iter;
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
    PROFILE()
    std::list<sItem*>::iterator iter;
    for (int i =0; i < ITEMLIST_SUM; ++i)
    {
        if (mActItemID[i] == container || mActItemID[i] == CONTAINER_UNKNOWN)
        {
            for (iter = mItemList[i].begin(); iter!= mItemList[i].end(); ++iter)
            {
                if ((*iter)->tag == item)
                {
                    //GuiManager::getSingleton().delItem(mSlotID[i], *iter);
                    delete *iter;
                    mItemList[i].erase(iter);
                    return;
                }
            }
        }
    }
}

//================================================================================================
// Add an Item to a given container.
//================================================================================================
bool Item::addItem(sItem *tmpItem, int container)
{
    PROFILE()
    for (int i =0; i < ITEMLIST_SUM; ++i)
    {
        if (mActItemID[i] == container)
        {
            mItemList[i].push_back(tmpItem);
            GuiManager::getSingleton().addItem(mSlotID[i], getItemGfxName(tmpItem->face),
                                               tmpItem->sumItems, tmpItem->d_name.c_str());
            return true;
        }
    }
    return false;
}

//================================================================================================
// Update the item location.
//================================================================================================
bool Item::update(sItem *tmpItem, int newContainerID, bool /*bflag*/)
{
    PROFILE()
    int actContainerID = getContainerID(tmpItem->tag);
    // The item doesn't has a container yet.
    if (actContainerID == CONTAINER_UNKNOWN)
    {
        return addItem(tmpItem, newContainerID);
    }
    // Move the item into an existing container.
    if (actContainerID != newContainerID)
    {
        delItem(tmpItem->tag, actContainerID);
        return addItem(tmpItem, newContainerID);
    }
    // Not supported container.
    return false;
}

//================================================================================================
// Moves an item from the ground to the inventory.
//================================================================================================
void Item::getInventoryItemFromFloor(int slotNr)
{
    PROFILE()
    char buffer[256];
    if (slotNr >= (int)mItemList[ITEMLIST_GROUND].size()) return;
    std::list<sItem*>::iterator iter;
    for (iter = mItemList[ITEMLIST_GROUND].begin(); slotNr-- && iter != mItemList[ITEMLIST_GROUND].end();)  ++iter;
    sprintf(buffer, "drop %s", (*iter)->d_name.c_str());
    GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, buffer);
    // move item to Backpack.
    sprintf(buffer, "mv %d %d %d", mActItemID[ITEMLIST_BACKPACK], (*iter)->tag, (*iter)->sumItems);
    GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, buffer);
    //Network::getSingleton().cs_write_string(buffer);
    delete *iter;
    mItemList[ITEMLIST_GROUND].erase(iter);
    return;
}

//================================================================================================
// Moves an item from the inventory to the ground.
//================================================================================================
void Item::dropInventoryItemToFloor(int slotNr)
{
    PROFILE()
    //int sumItems = 1;
    //sound_play_effect(SOUND_DROP, 0, 0, 100);
    char buffer[256];
    std::list<sItem*>::iterator iter;
    for (iter = mItemList[ITEMLIST_BACKPACK].begin(); slotNr-- && iter != mItemList[ITEMLIST_BACKPACK].end();)  ++iter;
    sprintf(buffer, "drop %s", (*iter)->d_name.c_str());
    GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, buffer);
    // move item TO Ground.
    sprintf(buffer, "mv %d %d %d", mActItemID[ITEMLIST_GROUND], (*iter)->tag, (*iter)->sumItems);
    //Network::getSingleton().cs_write_string(buffer);
    GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, buffer);
    //GuiManager::getSingleton().delItem(GuiManager::WIN_INVENTORY, *iter);
    delete *iter;
    mItemList[ITEMLIST_BACKPACK].erase(iter);
}

//================================================================================================
// End of Drag'n'Drop.
//================================================================================================
void Item::dropItem(int srcWindow, int srcItemSlot, int dstWindow, int dstItemSlot)
{
    PROFILE()
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
    char buffer[256];
    sprintf(buffer, "drag and drop src: %d, %d dest: %d, %d", srcWindow, srcItemSlot, dstWindow, dstItemSlot);
    GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, buffer);
    return;
}

//================================================================================================
// Get the item pos in the item-texture-atlas.
//================================================================================================
const char *Item::getItemGfxName(int itemFace)
{
    PROFILE()
    const int MASK_FACEFILTER = ~0x8000; // Filter to extract the face number (gfx-id).
    return ObjectWrapper::getSingleton().getMeshName(itemFace & MASK_FACEFILTER);
}

//================================================================================================
// Just for Debug. Print the items of all containers.
//================================================================================================
void Item::printAllItems()
{
    PROFILE()
    String strTmp;
    std::list<sItem*>::iterator iter;
    const char *names[ITEMLIST_SUM] = {"Backpack", "Container:", "Ground:" };
    for (int c = 0; c < ITEMLIST_SUM; ++c)
    {
        GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, names[c], 0x00ff0000);
        if (mItemList[ITEMLIST_BACKPACK].empty())
        {
            GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, "<empty>");
        }
        else
        {
            for (iter = mItemList[ITEMLIST_BACKPACK].begin(); iter!= mItemList[ITEMLIST_BACKPACK].end(); ++iter)
            {
                strTmp = (*iter)->d_name +
                         " [" + StringConverter::toString(mActItemID[ITEMLIST_BACKPACK]) + "]"+
                         " [" + StringConverter::toString((*iter)->tag) + "]"+
                         " [" + ObjectWrapper::getSingleton().getMeshName((*iter)->face & ~0x8000) + "]";
                GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, strTmp.c_str());
            }
        }
    }
}
