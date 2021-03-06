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

#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreSceneManager.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>
#include <OgreStringConverter.h>
#include <OgreOverlayContainer.h>
#include "events.h"
#include "sound.h"
#include "option.h"
#include "logger.h"
#include "profiler.h"
#include "network.h"
#include "network_account.h"
#include "gui/gui_manager.h"
#include "tile/tile_manager.h"
#include "tile/tile_map.h"
#include "object/object_manager.h"
#include "object/object_element_avatar.h"
#include "object/object_element_animate3d.h"
#include "spell_manager.h"
#include "network_serverfile.h"
#include "assert.h"


using namespace Ogre;

static const unsigned int MIN_LEN_LOGIN_NAME =  3;
static const unsigned int MAX_LEN_LOGIN_NAME = 12;
static const unsigned int MIN_LEN_LOGIN_PSWD =  6;
static const unsigned int MAX_LEN_LOGIN_PSWD = 17;
static const Real CAMERA_POS_Y = TileManager::HALF_RENDER_SIZE * (TileManager::CHUNK_SIZE_Z+1);
static const Real CAMERA_POS_Z = CAMERA_POS_Y + TileManager::HALF_RENDER_SIZE * (TileManager::CHUNK_SIZE_Z-6)/2;
static const int  CAMERA_TURN_DELAY = 50; // The speed of the camera turning.
//static const int  CAMERA_TURN_MAX   = 45; // Maximum degree of camera turning by user.
static const int  CAMERA_TURN_MAX   = 90; // Maximum degree of camera turning by user.
static const char *GUI_LOADING_OVERLAY = "GUI_LOADING_OVERLAY";
static const char *GUI_LOADING_OVERLAY_ELEMENT = "GUI_LOADING_OVERLAY_ELEMENT";
static const unsigned long SERVER_TIMEOUT = 5000; // Server timeout in ms.
static const char BINARY_CMD_NEXT = '\0'; // Next commad.

//================================================================================================
// All paths of the whole project MUST be placed here!
//================================================================================================
static const char PATH_GFX_FONTS[] = "./media/textures/fonts/";
static const char PATH_GFX_ITEMS[] = "./media/textures/items/"; /**< The item graphics to build the Item-Atlas-Texture. **/
static const char PATH_GFX[]       = "./media/textures/";
static const char PATH_SND[]       = "./media/sound/";
static const char PATH_TXT[]       = "./media/xml/";
static const char PATH_SRV[]       = "./srv_files/";

//================================================================================================
// Filenames
//================================================================================================
static const char FILE_NPC_VISUALS[]     = "NPC_Visuals.xml";
static const char FILE_OPTIONS[]         = "./options.dat";
static const char FILE_CLIENT_SPELLS[]   = "./srv_files/client_spells";
static const char FILE_CLIENT_SKILLS[]   = "./srv_files/client_skills";
static const char FILE_CLIENT_SETTINGS[] = "./srv_files/client_settings";
static const char FILE_CLIENT_ANIMS[]    = "./srv_files/client_anims";
static const char FILE_BMAPS_UNIQUE[]    = "./srv_files/bmaps_unique";    /**< The objects from bmaps without animation states */
//const char FILE_BMAPS_P0[]        = "./bmaps.p0";
//const char FILE_DAIMONIN_P0[]     = "./daimonin.p0";
//const char FILE_ARCHDEF[]         = "./archdef.dat";
//const char FILE_BMAPS_TMP[]       = "./srv_files/bmaps.tmp";
//const char FILE_ANIMS_TMP[]       = "./srv_files/anims.tmp";

//================================================================================================
// Constructor.
//================================================================================================
void Events::Init(RenderWindow* win, SceneManager *SceneMgr)
{
    PROFILE()
    // ////////////////////////////////////////////////////////////////////
    // Create unbuffered key & mouse input.
    // ////////////////////////////////////////////////////////////////////
    mSceneManager = SceneMgr;
    mWindow = win;
    size_t windowHnd =0;
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    OIS::ParamList pl;
    pl.insert(std::make_pair(std::string("WINDOW"), StringConverter::toString(windowHnd)));
    mInputManager = OIS::InputManager::createInputSystem(pl);
    mInputKeyboard= static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
    mInputKeyboard->setEventCallback(this);
    mInputMouse= static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
    mInputMouse->setEventCallback(this);
    const OIS::MouseState &ms = mInputMouse->getMouseState();
    ms.width = mWindow->getWidth();
    ms.height= mWindow->getHeight();
    mMouse = Vector3::ZERO;
    mQuitGame = false;
    Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_VIEWPORT);
}

//================================================================================================
// Destructor.
//================================================================================================
void Events::freeRecources()
{
    PROFILE()
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
}

//================================================================================================
// Avatar movement.
//================================================================================================
void Events::checkTileBorderMovement()
{
    PROFILE()
    static Vector3 oldPos = ObjectManager::getSingleton().getAvatarPos()/TileManager::TILE_RENDER_SIZE;
    Vector3 actPos = ObjectManager::getSingleton().getAvatarPos()/TileManager::TILE_RENDER_SIZE;
    int dx = (int)oldPos.x - (int)actPos.x;
    int dz = (int)oldPos.z - (int)actPos.z;
    // Player moved over a tile border.
    if (dx || dz)
    {
        TileManager::getSingleton().scrollMap(dx, dz);
        ObjectManager::getSingleton().syncToMapScroll(dx, dz);
        oldPos = ObjectManager::getSingleton().getAvatarPos()/TileManager::TILE_RENDER_SIZE;
        /*
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
        */
    }
}

//================================================================================================
// Frame Start event.
//================================================================================================
bool Events::frameStarted(const FrameEvent& evt)
{
    PROFILE()
    static unsigned long timeWaitServer;
    static String strAccountName, strAccountPswd, strAccountPlayer;
    if (mWindow->isClosed() || mQuitGame)
        return false;

    switch (Option::getSingleton().getGameStatus())
    {
        case Option::GAME_STATUS_INIT_VIEWPORT:
        {
            // ////////////////////////////////////////////////////////////////////
            // Create one viewport, entire window.
            // ////////////////////////////////////////////////////////////////////
            mCamera = mSceneManager->createCamera("PlayerCam");
            mCamera->setQueryFlags(ObjectManager::QUERY_MASK_CAMERA);
            Viewport *VP = mWindow->addViewport(mCamera);
            mCamera->setAspectRatio(Real(VP->getActualWidth()) / Real(VP->getActualHeight()));
            mCameraZoom = STD_CAMERA_ZOOM;
            mCamera->setFOVy(Degree(mCameraZoom));
            mCamera->setPosition(0, CAMERA_POS_Y, CAMERA_POS_Z);
            mCamera->pitch(Degree(-36));
            mWorld = mSceneManager->getRootSceneNode()->createChildSceneNode();
            // ////////////////////////////////////////////////////////////////////
            // Create a minimal gui for some loading infos..
            // ////////////////////////////////////////////////////////////////////
            GuiManager::getSingleton().Init(mWindow->getWidth(), mWindow->getHeight(),
                                            Option::getSingleton().getIntValue(Option::CMDLINE_CREATE_MEDIA)?true:false,
                                            Option::getSingleton().getIntValue(Option::CMDLINE_GUI_INFORMATION)?true:false,
                                            PATH_TXT, PATH_GFX, PATH_GFX_FONTS, PATH_GFX_ITEMS);
            // Show the loading-gfx.
            Overlay *overlay = OverlayManager::getSingleton().create(GUI_LOADING_OVERLAY);
            overlay->setZOrder(400);
            OverlayElement *element = OverlayManager::getSingleton().createOverlayElement("Panel", GUI_LOADING_OVERLAY_ELEMENT);
            element->setMetricsMode(GMM_RELATIVE);
            element->setPosition(0.0, 0.0);
            element->setDimensions(0.5, 1.0);
            element->setMaterialName("GUI/LoadScreen" + StringConverter::toString((Root::getSingleton().getTimer()->getMilliseconds()&1)+1));
            overlay->add2D(static_cast<OverlayContainer*>(element));
            overlay->show();
            if (Option::getSingleton().getIntValue(Option::ERROR_NO_SHADRES))
            {
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_CRITICAL_ERROR);
                GuiManager::getSingleton().displaySystemMessage("*** Critical Error ***");
                GuiManager::getSingleton().displaySystemMessage("Your gfx-card has no shader support.");
                GuiManager::getSingleton().displaySystemMessage("Client3d will not run on your system.");
                GuiManager::getSingleton().displaySystemMessage("Press ESC to quit the client.");
                break;
            }
            GuiManager::getSingleton().displaySystemMessage("* Welcome to Daimonin *");
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_SOUND);
            GuiManager::getSingleton().displaySystemMessage("Starting the sound-manager...");
            break;
        }

        case Option::GAME_STATUS_INIT_SOUND:
        {
            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////// TESTING (Create the alpha values for a NPC texture) /////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            /*
                        Image img1, img2;
                        img1.load("Smitty_DATA.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                        img2.load("Smitty_MASK.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                        uchar *src = img1.getData();
                        uchar *dst = img2.getData();
                        for (size_t i = 0; i < img1.getWidth() * img1.getHeight(); ++i)
                        {
                            src+= 3;
                            *src++ = *dst++;
                            dst+= 3;
                        }
                        img1.save("c:/Smitty.png");
                        //
                        static SceneNode *mNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
                        mNode->attachObject(mSceneManager->createEntity("Entity_Smitty", "Smitty.mesh"));
                        mNode->setPosition(830, 80, 1200);
                        mNode->scale(2, 2, 2);
                        //////////////////////////////////////////////////////////////////////////////////////////////////////////
                        //////////////////////////////////////////////////////////////////////////////////////////////////////////
                        //////////////////////////////////////////////////////////////////////////////////////////////////////////
            */
            // ////////////////////////////////////////////////////////////////////
            // Init the sound and play the background music.
            // ////////////////////////////////////////////////////////////////////
            Sound::getSingleton().Init(PATH_SND, Option::getSingleton().getIntValue(Option::CMDLINE_SND_DEVICE));
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
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_GUI_WINDOWS);
            GuiManager::getSingleton().displaySystemMessage("Starting the gui-manager...");
            GuiManager::getSingleton().displaySystemMessage(" - Parsing windows.");
            break;
        }

        case Option::GAME_STATUS_INIT_GUI_WINDOWS:
        {
            GuiManager::getSingleton().parseWindows();
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_TILE);
            GuiManager::getSingleton().displaySystemMessage("Starting the tile-engine...");
            break;
        }

        case Option::GAME_STATUS_INIT_TILE:
        {
            TileManager::getSingleton().Init(mSceneManager, ObjectManager::QUERY_MASK_TILES_LAND,
                                             ObjectManager::QUERY_MASK_TILES_WATER,
                                             Option::getSingleton().getIntValue(Option::CMDLINE_TILEENGINE_LOD),
                                             Option::getSingleton().getIntValue(Option::CMDLINE_CREATE_MEDIA)?true:false);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_OBJECT);
            GuiManager::getSingleton().displaySystemMessage("Starting the object-manager...");
            break;
        }

        case Option::GAME_STATUS_INIT_OBJECT:
        {
            ObjectManager::getSingleton().init(mSceneManager);
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
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Welcome to ~Daimonin 3D~.");
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "You need a running server to start the game!");
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
            break;
        }

        case Option::GAME_STATUS_INIT_NET:
        {
            Network::getSingleton().freeRecources();
            Network::getSingleton().Init();
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_META);
            break;
        }

        case Option::GAME_STATUS_META:
        {
            //Network::getSingleton().CloseClientSocket();
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
            GuiManager::getSingleton().setText(GuiManager::TEXTBOX_SERVER_INFO1, "Select a server");
            break;
        }

        case Option::GAME_STATUS_STARTCONNECT:
        {
            if (GuiManager::getSingleton().getUserBreak(GuiManager::TABLE))
            {
                GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, false);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
                mQuitGame = true;
            }
            // A server was selected.
            int select = GuiManager::getSingleton().getActivated(GuiManager::TABLE);
            if (select >=0)
            {
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_SERVER_INFO2, " ");
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_SERVER_INFO3, " ");
                Network::getSingleton().setActiveServer(select);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_CONNECT);
            }
            break;
        }

        case Option::GAME_STATUS_CONNECT:
        {
            if (!Network::getSingleton().OpenActiveServerSocket())
            {
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Connection failed!");
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_SERVER_INFO2, "~#ffff0000Connection failed!");
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
                break;
            }
            Network::getSingleton().socket_thread_start();
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Connected. Sending Setup Command.");
            GuiManager::getSingleton().setText(GuiManager::TEXTBOX_SERVER_INFO2, "~#ff00ff00Connected!");
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_SETUP);
            break;
        }

        case Option::GAME_STATUS_SETUP:
        {
            ServerFile::getSingleton().checkFile(ServerFile::FILE_SKILLS,   FILE_CLIENT_SKILLS);
            ServerFile::getSingleton().checkFile(ServerFile::FILE_SPELLS,   FILE_CLIENT_SPELLS);
            ServerFile::getSingleton().checkFile(ServerFile::FILE_SETTINGS, FILE_CLIENT_SETTINGS);
            ServerFile::getSingleton().checkFile(ServerFile::FILE_BMAPS,    FILE_BMAPS_UNIQUE);
            ServerFile::getSingleton().checkFile(ServerFile::FILE_ANIMS,    FILE_CLIENT_ANIMS);
            std::stringstream strCmd;
            strCmd  <<
                    "dv "   << Network::DAI_VERSION_RELEASE << "." << Network::DAI_VERSION_MAJOR << "." << Network::DAI_VERSION_MINOR <<
                    " pv "  << Network::PROTOCOL_VERSION <<
                    " sn "  << "0" << // Sounds
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
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "~#ffff0000-------------------------------------------------");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "~#ffff0000Server timeout on setup command!");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "~#ffff0000-------------------------------------------------");
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
            }
            break;
        }

        case Option::GAME_STATUS_REQUEST_FILES:
        {
            // Wait until all files are upToDate.
            if (ServerFile::getSingleton().requestFiles())
            {
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "All files ok.");
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN);
            }
            break;
        }

        case Option::GAME_STATUS_LOGIN:
        {
            TileMap::getSingleton().clear_map(); // Eats up MUCH time in debug mode.
            GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, false);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_LOGIN, true);
            GuiManager::getSingleton().startTextInput(GuiManager::WIN_LOGIN, GuiManager::TEXTINPUT_LOGIN_NAME, MAX_LEN_LOGIN_NAME, true, true);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_NAME);
            break;
        }

        // Login on an existing account.
        case Option::GAME_STATUS_LOGIN_NAME:
        {
            static bool errorMsgNeedsToBeCleared = false;
            if (GuiManager::getSingleton().brokenTextInput())
            {
                Logger::log().error() << Logger::ICON_CLIENT << "User break on login.";
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
                break;
            }
            if (GuiManager::getSingleton().getUserAction() && errorMsgNeedsToBeCleared)
            {
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_LOGIN_WARN, " ");
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
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_LOGIN_WARN, strMsg.c_str());
                GuiManager::getSingleton().startTextInput(GuiManager::WIN_LOGIN, GuiManager::TEXTINPUT_LOGIN_NAME, MAX_LEN_LOGIN_NAME, true, true);
                errorMsgNeedsToBeCleared = true;
                break;
            }
            GuiManager::getSingleton().startTextInput(GuiManager::WIN_LOGIN, GuiManager::TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_PASWD);
            break;
        }

        case Option::GAME_STATUS_LOGIN_PASWD:
        {
            static bool errorMsgNeedsToBeCleared = false;
            if (GuiManager::getSingleton().brokenTextInput())
            {
                Logger::log().error() << Logger::ICON_CLIENT <<  "User break on login.";
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
                break;
            }
            if (GuiManager::getSingleton().getUserAction() && errorMsgNeedsToBeCleared)
            {
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_LOGIN_WARN, " ");
                errorMsgNeedsToBeCleared = false;
            }
            // Wait for user to finish the textinput.
            if (!GuiManager::getSingleton().finishedTextInput()) break;
            strAccountPswd = GuiManager::getSingleton().getTextInput();
            if (strAccountPswd.size() < MIN_LEN_LOGIN_PSWD || strAccountPswd.size() > MAX_LEN_LOGIN_PSWD)
            {
                String strMsg = "~#ffff0000Password length must be between " + StringConverter::toString(MIN_LEN_LOGIN_PSWD)+
                                " and " + StringConverter::toString(MAX_LEN_LOGIN_PSWD) + " chars!~";
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_LOGIN_WARN, strMsg.c_str());
                GuiManager::getSingleton().startTextInput(GuiManager::WIN_LOGIN, GuiManager::TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
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
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_LOGIN_WARN, "~#ffff0000Server timeout on account login!");
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
            }
            break;
        }

        case Option::GAME_STATUS_LOGIN_DONE:
        {
            GuiManager::getSingleton().clear(GuiManager::TABLE);
            String strTabel;
            for (int i = 0; i < NetworkAccount::getSingleton().getSumChars(); ++i)
            {
                const NetworkAccount::account *acc = NetworkAccount::getSingleton().getAccountEntry(i);
                strTabel = acc->name + ";";
                strTabel+= acc->race?  "Elfish ":"Human ";
                strTabel+= acc->gender?"Female;":"Male;";
                strTabel+= "Level " + StringConverter::toString(acc->level);
                GuiManager::getSingleton().addLine(GuiManager::TABLE, strTabel.c_str());
            }
            GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, true);
            GuiManager::getSingleton().setText(GuiManager::TEXTBOX_SERVER_INFO1, "Select a character");
            GuiManager::getSingleton().showWindow(GuiManager::WIN_LOGIN, false);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_CHOOSE_CHAR);
            break;
        }

        case Option::GAME_STATUS_LOGIN_CHOOSE_CHAR:
        {
            if (GuiManager::getSingleton().getUserBreak(GuiManager::TABLE))
            {
                GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, false);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
            }
            // A character was selected.
            int select = GuiManager::getSingleton().getActivated(GuiManager::TABLE);
            if (select >=0)
            {
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_SERVER_INFO2, " ");
                GuiManager::getSingleton().setText(GuiManager::TEXTBOX_SERVER_INFO3, " ");
                NetworkAccount::getSingleton().setSelected(select);
                GuiManager::getSingleton().showWindow(GuiManager::WIN_SERVERSELECT, false);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_CHARACTER);
            }
            break;
        }

        case Option::GAME_STATUS_LOGIN_CHARACTER:
        {
            std::string name = NetworkAccount::getSingleton().getSelectedChar();
            ObjectManager::getSingleton().setAvatarName(name);
            std::stringstream ssTemp;
            ssTemp << name;
            Network::getSingleton().send_command_binary(Network::CLIENT_CMD_ADDME, ssTemp);
//            GuiManager::getSingleton().addTextline(GuiManager::WIN_CHATWINDOW, GuiManager::LIST_MSGWIN, name.c_str());
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
            if (!once)
            {
                once = true;
                GuiManager::getSingleton().showWindow(GuiManager::WIN_STATISTICS, true);
                GuiManager::getSingleton().showWindow(GuiManager::WIN_PLAYERINFO, false);
                GuiManager::getSingleton().showWindow(GuiManager::WIN_FIRST_STEPS, true);
                mWindow->resetStatistics();
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Client3d commands:");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Press ~!keyg~ for grid. ");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Press ~PGUP/PGDN~ to rotate camera.");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Press ~HOME~ to freeze camera rotation.");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Press ~!keyi~ for Inventory.");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Example of user defined chars: :( :) :D :P !key-spc");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "---------------------------------------------------");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Press ~9~ to (re)load the mask demo!");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Press ~8~ to change equipment");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Press ~7~ for grass shader test");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Press ~!keye~, ~!keyf~, ~!keyw~, for creature shader test");
                GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "---------------------------------------------------");
                // Can crash the client...
                //ObjectManager::getSingleton().setNameNPC(ObjectNPC::HERO, strAccountName.c_str());
                //Sound::getSingleton().playStream(Sound::GREETS_VISITOR);
            }
            mIdleTime =0;
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_GAME_LOOP);
            break;
        }

        case Option::GAME_STATUS_GAME_LOOP:
        {
            if ((mIdleTime += evt.timeSinceLastFrame) > 45.0)
            {
                mIdleTime = 0;
                Sound::getSingleton().playStream(Sound::PLAYER_IDLE);
                break;
            }
            static unsigned long time = Root::getSingleton().getTimer()->getMilliseconds();
            if (Root::getSingleton().getTimer()->getMilliseconds() - time > 180.0)
            {
                RaySceneQuery *mRaySceneQuery = mSceneManager->createRayQuery(Ray());
                mRaySceneQuery->setRay(mCamera->getCameraToViewportRay(mMouse.x / mWindow->getWidth(), mMouse.y / mWindow->getHeight()));
                mRaySceneQuery->setQueryMask(ObjectManager::QUERY_MASK_NPC | ObjectManager::QUERY_MASK_CONTAINER);
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
                    ObjectManager::getSingleton().highlightObject(0);
                }
                mSceneManager->destroyQuery(mRaySceneQuery);
                time = Root::getSingleton().getTimer()->getMilliseconds();
            }
            ObjectManager::getSingleton().update(evt);
            TileMap::getSingleton().update();
            checkTileBorderMovement();
            break;
        }

        default:
        {
            Logger::log().error() << Logger::ICON_CLIENT << "frameStarted(): unknown state";
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
    PROFILE()
    if (Option::getSingleton().getGameStatus() < Option::GAME_STATUS_INIT_NET)
        return true;
    mInputMouse->capture();
    mInputKeyboard->capture();
    GuiManager::getSingleton().update(evt.timeSinceLastFrame);
    Sound::getSingleton().playGuiSounds(GuiManager::getSingleton().getActiveSounds());
    Network::getSingleton().update(evt.timeSinceLastFrame);
    // ////////////////////////////////////////////////////////////////////
    // Update camera movement.
    // ////////////////////////////////////////////////////////////////////
    if (mCameraRotating != NONE)
    {
        static Real cameraAngle = 0;
        Real step = CAMERA_TURN_DELAY * evt.timeSinceLastFrame;
        if (mCameraRotating == TURNBACK)
        {
            if (cameraAngle < -1 || cameraAngle > 1)
            {
                if (cameraAngle >0) step*= -1;
                cameraAngle+= step;
                mCamera->yaw(Degree(step));
                mCamera->setPosition(CAMERA_POS_Z *Math::Sin(Degree(cameraAngle)), CAMERA_POS_Y, CAMERA_POS_Z *Math::Cos(Degree(cameraAngle))+0);
            }
            else
            {
                mCamera->yaw(Degree(-cameraAngle));
                mCamera->setPosition(0, CAMERA_POS_Y, CAMERA_POS_Z);
                mCameraRotating = NONE;
                cameraAngle = 0;
            }
        }
        else if (mCameraRotating == POSITIVE && cameraAngle < CAMERA_TURN_MAX)
        {
            cameraAngle+= step;
            mCamera->yaw(Degree(step));
            mCamera->setPosition(CAMERA_POS_Z *Math::Sin(Degree(cameraAngle)), CAMERA_POS_Y, CAMERA_POS_Z *Math::Cos(Degree(cameraAngle))+0);
        }
        else if (mCameraRotating == NEGATIVE && cameraAngle >-CAMERA_TURN_MAX)
        {
            cameraAngle-= step;
            mCamera->yaw(Degree(-step));
            mCamera->setPosition(CAMERA_POS_Z *Math::Sin(Degree(cameraAngle)), CAMERA_POS_Y, CAMERA_POS_Z *Math::Cos(Degree(cameraAngle))+0);
        }
        TileManager::getSingleton().rotateCamera(cameraAngle);
    }
    // ////////////////////////////////////////////////////////////////////
    // Update frame counter.
    // ////////////////////////////////////////////////////////////////////
    static int skipFrames =0;
    if (--skipFrames <= 0)
    {
        const RenderTarget::FrameStats &stats = mWindow->getStatistics();
        GuiManager::getSingleton().setText(GuiManager::TEXTBOX_STAT_CUR_FPS,   StringConverter::toString((int)stats.lastFPS).c_str());
        GuiManager::getSingleton().setText(GuiManager::TEXTBOX_STAT_BEST_FPS,  StringConverter::toString((int)stats.bestFPS).c_str());
        GuiManager::getSingleton().setText(GuiManager::TEXTBOX_STAT_WORST_FPS, StringConverter::toString((int)stats.worstFPS).c_str());
        GuiManager::getSingleton().setText(GuiManager::TEXTBOX_STAT_SUM_BATCH, StringConverter::toString(stats.batchCount).c_str());
        GuiManager::getSingleton().setText(GuiManager::TEXTBOX_STAT_SUM_TRIS,  StringConverter::toString(stats.triangleCount).c_str());
        skipFrames = (int)stats.lastFPS; // Refresh only once per second.
    }
    return true;
}
