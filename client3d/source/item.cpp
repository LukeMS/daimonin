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

#include "item.h"
#include "logger.h"
#include "network.h"
#include "gui_manager.h"
//================================================================================================
// Init all static Elemnts.
//================================================================================================




//================================================================================================
//
//================================================================================================
Item::Item()
{
    actContainerTag = CONTAINER_UNKNOWN;
}

//================================================================================================
//
//================================================================================================
Item::~Item()
{
    for (int i= -1; i < 2; ++i) clearContainer(i);
}

//================================================================================================
//  Inserts op in the list of free items
//  Note that it don't clear all fields in item
//================================================================================================
void Item::remove_Item(sItem *op)
{
    /*
        // IF no op, or it is the player.
        if (!op || op == player || op == cpl.below || op == cpl.sack)
            return;
        op->env->inv_updated = 1;

        // Do we really want to do this?
        if (op->inv)
            remove_item_inventory(op);

        if (op->prev)
        {
            op->prev->next = op->next;
        }
        else
        {
            op->env->inv = op->next;
        }
        if (op->next)
        {
            op->next->prev = op->prev;
        }

        // add object to a list of free objects
        op->next = free_items;
        if (op->next != NULL)
            op->next->prev = op;
        free_items = op;

        // Clear all these values, since this item will get re-used.
        op->prev= 0;
        op->inv = 0;
        op->env = 0;
        op->tag = 0;
        op->face= 0;
        op->d_name = "";
        op->s_name = "";
        op->p_name = "";
        op->magical= op->cursed = op->damned = 0;
        op->unpaid = op->locked = op->applied = 0;
        op->flagsval = 0;
        op->weight = 0;
        op->animation_id = 0;
        op->last_anim = 0;
        op->anim_state = 0;
        op->nrof = 0;
        op->open = 0;
        op->type = 255;
        */
}

//================================================================================================
//  Returns pointer to the item which tag is given.
//  returns 0 if item was not found.
//================================================================================================
Item::sItem *Item::locate_Item(int container, unsigned int tag)
{
    std::list<sItem*>::const_iterator iter;
    if (container == CONTAINER_INVENTORY)
    {
        for (iter = HeroBackpack.begin(); iter!=HeroBackpack.end(); ++iter)
            if (tag == (*iter)->tag) return (*iter);
    }
    else if (container == CONTAINER_TILEGROUND)
    {
        for (iter = HeroTileGround.begin(); iter!=HeroTileGround.end(); ++iter)
            if (tag == (*iter)->tag) return (*iter);
    }
    else
    {
        for (iter = HeroBackpack.begin(); iter!=HeroBackpack.end(); ++iter)
            if (tag == (*iter)->tag) return (*iter);
    }
    return 0;
}


//================================================================================================
// called remove_item_inventory in client2d.
//================================================================================================
void Item::clearContainer(int container)
{
    std::list<sItem*>::const_iterator iter;
    if (container <0)
    {
        for (iter = HeroBackpack.begin(); iter!=HeroBackpack.end(); ++iter)
            delete (*iter);
    }
    else if (container = CONTAINER_TILEGROUND)
    {
        for (iter = HeroTileGround.begin(); iter!=HeroTileGround.end(); ++iter)
            delete (*iter);
    }
    else
    {
        for (iter = OpenContainer.begin(); iter!=OpenContainer.end(); ++iter)
            delete (*iter);
    }
}

//Logger::log().error() << (*iter)->d_name;
//GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, (*iter)->d_name.c_str());

//================================================================================================
// Add all Items to Inventory, Backpack, Ground or open Container.
//================================================================================================
void Item::ItemXYCmd(unsigned char *data, int len, int bflag)
{



    return;

    sItem *tmpItem = new sItem;
    int pos = 4;
    int mode= Network::getSingleton().GetInt_String(data);
    int loc = Network::getSingleton().GetInt_String(data + pos); pos+= 4;
    if (mode >= 0) // Put it into the Inventory.
    {
        clearContainer(loc);
    }
    else if (mode == -4) // Transfer Items between containers.
    {
        if (loc == actContainerTag)
            loc = CONTAINER_INVENTORY;
    }
    else if (mode == -1) // Toggle container (open/close).
    {
        clearContainer(CONTAINER_INVENTORY);
        if (loc == CONTAINER_INVENTORY)
        {
            actContainerTag = CONTAINER_UNKNOWN;
            return;
        }
        actContainerTag = loc;
        loc = CONTAINER_INVENTORY;
    }

    while (pos < len)
    {
        tmpItem->tag    = Network::getSingleton().GetInt_String(data + pos); pos += 4;
        tmpItem->flags  = Network::getSingleton().GetInt_String(data + pos); pos += 4;
        tmpItem->weight = Network::getSingleton().GetInt_String(data + pos); pos += 4;
        tmpItem->face   = Network::getSingleton().GetInt_String(data + pos); pos += 4;
        //request_face(face, 0);
        tmpItem->direction = data[pos++];
        if (loc != CONTAINER_TILEGROUND)
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
        tmpItem->animation_id = Network::getSingleton().GetShort_String(data + pos); pos += 2;
        tmpItem->anim_speed = data[pos++];
        tmpItem->nrof = Network::getSingleton().GetInt_String(data + pos); pos += 4;

        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, tmpItem->d_name.c_str());
        update_item(tmpItem, loc, bflag);
        if (update_item(tmpItem, loc, bflag) ==true)
            tmpItem = new sItem;
    }
    delete tmpItem;
    //    map_udate_flag = 2;
}


//================================================================================================
// Upates an item with new attributes.
//================================================================================================
bool Item::update_item(sItem *tmpItem, int loc, int bflag)
{
    bool ret;
/*
    item *ip = locate_item(tag);
    item *env= locate_item(loc);

        //HeroTileGround.push_back(tmpItem);

    // Need to do some special handling if this is the player that is being updated.
    if (player->tag == tag)
    {
        copy_name(player->d_name, name);
        player->weight = weight;
        player->face = face;
        get_flags(player, flags);
        if (player->inv)
            player->inv->inv_updated = 1;
        player->animation_id = anim;
        player->anim_speed = animspeed;
        player->nrof = 1;
        player->direction = direction;
        ret = false;
    }
    else
    {
        if (ip && ip->env == env)
        {
            ret = false;
        }
        else
        {
            ret = true;
            HeroTileGround.push_back(tmpItem);
        }
    }

*/
    return ret;
}
