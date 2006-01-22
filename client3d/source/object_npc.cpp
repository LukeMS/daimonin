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

///================================================================================================
/// Init all static Elemnts.
///================================================================================================
unsigned int NPC::mInstanceNr = 0; // mInstanceNr 0 = Player's Hero
SceneManager *NPC::mSceneMgr =0;

sPicture NPC::picSkin  = {1,   1, 255, 255 };
sPicture NPC::picHair  = {55,   0,  45,  32 };
uint32   NPC::color[MAX_NPC_COLORS] =
  {
    0x00e3ad91, 0x00f2dc91, 0x00c95211, 0x0037250b,
    0x00ffffff, 0x00000000, 0x00000000, 0x00000000,
    0x00ffffff, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
  };

static ParticleFX *tempPFX =0;

///================================================================================================
/// Free all recources.
///================================================================================================
void NPC::freeRecources()
{
  if (!--mInstanceNr) delete tempPFX;
  if (mAnim) delete mAnim;
  mTexture.setNull();
}

///================================================================================================
/// Init the model from the description file.
///================================================================================================
NPC::NPC(SceneNode *Node, const char *desc_filename, float Facing)
{
  if (!mSceneMgr) mSceneMgr = Event->GetSceneManager();
  if (!mInstanceNr) tempPFX = new ParticleFX(mNode, "SwordGlow", "Particle/SwordGlow");
  mNode = Node;
  mFacing = Degree(Facing);
  thisNPC = mInstanceNr++;
  mDescFile = PATH_MODEL_DESCRIPTION;
  mDescFile += desc_filename;
  if (!mInstanceNr)  Logger::log().headline("Init Actor Models");
  Logger::log().info()  << "Parse description file " << mDescFile << "...";
  if(!Option::getSingleton().openDescFile(mDescFile.c_str()))
  {
    Logger::log().success(false);
    Logger::log().error() << "CRITICAL: description file was not found!";
    return;
  }
  Logger::log().success(true);

  // mSceneMgr->setFog(FOG_LINEAR , ColourValue(.7,.7,.7), 0.005, 450, 800);
  // mSceneMgr->setFog(FOG_LINEAR , ColourValue(1,1,1), 0.005, 450, 800);

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
    mNode->attachObject(mEntityNPC);
    const AxisAlignedBox &AABB = mEntityNPC->getBoundingBox();
    Vector3 pos;
    mBoundingBox.x = Math::Abs(AABB.getMaximum().x) - Math::Abs(AABB.getMinimum().x) + TILE_SIZE/2;
    mBoundingBox.y = Math::Abs(AABB.getMinimum().y) /2;
    mBoundingBox.z = Math::Abs(AABB.getMaximum().z) - Math::Abs(AABB.getMinimum().z) + TILE_SIZE/2;

    mPosTileX = CHUNK_SIZE_X /2;
    mPosTileZ = CHUNK_SIZE_Z /2;
    pos.x = mPosTileX * TILE_SIZE + mBoundingBox.x;
    pos.y = (Real) (Event->getTileManager()->Get_Map_StretchedHeight(mPosTileX, mPosTileZ) + mBoundingBox.y);
    pos.z = mPosTileZ * TILE_SIZE + mBoundingBox.z;
    mNode->setPosition(pos);

    mAutoTurning = false;
    mAutoMoving = false;

    /*
        const Real CAMERA_Y = 500;
        Event->getCamera()->setProjectionType(PT_ORTHOGRAPHIC);
        Event->getCamera()->setFOVy(Degree(MAX_CAMERA_ZOOM));
        //    Event->getCamera()->setPosition(Vector3(pos.x, pos.y+CAMERA_Y, pos.z+CAMERA_Y));
        Event->getCamera()->setPosition(Vector3((CHUNK_SIZE_X * TILE_SIZE)/2, pos.y+CAMERA_Y, pos.z+CAMERA_Y));
        Event->getCamera()->pitch(Degree(-45));
    */

    Event->getCamera()->setProjectionType(PT_ORTHOGRAPHIC);
    Event->getCamera()->setFOVy(Degree(MAX_CAMERA_ZOOM));
    Event->getCamera()->setPosition(Vector3(CHUNK_SIZE_X *TILE_SIZE/2 , pos.y+ 400, CHUNK_SIZE_Z * TILE_SIZE + 680));
    Event->getCamera()->pitch(Degree(-25));

    /// Set the Init-pos of the TileEngine.
    pos = Vector3(0,0,0);
    Event->setWorldPos(pos);
  }
  else /// This is a NPC.
  {
    mEntityNPC = mSceneMgr->createEntity("NPC_"+StringConverter::toString(mInstanceNr), strTemp.c_str());
    mNode->attachObject(mEntityNPC);
  }

  /// ////////////////////////////////////////////////////////////////////
  /// We ignore the material of the mesh and create a own one.
  /// ////////////////////////////////////////////////////////////////////
  /// Clone the NPC-Material.
  String tmpName = "NPC_" + StringConverter::toString(thisNPC, 3, '0');
  MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("NPC");
  MaterialPtr mMaterial = tmpMaterial->clone(tmpName);
  //mMaterial->unload();
  mEntityNPC->getSubEntity(0)->setMaterialName(tmpName);
  //mMaterial->reload();


  /// Create a texture for the material.
  tmpName +="_Texture";
  Image image;
  image.load("Human_Male.png", "General");
  mTexture = TextureManager::getSingleton().loadImage(tmpName, "General", image, TEX_TYPE_2D, 3, 1.0f);
  mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(tmpName);
  //mMaterial->load();


  mNode->scale(.4,.4,.4); // Remove Me!!!!
  //    mNode->showBoundingBox(true); // Remove Me!!!!






  /// Create Animations and Animation sounds.
  mAnim = new Animate(mEntityNPC); // Description File must be open when you call me.
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
        mSceneMgr->removeEntity(mEntityWeapon);
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
      WeaponNr = ++mShield; // testing -> delete me!
      if (mEntityShield)
      {
        mEntityNPC->detachObjectFromBone("shield");
        mSceneMgr->removeEntity(mEntityShield);
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
      break;

      case BONE_HEAD:
      WeaponNr = ++mHelmet; // testing -> delete me!
      if (mEntityHelmet)
      {
        mEntityNPC->detachObjectFromBone("helmet");
        mSceneMgr->removeEntity(mEntityHelmet);
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
      break;

      case BONE_BODY:
      WeaponNr = ++mArmor; // testing -> delete me!
      if (mEntityArmor)
      {
        mEntityNPC->detachObjectFromBone("armor");
        mSceneMgr->removeEntity(mEntityArmor);
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

typedef std::list<Particle*> ActiveParticleList;


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
      mPosTileX = mWalkToX;
      mPosTileZ = mWalkToZ;

      mWalkToPos.x = mBoundingBox.x + mPosTileX * TILE_SIZE;
      mWalkToPos.y = mBoundingBox.y + Event->getTileManager()->Get_Avg_Map_Height(mPosTileX, mPosTileZ);
      mWalkToPos.z = mBoundingBox.z + mPosTileZ * TILE_SIZE;
      mNode->setPosition(mWalkToPos);
      mAutoMoving = false;
      mAnim->toggleAnimation(Animate::STATE_IDLE1);
    }
    /// We have to move on.
    else
    {
      /*
      // just a test...
      mAnim->toggleAnimation(STATE_WALK1);
      mTranslateVector.x = sin(mFacing.valueRadians())* mAnim->getAnimSpeed() * mWalking;
      mTranslateVector.z = cos(mFacing.valueRadians())* mAnim->getAnimSpeed() * mWalking;
      */

      //      mDeltaPos /= mDeltaPos.squaredLength();
      Vector3 NewTmpPosition = mNode->getPosition() - event.timeSinceLastFrame *  mDeltaPos;
      mPosTileX = (int) (NewTmpPosition.x / TILE_SIZE +1);
      mPosTileZ = (int) (NewTmpPosition.z / TILE_SIZE +1);
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
/// Turn the mob until it faces the given tile.
///================================================================================================
void NPC::faceToTile(int x, int z)
{
  float deltaX = x - mPosTileX;
  float deltaZ = z - mPosTileZ;

  /// This is the position of the mob.
  if (deltaX ==0 && deltaZ ==0) return;

  mNewFacing = Radian(Math::ATan(deltaX/deltaZ));
  if      (deltaZ <0) mNewFacing+=Degree(180);
  else if (deltaX <0) mNewFacing+=Degree(360);
  mAutoTurning = true;
}

///================================================================================================
/// Move the mob to the given tile.
///================================================================================================
void NPC::moveToTile(int x, int z)
{

  // only for this test (tile-world smaller than screen) needed.
  if (x < 0 || z < 0 || x > CHUNK_SIZE_X || z > CHUNK_SIZE_Z) return;


  if(mPosTileX == x && mPosTileZ == z || mAutoTurning || mAutoMoving) return;

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
/// Select a new texture.
///================================================================================================
void NPC::setTexture(int pos, int texColor, int textureNr)
{
  switch (pos)
  {
      case TEXTURE_POS_SKIN:
      {
        /// Blit the color over the whole head.
        texColor = color[texColor];
        /// Cretate a temporary buffer for the pixel operations.
        uint32 *buffer = new uint32[picSkin.w * picSkin.h];
        PixelBox pb(picSkin.w, picSkin.h, 1, PF_A8R8G8B8 , buffer);
        /// Fill it with the color.
        for (int i=0; i < picSkin.w * picSkin.h; ++i) buffer[i] = texColor;

        /// Blit the face texture over it.
        if (textureNr >=0)
        {
        }
        /// Copy the buffer into the model-texture.
        mTexture->getBuffer()->blitFromMemory(pb, Box(0, picSkin.y, picSkin.x + picSkin.w , picSkin.y + picSkin.h));
        delete[] buffer;
      }
      break;

      case TEXTURE_POS_HAIR:
      {
        /// Blit the color over the whole head.
        texColor = color[texColor];
        /// Cretate a temporary buffer for the pixel operations.
        uint32 *buffer = new uint32[picHair.w * picHair.h];
        PixelBox pb(picHair.w, picHair.h, 1, PF_A8R8G8B8 , buffer);
        /// Fill it with the color.
        for (int i=0; i < picHair.w * picHair.h; ++i) buffer[i] = texColor;
        /// Blit the face texture over it.
        if (textureNr >=0) // -1 -> baldness.
        {
          /// Copy the buffer into the model-texture.
          mTexture->getBuffer()->blitFromMemory(
            PixelBox(picHair.w, picHair.h, 1, PF_A8R8G8B8, buffer),
            Box(picHair.x, picHair.y, picHair.x + picHair.w , picHair.y + picHair.h));
          Image image;
          image.load("Human_M_Default.jpg", "General");
          uint32 *copy = (uint32*)image.getData();
          pb = mTexture->getBuffer()->lock(Box(0,0, 256, 256), HardwareBuffer::HBL_READ_ONLY );
          uint32 *dest_data = (uint32*)pb.data;
          uint32 pixColor,  srcColor;
          for (unsigned int y = 0; y < 256 * 256; ++y)
          {
            if (!copy[y]) continue;
            pixColor = dest_data[y];
            srcColor = copy[y] & 0xff000000;
            srcColor >>= 8;
            if ((pixColor & 0x00ff0000) > srcColor )  pixColor-= srcColor;
            srcColor >>= 8;
            if ((pixColor & 0x0000ff00) > srcColor )  pixColor-= srcColor;
            srcColor >>= 8;
            if ((pixColor & 0x000000ff) > srcColor )  pixColor-= srcColor;
            dest_data[y] = pixColor;
          }
        }
        mTexture->getBuffer()->unlock();
        delete[] buffer;
      }
      break;


      default:
      Logger::log().warning() << "Unknown Texuture-pos (" << pos << ") for NPC.";
      break;
  }



  /*
    string strValue , strKeyword;
    if (!(Option::getSingleton().openDescFile(mDescFile.c_str())))
    {
      Logger::log().error() << "NPC::toggleTexture(...) -> description file was not found!";
      return;
    }
    /// Get material.
    strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Name";
    if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue)))
    {
      return;
    }
    MaterialPtr mpMaterial = MaterialManager::getSingleton().getByName(strValue);
    /// Get texture.
    if (texture >=0) // select a texture by value.
    {
      strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Texture_" + StringConverter::toString(texture, 2, '0');
      if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue)))
      {
        return;
      }
    }
    else /// toggle textures
    { // only for testing...
      static int actTexture[100];
      strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Texture_" + StringConverter::toString(actTexture[pos], 2, '0');
      if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue)))
      {
        actTexture[pos] =0;
        strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Texture_" + StringConverter::toString(actTexture[pos], 2, '0');
        if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue)))
        {
          return;
        }
      }
      ++actTexture[pos];
    }
    /// set new texture.
    mpMaterial->unload();
    mpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(strValue);
    mpMaterial->reload();
    mpMaterial.setNull();
  */
}
