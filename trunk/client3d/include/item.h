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

#ifndef ITEM_H
#define ITEM_H

#include <list>
#include <Ogre.h>

/** Item structure keeps all information of what the
 ** player knows about items in his inventory.
 *****************************************************************************/
class Item
{
public:
// ////////////////////////////////////////////////////////////////////
// Variables / Constants.
// ////////////////////////////////////////////////////////////////////
    enum
    {
        F_APPLIED   = 0x0000F,
        F_ETHEREAL  = 0x00080,
        F_LOCATION  = 0x000F0,
        F_INVISIBLE = 0x00100,
        F_UNPAID    = 0x00200,
        F_MAGIC     = 0x00400,
        F_CURSED    = 0x00800,
        F_DAMNED    = 0x01000,
        F_OPEN      = 0x02000,
        F_NOPICK    = 0x04000,
        F_LOCKED    = 0x08000,
        F_TRAPED    = 0x10000,
    };
    enum // This will replace the bitfield below...
    {
        IS_MAGICAL = 1 << 0, /**< item is magical   **/
        IS_CURSED  = 1 << 1, /**< item is cursed    **/
        IS_DAMNED  = 1 << 2, /**< item is damned    **/
        IS_LOCKED  = 1 << 3, /**< item is locked    **/
        IS_UNPAID  = 1 << 4, /**< item is unpaid    **/
        IS_TRAPED  = 1 << 5, /**< item is traped    **/
        IS_APPLIED = 1 << 6, /**< item is applied   **/
        IS_OPEN    = 1 << 7, /**< container is open **/
        IS_DIRTY   = 1 << 8, /**< An item in this container was updated. **/
    };
    typedef struct sItem
    {
bool magical     :
        1; /**< item is magical   **/
bool cursed      :
        1; /**< item is cursed    **/
bool damned      :
        1; /**< item is damned    **/
bool unpaid      :
        1; /**< item is unpaid    **/
bool locked      :
        1; /**< item is locked    **/
bool traped      :
        1; /**< item is traped    **/
bool applied     :
        1; /**< item is applied   **/
bool open        :
        1; /**< container is open **/
bool inv_updated :
        1; /**< container was updated **/
        Ogre::String d_name; /**< item's full name w/o status information **/
        Ogre::String s_name; /**< item's singular name as sent to us **/
        Ogre::String p_name; /**< item's plural name as sent to us **/
        Ogre::String flags;  /**< item's status information **/
        unsigned int tag;   /**< item identifier (0 = free) **/
        int nrof;           /**< number of items **/
        int weight;         /**< how much item weights **/
        short face;         /**< index for face array **/
        unsigned short animation_id; /**< Index into animation array **/
        unsigned short anim_speed;   /**< how often to animate **/
        unsigned short anim_state;   /**< last face in sequence drawn **/
        unsigned short last_anim;    /**< how many ticks have passed since we last animated **/
        /**<  when item's inventory is modified, draw routines can use this to redraw things **/
        unsigned int flagsval;    /**< unmodified flags value as sent from the server **/
        unsigned char apply_type; /**< how item is applied (worn/wield/etc) **/
        unsigned char type;       /**< Item type for ordering **/
        unsigned char itype;
        unsigned char stype;
        unsigned char item_qua;
        unsigned char item_con;
        unsigned char item_skill;
        unsigned char item_level;
        unsigned char direction;
    }
    sItem;
    enum
    {
        ITEMLIST_BACKPACK,  // Items in backpack.
        /** A container is an item which can hold other items (e.g. chest, sack, etc).
            Every time we open a container, the server send us all containing items.
            So we can forget about the inventory of closed containers.
            Therefore we only need 1 itemlist for open containers. **/
        ITEMLIST_CONTAINER, // Items in the actual open container.
        ITEMLIST_GROUND,    // Items on the ground.
        ITEMLIST_SUM
    };
    enum
    {
        CONTAINER_UNKNOWN   = -1, /**< Currently no container is open. **/
    };
    enum
    {
        MODE_KEEP_ITEMS = -4, /**< Keep the items. **/
        MODE_TOGGLE_OPEN= -1  /**< Toggle the container (open/close). **/
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static Item &getSingleton()
    {
        static Item Singleton;
        return Singleton;
    }
    void dropItem(int srcWindow, int srcSlot, int dstWindow, int dstSlot);
    void ItemXYCmd(unsigned char *data, int len, bool bflag);
    void clearContainer(int container);
    void delItem(unsigned int item, int container);
    void addItem(sItem *tmpItem, int container);
    void dropInventoryItemToFloor(int slotNr);
    void getInventoryItemFromFloor(int slotNr);
    bool update(sItem *tmpItem, int newContainerID, bool bflag);
    int  getContainerID(unsigned int ItemID);
    sItem *locateItem(int container, unsigned int tag);
    void printAllItems();
    void setBackpackID(int id)
    {
        mActItemID[ITEMLIST_BACKPACK] = id;
    }
private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    std::list<sItem*> mItemList[ITEMLIST_SUM];
    int mActItemID[ITEMLIST_SUM]; /**< ID of the actual itemContainer **/
    int mWindowID[ITEMLIST_SUM];  /**< ID of window containing the slots for the items **/
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    Item();
    ~Item();
    Item(const Item&); // disable copy-constructor.
};
#endif
