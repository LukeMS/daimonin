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

#include "particle_manager.h"
#include "object_player.h"
#include "object_manager.h"
#include "sound.h"
#include "option.h"
#include "logger.h"
#include "spell_manager.h"
#include "events.h"

// #define WRITE_MODELTEXTURE_TO_FILE

///================================================================================================
/// Init all static Elemnts.
///================================================================================================
uchar *ObjectPlayer::texImageBuf = 0;

const uint32 MASK_COLOR = 0xffc638db; /// This is our mask. Pixel with this color will not be drawn.

ObjectPlayer::sPicture ObjectPlayer::picFace =
    {
        65, 75, // w, h
        324, 13,  // dst pos.
        188,  1,  // src pos.
        0, 79 // offest next src pic.
    };
ObjectPlayer::sPicture ObjectPlayer::picHair =
    {
        49, 85, // w, h
        130, 3,  // dst pos.
        254, 1,  // src pos.
        0, 86 // offest next src pic.
    };
ObjectPlayer::sPicture ObjectPlayer::picBody[2] =
    {
        { // Back
            186, 153, // w, h
            61,  70,  // dst pos.
            1,  155,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front
            186, 153, // w, h
            261,  70,  // dst pos.
            1,  1,  // src pos.
            0, 0 // offest next src pic.
        }
    };
ObjectPlayer::sPicture ObjectPlayer::picArms[4] =
    {
        { // Back Left
            38, 81, // w, h
            56,  155,  // dst pos.
            1,  529,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Back Right
            38, 81, // w, h
            212,  155,  // dst pos.
            40,  529,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front Left
            38, 81, // w, h
            256,  155,  // dst pos.
            79,  529,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front Right
            38, 81, // w, h
            412,  155,  // dst pos.
            118, 529,  // src pos.
            0, 0 // offest next src pic.
        }
    };
ObjectPlayer::sPicture ObjectPlayer::picHands[4] =
    {
        { // Back
            29, 57, // w, h
            58,  236,  // dst pos.
            261, 297,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Back
            29, 57, // w, h
            221, 236,  // dst pos.
            261, 355,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front
            29, 57, // w, h
            259, 236,  // dst pos.
            261, 413,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front
            29, 57, // w, h
            421, 236,  // dst pos.
            261, 471,  // src pos.
            0, 0 // offest next src pic.
        }
    };
ObjectPlayer::sPicture ObjectPlayer::picBelt[2] =
    {
        { // Back
            129, 22, // w, h
            89, 223,  // dst pos.
            157, 529,  // src pos.
            0, 87 // offest next src pic.
        },
        { // Front
            129, 22, // w, h
            291, 223,  // dst pos.
            157, 529,  // src pos.
            0, 87 // offest next src pic.
        }
    };
ObjectPlayer::sPicture ObjectPlayer::picLegs[2] =
    {
        { // Back
            129, 219, // w, h
            89, 245,  // dst pos.
            1,  309,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front
            129, 219, // w, h
            291, 245,  // dst pos.
            131,  309,  // src pos.
            0, 0 // offest (x,y) for next src pic.
        }
    };
ObjectPlayer::sPicture ObjectPlayer::picShoes[2] =
    {
        { // Left
            43, 63, // w, h
            304, 446,  // dst pos.
            157,  575,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Right
            43, 63, // w, h
            363, 446,  // dst pos.
            201, 575,  // src pos.
            0, 0 // offest next src pic.
        }
    };

const Real WALK_PRECISON = 1.0;
const int TURN_SPEED    = 200;

///================================================================================================
/// .
///================================================================================================
ObjectPlayer::~ObjectPlayer()
{}

///================================================================================================
/// Free all recources.
///================================================================================================
void ObjectPlayer::freeRecources()
{
    if (!mIndex)
    {
        //    if (tempPFX) delete tempPFX;
        if (texImageBuf) delete[] texImageBuf;
    }
    if (mAnim) delete mAnim;
    mTexture.setNull();
}

///================================================================================================
/// Init the model from the description file.
///================================================================================================
ObjectPlayer::ObjectPlayer(sObject &obj):ObjectNPC(obj)
{
    /// ////////////////////////////////////////////////////////////////////
    /// The first Object is our Hero.
    /// ////////////////////////////////////////////////////////////////////
    mEntity->setQueryFlags(QUERY_NPC_MASK);
    if (!mIndex)
    {
        texImageBuf = new uchar[MAX_MODEL_TEXTURE_SIZE * MAX_MODEL_TEXTURE_SIZE * sizeof(uint32)];
        Vector3 pos = mNode->getPosition();
        Event->setWorldPos(pos, 0, 0, CEvent::WSYNC_MOVE);
    }
    /// ////////////////////////////////////////////////////////////////////
    /// We ignore the material of the mesh and create an own material.
    /// ////////////////////////////////////////////////////////////////////
    /// Clone the ObjectNPC-Material.
    String tmpName = "Player_" + StringConverter::toString(mIndex, 3, '0');
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("NPC");
    MaterialPtr mMaterial = tmpMaterial->clone(tmpName);
    mEntity->getSubEntity(0)->setMaterialName(tmpName);
    /// Create a texture for the material.
    Image image;
    image.loadDynamicImage(texImageBuf, MAX_MODEL_TEXTURE_SIZE, MAX_MODEL_TEXTURE_SIZE, PF_A8R8G8B8);
    tmpName +="_Texture";
    mTexture = TextureManager::getSingleton().loadImage(tmpName, "General", image, TEX_TYPE_2D, 3, 1.0f);
    mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(tmpName);
    //mNode->showBoundingBox(true); // Remove Me!!!!

    /// Set the default Colors of the model.
    setTexture(0, 0, 0);
    setTexture(2, 1, 0);
    setTexture(3, 2, 0);
    setTexture(4, 3, 0);
    setTexture(5, 4, 0);
    setTexture(6, 5, 0);
    setTexture(7, 6, 0);

    mEntityWeapon =0;
    mEntityShield =0;
    mEntityArmor  =0;
    mEntityHelmet =0;
}

///================================================================================================
/// Toggle ObjectNPC equipment.
///================================================================================================
void ObjectPlayer::toggleMesh(int Bone, int WeaponNr)
{
    /*
    Bone_Right_Hand: "RFingers"
    Bone_Left_Hand : "LFingers"
    Bone_Head      : "Head"
    Bone_Body      : "Spline1"
    */

    switch (Bone)
    {
        case BONE_WEAPON_HAND:
        {
            if (mEntityWeapon)
            {
                mEntity->detachObjectFromBone(mEntityWeapon);
                mEntityWeapon =0;
            }
            mEntityWeapon = (Entity*) ObjectManager::getSingleton().getWeaponEntity(WeaponNr);
            if (!mEntityWeapon) break;
            mEntity->attachObjectToBone("RFingers", mEntityWeapon);
            break;
        }
    }
}

///================================================================================================
/// .
///================================================================================================
void ObjectPlayer::raiseWeapon(bool raise)
{
    if (!raise)
    {
        mEntity->detachObjectFromBone(mEntityWeapon);
        mEntityWeapon =0;
        return;
    }
    if (!mEntityWeapon)
    {
        mEntityWeapon = (Entity*) ObjectManager::getSingleton().getWeaponEntity(0);
        mEntity->attachObjectToBone("RFingers", mEntityWeapon);
    }
}


///================================================================================================
/// Update object.
///================================================================================================
void ObjectPlayer::update(const FrameEvent& event)
{
    mAnim->update(event);
    ///  Finish the current (non movement) anim first.
    if (!mAnim->isMovement()) return;

    mTranslateVector = Vector3(0,0,0);
    if (mFacing.valueDegrees() >= 360) mFacing -= Degree(360);
    if (mFacing.valueDegrees() <    0) mFacing += Degree(360);

    if (mAutoTurning)
    {
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0);
        int turningDirection;
        int deltaDegree = ((int)mFacing.valueDegrees() - (int)mNewFacing.valueDegrees());
        if (deltaDegree <   0) deltaDegree += 360;
        if (deltaDegree < 180) turningDirection = -1; else turningDirection = 1;
        mFacing += Degree(event.timeSinceLastFrame * TURN_SPEED * turningDirection);
        mNode->yaw(Degree(event.timeSinceLastFrame * TURN_SPEED * turningDirection));
        /// Are we facing into the right direction (+/- 1 degree)?
        if (deltaDegree <= .5) mAutoTurning = false;
    }
    else if (mAutoMoving)
    {
        /// We are very close to destination.
        mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_WALK, 0);
        Vector3 dist = mWalkToPos - mNode->getPosition();
        dist.y =0;
        if(dist.squaredLength() < WALK_PRECISON)
        {
            /// Set the exact destination pos.
            mWalkToPos.x = mBoundingBox.x + mActPos.x * TILE_SIZE_X;
            mWalkToPos.z = mBoundingBox.z + mActPos.z * TILE_SIZE_Z;
            mWalkToPos.y = TileManager::getSingleton().getAvgMapHeight(mDstPos.x, mDstPos.z) - mBoundingBox.y;
            mNode->setPosition(mWalkToPos);
            if (!mIndex) Event->setWorldPos(mWalkToPos, mActPos.x - mDstPos.x, mActPos.z - mDstPos.z, CEvent::WSYNC_MOVE);
            mAutoMoving = false;
            mAnim->toggleAnimation(ObjectAnimate::ANIM_GROUP_IDLE, 0);
        }
        else
        {
            /// We have to move on.
            Vector3 NewTmpPosition = - event.timeSinceLastFrame * mDeltaPos;;
            //ParticleManager::getSingleton().pauseAll(true);

            mNode->setPosition(mNode->getPosition() + NewTmpPosition);
            if (!mIndex) Event->setWorldPos(NewTmpPosition, 0, 0, CEvent::WSYNC_OFFSET);
            //ParticleManager::getSingleton().pauseAll(false);
        }
        return;
    }
    if (mAnim->isMovement() && mTurning)
    {
        mFacing += Degree(event.timeSinceLastFrame * TURN_SPEED * mTurning);
        mNode->yaw(Degree(event.timeSinceLastFrame * TURN_SPEED * mTurning));
    }
}

///================================================================================================
/// Cast a spell.
///================================================================================================
void ObjectPlayer::castSpell(int spell)
{
    //  if (!askServer.AllowedToCast(spell)) return;
    SpellManager::getSingleton().addObject(spell, mIndex);
}

///================================================================================================
/// Turn the player until it faces the given tile.
///================================================================================================
void ObjectPlayer::faceToTile(int x, int z)
{
    float deltaX = x - mActPos.x;
    float deltaZ = z - mActPos.z;

    /// This is the position of the player.
    if (deltaX ==0 && deltaZ ==0) return;

    mNewFacing = Radian(Math::ATan(deltaX/deltaZ));
    if      (deltaZ <0) mNewFacing+=Degree(180);
    else if (deltaX <0) mNewFacing+=Degree(360);
    mAutoTurning = true;
}

///================================================================================================
/// Move the player to the given tile.
///================================================================================================
void ObjectPlayer::moveToTile(int x, int z)
{
    if(mActPos.x == x && mActPos.z == z || mAutoTurning || mAutoMoving) return;

    /// Split into waypoints (distance = 1 tile)
    // todo

    // testing: limit the moving distance.
    if (x > mActPos.x+1) x = mActPos.x+1;
    if (x < mActPos.x-1) x = mActPos.x-1;
    if (z > mActPos.z+1) z = mActPos.z+1;
    if (z < mActPos.z-1) z = mActPos.z-1;

    /// Turn the head into the moving direction.
    faceToTile(x, z);
    /// Move it.
    mWalkToPos.x = x * TILE_SIZE_X + mBoundingBox.x;
    mWalkToPos.y = (Real) (TileManager::getSingleton().getAvgMapHeight(x, z) - mBoundingBox.y);
    mWalkToPos.z = z * TILE_SIZE_Z + mBoundingBox.z;
    mDeltaPos = mNode->getPosition() - mWalkToPos;
    if (!mIndex) Event->setWorldPos(mDeltaPos, 0, 0, CEvent::WSYNC_INIT);
    mDstPos.x = x;
    mDstPos.z = z;
    mAutoMoving = true;
}

///================================================================================================
/// Stop movement instantly.
///================================================================================================
void ObjectPlayer::stopMovement()
{
    mNewFacing = mFacing;
    mWalkToPos = mNode->getPosition();
    mAutoTurning = false;
    mAutoMoving = false;
}

///================================================================================================
/// Attack an enemy.
///================================================================================================
void ObjectPlayer::attackShortRange()
{
    if (mAttacking) return;
    mAttacking = true;
    int x, z;
//    Vector3 pos = ObjectManager::getSingleton().getEnemyPos();
}

///================================================================================================
/// Is player currently moving?
///================================================================================================
bool ObjectPlayer::isMoving()
{
    return mAutoMoving;
}

///================================================================================================
/// Draw a part of the texture.
///================================================================================================
inline void ObjectPlayer::drawBopyPart(sPicture &picPart, Image &image, uint32 texNumber, uint32 texColor)
{
    texNumber = 0; // delete me!

    uint32 srcColor, dstColor;
    uint32 *texRace = (uint32*)image.getData();
    uint32 *buffer  = new uint32[picPart.w * picPart.h];
    uint32 *buf = buffer;
    int width = (int)image.getWidth();
    /// Get the color information from the top line of the texture-shadow picture.
    /// We have to swap R and B Color information (default texture format is A8R8G8B8).
    srcColor = texRace[texColor & 0xff];
    texColor = (srcColor & 0xff00ff00);
    texColor+= (srcColor & 0x000000ff) << 16;
    texColor+= (srcColor & 0x00ff0000) >> 16;
    /// Get the current model-texture fragment.
    PixelBox pb(picPart.w, picPart.h, 1, PF_A8R8G8B8 , buffer);
    mTexture->getBuffer()->blitToMemory(
        Box(picPart.dstX,
            picPart.dstY,
            picPart.dstX + picPart.w,
            picPart.dstY + picPart.h),
        pb);
    /// Fill the buffer with the selected color (darkened by the shadow texture).
    for (int y=0; y < picPart.h; ++y)
    {
        for (int x=0; x < picPart.w; ++x)
        {
            srcColor = texRace[(y+picPart.srcY)*width + (x+picPart.srcX)];
            if (srcColor != MASK_COLOR)
            {
                dstColor = texColor;
                if (srcColor != 0xffffffff) /// darkening.
                {
                    srcColor = 0xff - (srcColor & 0xff);
                    if ((dstColor & 0x0000ff) >= srcColor ) dstColor-= srcColor; else dstColor-= dstColor & 0x0000ff;
                    srcColor <<= 8;
                    if ((dstColor & 0x00ff00) >= srcColor ) dstColor-= srcColor; else dstColor-= dstColor & 0x00ff00;
                    srcColor <<= 8;
                    if ((dstColor & 0xff0000) >= srcColor ) dstColor-= srcColor; else dstColor-= dstColor & 0xff0000;
                }
                *buffer = dstColor;
            }
            ++buffer;
        }
    }
    /// Copy the buffer back into the model-texture.
    mTexture->getBuffer()->blitFromMemory(
        pb, Box(picPart.dstX,
                picPart.dstY,
                picPart.dstX + picPart.w,
                picPart.dstY + picPart.h));

    delete[] buf;
#ifdef WRITE_MODELTEXTURE_TO_FILE
    /// Writes the just blitted model-texture as png to disk.
    {
        Image img;
        uint32 *sysFontBuf = new uint32[mTexture->getWidth()*mTexture->getHeight()];
        mTexture->getBuffer()->blitToMemory(PixelBox(mTexture->getWidth(), mTexture->getHeight(), 1, PF_A8R8G8B8, sysFontBuf));
        img = img.loadDynamicImage((uchar*)sysFontBuf, mTexture->getWidth(), mTexture->getHeight(), PF_A8R8G8B8);
        img.save("Texture_Changed.png");
    }
#endif

}

///================================================================================================
/// Select a new texture.
///================================================================================================
void ObjectPlayer::setTexture(int pos, int textureColor, int textureNr)
{
    /// Load the shadow texture.
    Image image;
    image.load("shadow.png", "General");
    switch (pos)
    {
        case TEXTURE_POS_SKIN:
        {
            drawBopyPart(picFace, image, textureNr, textureColor);
            for (int side = 0; side < 4; ++side) drawBopyPart(picArms[side], image,  textureNr, textureColor);
            break;
        }

        case TEXTURE_POS_FACE:
        {
            drawBopyPart(picFace, image, textureNr, textureColor);
            break;
        }

        case TEXTURE_POS_HAIR:
        {
            drawBopyPart(picHair, image, textureNr, textureColor);
            break;
        }

        case TEXTURE_POS_BODY:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picBody[side], image,  textureNr, textureColor);
            break;
        }

        case TEXTURE_POS_LEGS:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picLegs[side], image,  textureNr, textureColor);
            break;
        }

        case TEXTURE_POS_BELT:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picBelt[side], image, textureNr, textureColor);
            break;
        }

        case TEXTURE_POS_SHOES:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picShoes[side], image,  textureNr, textureColor);
            break;
        }

        case TEXTURE_POS_HANDS:
        {
            for (int side = 0; side < 4; ++side) drawBopyPart(picHands[side], image,  textureNr, textureColor);
            break;
        }

        default:
        Logger::log().warning() << "Unknown Texuture-pos (" << pos << ") for ObjectNPC.";
        break;
    }
}
