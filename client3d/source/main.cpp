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
#include <OgreConfigFile.h>
#include <OgreLogManager.h>
#include <OgreRenderWindow.h>
#include <OgreWindowEventUtilities.h>
#include <OgreResourceGroupManager.h>
#include "logger.h"
#include "events.h"
#include "option.h"
#include "profiler.h"

using namespace Ogre;

//================================================================================================
// Parse the command line.
//================================================================================================
static bool parseCmdLine(const char *cmd, const char *value)
{
    int options =0;
    if (cmd[0] == '-')
    {
        if ((cmd[1] == 'g' || !stricmp(cmd, "--guiinfo")))
        {
            Logger::log().info() << Logger::ICON_CLIENT << "You told me to print gui infos.";
            Option::getSingleton().setIntValue(Option::CMDLINE_GUI_INFORMATION, true);
            ++options;
        }
        if ((cmd[1] == 'm' || !stricmp(cmd, "--media")))
        {
            Logger::log().info() << Logger::ICON_CLIENT << "You told me to create the media stuff:";
            Logger::log().info() << Logger::ICON_CLIENT << "- Item Atlastexture";
            Logger::log().info() << Logger::ICON_CLIENT << "- Tile Atlastexture";
            Logger::log().info() << Logger::ICON_CLIENT << "- Raw fonts";
            Option::getSingleton().setIntValue(Option::CMDLINE_CREATE_MEDIA, true);
            ++options;
        }
        if ((cmd[1] == 's' || !stricmp(cmd, "--server")))
        {
            Logger::log().info() << Logger::ICON_CLIENT << "You told me to use Server " << value;
            Option::getSingleton().setStrValue(Option::CMDLINE_SERVER_NAME, value);
            ++options;
        }
        if ((cmd[1] == 'p' || !stricmp(cmd, "--port")))
        {
            Logger::log().info() << Logger::ICON_CLIENT << "You told me to connect on port " << value;
            Option::getSingleton().setStrValue(Option::CMDLINE_SERVER_PORT, value);
            ++options;
        }
        if ((cmd[1] == 'x' || !stricmp(cmd, "--sound off")))
        {
            Logger::log().info() << Logger::ICON_CLIENT << "You told me to disable the Sound.";
            Option::getSingleton().setIntValue(Option::CMDLINE_OFF_SOUND, true);
            ++options;
        }
        if ((cmd[1] == 'd' || !stricmp(cmd, "--device")))
        {
            Logger::log().info() << Logger::ICON_CLIENT << "You told me to use Sound device #" << value;
            Option::getSingleton().setIntValue(Option::CMDLINE_SND_DEVICE, atoi(value));
            ++options;
        }
        if (!stricmp(cmd, "--bbox"))
        {
            Logger::log().info() << Logger::ICON_CLIENT << "You told me to display bounding-boxes.";
            Option::getSingleton().setIntValue(Option::CMDLINE_SHOW_BOUNDING_BOX, true);
            ++options;
        }
        if (!stricmp(cmd, "--lod"))
        {
            Logger::log().info() << Logger::ICON_CLIENT << "You told me to to set LoD " << value << " for the TileEngine.";
            Option::getSingleton().setIntValue(Option::CMDLINE_TILEENGINE_LOD, atoi(value));
            ++options;
        }
    }
    if (!options)
    {
        std::cout << "\nusage:\n"
                  << "--guiinfo               -g        Prints some informations about the gui\n"
                  << "--media                 -m        Create atlastextures and raw-fonts\n"
                  << "--server <name>         -s <name> Manually select a server\n"
                  << "--port   <num>          -p <num>  Manually select a server port\n"
                  << "--lod    <num>                    Set LoD for the TileEngine\n"
                  << "--sound off             -x        Disable Sound\n"
                  << "--device <num>          -d <num>  Choose a Sound-device\n"
                  << "--bbox                            Show bounding-boxes\n"
                  << std::endl;
        return false;
    }
    return true;
}

//================================================================================================
// Define the source of resources (other than current folder)
//================================================================================================
static void setupResources(void)
{
    // Load resource paths from config file
    ConfigFile cf;
    cf.load("resources.cfg");
    // Go through all sections & settings in the file
    ConfigFile::SectionIterator seci = cf.getSectionIterator();
    String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        ConfigFile::SettingsMultiMap *settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
}

//================================================================================================
// Class to overwrite the standard Ogre::LogListener.
//================================================================================================
class NewLogListener: public Ogre::LogListener
{
private:
    LogManager *mOgreLogManager;
    Log *mOgreLog;

public:
    NewLogListener()
    {
        mOgreLogManager = new Ogre::LogManager();
        mOgreLog = mOgreLogManager->createLog("Ogre.log", false, true, true);
        mOgreLog->addListener(this);
    }
    ~NewLogListener()
    {
        mOgreLog->removeListener(this);
    }
    void messageLogged(const String &message, LogMessageLevel lml, bool /*maskDebug*/, const String &/*logName*/)
    {
        if (lml == LML_CRITICAL)
            Logger::log().error() << Logger::ICON_OGRE3D << message;
        else
        {
            if (message.substr(0,8) =="WARNING:")
                Logger::log().warning()  << Logger::ICON_OGRE3D << message.substr(9);
            else
                Logger::log().info()  << Logger::ICON_OGRE3D << message;
        }
    }
};

//================================================================================================
// Write the excepition into the logfile.
//================================================================================================
static void LogException(Exception& e)
{
    std::string s = e.getFullDescription();
    size_t found = s.find('\n');
    while (found != std::string::npos)
    {
        s.replace(found, 1, "<br>\n");
        found = s.find('\n', found+6);
    }
    Logger::log().error() << Logger::ICON_CLIENT << "<img src='media/textures/logger/ogre.png'>" << s;
}

//================================================================================================
// Entry point.
//================================================================================================
int main(int argc, char **argv)
{
    // ////////////////////////////////////////////////////////////////////
    // Make the run button of visualC work.
    // ////////////////////////////////////////////////////////////////////
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
    TCHAR tchBuffer[MAX_PATH + 1];
    if (GetCurrentDirectory(MAX_PATH, tchBuffer) >0)
    {
        String strPath = tchBuffer;
        // If the client was started from within visualC, set the correct path.
        if (StringUtil::endsWith(strPath, "visualc", true))
            SetCurrentDirectory("..\\..\\..\\");
    }
#endif
    Logger::log().headline() << "Init Logfile";
    Option::getSingleton().setGameStatus(Option::GAME_STATUS_CHECK_HARDWARE);
    Logger::log().headline() << "Parse CmdLine";
    while (--argc)
    {
        if (argc > 1)
        {
            if (!parseCmdLine(argv[argc-1], argv[argc]))
            {
                Logger::log().error() << Logger::ICON_CLIENT << "Unknown cmdline " << argv[argc-1] << " " << argv[argc];
                return 0;
            }
            --argc;
        }
        else
        {
            if (!parseCmdLine(argv[argc], "0"))
            {
                Logger::log().error() << Logger::ICON_CLIENT << "Unknown cmdline " << argv[argc-1];
                return 0;
            }
        }
    }
    Root *root =0;
    RenderWindow *window=0;
    Logger::log().headline() << "Init Ogre";
    NewLogListener *logListener = new NewLogListener; // Must be done before creating Ogre::Root.
    try
    {
        root = OGRE_NEW Root("", "ogre.cfg");
        String debugPostfix ="";
#ifdef _DEBUG
        debugPostfix = "_d";
#endif
        String pluginFolder = PLUGIN_FOLDER;
        pluginFolder+= "/";
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        root->loadPlugin(pluginFolder + "RenderSystem_Direct3D9" + debugPostfix);
#endif
        root->loadPlugin(pluginFolder + "RenderSystem_GL"        + debugPostfix);
        root->loadPlugin(pluginFolder + "Plugin_ParticleFX"      + debugPostfix);
        root->loadPlugin(pluginFolder + "Plugin_CgProgramManager"+ debugPostfix);
        ConfigFile cf;
        cf.load("resources.cfg");
        setupResources();
        // ////////////////////////////////////////////////////////////////////
        // Show the configuration dialog and initialise the system.
        // You can skip this and use root.restoreConfig() to load configuration
        // settings if you were sure there are valid ones saved in ogre.cfg.
        // ////////////////////////////////////////////////////////////////////
        if (!root->showConfigDialog())
        {
            Logger::log().info() << Logger::ICON_CLIENT << "User break in ConfigDialog";
            delete logListener;
            OGRE_DELETE root;
            return 0;
        }
        window = root->initialise(true, "Daimonin Ogre3d Client");
    }
    catch (Exception &e)
    {
        LogException(e);
        delete logListener;
        OGRE_DELETE root;
        return 0;
    }
    // ////////////////////////////////////////////////////////////////////
    // Check for GFX-Hardware.
    // ////////////////////////////////////////////////////////////////////
    if (!root->getRenderSystem()->getCapabilities()->hasCapability(RSC_FRAGMENT_PROGRAM)
            || (!root->getRenderSystem()->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM)))
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Your gfx-card doesn't support shader programs!";
        Option::getSingleton().setIntValue(Option::ERROR_NO_SHADRES, true);
    }
    TexturePtr mTexture, mTextur2;
    Option::getSingleton().setIntValue(Option::HIGH_TEXTURE_DETAILS, true);
    Option::getSingleton().setIntValue(Option::HIGH_TILES_DETAILS, true);
    /*
        try
        {
            // try to create a 64MB texture in Video Ram.
            mTexture = TextureManager::getSingleton().createManual("64 MB", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                       TEX_TYPE_2D, 4096, 4096, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY, 0);
            mTexture->unload();
            mTexture.setNull();
            Option::getSingleton().setIntValue(Option::HIGH_TEXTURE_DETAILS, true);
        }
        catch ( Exception& )
        {
            mTexture.setNull();
            try
            { // try to create a 32MB texture in Video Ram.
                mTexture = TextureManager::getSingleton().createManual("16MB #1", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                           TEX_TYPE_2D, 2048, 2048, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY, 0);
                mTextur2 = TextureManager::getSingleton().createManual("16MB #2", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                           TEX_TYPE_2D, 2048, 2048, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY, 0);
                mTexture->unload();
                mTextur2->unload();
                mTexture.setNull();
                mTextur2.setNull();
                Option::getSingleton().setIntValue(Option::HIGH_TILES_DETAILS, true);
            }
            catch ( Exception& )
            {
                Logger::log().warning() << Logger::ICON_CLIENT << "Your gfx-card seems to have less than 64 MB";
                Logger::log().warning() << Logger::ICON_CLIENT << "Switching to minimal Details.";
                Logger::log().warning() << Logger::ICON_CLIENT << "Can't load TTF's with size > 16 on this gfx-card. You must use raw-fonts instead!";
                Option::getSingleton().setIntValue(Option::HIGH_TEXTURE_DETAILS, false);
                Option::getSingleton().setIntValue(Option::HIGH_TILES_DETAILS, false);
            }
            Logger::log().warning() << Logger::ICON_CLIENT << "Your gfx-card seems to only have 64 MB";
            Logger::log().warning() << Logger::ICON_CLIENT << "High texture details will be disabled.";
            Logger::log().warning() << Logger::ICON_CLIENT << "Can't load TTF's with size > 16 on this gfx-card. You must use raw-fonts instead!";
            Option::getSingleton().setIntValue(Option::HIGH_TEXTURE_DETAILS, false);
        }
    */
    try
    {
        // ////////////////////////////////////////////////////////////////////
        // Set default mipmap level (NB some APIs ignore this)
        // ////////////////////////////////////////////////////////////////////
        //TextureManager::getSingleton().setDefaultNumMipmaps(SUM_MIPMAPS);

        // ////////////////////////////////////////////////////////////////////
        // Optional override method where you can perform resource group loading
        // Must at least do initialiseAllResourceGroups();
        // ////////////////////////////////////////////////////////////////////
        Logger::log().headline() << "Init Ogre Resources";
        ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        Events::getSingleton().Init(window, root->createSceneManager(ST_GENERIC));
        root->addFrameListener(&Events::getSingleton());
        {
            root->startRendering();
        }
        /*
                {
                    // If root->startRendering(); makes trouble on alt+tab use this:
                    root->getRenderSystem()->_initRenderTargets();
                    root->clearEventTimes();
                    while(1)
                    {
                        WindowEventUtilities::messagePump();
                        if(window->isActive())
                        {
                            if (!root->renderOneFrame()) break;
                        }
                        else
                        {
                            root->clearEventTimes();
                        }
                    }
                }
        */
        // ////////////////////////////////////////////////////////////////////
        // End of mainloop -> Clean up.
        // ////////////////////////////////////////////////////////////////////
        Logger::log().headline() << "Shutdown";
        Events::getSingleton().freeRecources();
    }
    catch (Exception& e)
    {
        LogException(e);
    }
    delete logListener;
    OGRE_DELETE root;
    return 0;
}
