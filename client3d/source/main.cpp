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

#include <string>
#include "logger.h"
#include "events.h"
#include "option.h"

const int SUM_MIPMAPS = 0;

///================================================================================================
/// Parse the command line.
///================================================================================================
bool parseCmdLine(const char *cmd, const char *value)
{
  if (cmd[0] == '-')
  {
    if      ((cmd[1] == 'l' || !stricmp(cmd, "--list"  )) && !stricmp(value, "gui"))
    {
      Logger::log().info() << "You told me to list all interactive gui-elements.";
      Option::getSingleton().setListGuiElements(true);
      return true;
    }
    else if ((cmd[1] == 'c' || !stricmp(cmd, "--create")) && !stricmp(value, "rawfonts"))
    {
      Logger::log().info() << "You told me to create a raw-font from every ttf." << FILE_GUI_WINDOWS;
      Option::getSingleton().setCreateRawFonts(true);
      return true;
    }
    else if ((cmd[1] == 'c' || !stricmp(cmd, "--create")) && !stricmp(value, "tileTextures"))
    {
      Logger::log().info() << "You told me to create all textures for the TileEngine.";
      Option::getSingleton().setCreateTileTextures(true);
      return true;
    }
    else if ((cmd[1] == 's' || !stricmp(cmd, "--server")))
    {
      Logger::log().info() << "You told me to use Server " << value;
      return true;
    }
    else if ((cmd[1] == 'p' || !stricmp(cmd, "--port"  )))
    {
      Logger::log().info() << "You told me to connect on port " << value;
      return true;
    }
  }
  cout << "\nusage:\n"
  << "--list gui              -l gui\n"
  << "--create rawFonts       -c rawFonts\n"
  << "--create tileTextures   -c tileTextures\n"
  << "--server <name>         -s <name>\n"
  << "--port   <num>          -p <num>\n";
  return false;
}

///================================================================================================
/// Define the source of resources (other than current folder)
///================================================================================================
void setupResources(void)
{
  /// Load resource paths from config file
  ConfigFile cf;
  cf.load("resources.cfg");
  /// Go through all sections & settings in the file
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

///================================================================================================
/// Write the excepition into the logfile.
///================================================================================================
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

///================================================================================================
/// Entry point.
///================================================================================================
int main(int argc, char *argv[])
{
  Logger::log().headline("Init Logfile");
  Option::getSingleton().setGameStatus(GAME_STATUS_CHECK_HARDWARE);
  Logger::log().headline("Parse CmdLine");
  while (--argc)
  {
    if (!parseCmdLine(argv[argc], argv[argc--])) return 0;
  }
  Root *root;
  RenderWindow *window;
  try
  {
    root = new Root();
    setupResources();
    /// ////////////////////////////////////////////////////////////////////
    /// Show the configuration dialog and initialise the system
    /// You can skip this and use root.restoreConfig() to load configuration
    /// settings if you were sure there are valid ones saved in ogre.cfg
    /// ////////////////////////////////////////////////////////////////////
    if(!root->showConfigDialog()) return 0;

    /// ////////////////////////////////////////////////////////////////////
    /// Get the SceneManager, in this case a generic one
    /// ////////////////////////////////////////////////////////////////////
    window   = root->initialise(true, "Daimonon - Client3d");
  }
  catch(Exception& e)
  {
    LogException(e);
    return 0;
  }
  /// ////////////////////////////////////////////////////////////////////
  /// Check for GFX-Hardware.
  /// ////////////////////////////////////////////////////////////////////
  if (!root->getRenderSystem()->getCapabilities()->hasCapability(RSC_VBO))
    Logger::log().error() << "Your gfx-card doesn't support hardware vertex/index buffer!";
  TexturePtr mTexture;
  try
  { /// try to create a 64MB texture in Video Ram.
    mTexture = TextureManager::getSingleton().createManual("test", "General",
               TEX_TYPE_2D, 4096, 4096, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
    //               TEX_TYPE_2D, 112048, 112048, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
    mTexture.getPointer()->unload();
    mTexture.setNull();
    Option::getSingleton().setHighTextureDetails(true);
  }
  catch( Exception& )
  {
    mTexture.setNull();
    Logger::log().warning() << "You are using an outdated videocard, or the device driver is broken!";
    Logger::log().warning() << "High texture details and large ttf-fonts support will be disabled.";
    Option::getSingleton().setHighTextureDetails(false);
  }
  try
  {
    /// ////////////////////////////////////////////////////////////////////
    /// Set default mipmap level (NB some APIs ignore this)
    /// ////////////////////////////////////////////////////////////////////
    //TextureManager::getSingleton().setDefaultNumMipmaps(SUM_MIPMAPS);

    /// ////////////////////////////////////////////////////////////////////
    /// Optional override method where you can perform resource group loading
    /// Must at least do initialiseAllResourceGroups();
    /// ////////////////////////////////////////////////////////////////////
    ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    Event= new CEvent(window, root->getSceneManager(ST_GENERIC));
    root->addFrameListener(Event);
    root->startRendering();
    /// ////////////////////////////////////////////////////////////////////
    /// End of mainloop -> Clean up.
    /// ////////////////////////////////////////////////////////////////////
    if (Event) delete Event;
    if (root) delete root;
  }
  catch(Exception& e)
  {
    LogException(e);
  }
  return 0;
}
