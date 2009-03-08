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

#include "events.h"
#include "sound.h"
#include "option.h"
#include "logger.h"
#include "network.h"
#include "tile_manager.h"
#include "tile_map.h"
#include "gui_manager.h"
#include "object_manager.h"
#include "object_hero.h"
#include "object_visuals.h"
#include "particle_manager.h"
#include "spell_manager.h"
#include "network_serverfile.h"

using namespace Ogre;

static const unsigned int MIN_LEN_LOGIN_NAME =  3;
static const unsigned int MAX_LEN_LOGIN_NAME = 12;
static const unsigned int MIN_LEN_LOGIN_PSWD =  6;
static const unsigned int MAX_LEN_LOGIN_PSWD = 17;
const Real CAMERA_OFFSET = TileManager::TILE_SIZE*20;
const Real CAMERA_RELA_Z = TileManager::TILE_SIZE*3;
const char *GUI_LOADING_OVERLAY = "GUI_LOADING_OVERLAY";
const char *GUI_LOADING_OVERLAY_ELEMENT = "GUI_LOADING_OVERLAY_ELEMENT";
const unsigned long SERVER_TIMEOUT = 5000; // Server timeout in ms.
const char BINARY_CMD_NEXT = '\0'; // Next commad.

//================================================================================================
// Constructor.
//================================================================================================
void Events::Init(RenderWindow* win, SceneManager *SceneMgr)
{
    // ////////////////////////////////////////////////////////////////////
    // Create unbuffered key & mouse input.
    // ////////////////////////////////////////////////////////////////////
    mSceneManager = SceneMgr;
    mWindow = win;
    size_t windowHnd =0;
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    std::ostringstream windowHndStr;
    windowHndStr << windowHnd;
    OIS::ParamList pl;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
    mInputManager = OIS::InputManager::createInputSystem(pl);
    mInputKeyboard= static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
    mInputKeyboard->setEventCallback(this);
    mInputMouse= static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
    mInputMouse->setEventCallback(this);
    unsigned int width, height, depth;
    int left, top;
    mWindow->getMetrics(width, height, depth, left, top);
    const OIS::MouseState &ms = mInputMouse->getMouseState();
    ms.width = width;
    ms.height = height;
    mIdleTime =0;
    mMouse = Vector3::ZERO;
    mQuitGame = false;
    Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_VIEWPORT);
}

//================================================================================================
// Destructor.
//================================================================================================
void Events::freeRecources()
{
    if (mInputManager)
    {
        mInputManager->destroyInputObject(mInputMouse);
        mInputManager->destroyInputObject(mInputKeyboard);
        OIS::InputManager::destroyInputSystem(mInputManager);
    }
    TileManager  ::getSingleton().freeRecources();
    ObjectManager::getSingleton().freeRecources();
    Network      ::getSingleton().freeRecources();
    GuiManager   ::getSingleton().freeRecources();
    Sound        ::getSingleton().freeRecources();
    ObjectVisuals::getSingleton().freeRecources();
}

//================================================================================================
// Player has moved over a tile border. Update the world positions.
//================================================================================================
void Events::setWorldPos(int deltaX, int deltaZ)
{
    //ParticleManager::getSingleton().pauseAll(true);
    //TileManager::getSingleton().scrollMap(deltaX, deltaZ);
    std::stringstream strCmd;
    if      (deltaX >0 && deltaZ >0) strCmd << (char) 1; // sw
    else if (deltaX <0 && deltaZ >0) strCmd << (char) 3; // se
    else if (deltaX >0 && deltaZ <0) strCmd << (char) 7; // nw
    else if (deltaX <0 && deltaZ <0) strCmd << (char) 9; // ne
    else if (deltaX==0 && deltaZ <0) strCmd << (char) 8; // n
    else if (deltaX==0 && deltaZ >0) strCmd << (char) 2; // s
    else if (deltaX >0 && deltaZ==0) strCmd << (char) 4; // w
    else if (deltaX <0 && deltaZ==0) strCmd << (char) 6; // e
    Network::getSingleton().send_command_binary(Network::CLIENT_CMD_MOVE, strCmd);
    //TileManager::getSingleton().changeChunks();
    //Vector3 deltaPos = ObjectManager::getSingleton().synchToWorldPos(deltaX, deltaZ);
    //ParticleManager::getSingleton().synchToWorldPos(deltaPos);
    //ParticleManager::getSingleton().pauseAll(false);
}

//================================================================================================
// Frame Start event.
//================================================================================================
bool Events::frameStarted(const FrameEvent& evt)
{
    static unsigned long timeWaitServer;
    static String strAccountName, strAccountPswd, strAccountPlayer;
    if (mWindow->isClosed() || mQuitGame)
        return false;

    switch (Option::getSingleton().getGameStatus())
    {
        case Option::GAME_STATUS_INIT_VIEWPORT:
        {
            // ////////////////////////////////////////////////////////////////////
            // Commandline argument to create Imposters.
            // ////////////////////////////////////////////////////////////////////
            const char *meshName = Option::getSingleton().getStrValue(Option::CMDLINE_CREATE_IMPOSTERS);
            if (strlen(meshName) > 1)
                if (!ObjectManager::getSingleton().createFlipBook(meshName, 16)) return false;
            // ////////////////////////////////////////////////////////////////////
            // Create one viewport, entire window.
            // ////////////////////////////////////////////////////////////////////
            mCameraZoom = STD_CAMERA_ZOOM;
            mCamera = mSceneManager->createCamera("PlayerCam");
            Viewport *VP = mWindow->addViewport(mCamera);
            mCamera->setAspectRatio(Real(VP->getActualWidth()) / Real(VP->getActualHeight()));
            mCamera->setFOVy(Degree(mCameraZoom));
            mCamera->setQueryFlags(ObjectManager::QUERY_CAMERA_MASK);
            mCamera->setPosition(0, CAMERA_OFFSET, CAMERA_OFFSET+CAMERA_RELA_Z);
            mCamera->pitch(Degree(-36));
            mWorld = mSceneManager->getRootSceneNode()->createChildSceneNode();
            // ////////////////////////////////////////////////////////////////////
            // Create a minimal gui for some loading infos..
            // ////////////////////////////////////////////////////////////////////
            GuiManager::getSingleton().Init(mWindow->getWidth(), mWindow->getHeight());
            GuiManager::getSingleton().loadRawFont(FILE_SYSTEM_FONT);
            // Show the loading-gfx.
            Overlay *overlay = OverlayManager::getSingleton().create(GUI_LOADING_OVERLAY);
            overlay->setZOrder(400);
            OverlayElement *element = OverlayManager::getSingleton().createOverlayElement("Panel", GUI_LOADING_OVERLAY_ELEMENT);
            element->setMetricsMode(GMM_RELATIVE);
            element->setPosition(0.0, 0.0);
            element->setDimensions(0.5, 1.0);
            if (Root::getSingleton().getTimer()->getMilliseconds() & 1)
                element->setMaterialName("GUI/LoadScreen1");
            else
                element->setMaterialName("GUI/LoadScreen2");
            overlay->add2D(static_cast<OverlayContainer*>(element));
            overlay->show();
            GuiManager::getSingleton().displaySystemMessage("* Welcome to Daimonin *");
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_SOUND);
            GuiManager::getSingleton().displaySystemMessage("Starting the sound-manager...");
            break;
        }

        case Option::GAME_STATUS_INIT_SOUND:
        {
            // ////////////////////////////////////////////////////////////////////
            // Init the sound and play the background music.
            // ////////////////////////////////////////////////////////////////////
            Sound::getSingleton().Init();
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_LIGHT);
            GuiManager::getSingleton().displaySystemMessage("Starting the light-manager...");
            break;
        }

        case Option::GAME_STATUS_INIT_LIGHT:
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
            */
            //mSceneManager->setFog(FOG_LINEAR , ColourValue(0.5,0.5,0.5), 0, 2300, 2700);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_SPELL);
            GuiManager::getSingleton().displaySystemMessage("Starting the spell-manager...");
            break;
        }

        case Option::GAME_STATUS_INIT_SPELL:
        {
            SpellManager::getSingleton().init(mSceneManager);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_PARTICLE);
            GuiManager::getSingleton().displaySystemMessage("Starting the particle-manager...");
            break;
        }

        case Option::GAME_STATUS_INIT_PARTICLE:
        {
            ParticleManager::getSingleton().update(0);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_GUI_IMAGESET);
            GuiManager::getSingleton().displaySystemMessage("Starting the gui-manager...");
            GuiManager::getSingleton().displaySystemMessage(" - Parsing Imageset.");
            break;
        }

        case Option::GAME_STATUS_INIT_GUI_IMAGESET:
        {
            GuiManager::getSingleton().displaySystemMessage(" - Parsing windows.");
            GuiManager::getSingleton().parseImageset(FILE_GUI_IMAGESET);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_GUI_WINDOWS);
            break;
        }

        case Option::GAME_STATUS_INIT_GUI_WINDOWS:
        {
            GuiManager::getSingleton().parseWindows(FILE_GUI_WINDOWS);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_TILE);
            GuiManager::getSingleton().displaySystemMessage("Starting the tile-engine...");
            break;
        }

        case Option::GAME_STATUS_INIT_TILE:
        {
            int lod = Option::getSingleton().getIntValue(Option::CMDLINE_TILEENGINE_LOD);
            bool createTextures = Option::getSingleton().getIntValue(Option::CMDLINE_CREATE_TILE_TEXTURES)>0?true:false;
            TileManager::getSingleton().Init(mSceneManager, ObjectManager::QUERY_TILES_LAND_MASK, ObjectManager::QUERY_TILES_WATER_MASK, lod, createTextures);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_GUI);
            GuiManager::getSingleton().displaySystemMessage("Starting the GUI...");
            break;
        }

        case Option::GAME_STATUS_INIT_GUI:
        {
            // Close the loading screen.
            GuiManager::getSingleton().displaySystemMessage("");
            OverlayManager::getSingleton().destroyOverlayElement(GUI_LOADING_OVERLAY_ELEMENT);
            OverlayManager::getSingleton().destroy(GUI_LOADING_OVERLAY);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_CHATWINDOW, true);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_TEXTWINDOW, true);
            GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Welcome to ~Daimonin 3D~.");
            GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "You need a running server to start the game!");
            /*
                        for (int i = 0; i < 120; ++i)
                        {
                            String txt = "Pos: "  + StringConverter::toString(i);
                          GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, txt.c_str());
                        }
            */
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
            break;
        }

        case Option::GAME_STATUS_INIT_NET:
        {
            Network::getSingleton().Init();
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_META);
            break;
        }

        case Option::GAME_STATUS_META:
        {
            Network::getSingleton().contactMetaserver();
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
            break;
        }

        case Option::GAME_STATUS_START:
        {
            GuiManager::getSingleton().resetTextInput();
            GuiManager::getSingleton().showWindow(GuiManager::WIN_LOGIN, false);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, true);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_STARTCONNECT);
            GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_SERVER_INFO1, GuiManager::MSG_SET_TEXT, "Select a server");
            break;
        }

        case Option::GAME_STATUS_STARTCONNECT:
        {
            if (GuiManager::getSingleton().sendMsg(GuiManager::GUI_TABLE, GuiManager::MSG_GET_USERBREAK))
            {
                GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, false);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_PLAY);
            }
            // A server was selected.
            int select = GuiManager::getSingleton().sendMsg(GuiManager::GUI_TABLE, GuiManager::MSG_GET_ACTIVATED);
            if (select >=0)
            {
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_SERVER_INFO2, GuiManager::MSG_SET_TEXT, " ");
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_SERVER_INFO3, GuiManager::MSG_SET_TEXT, " ");
                Network::getSingleton().setActiveServer(select);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_CONNECT);
            }
            break;
        }

        case Option::GAME_STATUS_CONNECT:
        {
            //GuiManager::getSingleton().showWindow(WIN_SERVERSELECT, false);
            if (!Network::getSingleton().OpenActiveServerSocket())
            {
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Connection failed!");
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_SERVER_INFO2, GuiManager::MSG_SET_TEXT, "~#ffff0000Connection failed!");
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
                break;
            }
            Network::getSingleton().socket_thread_start();
            GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Connected. Sending Setup Command.");
            GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_SERVER_INFO2, GuiManager::MSG_SET_TEXT, "~#ff00ff00Connected!");
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_SETUP);
            break;
        }

        case Option::GAME_STATUS_SETUP:
        {
            ServerFile::getSingleton().checkFiles();
            std::stringstream strCmd;
            strCmd  <<
            "cs "   << Network::VERSION_CS <<
            " sc "  << Network::VERSION_SC <<
            " sn "  << 1 <<
            " mz "  << 17 << "x" << 17 <<
            " skf " << ServerFile::getSingleton().getLength(ServerFile::FILE_SKILLS)  << "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_SKILLS)     << std::dec <<
            " spf " << ServerFile::getSingleton().getLength(ServerFile::FILE_SPELLS)  << "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_SPELLS)     << std::dec <<
            " bpf " << ServerFile::getSingleton().getLength(ServerFile::FILE_BMAPS)   << "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_BMAPS)      << std::dec <<
            " stf " << ServerFile::getSingleton().getLength(ServerFile::FILE_SETTINGS)<< "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_SETTINGS)   << std::dec <<
            " amf " << ServerFile::getSingleton().getLength(ServerFile::FILE_ANIMS)   << "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_ANIMS)      << std::dec;
            Network::getSingleton().send_command_binary(Network::CLIENT_CMD_SETUP, strCmd);
            timeWaitServer = Root::getSingleton().getTimer()->getMicroseconds();
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_WAITSETUP);
            break;
        }

        case Option::GAME_STATUS_WAITSETUP:
        {
            // Here we wait for the server to send SetupCmd().
            if ((Root::getSingleton().getTimer()->getMicroseconds() - timeWaitServer)/1000 > SERVER_TIMEOUT)
            {
                // Server doesn't understand our setup command. This is a bug!
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_SERVER_INFO3, GuiManager::MSG_SET_TEXT, "~#ffff0000Server timeout on setup command!");
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
            }
            break;
        }

        case Option::GAME_STATUS_REQUEST_FILES:
        {
            // Wait until all files are upToDate.
            if (ServerFile::getSingleton().requestFiles())
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN);
            break;
        }

        case Option::GAME_STATUS_LOGIN:
        {
            TileMap::getSingleton().clear_map();
            GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, false);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_LOGIN, true);
            GuiManager::getSingleton().startTextInput(GuiManager::WIN_LOGIN, GuiManager::GUI_TEXTINPUT_LOGIN_NAME, MAX_LEN_LOGIN_NAME, true, true);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_NAME);
            break;
        }

        // Login on an existing account.
        case Option::GAME_STATUS_LOGIN_NAME:
        {
            static bool errorMsgNeedsToBeCleared = false;
            if (GuiManager::getSingleton().brokenTextInput())
            {
                Logger::log().error() <<  "User break on login.";
                Network::getSingleton().CloseSocket();
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
                break;
            }
            if (GuiManager::getSingleton().getUserAction() && errorMsgNeedsToBeCleared)
            {
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_LOGIN_WARN, GuiManager::MSG_SET_TEXT, " ");
                errorMsgNeedsToBeCleared = false;
            }
            // Wait for user to finish the textinput.
            if (!GuiManager::getSingleton().finishedTextInput()) break;
            // Check length of the username.
            strAccountName = GuiManager::getSingleton().getTextInput();
            if (strAccountName.size() < MIN_LEN_LOGIN_NAME || strAccountName.size() > MAX_LEN_LOGIN_NAME)
            {
                String strMsg = "~#ffff0000Username length must be between " + StringConverter::toString(MIN_LEN_LOGIN_NAME)+
                                " and " + StringConverter::toString(MAX_LEN_LOGIN_NAME) + " chars!~";
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_LOGIN_WARN, GuiManager::MSG_SET_TEXT, strMsg.c_str());
                GuiManager::getSingleton().startTextInput(GuiManager::WIN_LOGIN, GuiManager::GUI_TEXTINPUT_LOGIN_NAME, MAX_LEN_LOGIN_NAME, true, true);
                errorMsgNeedsToBeCleared = true;
                break;
            }
            GuiManager::getSingleton().startTextInput(GuiManager::WIN_LOGIN, GuiManager::GUI_TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_PASWD);
            break;
        }

        case Option::GAME_STATUS_LOGIN_PASWD:
        {
            static bool errorMsgNeedsToBeCleared = false;
            if (GuiManager::getSingleton().brokenTextInput())
            {
                Logger::log().error() <<  "User break on login.";
                Network::getSingleton().CloseSocket();
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
                break;
            }
            if (GuiManager::getSingleton().getUserAction() && errorMsgNeedsToBeCleared)
            {
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_LOGIN_WARN, GuiManager::MSG_SET_TEXT, " ");
                errorMsgNeedsToBeCleared = false;
            }
            // Wait for user to finish the textinput.
            if (!GuiManager::getSingleton().finishedTextInput()) break;
            strAccountPswd = GuiManager::getSingleton().getTextInput();
            if (strAccountPswd.size() < MIN_LEN_LOGIN_PSWD || strAccountPswd.size() > MAX_LEN_LOGIN_PSWD)
            {
                String strMsg = "~#ffff0000Password length must be between " + StringConverter::toString(MIN_LEN_LOGIN_PSWD)+
                                " and " + StringConverter::toString(MAX_LEN_LOGIN_PSWD) + " chars!~";
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_LOGIN_WARN, GuiManager::MSG_SET_TEXT, strMsg.c_str());
                GuiManager::getSingleton().startTextInput(GuiManager::WIN_LOGIN, GuiManager::GUI_TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
                errorMsgNeedsToBeCleared = true;
                break;
            }
            // ////////////////////////////////////////////////////////////////////
            // Password length fit. Try to login on the current account..
            // ////////////////////////////////////////////////////////////////////
            std::stringstream strAccount;
            // BUG! Why is here a '\0' needed in front of the string?????
            strAccount<< BINARY_CMD_NEXT << strAccountName << BINARY_CMD_NEXT << strAccountPswd;
            Network::getSingleton().send_command_binary(Network::CLIENT_CMD_LOGIN, strAccount);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_NAME_WAIT);
            timeWaitServer = Root::getSingleton().getTimer()->getMicroseconds();
            break;
        }

        case Option::GAME_STATUS_LOGIN_NAME_WAIT:
        {
            if ((Root::getSingleton().getTimer()->getMicroseconds() - timeWaitServer)/1000 > SERVER_TIMEOUT)
            {
                // Server doesn't understand our login command. Can only happen while coding/testing.
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_LOGIN_WARN, GuiManager::MSG_SET_TEXT, "~#ffff0000Server timeout on account login!");
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
            }
            break;
        }

        case Option::GAME_STATUS_LOGIN_DONE:
        {
            GuiManager::getSingleton().sendMsg(GuiManager::GUI_TABLE, GuiManager::MSG_CLEAR);
            String strTabel;
            for (int i = 0; i < ObjectHero::getSingleton().getSumChars(); ++i)
            {
                strTabel = ObjectHero::getSingleton().getCharName(i) + ";";
                strTabel+= ObjectHero::getSingleton().getCharRace(i);
                strTabel+= " ";
                strTabel+= ObjectHero::getSingleton().getCharGender(i);
                strTabel+= ";";
                strTabel+= StringConverter::toString(ObjectHero::getSingleton().getCharLevel(i));
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TABLE, GuiManager::MSG_ADD_ROW, strTabel.c_str());
            }
            GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, true);
            GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_SERVER_INFO1, GuiManager::MSG_SET_TEXT, "Select a character");
            GuiManager::getSingleton().showWindow(GuiManager::WIN_LOGIN, false);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_CHOOSE_CHAR);
            break;
        }

        case Option::GAME_STATUS_LOGIN_CHOOSE_CHAR:
        {
            if (GuiManager::getSingleton().sendMsg(GuiManager::GUI_TABLE, GuiManager::MSG_GET_USERBREAK))
            {
                GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, false);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
            }
            // A character was selected.
            int select = GuiManager::getSingleton().sendMsg(GuiManager::GUI_TABLE, GuiManager::MSG_GET_ACTIVATED);
            if (select >=0)
            {
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_SERVER_INFO2, GuiManager::MSG_SET_TEXT, " ");
                GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_SERVER_INFO3, GuiManager::MSG_SET_TEXT, " ");
                ObjectHero::getSingleton().setSelected(select);
                GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, false);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_CHARACTER);
            }
            break;
        }

        case Option::GAME_STATUS_LOGIN_CHARACTER:
        {
            std::stringstream ssTemp;
            ssTemp << ObjectHero::getSingleton().getSelectedCharName();
            Network::getSingleton().send_command_binary(Network::CLIENT_CMD_ADDME, ssTemp);
//            GuiManager::getSingleton().addTextline(GuiManager::WIN_CHATWINDOW, GuiManager::GUI_LIST_MSGWIN, name.c_str());
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_CHARACTER_WAIT);
            break;
        }

        case Option::GAME_STATUS_LOGIN_CHARACTER_WAIT:
        {
            //Option::getSingleton().setGameStatus(Option::GAME_STATUS_PLAY);
            break;
        }

        case Option::GAME_STATUS_PLAY:
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
                    GuiManager::getSingleton().showWindow(GuiManager::WIN_STATISTICS, true);
                    GuiManager::getSingleton().showWindow(GuiManager::WIN_PLAYERINFO, true);
                    mWindow->resetStatistics();
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Client3d commands:");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~1 ... 8~ to change cloth.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Hold shift for a ranged attack.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~!keyg~ for grid. ");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~!keya~ to change Idle animation.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~!keyb~ to change Attack animation.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~!keyc~ to change Agility animation.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~!keyp~ to ready/unready primary weapon.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~!keys~ to ready/unready secondary weapon.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~!keyq~ to start attack animation.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~PGUP/PGDOWN~ to rotate camera.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~HOME~ to freeze camera rotation.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Press ~!keyi~ for Inventory.");
                    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Example of user defined chars: :( :) :D :P !key-spc");
                    // Can crash the client...
                    //ObjectManager::getSingleton().setNameNPC(ObjectNPC::HERO, strAccountName.c_str());
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
                mRaySceneQuery->setRay(mCamera->getCameraToViewportRay(mMouse.x / mWindow->getWidth(), mMouse.y / mWindow->getHeight()));
                mRaySceneQuery->setQueryMask(ObjectManager::QUERY_NPC_MASK | ObjectManager::QUERY_CONTAINER);
                RaySceneQueryResult &result = mRaySceneQuery->execute();
                if (!result.empty() && !GuiManager::getSingleton().mouseInsideGui())
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
            ParticleManager::getSingleton().update(evt.timeSinceLastFrame);
            TileMap::getSingleton().update();
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
bool Events::frameEnded(const FrameEvent& evt)
{
    if (Option::getSingleton().getGameStatus() < Option::GAME_STATUS_INIT_NET)
        return true;
    mInputMouse->capture();
    mInputKeyboard->capture();
    int element = GuiManager::getSingleton().update(evt.timeSinceLastFrame);
    if (element >=0) elementClicked(element);
    Network::getSingleton().update();
    // ////////////////////////////////////////////////////////////////////
    // Update camera movement.
    // ////////////////////////////////////////////////////////////////////
    if (mCameraRotating != NONE)
    {
        static Real cameraAngle= 0;
        const int CAMERA_TURN_DELAY = 50;
        Real step = CAMERA_TURN_DELAY * evt.timeSinceLastFrame;
        Vector3 actPos = mCamera->getPosition();
        if (mCameraRotating == TURNBACK)
        {
            if (cameraAngle >0) step*= -1;
            if (cameraAngle < -1 || cameraAngle > 1)
            {
                cameraAngle+= step;
                mCamera->yaw(Degree(step));
                mCamera->setPosition(CAMERA_OFFSET*Math::Sin(Degree(cameraAngle)), actPos.y, CAMERA_OFFSET *Math::Cos(Degree(cameraAngle))+CAMERA_RELA_Z);
            }
            else
            {
                mCamera->yaw(Degree(-cameraAngle));
                mCamera->setPosition(0, actPos.y, CAMERA_OFFSET+CAMERA_RELA_Z);
                mCameraRotating = NONE;
                cameraAngle = 0;
            }
        }
        else if (mCameraRotating == POSITIVE && cameraAngle < 45)
        {
            cameraAngle+= step;
            mCamera->yaw(Degree(step));
            mCamera->setPosition(CAMERA_OFFSET*Math::Sin(Degree(cameraAngle)), actPos.y, CAMERA_OFFSET *Math::Cos(Degree(cameraAngle))+CAMERA_RELA_Z);
        }
        else if (mCameraRotating == NEGATIVE && cameraAngle >-45)
        {
            cameraAngle-= step;
            mCamera->yaw(Degree(-step));
            mCamera->setPosition(CAMERA_OFFSET*Math::Sin(Degree(cameraAngle)), actPos.y, CAMERA_OFFSET *Math::Cos(Degree(cameraAngle))+CAMERA_RELA_Z);
        }
        TileManager::getSingleton().rotate(cameraAngle);
    }
    // ////////////////////////////////////////////////////////////////////
    // Update frame counter.
    // ////////////////////////////////////////////////////////////////////
    static int skipFrames =0;
    if (--skipFrames <= 0)
    {
        const RenderTarget::FrameStats& stats = mWindow->getStatistics();
        std::stringstream strBuf;
        strBuf << std::fixed << std::setprecision(1) << stats.lastFPS;
        GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_STAT_CUR_FPS, GuiManager::MSG_SET_TEXT, strBuf.str().c_str());
        strBuf.rdbuf()->str(""); // delete stringstream buffer.
        strBuf << std::fixed << std::setprecision(1) << stats.bestFPS;
        GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_STAT_BEST_FPS, GuiManager::MSG_SET_TEXT, strBuf.str().c_str());
        strBuf.rdbuf()->str(""); // delete stringstream buffer.
        strBuf << std::fixed << std::setprecision(1) << stats.worstFPS;
        GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_STAT_WORST_FPS, GuiManager::MSG_SET_TEXT, strBuf.str().c_str());
        strBuf.rdbuf()->str(""); // delete stringstream buffer.
        strBuf << std::fixed << std::setprecision(1) << stats.triangleCount;
        GuiManager::getSingleton().sendMsg(GuiManager::GUI_TEXTBOX_STAT_SUM_TRIS, GuiManager::MSG_SET_TEXT, strBuf.str().c_str());
        skipFrames = 100;
    }
    return true;
}
