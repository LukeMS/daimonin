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

#include <Ogre.h>
#include "logger.h"
#include "events.h"
#include "option.h"

const int SUM_MIPMAPS = 0;

//================================================================================================
// Parse the command line.
//================================================================================================
bool parseCmdLine(const char *cmd, const char *value)
{
    int options =0;
    if (cmd[0] == '-')
    {
        if ((cmd[1] == 'l' || !stricmp(cmd, "--list")) && !stricmp(value, "gui"))
        {
            Logger::log().info() << "You told me to list all interactive gui-elements.";
            Option::getSingleton().setIntValue(Option::CMDLINE_LOG_GUI_ELEMENTS, true);
            ++options;
        }
        if ((cmd[1] == 'c' || !stricmp(cmd, "--create")) && !stricmp(value, "rawfonts"))
        {
            Logger::log().info() << "You told me to create a raw-font from every ttf." << FILE_GUI_WINDOWS;
            Option::getSingleton().setIntValue(Option::CMDLINE_CREATE_RAW_FONTS, true);
            ++options;
        }
        if ((cmd[1] == 'c' || !stricmp(cmd, "--create")) && !stricmp(value, "tileTextures"))
        {
            Logger::log().info() << "You told me to create all textures for the TileEngine.";
            Option::getSingleton().setIntValue(Option::CMDLINE_CREATE_TILE_TEXTURES, true);
            ++options;
        }
        if ((cmd[1] == 's' || !stricmp(cmd, "--server")))
        {
            Logger::log().info() << "You told me to use Server " << value;
            Option::getSingleton().setStrValue(Option::CMDLINE_SERVER_NAME, value);
            ++options;
        }
        if ((cmd[1] == 'p' || !stricmp(cmd, "--port")))
        {
            Logger::log().info() << "You told me to connect on port " << value;
            Option::getSingleton().setStrValue(Option::CMDLINE_SERVER_PORT, value);
            ++options;
        }
        if ((cmd[1] == 'f' || !stricmp(cmd, "--fallback")))
        {
            Logger::log().info() << "You told me to disable the TileEngine.";
            Option::getSingleton().setIntValue(Option::CMDLINE_FALLBACK, true);
            ++options;
        }
        if ((cmd[1] == 'x' || !stricmp(cmd, "--sound off")))
        {
            Logger::log().info() << "You told me to disable the Sound.";
            Option::getSingleton().setIntValue(Option::CMDLINE_OFF_SOUND, true);
            ++options;
        }
        if (!stricmp(cmd, "--bbox"))
        {
            Logger::log().info() << "You told me to display bounding-boxes.";
            Option::getSingleton().setIntValue(Option::CMDLINE_SHOW_BOUNDING_BOX, true);
            ++options;
        }
    }
    if (!options)
    {
        cout << "\nusage:\n"
        << "--list gui              -l  gui\n"
        << "--create rawFonts       -c  rawFonts\n"
        << "--create tileTextures   -c  tileTextures\n"
        << "--server <name>         -s  <name>\n"
        << "--port   <num>          -p  <num>\n"
        << "--fallback              -f  disable TileEngine\n"
        << "--sound off             -x  disable Sound\n"
        << "--bbox                      show bounding-boxes\n";
        return false;
    }
    return true;
}

//================================================================================================
// Define the source of resources (other than current folder)
//================================================================================================
void setupResources(void)
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
// Write the excepition into the logfile.
//================================================================================================
void LogException(Exception& e)
{
    std::string s = e.getFullDescription();
    size_t found = s.find('\n');
    while (found != std::string::npos)
    {
        s.replace(found, 1, "<br>\n");
        found = s.find('\n', found+6);
    }
    Logger::log().error() << s;
}

//================================================================================================
// Entry point.
//================================================================================================
int main(int argc, char **argv)
{
    Logger::log().headline("Init Logfile");
    Option::getSingleton().setGameStatus(Option::GAME_STATUS_CHECK_HARDWARE);
    Logger::log().headline("Parse CmdLine");
    while (--argc)
    {
        if (argc > 1)
        {
            if (!parseCmdLine(argv[--argc], argv[argc]))
                return 0;
        }
        else
        {
            if (!parseCmdLine(argv[argc], "0"))
                return 0;
        }
    }
    Root *root =0;
    RenderWindow *window=0;
    try
    {
        root = new Root();
        setupResources();
        // ////////////////////////////////////////////////////////////////////
        // Show the configuration dialog and initialise the system
        // You can skip this and use root.restoreConfig() to load configuration
        // settings if you were sure there are valid ones saved in ogre.cfg
        // ////////////////////////////////////////////////////////////////////
        if (!root->showConfigDialog())
        {
            delete root;
            return 0;
        }
        // ////////////////////////////////////////////////////////////////////
        // Get the SceneManager, in this case a generic one
        // ////////////////////////////////////////////////////////////////////
        window   = root->initialise(true, PRG_NAME);
    }
    catch (Exception& e)
    {
        LogException(e);
        return 0;
    }

    // ////////////////////////////////////////////////////////////////////
    // Check for GFX-Hardware.
    // ////////////////////////////////////////////////////////////////////
    if (!root->getRenderSystem()->getCapabilities()->hasCapability(RSC_VBO))
        Logger::log().error() << "Your gfx-card doesn't support hardware vertex/index buffer!";
    TexturePtr mTexture, mTextur2;

    Option::getSingleton().setIntValue(Option::HIGH_TEXTURE_DETAILS, true);
    Option::getSingleton().setIntValue(Option::HIGH_TILES_DETAILS, true);
    /*
        try
        { // try to create a 64MB texture in Video Ram.
            mTexture = TextureManager::getSingleton().createManual("64 MB", "General",  TEX_TYPE_2D, 4096, 4096, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
            mTexture.getPointer()->unload();
            mTexture.setNull();
            Option::getSingleton().setIntValue(Option::HIGH_TEXTURE_DETAILS, true);
        }
        catch( Exception& )
        {
            mTexture.setNull();
            try
            { // try to create a 32MB texture in Video Ram.
                mTexture = TextureManager::getSingleton().createManual("16MB nr. 1", "General",  TEX_TYPE_2D, 2048, 2048, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
                mTextur2 = TextureManager::getSingleton().createManual("16MB nr. 2", "General",  TEX_TYPE_2D, 2048, 2048, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
                mTexture.getPointer()->unload();
                mTextur2.getPointer()->unload();
                mTexture.setNull();
                mTextur2.setNull();
                Option::getSingleton().setIntValue(Option::HIGH_TILES_DETAILS, true);
            }
            catch( Exception& )
            {
                Logger::log().warning() << "Your gfx-card seems to have less than 64 MB";
                Logger::log().warning() << "Switching to minimal Details.";
                Option::getSingleton().setIntValue(Option::HIGH_TEXTURE_DETAILS, false);
                Option::getSingleton().setIntValue(Option::HIGH_TILES_DETAILS, false);
            }
            Logger::log().warning() << "Your gfx-card seems to only have 64 MB";
            Logger::log().warning() << "High texture details and large ttf-fonts support will be disabled.";
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
        ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        Event= new CEvent(window, root->createSceneManager(ST_GENERIC, "RefAppSMInstance"));
        root->addFrameListener(Event);
        root->startRendering();
        // ////////////////////////////////////////////////////////////////////
        // End of mainloop -> Clean up.
        // ////////////////////////////////////////////////////////////////////
        if (Event)
            delete Event;
        if (root)
            delete root;
    }
    catch (Exception& e)
    {
        LogException(e);
    }
    return 0;
}


