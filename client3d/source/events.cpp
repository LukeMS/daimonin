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
#include "gui_textinput.h"
#include "object_manager.h"
#include "object_visuals.h"
#include "particle_manager.h"
#include "spell_manager.h"
#include "network_serverfile.h"

using namespace Ogre;

static const unsigned int MIN_LEN_LOGIN_NAME =  2;
static const unsigned int MAX_LEN_LOGIN_NAME = 12;
static const unsigned int MIN_LEN_LOGIN_PSWD =  6;
static const unsigned int MAX_LEN_LOGIN_PSWD = 17;
const Real CAMERA_OFFSET = 1325;
const char *GUI_LOADING_OVERLAY = "GUI_LOADING_OVERLAY";
const char *GUI_LOADING_OVERLAY_ELEMENT = "GUI_LOADING_OVERLAY_ELEMENT";

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
    if      (deltaX >0 && deltaZ >0) Network::getSingleton().send_command("/sw", -1, Network::SC_FIRERUN);
    else if (deltaX <0 && deltaZ >0) Network::getSingleton().send_command("/se", -1, Network::SC_FIRERUN);
    else if (deltaX >0 && deltaZ <0) Network::getSingleton().send_command("/nw", -1, Network::SC_FIRERUN);
    else if (deltaX <0 && deltaZ <0) Network::getSingleton().send_command("/ne", -1, Network::SC_FIRERUN);
    else if (deltaX==0 && deltaZ <0) Network::getSingleton().send_command("/n" , -1, Network::SC_FIRERUN);
    else if (deltaX==0 && deltaZ >0) Network::getSingleton().send_command("/s" , -1, Network::SC_FIRERUN);
    else if (deltaX >0 && deltaZ==0) Network::getSingleton().send_command("/w" , -1, Network::SC_FIRERUN);
    else if (deltaX <0 && deltaZ==0) Network::getSingleton().send_command("/e" , -1, Network::SC_FIRERUN);

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
    static String strAccountName, strAccountPswd;
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
            mCamera->setPosition(0, CAMERA_OFFSET, CAMERA_OFFSET);
            mCamera->pitch(Degree(-36));
            mWorld = mSceneManager->getRootSceneNode()->createChildSceneNode();
            // ////////////////////////////////////////////////////////////////////
            // Create a minimal gui for some loading infos..
            // ////////////////////////////////////////////////////////////////////
            GuiManager::getSingleton().Init(mWindow->getWidth(), mWindow->getHeight());
            GuiTextout::getSingleton().loadRawFont(FILE_SYSTEM_FONT);
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
            GuiImageset::getSingleton().parseXML(FILE_GUI_IMAGESET);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_GUI_WINDOWS);
            GuiManager::getSingleton().displaySystemMessage(" - Parsing windows.");
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
            GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_CHATWINDOW, true);
            GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_TEXTWINDOW, true);
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Welcome to ~Daimonin 3D~.");
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "You need a running server to start the game!");
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
            GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_LOGIN, false);
            GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_SERVERSELECT, true);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_STARTCONNECT);
            break;
        }

        case Option::GAME_STATUS_STARTCONNECT:
        {
            if (GuiManager::getSingleton().getTableUserBreak(GuiManager::GUI_WIN_SERVERSELECT, GuiImageset::GUI_TABLE))
            {
                GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_SERVERSELECT, false);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_PLAY);
            }
            // Print server infos.
            int select;
            select = GuiManager::getSingleton().getTableSelection(GuiManager::GUI_WIN_SERVERSELECT, GuiImageset::GUI_TABLE);
            if (select >=0)
            {
                for (int i =0; i< 4;++i)
                {
                    GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_SERVERSELECT,
                                                           GuiManager::GUI_MSG_TXT_CHANGED,
                                                           GuiImageset::GUI_TEXTBOX_SERVER_INFO1 + i,
                                                           (void*)Network::getSingleton().get_metaserver_info(select, i));
                }
            }
            // A server was selected.
            select = GuiManager::getSingleton().getTableActivated(GuiManager::GUI_WIN_SERVERSELECT, GuiImageset::GUI_TABLE);
            if (select >=0)
            {
                GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_SERVERSELECT, false);
                Network::getSingleton().setActiveServer(select);
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_CONNECT);
            }
            break;
        }

        case Option::GAME_STATUS_CONNECT:
        {
            //GuiManager::getSingleton().showWindow(GUI_WIN_SERVERSELECT, false);
            Network::GameStatusVersionFlag = false;
            if (!Network::getSingleton().OpenActiveServerSocket())
            {
                GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Connection failed!");
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
                break;
            }
            Network::getSingleton().socket_thread_start();
            GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Connected. Sending Setup Command.");
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_SETUP);
            break;
        }

        case Option::GAME_STATUS_SETUP:
        {
            ServerFile::getSingleton().checkFiles();
            std::stringstream strCmd;
            strCmd <<
            "cs "  << Network::VERSION_CS <<
            " sc " << Network::VERSION_SC <<
            " sn " << 1 <<
            " mz "    << 17 << "x" << 17 <<
            " skf " << ServerFile::getSingleton().getLength(ServerFile::FILE_SKILLS)  << "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_SKILLS)     << std::dec <<
            " spf " << ServerFile::getSingleton().getLength(ServerFile::FILE_SPELLS)  << "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_SPELLS)     << std::dec <<
            " bpf " << ServerFile::getSingleton().getLength(ServerFile::FILE_BMAPS)   << "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_BMAPS)      << std::dec <<
            " stf " << ServerFile::getSingleton().getLength(ServerFile::FILE_SETTINGS)<< "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_SETTINGS)   << std::dec <<
            " amf " << ServerFile::getSingleton().getLength(ServerFile::FILE_ANIMS)   << "|" << std::hex<< ServerFile::getSingleton().getCRC(ServerFile::FILE_ANIMS)      << std::dec << '\0';
            //Network::getSingleton().cs_write_string((char*)strCmd.str().c_str());
            Network::getSingleton().send_command_binary(Network::CLIENT_CMD_SETUP, (unsigned char*)strCmd.str().c_str(), (int)strlen(strCmd.str().c_str()), Network::SEND_CMD_FLAG_STRING);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_WAITSETUP);
            break;
        }

        case Option::GAME_STATUS_WAITSETUP:
        {
            // Here we wait for the server to send SetupCmd().
            // ToDO: Set a timer, so it won't be a endless loop if the SetupCmd changed and client don't understand it.
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
            GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_SERVERSELECT, false);
            GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_LOGIN, true);
            GuiManager::getSingleton().startTextInput(GuiManager::GUI_WIN_LOGIN, GuiImageset::GUI_TEXTINPUT_LOGIN_NAME, MAX_LEN_LOGIN_NAME, true, true);
            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_NAME);
            break;
        }

        // Login on an existing account.
        case Option::GAME_STATUS_LOGIN_NAME:
        {

            //Logger::log().error() <<  "TEST: " << test.str().size();

            if (GuiManager::getSingleton().brokenTextInput())
            {
                Logger::log().error() <<  "User break on login.";
                Network::getSingleton().CloseSocket();
                Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
                break;
            }
            // Wait for user to finish the textinput.
            if (!GuiManager::getSingleton().finishedTextInput()) break;
            // Check length of the username.
            String strAccountName = GuiManager::getSingleton().getTextInput();
            if (strAccountName.size() < MIN_LEN_LOGIN_NAME || strAccountName.size() > MAX_LEN_LOGIN_NAME)
            {
                String strMsg = "~#ffff0000Username length must be between " + StringConverter::toString(MIN_LEN_LOGIN_NAME)+
                                " and " + StringConverter::toString(MAX_LEN_LOGIN_NAME) + " chars!~";
                GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_LOGIN, GuiManager::GUI_MSG_TXT_CHANGED, GuiImageset::GUI_TEXTBOX_LOGIN_WARN, (void*)strMsg.c_str());
                GuiManager::getSingleton().startTextInput(GuiManager::GUI_WIN_LOGIN, GuiImageset::GUI_TEXTINPUT_LOGIN_NAME, MAX_LEN_LOGIN_NAME, true, true);
                break;
            }
            std::stringstream strAccount;
            strAccount << strAccountName << '\0' << strAccountName << "123" << '\0';
            Network::getSingleton().send_command_binary(Network::CLIENT_CMD_LOGIN, strAccount);

            Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_NAME_WAIT);
            break;
        }

/*
void client_send_checkname(char *buf)
{

    SockList_COMMAND(&sl, CLIENT_CMD_CHECKNAME, 0);
    SockList_AddString(&sl, buf);
    send_socklist_binary(&sl);


 static inline void SockList_AddString(SockList *const sl, const char *const buf)
{
    int len = strlen(buf);

    if(sl->buf)
    {
        memcpy(sl->buf+sl->len,buf,len);
        *(sl->buf+sl->len+len++) = 0; // ensure the string is send with 0 end marker
    }
    else
    {
        memcpy(sl->defbuf+sl->len,buf,len);
        *(sl->defbuf+sl->len+len++) = 0;
    }
    sl->len+=len;

}


    // Contains the base information we use to make up a packet we want to send.
typedef struct SockList
{
    int             cmd; // the binary command tag we want send
    int             flags; // the flags for send function
    int             len; // How much data in buf
    int             pos; // Start of data in buf
    char            *buf;
    char            defbuf[MAX_DATA_TAIL_LENGTH];
}
SockList;

// help define for a clean socklist init
#define         SockList_INIT(_sl_, _buf_) {memset( (_sl_), 0, sizeof(SockList) );(_sl_)->buf=(_buf_);}
#define         SockList_COMMAND(__sl, __cmd, __flags) {(__sl)->cmd=(__cmd);(__sl)->flags=(__flags);}
#define         SockList_AddChar(__sl, __c) (__sl)->buf?*((__sl)->buf+(__sl)->len++):(((__sl)->defbuf[(__sl)->len++])= (unsigned char)(__c))
#define send_socklist_binary(_sl_) send_command_binary((_sl_)->cmd, (_sl_)->buf?(_sl_)->buf:(_sl_)->defbuf, (_sl_)->len, (_sl_)->flags )
}
*/
        case Option::GAME_STATUS_LOGIN_NAME_WAIT:
        {
            //Logger::log().error() << "GAME_STATUS_LOGIN_NAME_WAIT";

            //Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_NAME_WAIT);
            break;
        }


           // Logger::log().error() << "Account name entered!";

        /*
                    // Check length of the username.
                    strAccountName = GuiManager::getSingleton().getTextInput();

        #ifndef AUTO_FILL_PASWD
                    Option::getSingleton().setLoginType(Option::LOGIN_EXISTING_PLAYER);
        #endif
                    // C -> Create new hero , L -> Login.
                    String strServer = Option::getSingleton().getLoginType() == Option::LOGIN_NEW_PLAYER?"C":"L";
                    strServer += strAccountName;
                    Network::getSingleton().send_reply((char*)strServer.c_str());
                    GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_LOGIN, GuiManager::GUI_MSG_TXT_CHANGED, GuiImageset::GUI_TEXTBOX_LOGIN_WARN, (void*)"");
                    // now wait again for next server question
                    Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_WAIT);
                    break;
                }

                case Option::GAME_STATUS_PSWD_INIT:
                {
                    GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_LOGIN, true);
        #ifndef AUTO_FILL_PASWD
                    GuiManager::getSingleton().startTextInput(GuiManager::GUI_WIN_LOGIN, GuiImageset::GUI_TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
                    Option::getSingleton().setGameStatus(Option::GAME_STATUS_PSWD_USER);
        #else
                    Network::getSingleton().send_reply("NIX_PASWD");
                    Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_WAIT);
        #endif
                    break;
                }

                case Option::GAME_STATUS_PSWD_USER:
                {
                    if (GuiManager::getSingleton().brokenTextInput())
                    {
                        Network::getSingleton().CloseSocket();
                        Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
                        break;
                    }
                    if (!GuiManager::getSingleton().finishedTextInput())
                        break;
                    strAccountPswd = GuiManager::getSingleton().getTextInput();
                    if (strAccountPswd.size() < MIN_LEN_LOGIN_PSWD || strAccountPswd.size() > MAX_LEN_LOGIN_PSWD)
                    {
                        String strMsg = "~#ffff0000Password length must be between " + StringConverter::toString(MIN_LEN_LOGIN_PSWD)+
                                        " and "  + StringConverter::toString(MAX_LEN_LOGIN_PSWD) + " chars!~";
                        GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_LOGIN, GuiManager::GUI_MSG_TXT_CHANGED, GuiImageset::GUI_TEXTBOX_LOGIN_WARN, (void*)strMsg.c_str());
                        GuiManager::getSingleton().startTextInput(GuiManager::GUI_WIN_LOGIN, GuiImageset::GUI_TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
                        break;
                    }
                    if (strAccountPswd == strAccountName)
                    {
                        GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_LOGIN, GuiManager::GUI_MSG_TXT_CHANGED, GuiImageset::GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Password can't be same as character name!~");
                        GuiManager::getSingleton().startTextInput(GuiManager::GUI_WIN_LOGIN, GuiImageset::GUI_TEXTINPUT_LOGIN_PASSWD, MAX_LEN_LOGIN_PSWD, false, false);
                        break;
                    }
                    Logger::log().info() << "Login: send password <*****>";
                    Network::getSingleton().send_reply((char*)strAccountPswd.c_str());
                    Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_WAIT);
                    GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_LOGIN, GuiManager::GUI_MSG_TXT_CHANGED, GuiImageset::GUI_TEXTBOX_LOGIN_WARN, (void*)"");
                    GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_LOGIN, false);
                    break;
                }

                case Option::GAME_STATUS_VRFY_INIT:
                {
                    GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_LOGIN, true);
        #ifndef AUTO_FILL_PASWD
                    GuiManager::getSingleton().startTextInput(GuiManager::GUI_WIN_LOGIN, GuiImageset::GUI_TEXTINPUT_LOGIN_VERIFY, MAX_LEN_LOGIN_PSWD, false, false);
                    Option::getSingleton().setGameStatus(Option::GAME_STATUS_VRFY_USER);
        #else
                    Network::getSingleton().send_reply("NIX_PASWD");
                    Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_WAIT);
        #endif
                    break;
                }

                case Option::GAME_STATUS_VRFY_USER:
                {
                    TileManager::getSingleton().map_transfer_flag = 0;
                    if (GuiManager::getSingleton().brokenTextInput())
                    {
                        Network::getSingleton().CloseSocket();
                        Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
                        break;
                    }
                    if (!GuiManager::getSingleton().finishedTextInput())
                        break;
                    Logger::log().info() << "Login: send verify password <*****>";
                    Network::getSingleton().send_reply((char*)GuiManager::getSingleton().getTextInput());
                    Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_WAIT);
                    break;
                }

                case Option::GAME_STATUS_NEW_CHAR:
                {
                    // Send the new created character to server.
                    Option::getSingleton().setLoginType(Option::LOGIN_NEW_PLAYER);
                    // "nc %s %d %d %d %d %d %d %d %d", nc->char_arch[nc->gender_selected], nc->stats[0], nc->stats[1], nc->stats[2], nc->stats[3], nc->stats[4], nc->stats[5], nc->stats[6], nc->skill_selected);
                    char buf[] = "nc human_male 14 14 13 12 12 12 12 0";
                    Network::getSingleton().cs_write_string(buf);
                    GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_LOGIN, false);
                    Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_WAIT);
                    break;
                }
        */
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
                    GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_STATISTICS, true);
                    GuiManager::getSingleton().showWindow(GuiManager::GUI_WIN_PLAYERINFO, true);
                    mWindow->resetStatistics();
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Client3d commands:");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~1 ... 8~ to change cloth.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Hold shift for a ranged attack.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~!keyg~ for grid. ");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~!keya~ to change Idle animation.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~!keyb~ to change Attack animation.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~!keyc~ to change Agility animation.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~!keyp~ to ready/unready primary weapon.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~!keys~ to ready/unready secondary weapon.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~!keyq~ to start attack animation.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~PGUP/PGDOWN~ to rotate camera.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~HOME~ to freeze camera rotation.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Press ~!keyi~ for Inventory.");
                    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Example of user defined chars: :( :) :D :P !key-spc");
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

            /*
                    static Real time = evt.timeSinceLastFrame+1.0;
                    if (evt.timeSinceLastFrame > time)
                    {
                      TileManager::getSingleton().ChangeChunks();
                       time = evt.timeSinceLastFrame+1.0;
                    }
            */
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
    GuiManager::getSingleton().update(evt.timeSinceLastFrame);
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
                mCamera->setPosition(CAMERA_OFFSET*Math::Sin(Degree(cameraAngle)), actPos.y, CAMERA_OFFSET *Math::Cos(Degree(cameraAngle)));
            }
            else
            {
                mCamera->yaw(Degree(-cameraAngle));
                mCamera->setPosition(0, actPos.y, CAMERA_OFFSET);
                mCameraRotating = NONE;
                cameraAngle = 0;
            }
        }
        else if (mCameraRotating == POSITIVE && cameraAngle < 45)
        {
            cameraAngle+= step;
            mCamera->yaw(Degree(step));
            mCamera->setPosition(CAMERA_OFFSET*Math::Sin(Degree(cameraAngle)), actPos.y, CAMERA_OFFSET *Math::Cos(Degree(cameraAngle)));
        }
        else if (mCameraRotating == NEGATIVE && cameraAngle >-45)
        {
            cameraAngle-= step;
            mCamera->yaw(Degree(-step));
            mCamera->setPosition(CAMERA_OFFSET*Math::Sin(Degree(cameraAngle)), actPos.y, CAMERA_OFFSET *Math::Cos(Degree(cameraAngle)));
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
        GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_STATISTICS, GuiManager::GUI_MSG_TXT_CHANGED, GuiImageset::GUI_TEXTVALUE_STAT_CUR_FPS  , (void*)strBuf.str().c_str());
        strBuf.rdbuf()->str(""); // delete stringstream buffer.
        strBuf << std::fixed << std::setprecision(1) << stats.bestFPS;
        GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_STATISTICS, GuiManager::GUI_MSG_TXT_CHANGED, GuiImageset::GUI_TEXTVALUE_STAT_BEST_FPS , (void*)strBuf.str().c_str());
        strBuf.rdbuf()->str(""); // delete stringstream buffer.
        strBuf << std::fixed << std::setprecision(1) << stats.worstFPS;
        GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_STATISTICS, GuiManager::GUI_MSG_TXT_CHANGED, GuiImageset::GUI_TEXTVALUE_STAT_WORST_FPS, (void*)strBuf.str().c_str());
        strBuf.rdbuf()->str(""); // delete stringstream buffer.
        strBuf << std::fixed << std::setprecision(1) << stats.triangleCount;
        GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_STATISTICS, GuiManager::GUI_MSG_TXT_CHANGED, GuiImageset::GUI_TEXTVALUE_STAT_SUM_TRIS , (void*)strBuf.str().c_str());
        skipFrames = 10;
    }
    return true;
}
