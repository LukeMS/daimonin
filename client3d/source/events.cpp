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

#include <OgreKeyEvent.h>
#include "events.h"
#include "sound.h"
#include "object_manager.h"
#include "option.h"
#include "logger.h"
#include "network.h"
#include "tile_manager.h"
#include "gui_manager.h"
#include "object_manager.h"
#include "object_visuals.h"
#include "particle_manager.h"
#include "spell_manager.h"
#include "network_serverfile.h"

using namespace Ogre;

//================================================================================================
// Global variables.
//================================================================================================
CEvent *Event=0;

//================================================================================================
// Constructor.
//================================================================================================
CEvent::CEvent(RenderWindow* win, SceneManager *SceneMgr)
{
    // ////////////////////////////////////////////////////////////////////
    // Create unbuffered key & mouse input.
    // ////////////////////////////////////////////////////////////////////
    mSceneManager = SceneMgr;
    mWindow = win;
    mEventProcessor = new EventProcessor();
    mEventProcessor->initialise(win);
    mEventProcessor->startProcessingEvents();
    mInputDevice =  mEventProcessor->getInputReader();
    mTimeUntilNextToggle = 0;
    mIdleTime =0;
    mDayTime = 15;
    mCameraZoom = MAX_CAMERA_ZOOM;
    mMouseX = mMouseY =0;
    mQuitGame = false;
    Option::getSingleton().setGameStatus(GAME_STATUS_INIT_VIEWPORT);
}

//================================================================================================
// Destructor.
//================================================================================================
CEvent::~CEvent()
{
    if (mEventProcessor)  delete mEventProcessor;
    TileManager  ::getSingleton().freeRecources();
    ObjectManager::getSingleton().freeRecources();
    GuiManager   ::getSingleton().freeRecources();
    Sound        ::getSingleton().freeRecources();
    ObjectVisuals::getSingleton().freeRecources();
}

//================================================================================================
// Player has moved over a tile border. Update the world positions.
//================================================================================================
void CEvent::setWorldPos(int deltaX, int deltaZ)
{
    TileManager::getSingleton().scrollMap(deltaX, deltaZ); // server has to do this!
    Vector3 deltaPos = ObjectManager::getSingleton().synchToWorldPos(deltaX, deltaZ);
    ParticleManager::getSingleton().synchToWorldPos(deltaPos);
}

//================================================================================================
// Frame Start event.
//================================================================================================
bool CEvent::frameStarted(const FrameEvent& evt)
{
    static Overlay *mOverlay;
    if (mWindow->isClosed() || mQuitGame)
        return false;

    switch (Option::getSingleton().getGameStatus())
    {
        case GAME_STATUS_INIT_VIEWPORT:
        {
            // ////////////////////////////////////////////////////////////////////
            // Create one viewport, entire window
            // ////////////////////////////////////////////////////////////////////
            mCamera = mSceneManager->createCamera("PlayerCam");
            Viewport *VP = mWindow->addViewport(mCamera);
            // Alter the camera aspect ratio to match the viewport
            mCamera->setAspectRatio(Real(VP->getActualWidth()) / Real(VP->getActualHeight()));
            mCamera->setProjectionType(PT_ORTHOGRAPHIC);
            mCamera->setFOVy(Degree(MAX_CAMERA_ZOOM));
            mCamera->setPosition(Vector3(0, 450, 900));
            mCamera->pitch(Degree(-25));
            // ////////////////////////////////////////////////////////////////////
            // Create the world.
            // ////////////////////////////////////////////////////////////////////
            mWorld = mSceneManager->getRootSceneNode()->createChildSceneNode();
            // ////////////////////////////////////////////////////////////////////
            // Create a minimal gui for some loading infos..
            // ////////////////////////////////////////////////////////////////////
            GuiManager::getSingleton().Init(mWindow->getWidth(), mWindow->getHeight());
            GuiTextout::getSingleton().loadRawFont(FILE_SYSTEM_FONT);
            // Set next state.
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_SOUND);
            // Show the loading-gfx.
            mOverlay = OverlayManager::getSingleton().getByName ("Overlay/Loading");
            mOverlay->show();
            if (Root::getSingleton().getTimer()->getMilliseconds() & 1)
                mOverlay->getChild("OverlayElement/Screen1")->hide();
            else
                mOverlay->getChild("OverlayElement/Screen2")->hide();
            GuiManager::getSingleton().displaySystemMessage("* Welcome to Daimonin *");
            GuiManager::getSingleton().displaySystemMessage("Starting the sound-system...");
        }
        break;

        case GAME_STATUS_INIT_SOUND:
        {
            Sound::getSingleton().Init();
            //Sound::getSingleton().playStream(Sound::BG_MUSIC); // This sound is getting on my nerves...
            // Set next state.
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_LIGHT);
            GuiManager::getSingleton().displaySystemMessage("Starting lights...");
        }
        break;

        case GAME_STATUS_INIT_LIGHT:
        {
            /*
            // Todo: lightmanager
            Light *light;
            light = mSceneManager->createLight("Light_Vol");
            light->setType(Light::LT_POINT );
            light->setPosition(-100, 200, 800);
            //    light->setDiffuseColour(1.0, 1.0, 1.0);
            light->setSpecularColour(1.0, 1.0, 1.0);
            mWorld->attachObject(light);
            setLightMember(light, 0);

            light = mSceneManager->createLight("Light_Spot");
            light->setType(Light::LT_SPOTLIGHT);
            light->setDirection(0, -1, -1);
            light->setPosition (-125, 200, 100);
            light->setDiffuseColour(1.0, 1.0, 1.0);
            // light->setSpotlightRange(Radian(.2) , Radian(.6), 5.5);
            // light->setAttenuation(1000,1,0.005,0);

            mWorld->attachObject(light);
            setLightMember(light, 1);
            light->setVisible(false);
            mSceneManager->setAmbientLight(ColourValue(1.0, 1.0, 1.0));

            // mSceneMgr->setFog(FOG_LINEAR , ColourValue(.7,.7,.7), 0.005, 450, 800);
            // mSceneMgr->setFog(FOG_LINEAR , ColourValue(1,1,1), 0.005, 450, 800);

            */
            // Set next state.
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_SPELL);
            GuiManager::getSingleton().displaySystemMessage("Starting the spells...");
        }
        break;

        case GAME_STATUS_INIT_SPELL:
        {
            SpellManager::getSingleton().init(mSceneManager);
            // Set next state.
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_PARTICLE);
            GuiManager::getSingleton().displaySystemMessage("Starting the particles...");
            break;
        }


        case GAME_STATUS_INIT_PARTICLE:
        {
            ParticleManager::getSingleton().update(0);
            // Set next state.
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_GUI_IMAGESET);
            GuiManager::getSingleton().displaySystemMessage("Starting the gui...");
            GuiManager::getSingleton().displaySystemMessage(" - Parsing Imageset");
            break;
        }


        case GAME_STATUS_INIT_GUI_IMAGESET:
        {
            GuiImageset::getSingleton().parseXML(FILE_GUI_IMAGESET);
            // Set next state.
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_GUI_WINDOWS);
            GuiManager::getSingleton().displaySystemMessage(" - Parsing windows");
            break;
        }

        case GAME_STATUS_INIT_GUI_WINDOWS:
        {
            Logger::log().headline("Starting GUI");
            GuiManager::getSingleton().parseWindows(FILE_GUI_WINDOWS);
            // Set next state.
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_TILE);
            GuiManager::getSingleton().displaySystemMessage("Starting the tile-engine...");
            break;
        }

        case GAME_STATUS_INIT_TILE:
        {
            // As events are handled in the gui.
            // The Listeners must be added after gui was init.
            mEventProcessor->addKeyListener(this);
            mEventProcessor->addMouseMotionListener(this);
            mEventProcessor->addMouseListener(this);
            if (Option::getSingleton().getIntValue(Option::HIGH_TEXTURE_DETAILS))
                TileManager::getSingleton().Init(mSceneManager, 128);
            else
                TileManager::getSingleton().Init(mSceneManager, 16);
            // Set next state.
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_OBJECT);
            GuiManager::getSingleton().displaySystemMessage("Starting the objects...");
            //Vector3 pos = Vector3::ZERO;
            //setWorldPos(pos);
        }
        break;

        case GAME_STATUS_INIT_OBJECT:
        {
            ObjectManager::getSingleton().init();
            // Set next state.
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_NET);
            ObjectVisuals::getSingleton().Init();
            GuiManager::getSingleton().displaySystemMessage("");
            OverlayManager::getSingleton().destroy(mOverlay);
            GuiManager::getSingleton().showWindow(GUI_WIN_STATISTICS, true);
            GuiManager::getSingleton().showWindow(GUI_WIN_PLAYERINFO, true);
            GuiManager::getSingleton().showWindow(GUI_WIN_PLAYERCONSOLE, true);
            GuiManager::getSingleton().showWindow(GUI_WIN_TEXTWINDOW, true);
            //GuiManager::getSingleton().showWindow(GUI_WIN_LOGIN, true);

            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Welcome to ~Daimonin 3D~.");
            /*
                        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"");
                        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Press ~right~ MB on ground to move.");
                        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Press ~1 ... 8~ to change cloth.");
                        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Press ~t~ for texture quality. ");
                        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Press ~a~ to change Idle animation.");
                        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Press ~b~ to change Attack animation.");
                        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Press ~i, o, p~ to change utils.");
                        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Press ~LMB~ for selection.");
            */
            mWindow->resetStatistics();
            Option::getSingleton().setGameStatus(GAME_STATUS_PLAY);
            break;
        }

        case GAME_STATUS_INIT_NET:
        {
            if (Option::getSingleton().getIntValue(Option::UPDATE_NETWORK))
            {
                Network::getSingleton().Init();
                Option::getSingleton().setGameStatus(GAME_STATUS_META);
                break;
            }
            //clear_metaserver_data();
            Option::getSingleton().setGameStatus(GAME_STATUS_PLAY);
            break;
        }

        case GAME_STATUS_META:
        {
            Network::getSingleton().contactMetaserver();
            Option::getSingleton().setGameStatus(GAME_STATUS_START);
            break;
        }

        case GAME_STATUS_START:
        {
            Network::getSingleton().SOCKET_CloseClientSocket();
            Option::getSingleton().setGameStatus(GAME_STATUS_WAITLOOP);
            break;
        }

        case GAME_STATUS_WAITLOOP:
        {
            // Wait for user to select a server.

            // User has select a server.
            Option::getSingleton().setGameStatus(GAME_STATUS_STARTCONNECT);
            break;
        }

        case GAME_STATUS_STARTCONNECT:
        {
            Option::getSingleton().setGameStatus(GAME_STATUS_CONNECT);
            break;
        }

        case GAME_STATUS_CONNECT:
        {
            Network::GameStatusVersionFlag = false;
            if (!Network::getSingleton().SOCKET_OpenClientSocket("127.0.0.1", 13327))
            {
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)"connection failed!");
                Option::getSingleton().setGameStatus(GAME_STATUS_START);
            }
            Option::getSingleton().setGameStatus(GAME_STATUS_VERSION);
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)"connected. exchange version.");
            break;
        }

        case GAME_STATUS_VERSION:
        {
            Network::getSingleton().SendVersion();
            Option::getSingleton().setGameStatus(GAME_STATUS_WAITVERSION);
            break;
        }

        case GAME_STATUS_WAITVERSION:
        {
            // perhaps here should be a timer ???
            // remember, the version exchange server<->client is asynchron
            // so perhaps the server send his version faster
            // as the client send it to server
            if (Network::GameStatusVersionFlag) // wait for version answer when needed
            {
                if (!Network::GameStatusVersionOKFlag)
                {
                    Option::getSingleton().setGameStatus(GAME_STATUS_START);
                }
                else
                {
                    GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)"version confirmed.");
                    Option::getSingleton().setGameStatus(GAME_STATUS_SETUP);
                }
            }
            break;
        }

        case GAME_STATUS_SETUP:
        {
            static char buf[1024];
            ServerFile::getSingleton().checkFiles();
            sprintf(buf, "setup sound %d map2cmd 1 mapsize %dx%d darkness 1 facecache 1"
                    " skf %d|%x spf %d|%x bpf %d|%x stf %d|%x amf %d|%x",
                    //   SoundStatus, TileMap::getSingleton().MapStatusX, TileMap::getSingleton().MapStatusY,
                    1, 17,17,
                    ServerFile::getSingleton().getLength(SERVER_FILE_SKILLS),
                    ServerFile::getSingleton().getCRC   (SERVER_FILE_SKILLS),
                    ServerFile::getSingleton().getLength(SERVER_FILE_SPELLS),
                    ServerFile::getSingleton().getCRC   (SERVER_FILE_SPELLS),
                    ServerFile::getSingleton().getLength(SERVER_FILE_BMAPS),
                    ServerFile::getSingleton().getCRC   (SERVER_FILE_BMAPS),
                    ServerFile::getSingleton().getLength(SERVER_FILE_SETTINGS),
                    ServerFile::getSingleton().getCRC   (SERVER_FILE_SETTINGS),
                    ServerFile::getSingleton().getLength(SERVER_FILE_ANIMS),
                    ServerFile::getSingleton().getCRC   (SERVER_FILE_ANIMS));
            Network::getSingleton().cs_write_string(buf, (int)strlen(buf));
            buf[strlen(buf)] =0;
            Logger::log().info() << "Send: setup " << buf;
            //mRequest_file_chain = 0;
            //mRequest_file_flags = 0;
            Option::getSingleton().setGameStatus(GAME_STATUS_WAITSETUP);
            break;
        }

        default:
        {
            static bool once = false;
            ObjectManager::getSingleton().update(ObjectManager::OBJECT_NPC, evt);
            ParticleManager::getSingleton().moveNodeObject(evt);
            mIdleTime += evt.timeSinceLastFrame;
            if (mIdleTime > 1.0)
            {
                mIdleTime = 0;
                if (!once)
                {
                    Sound::getSingleton().playStream(Sound::GREETS_VISITOR);
                    sObject obj;
                    obj.meshName  = "Tentacle_N_Small.mesh";
                    //obj.meshName  = "Ogre_Big.mesh";
                    obj.nickName  = "michtoen";
                    obj.type      = ObjectManager::OBJECT_NPC;
                    obj.boundingRadius = 2;
                    obj.friendly  = -1;
                    obj.attack    = 50;
                    obj.defend    = 50;
                    obj.maxHP     = 50;
                    obj.maxMana   = 50;
                    obj.maxGrace  = 50;
                    obj.pos.x     = 7;
                    obj.pos.z     = 12;
                    obj.pos.subX  = 2;
                    obj.pos.subZ  = 5;
                    obj.level     = 0;
                    obj.facing    = -90;
                    obj.particleNr=-1;
                    ObjectManager::getSingleton().addMobileObject(obj);
                    /*
                                        obj.meshName  = "Ogre_Big.mesh";
                                        obj.nickName  = "son of michtoen";
                                        obj.type      = ObjectManager::OBJECT_NPC;
                                        obj.friendly  = -1;
                                        obj.attack    = 50;
                                        obj.defend    = 50;
                                        obj.maxHP     = 50;
                                        obj.maxMana   = 50;
                                        obj.maxGrace  = 50;
                                        obj.pos.x     = 9;
                                        obj.pos.z     = 12;
                                        obj.pos.subX  = 4;
                                        obj.pos.subZ  = 4;
                                        obj.level     = 0;
                                        obj.facing    = 0;
                                        obj.particleNr=-1;
                                        ObjectManager::getSingleton().addMobileObject(obj);
                    */
                    once = true;
                }
                else
                {
                    //  Sound::getSingleton().playStream(Sound::PLAYER_IDLE);
                }
            }

            GuiManager::getSingleton().update(evt.timeSinceLastFrame);
            static unsigned long time = Root::getSingleton().getTimer()->getMilliseconds();
            if (Root::getSingleton().getTimer()->getMilliseconds() - time > 80.0)
            {
                //TileManager::getSingleton().ChangeChunks();
                time = Root::getSingleton().getTimer()->getMilliseconds();
            }


            /*
                    static Real time = evt.timeSinceLastFrame+1.0;
                    if (evt.timeSinceLastFrame > time)
                    {
                      TileManager::getSingleton().ChangeChunks();
                       time = evt.timeSinceLastFrame+1.0;
                    }
            */

            ParticleManager::getSingleton().update(evt.timeSinceLastFrame);
            break;
        }
    }
    return true;
}

//================================================================================================
// Frame End event.
//================================================================================================
bool CEvent::frameEnded(const FrameEvent&)
{
    if (Option::getSingleton().getGameStatus() <= GAME_STATUS_INIT_NET) return true;
    Network::getSingleton().update();
    const RenderTarget::FrameStats& stats = mWindow->getStatistics();
    static int skipFrames = 0;
    if (--skipFrames <= 0)
    {
        static char buffer[16];
        skipFrames = 10;
        sprintf(buffer, "%.1f", stats.lastFPS);
        GuiManager::getSingleton().sendMessage(GUI_WIN_STATISTICS, GUI_MSG_TXT_CHANGED, GUI_TEXTVALUE_STAT_CUR_FPS  , (void*)buffer);
        sprintf(buffer, "%.1f", stats.bestFPS);
        GuiManager::getSingleton().sendMessage(GUI_WIN_STATISTICS, GUI_MSG_TXT_CHANGED, GUI_TEXTVALUE_STAT_BEST_FPS , (void*)buffer);
        sprintf(buffer, "%.1f", stats.worstFPS);
        GuiManager::getSingleton().sendMessage(GUI_WIN_STATISTICS, GUI_MSG_TXT_CHANGED, GUI_TEXTVALUE_STAT_WORST_FPS, (void*)buffer);
        sprintf(buffer, "%d", stats.triangleCount);
        GuiManager::getSingleton().sendMessage(GUI_WIN_STATISTICS, GUI_MSG_TXT_CHANGED, GUI_TEXTVALUE_STAT_SUM_TRIS , (void*)buffer);
    }
    return true;
}

