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

#include <OgreKeyEvent.h>
#include "events.h"
#include "gui_manager.h"
#include "object_manager.h"
#include "option.h"

using namespace Ogre;

///================================================================================================
/// Buffered Key Events.
///================================================================================================
void CEvent::keyPressed(KeyEvent *e)
{
  mIdleTime =0;
  static Real g_pitch = 0.2;
  if (GuiManager::getSingleton().hasFocus())
  {
    GuiManager::getSingleton().keyEvent(e->getKeyChar(), e->getKey());
    e->consume();
    return;
  }
  switch (e->getKey())
  {
      /// ////////////////////////////////////////////////////////////////////
      /// Player Movemment.
      /// ////////////////////////////////////////////////////////////////////
      case KC_UP:
      //      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_WALK, 1);
      //mCamera->  moveRelative (Vector3(0,100,0));
      break;

      case KC_DOWN:
      //      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_WALK, -1);
      //mCamera->  moveRelative (Vector3(0,-100,0));
      break;

      case KC_RIGHT:
      //mCamera->  moveRelative (Vector3(100,0,0));
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TURN, -1);
      break;

      case KC_LEFT:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TURN,  1);
      //mCamera->  moveRelative (Vector3(-100,0,0));
      break;

      case KC_F1:
      ObjectManager::getSingleton().toggleAnimGroup(OBJECT_PLAYER);
      break;

      case KC_A:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_ATTACK1);
      break;

      case KC_B:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_BLOCK1);
      break;

      case KC_C:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_CAST1);
      break;

      case KC_S:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_SLUMP1);
      break;

      case KC_D:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_DEATH1);
      break;

      case KC_H:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_HIT1);
      break;

      case KC_1:
      //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_WEAPON_HAND, 1);
      break;

      case KC_2:
      //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_SHIELD_HAND, 1);
      break;

      case KC_3:
      //ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,0, -1);
      break;

      case KC_4:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_SKIN, 0);
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_HAIR, 10);
      break;

      case KC_5:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_SKIN, 1);
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_HAIR, 4);
      break;

      case KC_6:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_SKIN, 2);
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_HAIR, 4);
      break;

      case KC_7:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_SKIN, 3);
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_HAIR, 4);
      break;

      case KC_8:
      //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_HEAD, 1);
      break;

      case KC_9:
      //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_BODY, 1);
      break;

      case KC_J:
      // ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TURN,  1);
      //mCamera->yaw(Degree(10));
      {
        static bool once = true;
        if (once)
        {
          Entity * entity = mSceneManager->createEntity("test01", "tree1.mesh");
          const AxisAlignedBox &AABB = entity->getBoundingBox();
          Vector3 pos = mTileManager->get_TileInterface()->get_Selection();
          pos.x = (pos.x +0.5) * TILE_SIZE + (fabs(AABB.getMaximum().x) - fabs(AABB.getMinimum().x))/2;
          pos.y+= fabs(AABB.getMaximum().y) - TILE_SIZE/2;
          pos.z = (pos.z +0.5) * TILE_SIZE + (fabs(AABB.getMaximum().z) - fabs(AABB.getMinimum().z))/2;
          SceneNode *node = mSceneManager->getRootSceneNode()->createChildSceneNode();
          node->attachObject(entity);
          node->setPosition(pos.x, pos.y, pos.z);
          once = false;
        }
      }
      break;

      case KC_K:
      {
        static bool once = true;
        if (once)
        {
          ParticleSystem *pSys = ParticleSystemManager::getSingleton().createSystem("Node", "Particle/GreenyNimbus");
          SceneNode *node = mSceneManager->getRootSceneNode()->createChildSceneNode();
          Vector3 pos = mTileManager->get_TileInterface()->get_Selection();
          pos.x = (pos.x +0.5) * TILE_SIZE ;
          pos.y =  pos.y - TILE_SIZE/2;
          pos.z = (pos.z +0.5) * TILE_SIZE ;
          node->attachObject(pSys);
          node->setPosition(pos.x, pos.y, pos.z);
          once = false;
        }
      }
      break;

      case KC_G:
      mTileManager->ToggleGrid();
      break;

      case KC_I:
      ObjectManager::getSingleton().Event(OBJECT_NPC, OBJ_ANIMATION, Animate::STATE_ATTACK1);
      break;

      case KC_P:
      ObjectManager::getSingleton().Event(OBJECT_NPC, OBJ_TEXTURE, 0, 1);
      break;

      case KC_Q:
      ObjectManager::getSingleton().Event(OBJECT_NPC, OBJ_TEXTURE, 0, 1);
      break;

      /// ////////////////////////////////////////////////////////////////////
      /// Engine settings.
      /// ////////////////////////////////////////////////////////////////////
      case KC_Y:
      mSceneDetailIndex = (mSceneDetailIndex+1)%3 ;
      switch(mSceneDetailIndex)
      {
          case 0 : mCamera->setDetailLevel(SDL_SOLID) ;     break ;
          case 1 : mCamera->setDetailLevel(SDL_WIREFRAME) ; break ;
          case 2 : mCamera->setDetailLevel(SDL_POINTS) ;    break ;
      }
      break;

      case KC_T:
      {
        static int pixels =128;
        //change pixel size of terrain textures
        pixels /= 2; // shrink pixel value
        if (pixels < 8) pixels = 128; // if value is too low resize to maximum
        mTileManager->SetTextureSize(pixels);
        mTimeUntilNextToggle = .5;
      }
      break;

      case KC_F:
      {
        static TextureFilterOptions mFiltering = TFO_BILINEAR;
        static int mAniso = 1;
        switch(mFiltering)
        {
            case TFO_BILINEAR:
            mFiltering = TFO_TRILINEAR;
            mAniso = 1;
            break;
            case TFO_TRILINEAR:
            mFiltering = TFO_ANISOTROPIC;
            mAniso = 8;
            break;
            case TFO_ANISOTROPIC:
            mFiltering = TFO_BILINEAR;
            mAniso = 1;
            break;
            default:
            break;
        }
        MaterialManager::getSingleton().setDefaultTextureFiltering(mFiltering);
        MaterialManager::getSingleton().setDefaultAnisotropy(mAniso);
      }
      break;

      case KC_L:
      Option::getSingleton().mStartNetwork = true;
      break;

      case KC_W:
      if (mDayTime)
      {
        mDayTime =0;
        mLight[LIGHT_VOL ]->setVisible(false);
        mLight[LIGHT_SPOT]->setVisible(true);
      }
      else
      {
        mDayTime =15;
        mLight[LIGHT_VOL ]->setVisible(true);
        mLight[LIGHT_SPOT]->setVisible(false);
      }
      break;

      case KC_PGUP:
      {
        mCamera->pitch(Radian(-0.1));
        g_pitch -= 0.1;
        Vector3 pos = mCamera->getPosition();
        pos.y += 30;
        mCamera->setPosition(pos);
      }
      break;

      case KC_PGDOWN:
      {
        mCamera->pitch(Radian(+0.1));
        g_pitch += 0.1;
        Vector3 pos = mCamera->getPosition();
        pos.y -= 30;
        mCamera->setPosition(pos);
      }
      break;

      case KC_SUBTRACT:
      {
        if (mCameraZoom < MAX_CAMERA_ZOOM) mCameraZoom += 5;
        mCamera->setFOVy(Degree(mCameraZoom));
      }
      break;

      case KC_ADD:
      {
        if (mCameraZoom > MIN_CAMERA_ZOOM) mCameraZoom -= 5;
        mCamera->setFOVy(Degree(mCameraZoom));
      }
      break;

      /// ////////////////////////////////////////////////////////////////////
      /// Screenshot.
      /// ////////////////////////////////////////////////////////////////////
      case KC_SYSRQ:
      {
        static int mNumScreenShots=0;
        char tmp[20];
        sprintf(tmp, "screenshot_%d.png", ++mNumScreenShots);
        mWindow->writeContentsToFile(tmp);
        mTimeUntilNextToggle = 0.5;
      }
      break;

      /// ////////////////////////////////////////////////////////////////////
      /// Exit game.
      /// ////////////////////////////////////////////////////////////////////
      case KC_ESCAPE:
      mQuitGame = true;
      break;

      default:
      break;
  }
  // e->consume();
}

void CEvent::keyClicked(KeyEvent* )
{
}

void CEvent::keyReleased(KeyEvent* e)
{
  switch (e->getKey())
  {
      /// ////////////////////////////////////////////////////////////////////
      /// Player Movemment.
      /// ////////////////////////////////////////////////////////////////////
      case KC_UP:
      case KC_DOWN:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_WALK, 0);
      break;

      case KC_RIGHT:
      case KC_LEFT:
      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TURN, 0);
      break;

      case KC_J:
      case KC_K:
      ObjectManager::getSingleton().Event(OBJECT_NPC, OBJ_TURN,  0);
      break;

      case KC_G:
      ObjectManager::getSingleton().Event(OBJECT_NPC, OBJ_WALK,  0);
      break;

      default:
      break;
  }
}

///================================================================================================
/// Buffered Mouse Events.
///================================================================================================
void CEvent::mouseMoved (MouseEvent *e)
{
  mMouseX = e->getX();
  mMouseY = e->getY();
  if (mMouseX > 0.995) mMouseX = 0.995;
  if (mMouseY > 0.990) mMouseY = 0.990;
  GuiManager::getSingleton().mouseEvent(M_MOVED, mMouseX, mMouseY);

}

void CEvent::mousePressed (MouseEvent *e)
{
  mMouseX = e->getX();
  mMouseY = e->getY();

  int button = e->getButtonID();
  if (button & InputEvent::BUTTON0_MASK ) // LeftButton.
  {
    if (GuiManager::getSingleton().mouseEvent(M_PRESSED, mMouseX, mMouseY))
    { // Button was pressed in a gui_window.
    }
    else
    {
      //pgTileManager->get_TileInterface()->pick_Tile(mMouseX, mMouseY);
    }
  }
#ifdef WIN32
  else if (button & InputEvent::BUTTON1_MASK )
#else
  else if (button & InputEvent::BUTTON2_MASK )
#endif
  {
    /// activate mouse picking of tiles
    mTileManager->get_TileInterface()->pick_Tile(mMouseX, mMouseY);

    /// Move the player.
    Vector3 pos = mTileManager->get_TileInterface()->get_Selection();
    ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_GOTO, (int)pos.x, (int) pos.z);
  }
  e->consume();
}

void CEvent::mouseDragged(MouseEvent *e)
{
  mouseMoved(e);
  e->consume();
}

void CEvent::mouseClicked (MouseEvent *e)
{
  mouseMoved(e);
  e->consume();
}

void CEvent::mouseEntered (MouseEvent *e)
{
  mouseMoved(e);
  e->consume();
}

void CEvent::mouseExited  (MouseEvent *e)
{
  mouseMoved(e);
  e->consume();
}

void CEvent::mouseReleased(MouseEvent *e)
{
  GuiManager::getSingleton().mouseEvent(M_RELEASED, mMouseX, mMouseY);
  //  mouseMoved(e);
  e->consume();
}
