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

#ifndef ObjectEquipment_H
#define ObjectEquipment_H

#include "Ogre.h"
#include "define.h"

using namespace Ogre;

class ObjectEquipment
{
public:
    typedef struct
    {
        short w, h;             /**< width and height of the image. **/
        short dstX, dstY;       /**< pos of the image in the model-texture. **/
        short srcX, srcY;       /**< pos of the image in the race-template-texture. **/
        short offsetX, offsetY; /**< offset for the next source image. **/
    }
    sPicture;
    enum
    {
        PARTICLE_FX_FIRE,
        PARTICLE_FX_SUM
    };
    enum
    {
        ITEM_WEAPON,
        ITEM_ARMOR_SHIELD,
        ITEM_ARMOR_HEAD,
        ITEM_ARMOR_BODY,
        ITEM_ARMOR_LEGS,
        ITEM_SUM
    };
    enum
    {
        ME /**< ME (mIndex == 0) is our Hero. **/
    };
    enum
    {
        BONE_WEAPON_HAND, BONE_SHIELD_HAND, BONE_HEAD, BONE_BODY, BONE_SUM
    };
    enum
    {
        TEXTURE_POS_SKIN, TEXTURE_POS_FACE, TEXTURE_POS_HAIR,
        TEXTURE_POS_LEGS, TEXTURE_POS_BODY,
        TEXTURE_POS_BELT, TEXTURE_POS_SHOES, TEXTURE_POS_HANDS
    };

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectEquipment(Entity *parent);
    ~ObjectEquipment();
    void freeRecources();
    void setTexture(int pos, int textureColor, int textureNr);
    void drawBopyPart(sPicture &picPart, Image &image, uint32 texNumber, uint32 texColor);
    void equipItem(int bone, int type, int itemID, int particleID =-1);
    void dropItem(int bone);
    void raiseWeapon(bool raise);

private:
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectEquipment(const ObjectEquipment&); // disable copy-constructor.

    static sPicture picHands[4], picArms[4], picShoes[2], picBody[2], picLegs[2], picFace, picHair, picBelt[2];
    static uchar *texImageBuf;
    enum
    {
        SIDE_BACK,
        SIDE_FRONT
    };
    TexturePtr mTexture;
    Entity *mParentEntity, *mEntity[BONE_SUM];
    ParticleSystem *mPSystem[BONE_SUM];
    static unsigned long mIndex;
};
#endif
