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
#include "textwindow.h"
#include "spell_manager.h"
#include "event.h"
#include "TileManager.h"

extern Camera *mCamera;

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
unsigned int NPC::mInstanceNr = 0; // mInstanceNr = Player's Hero
static ParticleFX *tempPFX =0;

//=================================================================================================
// Init the model from the description file.
//=================================================================================================
NPC::NPC(SceneManager *SceneMgr, SceneNode *Node, const char *desc_filename, Radian Facing)
{
  if (!mInstanceNr) tempPFX = new ParticleFX(mNode, "SwordGlow", "Particle/SwordGlow");
  mNode = Node;
  // mFacing = Facing;
  thisNPC = mInstanceNr;
  mDescFile = DIR_MODEL_DESCRIPTION;
  mDescFile += desc_filename;
  if (!mInstanceNr)
  {
    Logger::log().headline("Init Actor Models");
  }
  bool status = Option::getSingleton().openDescFile(mDescFile.c_str());
  Logger::log().info()  << "Parse description file " << mDescFile << "..." << Logger::success(status);
  if(!status)
  {
    Logger::log().error() << "CRITICAL: description file was not found!"; return;
  }
  mSceneMgr = SceneMgr;

  // mSceneMgr->setFog(FOG_LINEAR , ColourValue(.7,.7,.7), 0.005, 450, 800);
  // mSceneMgr->setFog(FOG_LINEAR , ColourValue(.0,.0,.0), 0.005, 450, 800);
  // mSceneMgr->setFog(FOG_LINEAR , ColourValue(1,1,1), 0.005, 450, 800);

  string strTemp;
  Option::getSingleton().getDescStr("MeshName", strTemp);

  /// The first NPC is the player.
  if (!thisNPC)
  {
    mEntityNPC = mSceneMgr->createEntity("Player [polyveg]", strTemp.c_str());
    mNode->attachObject(mEntityNPC);
    // mNode->roll(Radian(45));
    // mNode->pitch(Degree(45));
    // mNode->yaw(Degree(180));
//    mNode->scale(Vector3(.5,.5,.5));

    /*
    //====== geck0's settings =====
    mNode->scale(Vector3(.3,.3,.3));
    mNode->setPosition(Vector3(1050, 60, 1050));
    Event->getCamera()->setProjectionType(PT_ORTHOGRAPHIC);
    Event->getCamera()->setFOVy(Degree(90));
    Event->getCamera()->setPosition(Vector3(1500, 500, 1500));
    Event->getCamera()->yaw(Degree(45));
    Event->getCamera()->pitch(Degree(-35.264));
    */
    const Real CAMERA_Y = 500;
    
    Vector3 pos = Vector3(TILE_SIZE * CHUNK_SIZE_X /2 + TILE_SIZE/2, 0,
                          TILE_SIZE * CHUNK_SIZE_Z /2 - TILE_SIZE/2);
    Event->getCamera()->setProjectionType(PT_ORTHOGRAPHIC);
    Event->getCamera()->setFOVy(Degree(MAX_CAMERA_ZOOM));
    Event->getCamera()->setPosition(Vector3(pos.x, pos.y+CAMERA_Y, pos.z+CAMERA_Y));
    Event->getCamera()->pitch(Degree(-45));

    Real height = Event->pgTileManager->Get_Map_Height((short)(pos.x)/TILE_SIZE, (short)(pos.z)/TILE_SIZE)*3;
    pos.y += height;
    pos.z += height;
    mNode->setPosition(pos);

    /// Set the Init-pos of the TileEngine.
    pos = Vector3(0,0,0);
    Event->setWorldPos(pos);
  }
  else /// This is a NPC.
  {
    mEntityNPC = mSceneMgr->createEntity("NPC_"+StringConverter::toString(mInstanceNr), strTemp.c_str());
    mNode->attachObject(mEntityNPC);
  }


  // Create Animations and Animation sounds.
  mAnim = new Animate(mEntityNPC); // Description File must be open when you call me.
  mTurning      =0;
  mWalking      =0;
  mEntityWeapon =0;
  mEntityShield =0;
  mEntityArmor  =0;
  mEntityHelmet =0;
  ++mInstanceNr;
  return;
}

//=================================================================================================
// Select a new texture.
//=================================================================================================
void NPC::toggleTexture(int pos, int texture)
{
  string strValue , strKeyword;
  if (!(Option::getSingleton().openDescFile(mDescFile.c_str())))
  {
    Logger::log().error() << "NPC::toggleTexture(...) -> description file was not found!";
    return;
  }
  // Get material.
  strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Name";
  if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue)))
  {
    return;
  }
  MaterialPtr mpMaterial = MaterialManager::getSingleton().getByName(strValue);
  // Get texture.
  if (texture >=0) // select a texture by value.
  {
    strKeyword = "Material_" + StringConverter::toString(pos, 2, '0') + "_Texture_" + StringConverter::toString(texture, 2, '0');
    if (!(Option::getSingleton().getDescStr(strKeyword.c_str(), strValue)))
    {
      return;
    }
  }
  else // toggle textures
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
  // set new texture.
  mpMaterial->unload();
  mpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(strValue);
  mpMaterial->reload();
  mpMaterial.setNull();
}

//=================================================================================================
// Toggle npc equipment.
//=================================================================================================
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
        mEntityNPC->attachObjectToBone(mStrTemp, mEntityArmor, Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(posX, posY, posZ));
      }
      else mArmor =0;  // testing -> delete me!
      break;
  }
}

typedef std::list<Particle*> ActiveParticleList;


//=================================================================================================
// Update npc.
//=================================================================================================
void NPC::update(const FrameEvent& event)
{
  mAnim->update(event);
  mTranslateVector = Vector3(0,0,0);
  if (mAnim->isMovement())
  {
    if (mTurning)
    {
      mFacing += Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mTurning);
      mNode->yaw(Radian(event.timeSinceLastFrame * mAnim->getTurnSpeed() * mTurning));
    }
    if (mWalking)
    {
      // just a test...
      mAnim->toggleAnimation(STATE_WALK1);
      mTranslateVector.x = sin(mFacing.valueRadians())* mAnim->getAnimSpeed() * mWalking;
      mTranslateVector.z = cos(mFacing.valueRadians())* mAnim->getAnimSpeed() * mWalking;

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
        Real height = Event->pgTileManager->Get_Map_Height((short)(pPos.x)/TILE_SIZE, (short)(pPos.z)/TILE_SIZE)*3;
        mNode->setPosition(pPos.x, pPos.y-370 + height, tt -390 +height);
      }
    }
    else
    {
      mAnim->toggleAnimation(STATE_IDLE1);
    }
  }
}

//=================================================================================================
// Cast a spell.
//=================================================================================================
void NPC::castSpell(int spell)
{
  //  if (!askServer.AllowedToCast(spell)) return;
  SpellManager::getSingleton().addObject(spell, thisNPC);
}
