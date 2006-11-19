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
#include "option.h"
#include "logger.h"
#include "network.h"
#include "tile_manager.h"
#include "tile_map.h"
#include "gui_manager.h"
#include "gui_textinput.h"
#include "object_manager.h"
#include "object_visuals.h"
#include "particle_manager.h"
#include "spell_manager.h"
#include "network_serverfile.h"

using namespace Ogre;

const unsigned int MIN_LEN_LOGIN_NAME =  2;
const unsigned int MAX_LEN_LOGIN_NAME = 12;
const unsigned int MIN_LEN_LOGIN_PSWD =  6;
const unsigned int MAX_LEN_LOGIN_PSWD = 17;

#define AUTO_FILL_PASWD // Delete me!!!

// cmds for fire/move/run - used from move_keys()
/*
static char    *directions[10] =
{
    "null", "/sw", "/s", "/se", "/w", "/stay", "/e", "/nw", "/n", "/ne"
};

static char    *directions_name[10] =
{
    "null", "southwest", "south", "southeast", "west", "stay", "east", "northwest", "north", "northeast"
};
static char    *directionsrun[10] =
{
    "/run 0", "/run 6", "/run 5", "/run 4", "/run 7",\
    "/run 5", "/run 3", "/run 8", "/run 1", "/run 2"
};
static char    *directionsfire[10] =
{
    "fire 0", "fire 6", "fire 5", "fire 4", "fire 7",\
    "fire 0", "fire 3", "fire 8", "fire 1", "fire 2"
};
*/


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
    if (mEventProcessor)
        delete mEventProcessor;
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
    //ParticleManager::getSingleton().pauseAll(true);
    //TileManager::getSingleton().scrollMap(deltaX, deltaZ);
    if (deltaX >0 && deltaZ >0) Network::getSingleton().send_command("/sw", -1, SC_FIRERUN);
    else if (deltaX <0 && deltaZ >0) Network::getSingleton().send_command("/se", -1, SC_FIRERUN);
    else if (deltaX >0 && deltaZ <0) Network::getSingleton().send_command("/nw", -1, SC_FIRERUN);
    else if (deltaX <0 && deltaZ <0) Network::getSingleton().send_command("/ne", -1, SC_FIRERUN);
    else if (deltaX==0 && deltaZ <0) Network::getSingleton().send_command("/n" , -1, SC_FIRERUN);
    else if (deltaX==0 && deltaZ >0) Network::getSingleton().send_command("/s" , -1, SC_FIRERUN);
    else if (deltaX >0 && deltaZ==0) Network::getSingleton().send_command("/w" , -1, SC_FIRERUN);
    else if (deltaX <0 && deltaZ==0) Network::getSingleton().send_command("/e" , -1, SC_FIRERUN);

    //TileManager::getSingleton().changeChunks();
    //Vector3 deltaPos = ObjectManager::getSingleton().synchToWorldPos(deltaX, deltaZ);
    //ParticleManager::getSingleton().synchToWorldPos(deltaPos);
    //ParticleManager::getSingleton().pauseAll(false);
}

//================================================================================================
// Frame Start event.
//================================================================================================
bool CEvent::frameStarted(const FrameEvent& evt)
{
    static Overlay *mOverlay;
    static String strPlayerName, strPlayerPswd;
    if (mWindow->isClosed() || mQuitGame)
        return false;

    switch (Option::getSingleton().getGameStatus())
    {
        case GAME_STATUS_INIT_VIEWPORT:
        {
            // ////////////////////////////////////////////////////////////////////
            // Create one viewport, entire window.
            // ////////////////////////////////////////////////////////////////////
            mCamera = mSceneManager->createCamera("PlayerCam");
            Viewport *VP = mWindow->addViewport(mCamera);
            mCamera->setAspectRatio(Real(VP->getActualWidth()) / Real(VP->getActualHeight()));
            mCamera->setProjectionType(PT_ORTHOGRAPHIC);
            mCamera->setFOVy(Degree(MAX_CAMERA_ZOOM));
            mCamera->setPosition(Vector3(0, 0, 0));
            mCamera->pitch(Degree(-25));
            mWorld = mSceneManager->getRootSceneNode()->createChildSceneNode();
            // ////////////////////////////////////////////////////////////////////
            // Create a minimal gui for some loading infos..
            // ////////////////////////////////////////////////////////////////////
            GuiManager::getSingleton().Init(mWindow->getWidth(), mWindow->getHeight());
            GuiTextout::getSingleton().loadRawFont(FILE_SYSTEM_FONT);
            // Show the loading-gfx.
            mOverlay = OverlayManager::getSingleton().getByName ("Overlay/Loading");
            mOverlay->show();
            if (Root::getSingleton().getTimer()->getMilliseconds() & 1)
                mOverlay->getChild("OverlayElement/Screen1")->hide();
            else
                mOverlay->getChild("OverlayElement/Screen2")->hide();
            GuiManager::getSingleton().displaySystemMessage("* Welcome to Daimonin *");
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_SOUND);
            break;
        }

        case GAME_STATUS_INIT_SOUND:
        {
            // ////////////////////////////////////////////////////////////////////
            // Init the sound and play the background music.
            // ////////////////////////////////////////////////////////////////////
            GuiManager::getSingleton().displaySystemMessage("Starting the sound-manager...");
            Sound::getSingleton().Init();
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_LIGHT);
            break;
        }

        case GAME_STATUS_INIT_LIGHT:
        {
            GuiManager::getSingleton().displaySystemMessage("Starting the light-manager...");
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
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_SPELL);
            break;
        }

        case GAME_STATUS_INIT_SPELL:
        {
            GuiManager::getSingleton().displaySystemMessage("Starting the spell-manager...");
            SpellManager::getSingleton().init(mSceneManager);
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_PARTICLE);
            break;
        }

        case GAME_STATUS_INIT_PARTICLE:
        {
            GuiManager::getSingleton().displaySystemMessage("Starting the particle-manager...");
            ParticleManager::getSingleton().update(0);
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_GUI_IMAGESET);
            break;
        }

        case GAME_STATUS_INIT_GUI_IMAGESET:
        {
            GuiManager::getSingleton().displaySystemMessage("Starting the gui-manager...");
            GuiManager::getSingleton().displaySystemMessage(" - Parsing Imageset.");
            GuiImageset::getSingleton().parseXML(FILE_GUI_IMAGESET);
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_GUI_WINDOWS);

            break;
        }

        case GAME_STATUS_INIT_GUI_WINDOWS:
        {
            GuiManager::getSingleton().displaySystemMessage(" - Parsing windows.");
            GuiManager::getSingleton().parseWindows(FILE_GUI_WINDOWS);
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_EVENT_LISTENER);
            break;
        }

        case GAME_STATUS_INIT_EVENT_LISTENER:
        {
            GuiManager::getSingleton().displaySystemMessage("Starting the event-listener...");
            // Because events are handled in the gui.
            // The Listeners must be added after gui was init.
            mEventProcessor->addKeyListener(this);
            mEventProcessor->addMouseMotionListener(this);
            mEventProcessor->addMouseListener(this);
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_TILE);
        }

        case GAME_STATUS_INIT_TILE:
        {
            GuiManager::getSingleton().displaySystemMessage("Starting the tile-engine...");
            if (Option::getSingleton().getIntValue(Option::HIGH_TEXTURE_DETAILS))
                TileManager::getSingleton().Init(mSceneManager, 128);
            else
                TileManager::getSingleton().Init(mSceneManager, 16);
            Option::getSingleton().setGameStatus(GAME_STATUS_INIT_NET);
            break;
        }

        case GAME_STATUS_INIT_NET:
        {
            GuiManager::getSingleton().displaySystemMessage("Starting the network...");
            GuiManager::getSingleton().displaySystemMessage("");
            Network::getSingleton().Init();
            // Close the loading screen.
            OverlayManager::getSingleton().destroy(mOverlay);
            mWindow->resetStatistics();
            GuiManager::getSingleton().showWindow(GUI_WIN_TEXTWINDOW, true);
            GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Welcome to ~Daimonin 3D~.");
            GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "");
            GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "You need a running server to start the game. Asking metaserver for active server now.");
            Option::getSingleton().setGameStatus(GAME_STATUS_META);
            break;
        }

        case GAME_STATUS_META:
        {
            GuiManager::getSingleton().clearTable(GUI_WIN_SERVERSELECT, GUI_TABLE);
            Network::getSingleton().clearMetaServerData();
            Network::getSingleton().contactMetaserver();
            Option::getSingleton().setGameStatus(GAME_STATUS_START);
            break;
        }

        case GAME_STATUS_START:
        {
            GuiManager::getSingleton().resetTextInput();
            GuiManager::getSingleton().showWindow(GUI_WIN_LOGIN, false);
            GuiManager::getSingleton().showWindow(GUI_WIN_SERVERSELECT, true);
            Option::getSingleton().setGameStatus(GAME_STATUS_STARTCONNECT);
            break;
        }

        case GAME_STATUS_STARTCONNECT:
        {
            if (GuiManager::getSingleton().getTableUserBreak(GUI_WIN_SERVERSELECT, GUI_TABLE))
            {
                GuiManager::getSingleton().showWindow(GUI_WIN_SERVERSELECT, false);
                Option::getSingleton().setGameStatus(GAME_STATUS_PLAY);
            }
            // Print server infos.
            int select;
            select = GuiManager::getSingleton().getTableSelection(GUI_WIN_SERVERSELECT, GUI_TABLE);
            if (select >=0)
            {
                for (int i =0; i< 4;++i)
                {
                    GuiManager::getSingleton().sendMessage(GUI_WIN_SERVERSELECT, GUI_MSG_TXT_CHANGED,
                                                           GUI_TEXTBOX_SERVER_INFO1 + i, (void*)Network::getSingleton().get_metaserver_info(select, i));
                }
            }
            // A server was selected.
            select = GuiManager::getSingleton().getTableActivated(GUI_WIN_SERVERSELECT, GUI_TABLE);
            if (select >=0)
            {
                GuiManager::getSingleton().showWindow(GUI_WIN_SERVERSELECT, false);
                Network::getSingleton().setActiveServer(select);
                Option::getSingleton().setGameStatus(GAME_STATUS_CONNECT);
            }
            break;
        }

        case GAME_STATUS_CONNECT:
        {
            //GuiManager::getSingleton().showWindow(GUI_WIN_SERVERSELECT, false);
            Network::GameStatusVersionFlag = false;
            if (!Network::getSingleton().OpenActiveServerSocket())
            {
                GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "connection failed!");
                Option::getSingleton().setGameStatus(GAME_STATUS_START);
                break;
            }
            Network::getSingleton().socket_thread_start();
            Option::getSingleton().setGameStatus(GAME_STATUS_VERSION);
            GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "connected. exchange version.");
            break;
        }

        case GAME_STATUS_VERSION:
        {
            Logger::log().info() << "Send version.";
            Network::getSingleton().SendVersion();
            Option::getSingleton().setGameStatus(GAME_STATUS_WAITVERSION);
            break;
        }

        case GAME_STATUS_WAITVERSION:
        {
            // perhaps here should be a timer? Remember, the version exchange server<->client is asynchron
            // so perhaps the server send his version faster as the client send it to server.
            if (Network::GameStatusVersionFlag) // wait for version answer when needed
            {
                if (!Network::GameStatusVersionOKFlag)
                {
                    Option::getSingleton().setGameStatus(GAME_STATUS_START);
                }
                else
                {
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "version confirmed.");
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
                    1, //   SoundStatus
                    CHUNK_SIZE_X, CHUNK_SIZE_Z,
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
            // Now we wait for user to select login or create character.
            //
            Option::getSingleton().setGameStatus(GAME_STATUS_ADDME); // only for testing....
            break;
        }

        case GAME_STATUS_ADDME:
        {
            Network::getSingleton().cs_write_string("addme", 5);
            // now wait for login request of the server.
            Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN);
            break;
        }

        case GAME_STATUS_LOGIN:
        {
            if (GuiManager::getSingleton().brokenTextInput())
            {
                Logger::log().info() << "Break Login.";
                Option::getSingleton().setGameStatus(GAME_STATUS_START);
                GuiManager::getSingleton().showWindow(GUI_WIN_SERVERSELECT, false);
                GuiManager::getSingleton().showWindow(GUI_WIN_LOGIN, false);
                //GameStatusLogin = FALSE;
            }
            break;
        }

        case GAME_STATUS_NAME_INIT:
        {
            GuiManager::getSingleton().showWindow(GUI_WIN_LOGIN, true);
            GuiManager::getSingleton().startTextInput(GUI_WIN_LOGIN, GUI_TEXTINPUT_LOGIN_NAME, MAX_LEN_LOGIN_NAME, true, true);
            Option::getSingleton().setGameStatus(GAME_STATUS_NAME_USER);
            break;
        }

        case GAME_STATUS_NAME_USER:
        {
            if (GuiManager::getSingleton().brokenTextInput())
            {
                Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN);
                break;
            }
            // Wait for user to finish the textinput.
            if (!GuiManager::getSingleton().finishedTextInput()) break;
            // Check length of the username.
            strPlayerName = GuiManager::getSingleton().getTextInput();
            if (strPlayerName.size() < MIN_LEN_LOGIN_NAME || strPlayerName.size() > MAX_LEN_LOGIN_NAME)
            {
                String strMsg = "~#ffff0000Username length must be between " + StringConverter::toString(MIN_LEN_LOGIN_NAME)+
                                " and " + StringConverter::toString(MAX_LEN_LOGIN_NAME) + " chars!~";
                GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED, GUI_TEXTBOX_LOGIN_WARN, (void*)strMsg.c_str());
                GuiManager::getSingleton().startTextInput(GUI_WIN_LOGIN, GUI_TEXTINPUT_LOGIN_NAME, MAX_LEN_LOGIN_NAME, true, true);
                break;
            }
            // C -> Create new hero , L -> Login.
            String strServer = Option::getSingleton().getLoginType() == Option::LOGIN_NEW_PLAYER?"C":"L";
            strServer += strPlayerName;
            Network::getSingleton().send_reply((char*)strServer.c_str());
            GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED, GUI_TEXTBOX_LOGIN_WARN, (void*)"");
            // now wait again for next server question
            Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN_WAIT);
            break;
        }

        case GAME_STATUS_PSWD_INIT:
        {
            GuiManager::getSingleton().showWindow(GUI_WIN_LOGIN, true);
#ifndef AUTO_FILL_PASWD
            GuiManager::getSingleton().startTextInput(GUI_WIN_LOGIN, GUI_TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
#endif
            Option::getSingleton().setGameStatus(GAME_STATUS_PSWD_USER);
#ifdef AUTO_FILL_PASWD
            Network::getSingleton().send_reply("NIX_PASWD");
            Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN_WAIT);
#endif
            break;
        }

        case GAME_STATUS_PSWD_USER:
        {
            if (GuiManager::getSingleton().brokenTextInput())
            {
                Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN);
                break;
            }
            if (!GuiManager::getSingleton().finishedTextInput())
                break;
            strPlayerPswd = GuiManager::getSingleton().getTextInput();
            if (strPlayerPswd.size() < MIN_LEN_LOGIN_PSWD || strPlayerPswd.size() > MAX_LEN_LOGIN_PSWD)
            {
                String strMsg = "~#ffff0000Password length must be between " + StringConverter::toString(MIN_LEN_LOGIN_PSWD)+
                                " and "  + StringConverter::toString(MAX_LEN_LOGIN_PSWD) + " chars!~";
                GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED, GUI_TEXTBOX_LOGIN_WARN, (void*)strMsg.c_str());
                GuiManager::getSingleton().startTextInput(GUI_WIN_LOGIN, GUI_TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
                break;
            }
            if (strPlayerPswd == strPlayerName)
            {
                GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED, GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Password can't be same as character name!~");
                GuiManager::getSingleton().startTextInput(GUI_WIN_LOGIN, GUI_TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
                break;
            }
            Logger::log().info() << "Login: send password <*****>";
            Network::getSingleton().send_reply((char*)strPlayerPswd.c_str());
            Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN_WAIT);
            GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED, GUI_TEXTBOX_LOGIN_WARN, (void*)"");
            GuiManager::getSingleton().showWindow(GUI_WIN_LOGIN, false);
            break;
        }

        case GAME_STATUS_VRFY_INIT:
        {
            GuiManager::getSingleton().showWindow(GUI_WIN_LOGIN, true);
#ifndef AUTO_FILL_PASWD
            GuiManager::getSingleton().startTextInput(GUI_WIN_LOGIN, GUI_TEXTINPUT_LOGIN_VERIFY, MAX_LEN_LOGIN_PSWD, false, false);
#endif
            Option::getSingleton().setGameStatus(GAME_STATUS_VRFY_USER);
#ifdef AUTO_FILL_PASWD
            Network::getSingleton().send_reply("NIX_PASWD");
            Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN_WAIT);
#endif
            break;
        }

        case GAME_STATUS_VRFY_USER:
        {
            TileManager::getSingleton().map_transfer_flag = 0;
            if (GuiManager::getSingleton().brokenTextInput())
            {
                Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN);
                break;
            }
            if (!GuiManager::getSingleton().finishedTextInput())
                break;
            Logger::log().info() << "Login: send verify password <*****>";
            Network::getSingleton().send_reply((char*)GuiManager::getSingleton().getTextInput());
            Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN_WAIT);
            break;
        }

        case GAME_STATUS_NEW_CHAR:
        {
            // Send the new created character to server.
            Option::getSingleton().setLoginType(Option::LOGIN_NEW_PLAYER);
            /*
            char buf[MAX_BUF];
            sprintf(buf, "nc %s %d %d %d %d %d %d %d %d", nc->char_arch[nc->gender_selected], nc->stats[0], nc->stats[1],
            nc->stats[2], nc->stats[3], nc->stats[4], nc->stats[5], nc->stats[6], nc->skill_selected);
            */
            char buf[] = "nc human_male 14 14 13 12 12 12 12 0";
            Network::getSingleton().cs_write_string(buf, (int)strlen(buf));
            GuiManager::getSingleton().showWindow(GUI_WIN_LOGIN, false);
            Option::getSingleton().setGameStatus(GAME_STATUS_PLAY);
            break;
        }

        case GAME_STATUS_PLAY:
        {
            static bool once = false;
            ObjectManager::getSingleton().update(ObjectManager::OBJECT_NPC, evt);
            mIdleTime += evt.timeSinceLastFrame;
            if (mIdleTime > 1.0)
            {
                mIdleTime = 0;
                if (!once)
                {
                    ObjectManager::getSingleton().init();
                    ObjectVisuals::getSingleton().Init();
                    mCamera->setPosition(Vector3(0, 450, 900));
                    GuiManager::getSingleton().showWindow(GUI_WIN_STATISTICS, true);
                    GuiManager::getSingleton().showWindow(GUI_WIN_PLAYERINFO, true);

                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Client3d commands:");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Press ~1 ... 8~ to change cloth.");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Press ~X~ for texture quality. ");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Press ~A~ to change Idle animation.");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Press ~B~ to change Attack animation.");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Press ~C~ to change Agility animation.");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Press ~P~ to ready/unready primary weapon.");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Press ~S~ to ready/unready secondary weapon.");
                    GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "Press ~Q~ to start attack animation.");

                    // Can crash the client...
                    //ObjectManager::getSingleton().setNameNPC(ObjectNPC::HERO, strPlayerName.c_str());

                    //Sound::getSingleton().playStream(Sound::GREETS_VISITOR);
                    once = true;
                }
                else
                {
                    //  Sound::getSingleton().playStream(Sound::PLAYER_IDLE);
                }
                break;
            }

            static unsigned long time = Root::getSingleton().getTimer()->getMilliseconds();
            if (Root::getSingleton().getTimer()->getMilliseconds() - time > 180.0)
            {
                RaySceneQuery *mRaySceneQuery = mSceneManager->createRayQuery(Ray());
                mRaySceneQuery->setRay(mCamera->getCameraToViewportRay(mMouseX, mMouseY));
                mRaySceneQuery->setQueryMask(ObjectManager::QUERY_NPC_MASK | ObjectManager::QUERY_CONTAINER);
                RaySceneQueryResult &result = mRaySceneQuery->execute();
                if (!result.empty())
                {
                    // Mouse is over an object.
                    RaySceneQueryResult::iterator itr = result.begin();
                    ObjectManager::getSingleton().highlightObject(itr->movable);
                }
                else
                {
                    // Mouse is (no longer) over an object.
                    ObjectVisuals::getSingleton().highlightOff();
                }
                mSceneManager->destroyQuery(mRaySceneQuery);
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

            if (TileManager::getSingleton().map_udate_flag)
            {
                TileMap::getSingleton().map_draw_map();
            }

             break;
        }

        default:
        {
            break;
        }
    }
    return true;
}

//================================================================================================
// Frame End event.
//================================================================================================
bool CEvent::frameEnded(const FrameEvent& evt)
{
    if (Option::getSingleton().getGameStatus() <= GAME_STATUS_INIT_NET)
        return true;
    GuiManager::getSingleton().update(evt.timeSinceLastFrame);
    if (Option::getSingleton().getGameStatus() > GAME_STATUS_CONNECT)
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
