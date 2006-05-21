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

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef NPC_H
#define NPC_H

#include "define.h"
#include "object_animate.h"

using namespace Ogre;

/// Defines:
/// Mob => (M)oveable (Ob)ject.
enum
{
    BONE_WEAPON_HAND, BONE_SHIELD_HAND, BONE_HEAD, BONE_BODY
};
enum
{
    TEXTURE_POS_SKIN, TEXTURE_POS_FACE, TEXTURE_POS_HAIR,
    TEXTURE_POS_LEGS, TEXTURE_POS_BODY,
    TEXTURE_POS_BELT, TEXTURE_POS_SHOES, TEXTURE_POS_HANDS
};

const int SIDE_BACK  =0;
const int SIDE_FRONT =1;
const int MAX_MODEL_TEXTURE_SIZE = 512;

class NPC
{
public:
    typedef struct
    {
        short w, h;             /// width and height of the image.
        short dstX, dstY;       /// pos of the image in the model-texture.
        short srcX, srcY;       /// pos of the image in the race-template-texture.
        short offsetX, offsetY; /// offset for the next source image.
    }
    sPicture;

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    NPC(const char *filename, int posX, int posY, float Facing);
    ~NPC()
    {}
    void freeRecources();
    void drawBopyPart(sPicture &part, Image &image, uint32 number, uint32 color);
    void moveToTile(int x, int z);
    void faceToTile(int x, int z);
    void walking(Real walk)
    {
        mWalking = walk;
    }
    void turning(Real turn)
    {
        mTurning = turn;
    }
    const Vector3 &getPos()
    {
        return mNode->getPosition();
    }
    void getMapPos(int &x, int &z)
    {
        x = mPosX;
        z = mPosZ;
    }
    const Vector3 &getWorldPos()
    {
        return mTranslateVector;
    }
    const SceneNode *getNode()
    {
        return mNode;
    }
    void update(const FrameEvent& event);
    void castSpell(int spell);
    void setTexture(int pos, int color, int textureNr);
    void toggleMesh   (int pos, int WeaponNr);
    void toggleAnimation(int animGroup, int animNr)
    {
        mAnim->toggleAnimation(animGroup, animNr);
    }
    Real getFacing()
    {
        return mFacing.valueRadians();
    }
    void move(Vector3 &pos);
private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    static unsigned int mInstanceNr; /// mInstanceNr = 0 -> Player's Hero
    static SceneManager *mSceneMgr;
    static sPicture picHands[4], picArms[4], picShoes[2], picBody[2], picLegs[2], picFace, picHair, picBelt[2];
    static uchar *texImageBuf;

    unsigned int thisNPC;
    TexturePtr mTexture;
    Real mWalking, mTurning;
    Degree mFacing, mNewFacing;
    int mPosX, mPosZ;   /// the actual tile-pos of the NPC.
    int mWalkToX, mWalkToZ;     /// the destination tile-pos.
    int maxHealth, actHealth;
    int maxMana  , actMana;
    bool mAutoTurning, mAutoMoving;
    SceneNode *mNode;
    Entity *mEntityNPC, *mEntityWeapon, *mEntityShield, *mEntityHelmet, *mEntityArmor;
    Vector3 mTranslateVector, mWalkToPos, mBoundingBox, mDeltaPos;
    Animate *mAnim;
    Real animOffset; /// every npc gets a random animation offset. preventing of  synchronous "dancing"
    std::string mDescFile;

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    NPC(const NPC&); // disable copy-constructor.
};

#endif
