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
    enum
    {
        TYPE_PLAYER              =  1,
        TYPE_BULLET              =  2,
        TYPE_ROD                 =  3,
        TYPE_TREASURE            =  4,
        TYPE_POTION              =  5,
        TYPE_FOOD                =  6,
        TYPE_POISON              =  7,
        TYPE_BOOK                =  8,
        TYPE_CLOCK               =  9,
        TYPE_FBULLET             = 10,
        TYPE_FBALL               = 11,
        TYPE_LIGHTNING           = 12,
        TYPE_ARROW               = 13,
        TYPE_BOW                 = 14,
        TYPE_WEAPON              = 15,
        TYPE_ARMOUR              = 16,
        TYPE_PEDESTAL            = 17,
        TYPE_ALTAR               = 18,
        TYPE_CONFUSION           = 19,
        TYPE_LOCKED_DOOR         = 20,
        TYPE_SPECIAL_KEY         = 21,
        TYPE_MAP                 = 22,
        TYPE_DOOR                = 23,
        TYPE_KEY                 = 24,
        TYPE_MMISSILE            = 25,
        TYPE_TIMED_GATE          = 26,
        TYPE_TRIGGER             = 27,
        TYPE_GRIMREAPER          = 28,
        TYPE_MAGIC_EAR           = 29,
        TYPE_TRIGGER_BUTTON      = 30,
        TYPE_TRIGGER_ALTAR       = 31,
        TYPE_TRIGGER_PEDESTAL    = 32,
        TYPE_SHIELD              = 33,
        TYPE_HELMET              = 34,
        TYPE_HORN                = 35,
        TYPE_MONEY               = 36,
        TYPE_CLASS               = 37,
        TYPE_GRAVESTONE          = 38,
        TYPE_AMULET              = 39,
        TYPE_PLAYERMOVER         = 40,
        TYPE_TELEPORTER          = 41,
        TYPE_CREATOR             = 42,
        TYPE_SKILL               = 43,
        TYPE_EXPERIENCE          = 44,
        TYPE_EARTHWALL           = 45,
        TYPE_GOLEM               = 46,
        TYPE_BOMB                = 47,
        TYPE_THROWN_OBJ          = 48,
        TYPE_BLINDNESS           = 49,
        TYPE_GOD                 = 50,
        TYPE_DETECTOR            = 51,
        TYPE_SPEEDBALL           = 52,
        TYPE_DEAD_OBJECT         = 53,
        TYPE_DRINK               = 54,
        TYPE_MARKER              = 55,
        TYPE_HOLY_ALTAR          = 56,
        TYPE_PLAYER_CHANGER      = 57,
        TYPE_BATTLEGROUND        = 58,
        TYPE_PEACEMAKER          = 59,
        TYPE_GEM                 = 60,
        TYPE_FIRECHEST           = 61,
        TYPE_FIREWALL            = 62,
        TYPE_ANVIL               = 63,
        TYPE_CHECK_INV           = 64,
        TYPE_MOOD_FLOOR          = 65,
        TYPE_EXIT                = 66,
        TYPE_ENCOUNTER           = 67,
        TYPE_SHOP_FLOOR          = 68,
        TYPE_SHOP_MAT            = 69,
        TYPE_RING                = 70,
        TYPE_FLOOR               = 71,
        TYPE_FLESH               = 72,
        TYPE_INORGANIC           = 73,
        TYPE_LIGHT_APPLY         = 74,
        TYPE_LIGHTER             = 75,
        TYPE_TRAP_PART           = 76,
        TYPE_WALL                = 77,
        TYPE_LIGHT_SOURCE        = 78,
        TYPE_MISC_OBJECT         = 79,
        TYPE_MONSTER             = 80,
        TYPE_SPAWN_GENERATOR     = 81,
        TYPE_RESERVED_082        = 82,
        TYPE_RESERVED_083        = 83,
        TYPE_RESERVED_084        = 84,
        TYPE_SPELLBOOK           = 85,
        TYPE_RESERVED_086        = 86,
        TYPE_CLOAK               = 87,
        TYPE_CONE                = 88,
        TYPE_AURA                = 89,
        TYPE_SPINNER             = 90,
        TYPE_GATE                = 91,
        TYPE_BUTTON              = 92,
        TYPE_CF_HANDLE           = 93,
        TYPE_HOLE                = 94,
        TYPE_TRAPDOOR            = 95,
        TYPE_WORD_OF_RECALL      = 96,
        TYPE_PARAIMAGE           = 97,
        TYPE_SIGN                = 98,
        TYPE_BOOTS               = 99,
        TYPE_GLOVES              =100,
        TYPE_RESERVED_101        =101,
        TYPE_RESERVED_102        =102,
        TYPE_CONVERTER           =103,
        TYPE_BRACERS             =104,
        TYPE_POISONING           =105,
        TYPE_SAVEBED             =106,
        TYPE_POISONCLOUD         =107,
        TYPE_FIREHOLES           =108,
        TYPE_WAND                =109,
        TYPE_ABILITY             =110,
        TYPE_SCROLL              =111,
        TYPE_DIRECTOR            =112,
        TYPE_GIRDLE              =113,
        TYPE_FORCE               =114,
        TYPE_POTION_EFFECT       =115,
        TYPE_RESERVED_116        =116,
        TYPE_RESERVED_117        =117,
        TYPE_RESERVED_118        =118,
        TYPE_RESERVED_119        =119,
        TYPE_RESERVED_120        =120,
        TYPE_CLOSE_CON           =121,
        TYPE_CONTAINER           =122,
        TYPE_ARMOUR_IMPROVER     =123,
        TYPE_WEAPON_IMPROVER     =124,
        TYPE_RESERVED_125        =125,
        TYPE_RESERVED_126        =126,
        TYPE_RESERVED_127        =127,
        TYPE_RESERVED_128        =128,
        TYPE_RESERVED_129        =129,
        TYPE_SKILLSCROLL         =130,
        TYPE_RESERVED_131        =131,
        TYPE_RESERVED_132        =132,
        TYPE_RESERVED_133        =133,
        TYPE_RESERVED_134        =134,
        TYPE_RESERVED_135        =135,
        TYPE_RESERVED_136        =136,
        TYPE_RESERVED_137        =137,
        TYPE_DEEP_SWAMP          =138,
        TYPE_IDENTIFY_ALTAR      =139,
        TYPE_RESERVED_140        =140,
        TYPE_CANCELLATION        =141,
        TYPE_SHOULDER            =142,
        TYPE_LEGS                =143,
        TYPE_RESERVED_144        =144,
        TYPE_RESERVED_145        =145,
        TYPE_RESERVED_146        =146,
        TYPE_RESERVED_147        =147,
        TYPE_RESERVED_148        =148,
        TYPE_RESERVED_149        =149,
        TYPE_MENU                =150,
        TYPE_BALL_LIGHTNING      =151,
        TYPE_SWARM_SPELL         =153,
        TYPE_RUNE                =154,
        TYPE_RESERVED_155        =155,
        TYPE_POWER_CRYSTAL       =156,
        TYPE_CORPSE              =157,
        TYPE_DISEASE             =158,
        TYPE_SYMPTOM             =159,
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

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static Item &getSingleton()
    {
        static Item Singleton;
        return Singleton;
    }
    /** A container is an item which can hold other items (e.g. chest, sack, etc).
        Every time we open a container, the server send us all containing items.
      . So we can forget about the inventory of closed containers.
        Server can hold only 1 Container at once open.
        Therefore we only need 1 Item list for all containers. **/
    std::list<sItem*> HeroTileGround; /**< The items on the tile our hero stands on.**/
    std::list<sItem*> HeroBackpack;   /**< The items in the backpack.**/
    std::list<sItem*> OpenContainer;  /**< The items in the container currently opened. **/
    int mActOpenContainerID; /**< ID of the actual open container. **/
    int mActHeroContainerID; /**< ID of hero's container (inventory). **/
    int mActGrndContainerID; /**< ID of ground tile container. **/
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
    void ItemXYCmd(unsigned char *data, int len, bool bflag);
    void clearContainer(int container);
    void delItem(unsigned int item, int container);
    void addItem(sItem *tmpItem, int container);
    void dropInventoryItemToFloor(int slotNr);
    bool update(sItem *tmpItem, int newContainerID, bool bflag);
    int  getContainerID(unsigned int ItemID);
    sItem *locateItem(int container, unsigned int tag);
    sItem *getBackpackItem(int slotPosition);
    void printAllItems();
private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    Item();
    ~Item();
    Item(const Item&); // disable copy-constructor.
};
#endif
