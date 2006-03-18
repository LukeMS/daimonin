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

#include "OgreParticleSystem.h"
#include "object_npc.h"
#include "particle.h"
#include "sound.h"
#include "option.h"
#include "logger.h"
#include "spell_manager.h"
#include "events.h"
#include "TileManager.h"
#include "gui_manager.h"

// #define WRITE_MODELTEXTURE_TO_FILE

///================================================================================================
/// Init all static Elemnts.
///================================================================================================
unsigned int NPC::mInstanceNr = 0; // mInstanceNr 0 = Player's Hero
SceneManager *NPC::mSceneMgr =0;
uchar *NPC::texImageBuf = 0;

const uint32 MASK_COLOR = 0xffc638db; /// This is our mask. Pixel with this color will not be drawn.

NPC::sPicture NPC::picFace =
  {
    65, 75, // w, h
    324, 13,  // dst pos.
    188,  1,  // src pos.
    0, 79 // offest next src pic.
  };
NPC::sPicture NPC::picHair =
  {
    49, 85, // w, h
    130, 3,  // dst pos.
    254, 1,  // src pos.
    0, 86 // offest next src pic.
  };
NPC::sPicture NPC::picBody[2] =
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
NPC::sPicture NPC::picArms[4] =
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
NPC::sPicture NPC::picHands[4] =
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
NPC::sPicture NPC::picBelt[2] =
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
NPC::sPicture NPC::picLegs[2] =
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
NPC::sPicture NPC::picShoes[2] =
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

typedef std::list<Particle*> ActiveParticleList;
static ParticleFX *tempPFX =0;

///================================================================================================
/// Free all recources.
///================================================================================================
void NPC::freeRecources()
{
  if (!--mInstanceNr)
  {
    if (tempPFX) delete tempPFX;
    if (texImageBuf) delete[] texImageBuf;
  }
  if (mAnim) delete mAnim;
  mTexture.setNull();
}

///================================================================================================
/// Init the model from the description file.
///================================================================================================
NPC::NPC(const char *desc_filename, int posX, int posZ, float Facing)
{
  if (!mSceneMgr) mSceneMgr = Event->GetSceneManager();
  if (!mInstanceNr)
  {
    texImageBuf = new uchar[MAX_MODEL_TEXTURE_SIZE * MAX_MODEL_TEXTURE_SIZE * sizeof(uint32)];
    tempPFX = new ParticleFX(mNode, "SwordGlow", "Particle/SwordGlow");
    Logger::log().headline("Init Actor Models");
  }
  mFacing = Degree(Facing);
  thisNPC = mInstanceNr++;
  mDescFile = PATH_MODEL_DESCRIPTION;
  mDescFile += desc_filename;
  Logger::log().info()  << "Adding object from file: " << mDescFile << "...";
  if(!Option::getSingleton().openDescFile(mDescFile.c_str()))
  {
    Logger::log().success(false);
    Logger::log().error() << "CRITICAL: description file was not found!";
    return;
  }
  Logger::log().success(true);

  /// ////////////////////////////////////////////////////////////////////
  /// Build the mesh name.
  /// ////////////////////////////////////////////////////////////////////
  string strTemp = desc_filename;
  strTemp.replace(strTemp.find(".desc"), 5, ".mesh");
  /// ////////////////////////////////////////////////////////////////////
  /// The first NPC is our Hero.
  /// ////////////////////////////////////////////////////////////////////
  if (!thisNPC)
  {
    mEntityNPC = mSceneMgr->createEntity("Player [polyveg]", strTemp.c_str());
    mPosX = CHUNK_SIZE_X /2;
    mPosZ = CHUNK_SIZE_Z /2;
  }
  else /// This is a NPC.
  {
    mEntityNPC = mSceneMgr->createEntity("NPC_"+StringConverter::toString(thisNPC), strTemp.c_str());
    mPosX = posX;
    mPosZ = posZ;
  }
  const AxisAlignedBox &AABB = mEntityNPC->getBoundingBox();
  mBoundingBox.x = Math::Abs(AABB.getMaximum().x) - Math::Abs(AABB.getMinimum().x) + TILE_SIZE/2;
  mBoundingBox.y = Math::Abs(AABB.getMinimum().y);
  mBoundingBox.z = Math::Abs(AABB.getMaximum().z) - Math::Abs(AABB.getMinimum().z) + TILE_SIZE/2;
  Vector3 pos;
  pos.x = mPosX * TILE_SIZE + mBoundingBox.x;
  pos.y = (Real) (Event->getTileManager()->Get_Map_StretchedHeight(mPosX, mPosZ) + mBoundingBox.y);
  pos.z = mPosZ * TILE_SIZE + mBoundingBox.z;
  mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos, Quaternion(1.0,0.0,0.0,0.0));
  //mNode = Event->GetWorldNode()->createChildSceneNode(pos, Quaternion(1.0,0.0,0.0,0.0));
  mNode->attachObject(mEntityNPC);
  /// ////////////////////////////////////////////////////////////////////
  /// We ignore the material of the mesh and create an own material.
  /// ////////////////////////////////////////////////////////////////////
  /// Clone the NPC-Material.
  String tmpName = "NPC_" + StringConverter::toString(thisNPC, 3, '0');
  MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("NPC");
  MaterialPtr mMaterial = tmpMaterial->clone(tmpName);
  mEntityNPC->getSubEntity(0)->setMaterialName(tmpName);

  /// Create a texture for the material.
  Image image;
  image.loadDynamicImage(texImageBuf, MAX_MODEL_TEXTURE_SIZE, MAX_MODEL_TEXTURE_SIZE, PF_A8R8G8B8);
  tmpName +="_Texture";
  mTexture = TextureManager::getSingleton().loadImage(tmpName, "General", image, TEX_TYPE_2D, 3, 1.0f);
  mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(tmpName);
  //mNode->showBoundingBox(true); // Remove Me!!!!

  /// Set the default Colors of the model.
  setTexture(   0,   0,   0);
  setTexture(   2,   1,   0);
  setTexture(   3,   2,   0);
  setTexture(   4,   3,   0);
  setTexture(   5,   4,   0);
  setTexture(   6,   5,   0);
  setTexture(   7,   6,   0);

  /// Create Animations and Animation sounds.
  mAnim = new Animate(mEntityNPC); // Description File must be open when you call me.
  mAutoTurning = false;
  mAutoMoving = false;
  mTurning      =0;
  mWalking      =0;
  mEntityWeapon =0;
  mEntityShield =0;
  mEntityArmor  =0;
  mEntityHelmet =0;
}

///================================================================================================
/// Toggle npc equipment.
///================================================================================================
void  NPC::toggleMesh(int Bone, int WeaponNr)
{
  if (!(Option::getSingleton().openDescFile(mDescFile.c_str())))
  {
    Logger::log().error()  << "CRITICAL: description file: '" << mDescFile << "' was not found!\n";
    return;
  }
  static int mWeapon=0, mShield=0, mHelmet=0, mArmor =0; // testing -> delete me!
  string mStrTemp;

  switch (Bone)
  {
      case BONE_WEAPON_HAND:
      WeaponNr = ++mWeapon; // testing -> delete me!
      if (mEntityWeapon)
      {
        mEntityNPC->detachObjectFromBone("weapon");
        mSceneMgr->destroyEntity(mEntityWeapon);
        mEntityWeapon =0;
      }
      if (Option::getSingleton().getDescStr("M_Name_Weapon", mStrTemp, WeaponNr))
      {
        mEntityWeapon = mSceneMgr->createEntity("weapon", mStrTemp);
        Option::getSingleton().getDescStr("StartX_Weapon", mStrTemp, WeaponNr);
        Real posX = atof(mStrTemp.c_str());
        Option::getSingleton().getDescStr("StartY_Weapon", mStrTemp, WeaponNr);
        Real posY = atof(mStrTemp.c_str());
        Option::getSingleton().getDescStr("StartZ_Weapon", mStrTemp, WeaponNr);
        Real posZ = atof(mStrTemp.c_str());
        Option::getSingleton().getDescStr("Bone_Right_Hand", mStrTemp);
        mEntityNPC->attachObjectToBone(mStrTemp, mEntityWeapon, Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(posX, posY, posZ));
        if (WeaponNr==1)
        {
          mEntityNPC->attachObjectToBone(mStrTemp, tempPFX->getParticleFX(), Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(posX, posY, posZ+5));
        }
        if (WeaponNr==2)
        {
          mEntityNPC->detachObjectFromBone("SwordGlow");
        }
      }
      else mWeapon =0;  // testing -> delete me!
      break;

      case BONE_SHIELD_HAND:
      {
        WeaponNr = ++mShield; // testing -> delete me!
        if (mEntityShield)
        {
          mEntityNPC->detachObjectFromBone("shield");
          mSceneMgr->destroyEntity(mEntityShield);
          mEntityShield =0;
        }
        if (Option::getSingleton().getDescStr("M_Name_Shield", mStrTemp, WeaponNr))
        {
          mEntityShield = mSceneMgr->createEntity("shield", mStrTemp);     //  oben  links  vorne
          Option::getSingleton().getDescStr("StartX_Shield", mStrTemp, WeaponNr);
          Real posX = atof(mStrTemp.c_str());
          Option::getSingleton().getDescStr("StartY_Shield", mStrTemp, WeaponNr);
          Real posY = atof(mStrTemp.c_str());
          Option::getSingleton().getDescStr("StartZ_Shield", mStrTemp, WeaponNr);
          Real posZ = atof(mStrTemp.c_str());
          Option::getSingleton().getDescStr("Bone_Left_Hand", mStrTemp);
          mEntityNPC->attachObjectToBone(mStrTemp, mEntityShield, Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(posX, posY, posZ));
        }
        else mShield =0;  // testing -> delete me!
      }
      break;

      case BONE_HEAD:
      {
        { // delete me!
          if (WeaponNr) mNode->scale(1.1, 1.1, 1.1);
          else          mNode->scale(0.9, 0.9, 0.9);
          Vector3 sc = mNode->getScale();
          std::string scale= "model size: " + StringConverter::toString(sc.x);
          GuiManager::getSingleton().sendMessage(
            GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*) scale.c_str());
          return;
        }

        WeaponNr = ++mHelmet; // testing -> delete me!
        if (mEntityHelmet)
        {
          mEntityNPC->detachObjectFromBone("helmet");
          mSceneMgr->destroyEntity(mEntityHelmet);
          mEntityHelmet =0;
        }
        if (Option::getSingleton().getDescStr("M_Name_Helmet", mStrTemp, WeaponNr))
        {
          mEntityHelmet = mSceneMgr->createEntity("helmet", mStrTemp);
          Option::getSingleton().getDescStr("StartX_Helmet", mStrTemp, WeaponNr);
          Real posX = atof(mStrTemp.c_str());
          Option::getSingleton().getDescStr("StartY_Helmet", mStrTemp, WeaponNr);
          Real posY = atof(mStrTemp.c_str());
          Option::getSingleton().getDescStr("StartZ_Helmet", mStrTemp, WeaponNr);
          Real posZ = atof(mStrTemp.c_str());
          Option::getSingleton().getDescStr("Bone_Head", mStrTemp);
          mEntityNPC->attachObjectToBone(mStrTemp, mEntityHelmet, Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(posX, posY, posZ));
        }
        else mHelmet =0;  // testing -> delete me!
      }
      break;

      case BONE_BODY:
      WeaponNr = ++mArmor; // testing -> delete me!
      if (mEntityArmor)
      {
        mEntityNPC->detachObjectFromBone("armor");
        mSceneMgr->destroyEntity(mEntityArmor);
        mEntityArmor =0;
      }
      if (Option::getSingleton().getDescStr("M_Name_Armor", mStrTemp, WeaponNr))
      {
        mEntityArmor = mSceneMgr->createEntity("armor", mStrTemp);
        Option::getSingleton().getDescStr("StartX_Armor", mStrTemp, WeaponNr);
        Real posX = atof(mStrTemp.c_str());
        Option::getSingleton().getDescStr("StartY_Armor", mStrTemp, WeaponNr);
        Real posY = atof(mStrTemp.c_str());
        Option::getSingleton().getDescStr("StartZ_Armor", mStrTemp, WeaponNr);
        Real posZ = atof(mStrTemp.c_str());
        Option::getSingleton().getDescStr("Bone_Body", mStrTemp);
        mEntityNPC->attachObjectToBone(mStrTemp, mEntityArmor, Quaternion::IDENTITY, Vector3(posX, posY, posZ));
      }
      else mArmor =0;  // testing -> delete me!
      break;
  }
}

///================================================================================================
/// Update npc.
///================================================================================================
void NPC::update(const FrameEvent& event)
{
  mAnim->update(event);
  mTranslateVector = Vector3(0,0,0);
  if (mFacing.valueDegrees() >= 360) mFacing -= Degree(360);
  if (mFacing.valueDegrees() <    0) mFacing += Degree(360);

  if (mAutoTurning)
  {
    int turningDirection;
    int deltaDegree = ((int)mFacing.valueDegrees() - (int)mNewFacing.valueDegrees());
    if (deltaDegree <   0) deltaDegree += 360;
    if (deltaDegree < 180) turningDirection = -1; else turningDirection = 1;
    mFacing += Degree(event.timeSinceLastFrame * mAnim->getTurnSpeed() * turningDirection);
    mNode->yaw(Degree(event.timeSinceLastFrame * mAnim->getTurnSpeed() * turningDirection));
    /// Are we facing into the right direction (+/- 1 degree)?
    if (deltaDegree <= 1) mAutoTurning = false;
  }
  else if (mAutoMoving)
  {
    mAnim->toggleAnimation(Animate::STATE_WALK1);
    /// We are very close to destination.
    Vector3 dist = mWalkToPos - mNode->getPosition();
    if(dist.squaredLength() < 1)
    {
      /// Set the exact destination pos.
      mPosX = mWalkToX;
      mPosZ = mWalkToZ;

      mWalkToPos.x = mBoundingBox.x + mPosX * TILE_SIZE;
      mWalkToPos.y = mBoundingBox.y + Event->getTileManager()->Get_Avg_Map_Height(mPosX, mPosZ);
      mWalkToPos.z = mBoundingBox.z + mPosZ * TILE_SIZE;
      mNode->setPosition(mWalkToPos);
      mAutoMoving = false;
      mAnim->toggleAnimation(Animate::STATE_IDLE1);
    }
    else
    {
      /// We have to move on.
      /*
      // just a test...
      mAnim->toggleAnimation(STATE_WALK1);
      mTranslateVector.x = sin(mFacing.valueRadians())* mAnim->getAnimSpeed() * mWalking;
      mTranslateVector.z = cos(mFacing.valueRadians())* mAnim->getAnimSpeed() * mWalking;
      */

      //      mDeltaPos /= mDeltaPos.squaredLength();
      Vector3 NewTmpPosition = mNode->getPosition() - event.timeSinceLastFrame *  mDeltaPos;
      mPosX = (int) (NewTmpPosition.x / TILE_SIZE +1);
      mPosZ = (int) (NewTmpPosition.z / TILE_SIZE +1);
      //      NewTmpPosition.y = (Real) (Event->getTileManager()->Get_Avg_Map_Height(mPosTileX, mPosTileZ) + mBoundingBox.y);
      mNode->setPosition(NewTmpPosition);
    }
    return;
  }
  if (mAnim->isMovement())
  {
    if (mTurning)
    {
      mFacing += Degree(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mTurning);
      mNode->yaw(Degree(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mTurning));
    }
    if (mWalking)
    {
      // just a test...
      mAnim->toggleAnimation(Animate::STATE_WALK1);
      mTranslateVector.x = Math::Sin(mFacing.valueRadians())* mAnim->getAnimSpeed() * mWalking;
      mTranslateVector.z = Math::Cos(mFacing.valueRadians())* mAnim->getAnimSpeed() * mWalking;

      //   mTranslateVector = mNode->getOrientation().zAxis();
      mNode->translate(mTranslateVector);
      if(thisNPC) // All NPC's.
      {
        mFacing += Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mWalking);
        mNode->yaw(Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mWalking));
        mNode->translate(mTranslateVector);
      }
      else // Hero has moved.
      {
        static Vector3 pos = mNode->getPosition();
        pos+= mTranslateVector;
        Event->setWorldPos(mTranslateVector);

        // Add the terrain height to the y-pos of player.
        Vector3 myPos = mNode->getPosition();

        /// Iportant no bounds check for get_map_height right now.
        /// It will crash if you leave the map!

        Vector3 pPos = Event->getCamera()->getPosition();
        Real tt = pPos.z;
        // pPos.z = 22*30 -(pPos.z- 524+10);
        pPos.z -= 534;
        Real height = Event->getTileManager()->Get_Avg_Map_Height((short)(pPos.x)/TILE_SIZE+1, (short)(pPos.z)/TILE_SIZE);
        mNode->setPosition(pPos.x, pPos.y-390 + height, tt -390);
      }
    }
    else
    {
      mAnim->toggleAnimation(Animate::STATE_IDLE1);
    }
  }
}

///================================================================================================
/// Cast a spell.
///================================================================================================
void NPC::castSpell(int spell)
{
  //  if (!askServer.AllowedToCast(spell)) return;
  SpellManager::getSingleton().addObject(spell, thisNPC);
}

///================================================================================================
/// Turn the player until it faces the given tile.
///================================================================================================
void NPC::faceToTile(int x, int z)
{
  float deltaX = x - mPosX;
  float deltaZ = z - mPosZ;

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
void NPC::moveToTile(int x, int z)
{
  // only for testing (tile-world smaller than screen) needed.
  if (x < 0 || z < 0 || x > CHUNK_SIZE_X || z > CHUNK_SIZE_Z) return;

  if(mPosX == x && mPosZ == z || mAutoTurning || mAutoMoving) return;
  /// Turn the head into the moving direction.
  faceToTile(x, z);
  /// Move it.
  mWalkToPos.x = x * TILE_SIZE + mBoundingBox.x;
  mWalkToPos.y = (Real) (Event->getTileManager()->Get_Avg_Map_Height(x, z) + mBoundingBox.y);
  mWalkToPos.z = z * TILE_SIZE + mBoundingBox.z;
  mDeltaPos = mNode->getPosition() - mWalkToPos;
  mWalkToX = x;
  mWalkToZ = z;
  mAutoMoving = true;
}

///================================================================================================
/// Draw a part of the texture.
///================================================================================================
inline void NPC::drawBopyPart(sPicture &picPart, Image &image, uint32 texNumber, uint32 texColor)
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
void NPC::setTexture(int pos, int textureColor, int textureNr)
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
      Logger::log().warning() << "Unknown Texuture-pos (" << pos << ") for NPC.";
      break;
  }
}
