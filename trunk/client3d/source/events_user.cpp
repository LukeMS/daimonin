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
#include "particle_manager.h"
#include "option.h"

using namespace Ogre;

///================================================================================================
/// Buffered Key Events.
///================================================================================================
void CEvent::keyPressed(KeyEvent *e)
{
    mIdleTime =0;
    static Real g_pitch = 0.2;

    /// Is this keyEvent related to gui?
    if (GuiManager::getSingleton().keyEvent(e->getKeyChar(), e->getKey()))
    {
        e->consume();
        return;
    }

    /// InGame keyEvent.
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

        case KC_F2:
        GuiManager::getSingleton().startTextInput(GUI_WIN_PLAYERINFO, GUI_TEXTINPUT_PASSWORD, 20, true, true);
        break;


        case KC_A:
        {
         static int offset =1;
         ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_IDLE1 + offset);
         if (++offset>= 20) offset =1;
        }
        break;

        case KC_B:
        //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_BLOCK1);
        {
         static int offset =1;
         ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_ATTACK1 + offset);
         if (++offset> 5) offset =1;
        }

        break;

        case KC_C:
        //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_CAST1);
        break;

        case KC_S:
        //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_SLUMP1);
        break;

        case KC_D:
        //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_DEATH1);
        break;

        case KC_H:
        //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, Animate::STATE_HIT1);
        break;

        case KC_1:
        //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_WEAPON_HAND, 1);
        {
            static int color =0;
            ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_SKIN, color++);
        }
        break;

        case KC_2:
        //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_SHIELD_HAND, 1);
        {
            static int color =0;
            ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_FACE, color++);
        }
        break;

        case KC_3:
        //ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,0, -1);
        {
            static int color =0;
            ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_HAIR, color++);
        }
        break;

        case KC_4:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_BODY, color++);
        }
        break;

        case KC_5:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_LEGS, color++);
        }
        break;
        case KC_6:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_BELT, color++);
        }
        break;

        case KC_7:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_SHOES, color++);
        }
        break;

        case KC_8:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_TEXTURE, TEXTURE_POS_HANDS, color++);
        }
        //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_HEAD, 1);
        break;

        case KC_9:
        //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_BODY, 1);
        break;

        case KC_J:
        // ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TURN,  1);
        //mCamera->yaw(Degree(10));
        {
            static int tree = 0;
                Entity * entity = mSceneManager->createEntity("tree_"+StringConverter::toString(++tree), "tree1.mesh");
                const AxisAlignedBox &AABB = entity->getBoundingBox();
                Vector3 pos = mTileManager->get_TileInterface()->get_Selection();
                pos.x = (pos.x +0.5) * TILE_SIZE -(AABB.getMaximum() .x+ AABB.getMinimum().x)/2;
                pos.y+= fabs(AABB.getMaximum().y) - TILE_SIZE/2;
                pos.z = (pos.z +0.5) * TILE_SIZE -(AABB.getMaximum() .x+ AABB.getMinimum().x)/2;
                SceneNode *node = mSceneManager->getRootSceneNode()->createChildSceneNode();
                node->attachObject(entity);
                node->setPosition(pos.x, pos.y, pos.z);
        }
        break;

        case KC_K:
        break;

        case KC_G:
        mTileManager->ToggleGrid();
        break;

        case KC_I:
          ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_HEAD, 1);
        break;

        case KC_O:
          ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_SHIELD_HAND, 1);
        break;

        case KC_P:
          ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_WEAPON_HAND, 1);
        break;

        case KC_Q:
        ObjectManager::getSingleton().Event(OBJECT_NPC, OBJ_TEXTURE, 0, 1);
        break;

        /// ////////////////////////////////////////////////////////////////////
        /// Engine settings.
        /// ////////////////////////////////////////////////////////////////////
        case KC_Y:
        mSceneDetailIndex = (mSceneDetailIndex+1)%3;
        switch(mSceneDetailIndex)
        {
            case 0 :
            mCamera->setPolygonMode(PM_SOLID);
            break ;
            case 1 :
            mCamera->setPolygonMode(PM_WIREFRAME);
            break ;
            case 2 :
            mCamera->setPolygonMode(PM_POINTS);
            break ;
        }
        break;

        case KC_T:
        {
            static int pixels =128;
            //change pixel size of terrain textures
            pixels /= 2; // shrink pixel value
            if (pixels < MIN_TEXTURE_PIXEL)
                pixels = 128; // if value is too low resize to maximum
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
        //Option::getSingleton().setIntValue(Option::UPDATE_NETWORK, true);
        break;

        case KC_W:
        if (mDayTime)
        {
            mDayTime =0;
            // mLight[LIGHT_VOL ]->setVisible(false);
            // mLight[LIGHT_SPOT]->setVisible(true);
        }
        else
        {
            mDayTime =15;
            //mLight[LIGHT_VOL ]->setVisible(true);
            //mLight[LIGHT_SPOT]->setVisible(false);
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
            //if (mCameraZoom < MAX_CAMERA_ZOOM)
            mCameraZoom += 5;
            mCamera->setFOVy(Degree(mCameraZoom));
        }
        break;

        case KC_ADD:
        {
            if (mCameraZoom > MIN_CAMERA_ZOOM)
                mCameraZoom -= 5;
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
{}

void CEvent::keyReleased(KeyEvent* e)
{
    switch (e->getKey())
    {
        /// ////////////////////////////////////////////////////////////////////
        /// Player Movemment.
        /// ////////////////////////////////////////////////////////////////////
        case KC_UP:
        case KC_DOWN:
        /*
        //      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_WALK, 0);
        long time = clock();
        for (int i=0; i < 1000; ++i)
        {
            Event->getTileManager()->ChangeChunks();
        }
        Logger::log().info() << "Time to create Chunks: " << clock()-time << " ms";
        */
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
    if (mMouseX > 0.995)
        mMouseX = 0.995;
    if (mMouseY > 0.990)
        mMouseY = 0.990;
    GuiManager::getSingleton().mouseEvent(M_MOVED, mMouseX, mMouseY);

}

void CEvent::mousePressed (MouseEvent *e)
{
    mMouseX = e->getX();
    mMouseY = e->getY();

    int button = e->getButtonID();
    if (button & MouseEvent::BUTTON0_MASK ) // LeftButton.
    {
        if (GuiManager::getSingleton().mouseEvent(M_PRESSED, mMouseX, mMouseY))
        { // Button was pressed in a gui_window.
        }
        else
        {

            RaySceneQuery *mRaySceneQuery;
            mRaySceneQuery =mSceneManager->createRayQuery( Ray() );
            mRaySceneQuery->setRay(mCamera->getCameraToViewportRay(mMouseX, mMouseY));
            mRaySceneQuery->setQueryMask(QUERY_NPC_MASK);
            RaySceneQueryResult &result = mRaySceneQuery->execute();
            if (result.size())
            {
                RaySceneQueryResult::iterator itr = result.begin();
                String tt = itr->movable->getName();

                static bool once =true;
                static SceneNode *mNode =0;
                static Entity *mEntity;
                if (once ==true)
                {
                    mEntity = mSceneManager->createEntity("Selection", "selection.mesh");
                    mEntity->setQueryFlags(QUERY_NPC_SELECT_MASK);
                    once =false;
                }
                if (mNode) mNode->getParentSceneNode()->removeAndDestroyChild("SelNode");
                mNode = itr->movable->getParentSceneNode()->createChildSceneNode("SelNode");
                mNode->attachObject(mEntity);
                mNode->scale(.10,.10,.10);


                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)tt.c_str());
                mSceneManager->destroyQuery(mRaySceneQuery);
                const AxisAlignedBox &AABB = itr->movable->getBoundingBox();
                //            Math::Abs(AABB.getMinimum().y);
                //Vector3 pos = itr->movable->getParentNode()->getPosition();
                Vector3 pos = mNode->getPosition();
                pos.y= AABB.getMinimum().y +3;
                mNode->setPosition(pos);



            }
        }
    }

#ifdef WIN32
    else if (button & MouseEvent::BUTTON1_MASK )
#else

    else if (button & MouseEvent::BUTTON2_MASK )
#endif
    {
        if (!Option::getSingleton().getIntValue(Option::CMDLINE_FALLBACK))
        {
        /// activate mouse picking of tiles
        mTileManager->get_TileInterface()->pick_Tile(mMouseX, mMouseY);
        {
            Vector3 pos = mTileManager->get_TileInterface()->get_Selection();
            pos.x = (pos.x +0.5) * TILE_SIZE;
            pos.z = (pos.z +0.5) * TILE_SIZE;
            ParticleManager::getSingleton().addFreeObject(pos, "Particle/SelectionDust", 2.0);
        }


        /// Move the player.
        Vector3 pos = mTileManager->get_TileInterface()->get_Selection();
        ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_GOTO, (int)pos.x, (int) pos.z);
        }
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
