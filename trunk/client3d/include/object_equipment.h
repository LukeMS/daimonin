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

#ifndef ObjectEquipment_H
#define ObjectEquipment_H

#include <Ogre.h>

/**
 ** This class handles all equipment of an object like weapon, armour, etc.
 ** Equipment supports particle-effects and color change of the bodyparts.
 *****************************************************************************/
class ObjectEquipment
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
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
        BONE_PELVIS,
        BONE_CENTER,
        BONE_HEAD,
        BONE_NECK,
        BONE_SPINE,       BONE_SPINE1,
        BONE_LCLAVICLE,   BONE_RCLAVICLE,
        BONE_LUPPER_ARM,  BONE_RUPPER_ARM,
        BONE_LFORE_ARM,   BONE_RFORE_ARM,
        BONE_L_HAND,      BONE_R_HAND,
        BONE_WEAPON_HAND, BONE_SHIELD_HAND,
        BONE_LTHIGH,      BONE_RTHIGH,
        BONE_LCALF,       BONE_RCALF,
        BONE_LFOOT,       BONE_RFOOT,
        BONE_LTOES,       BONE_RTOES,
        BONE_SUM
    };
    enum
    {
        TEXTURE_POS_SKIN, TEXTURE_POS_FACE, TEXTURE_POS_HAIR,
        TEXTURE_POS_LEGS, TEXTURE_POS_BODY,
        TEXTURE_POS_BELT, TEXTURE_POS_SHOES, TEXTURE_POS_HANDS
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectEquipment(Ogre::Entity *parent);
    ~ObjectEquipment();
    void freeRecources();
    void setTexture(int pos, int textureColor, int textureNr =0);
    void drawBopyPart(sPicture &picPart, Ogre::uint32 texColor, Ogre::uint32 texNumber);
    void dropItem(int bone);
    void equipItem(unsigned int bone, int type, int itemID, int particleID =-1);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static sPicture picHands[4], picArms[4], picShoes[2], picBody[2], picLegs[2], picFace, picHair, picBelt[2];
    static Ogre::uchar *mTexImageBuf;
    static Ogre::Image shadowImage;
    enum
    {
        SIDE_BACK,
        SIDE_FRONT
    };
    Ogre::TexturePtr mTexture;
    struct _mItem
    {
        Ogre::Entity *entity;
        Ogre::ParticleSystem *particle;
    }
    mItem[BONE_SUM];
    Ogre::Entity *mParentEntity;
    static unsigned long mIndex;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectEquipment(const ObjectEquipment&); // disable copy-constructor.
};
#endif
