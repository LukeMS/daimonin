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

#include "define.h"
#include "gui_manager.h"
#include "gui_window.h"
#include "gui_gadget.h"
#include "gui_cursor.h"
#include "option.h"
#include "logger.h"
#include <Ogre.h>
#include <OgreFontManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <tinyxml.h>

using namespace Ogre;

GuiManager::_GuiElementNames GuiManager::GuiWindowNames[GUI_WIN_SUM]=
  {
    {"Statistics",    GUI_WIN_STATISTICS
    },
    {"PlayerInfo",    GUI_WIN_PLAYERINFO
    },
    { "Requester",    GUI_WIN_REQUESTER
    }
  };

GuiManager::_GuiElementNames GuiManager::GuiElementNames[GUI_ELEMENTS_SUM]=
  {
    // Buttons.
    { "ButtonClose",   GUI_BUTTON_CLOSE
    },
    { "ButtonOK",      GUI_BUTTON_OK
    },
    { "ButtonCancel",  GUI_BUTTON_CANCEL
    },
    { "ButtonMin",     GUI_BUTTON_MINIMIZE
    },
    { "ButtonMax",     GUI_BUTTON_MAXIMIZE
    },
    // Listboxes.
    { "Requester",    GUI_LIST_UP
    },
    { "Requester",    GUI_LIST_DOWN
    },
    { "Requester",    GUI_LIST_LEFT
    },
    { "Requester",    GUI_LIST_RIGHT
    },
    // TextValues.
    { "currentFPS",   GUI_TEXTVALUE_STAT_CUR_FPS
    },
    { "bestFPS",      GUI_TEXTVALUE_STAT_BEST_FPS
    },
    { "worstFPS",     GUI_TEXTVALUE_STAT_WORST_FPS
    },
    { "sumTris",      GUI_TEXTVALUE_STAT_SUM_TRIS
    }
  };


///=================================================================================================
/// .
///=================================================================================================
void GuiManager::Init(const char *XML_imageset_file, const char *XML_windows_file, int w, int h)
{
  Logger::log().headline("Init GUI");
  mScreenWidth   = w;
  mScreenHeight  = h;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gfx datas from the imageset.
  /////////////////////////////////////////////////////////////////////////
  if (!parseImagesetData(XML_imageset_file)) return;
  mImageSetImg.load(mStrImageSetGfxFile, "General");
  mSrcPixelBox = mImageSetImg.getPixelBox();
  /////////////////////////////////////////////////////////////////////////
  /// Parse the windows datas.
  /////////////////////////////////////////////////////////////////////////
  guiWindow = new GuiWindow[GUI_WIN_SUM];
  if (!parseWindowsData( XML_windows_file)) return;
}

///=================================================================================================
/// .
///=================================================================================================
void GuiManager::freeRecources()
{
  GuiCursor::getSingleton().freeRecources();
}

///=================================================================================================
/// .
///=================================================================================================
void GuiManager::keyEvent(const char keyChar, const unsigned char key)
{
  //    TextInput::getSingleton().keyEvent(e->getKeyChar(), e->getKey());
}

///=================================================================================================
/// Read the gfx-data for the given gadget.
///=================================================================================================
bool GuiManager::parseImagesetData(const char*fileImageSet)
{
  /////////////////////////////////////////////////////////////////////////
  /// Check for a working description file.
  /////////////////////////////////////////////////////////////////////////
  TiXmlElement *xmlRoot, *xmlElem, *xmlState;
  TiXmlDocument doc(fileImageSet);
  if (!doc.LoadFile(fileImageSet) || !(xmlRoot = doc.RootElement()) || !xmlRoot->Attribute("file"))
  {
    Logger::log().error() << "XML-File '" << fileImageSet << "' is broken or missing.";
    return false;
  }
  mStrImageSetGfxFile = xmlRoot->Attribute("file");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gfx coordinates.
  /////////////////////////////////////////////////////////////////////////
  const char *strTemp;
  for (xmlElem = xmlRoot->FirstChildElement("Image"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Image"))
  {
    mSrcEntry *Entry = new mSrcEntry;
    strTemp = xmlElem->Attribute("name");
    if (!strTemp)  continue;
    Entry->name   = strTemp;
    Entry->width  = atoi(xmlElem->Attribute("width"));
    Entry->height = atoi(xmlElem->Attribute("height"));
    /////////////////////////////////////////////////////////////////////////
    /// Parse the Position entries.
    /////////////////////////////////////////////////////////////////////////
    for (xmlState = xmlElem->FirstChildElement("State"); xmlState; xmlState = xmlState->NextSiblingElement("State"))
    {
      strTemp = xmlState->Attribute("name");
      if (!strTemp)  continue;
      _state *s = new _state;
      s->name = strTemp;
      s->x = atoi(xmlState->Attribute("posX"));
      s->y = atoi(xmlState->Attribute("posY"));
      Entry->state.push_back(s);
    }
    mvSrcEntry.push_back(Entry);
  }
  return true;
}

///=================================================================================================
/// Parse the cursor and windows data.
///=================================================================================================
bool GuiManager::parseWindowsData(const char *fileWindows)
{
  TiXmlElement *xmlRoot, *xmlElem;
  TiXmlDocument doc(fileWindows);
  const char *valString;
  /////////////////////////////////////////////////////////////////////////
  /// Check for a working window description.
  /////////////////////////////////////////////////////////////////////////
  if ( !doc.LoadFile(fileWindows) || !(xmlRoot = doc.RootElement()) )
  {
    Logger::log().error() << "XML-File '" << fileWindows << "' is missing or broken.";
    return false;
  }
  if ((valString = xmlRoot->Attribute("name")))
    Logger::log().info() << "Parsing '" << valString << "' in file" << fileWindows << ".";
  else
    Logger::log().error() << "File '" << fileWindows << "' has no name entry.";
  /////////////////////////////////////////////////////////////////////////
  /// Parse the mouse-cursor.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlElem = xmlRoot->FirstChildElement("Cursor")) && ((valString = xmlElem->Attribute("name"))))
  {
    mSrcEntry *srcEntry = getStateGfxPositions(valString);
    GuiCursor::getSingleton().Init(srcEntry->width, srcEntry->height, mScreenWidth, mScreenHeight);
    for (unsigned int i=0; i < srcEntry->state.size(); ++i)
    {
      GuiCursor::getSingleton().setStateImagePos(srcEntry->state[i]->name, srcEntry->state[i]->x, srcEntry->state[i]->y);
    }
    GuiCursor::getSingleton().draw(mSrcPixelBox);
  }
  else
  { // Create a dummy mouse-cursor.
    Logger::log().error() << "File '" << fileWindows << "' has no mouse-cursor defined.";
    GuiCursor::getSingleton().Init(32, 32, mScreenWidth, mScreenHeight);
    GuiCursor::getSingleton().setStateImagePos("Standard", 128, 128);
    GuiCursor::getSingleton().draw(mSrcPixelBox);
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the windows.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Window"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Window"))
  {
    if (!(valString = xmlElem->Attribute("name"))) continue;
    for (int i = 0; i < GUI_WIN_SUM; ++i)
    {
      if (GuiWindowNames[i].name == valString)
      {
        guiWindow[i].Init(xmlElem, this);
        break;
      }
    }
  }
  return true;
}

///=================================================================================================
/// .
///=================================================================================================
struct GuiManager::mSrcEntry *GuiManager::getStateGfxPositions(const char* guiImage)
{
  if (guiImage)
  {
    for (unsigned int j = 0; j < mvSrcEntry.size(); ++j)
    {
      if (!stricmp(guiImage, mvSrcEntry[j]->name.c_str())) return mvSrcEntry[j];
    }
  }
  return 0;
}

///=================================================================================================
/// .
///=================================================================================================
const char *GuiManager::mouseEvent(int mouseAction, Real rx, Real ry)
{
  GuiCursor::getSingleton().setPos(rx, ry);
  rx *= mScreenWidth;
  ry *= mScreenHeight;
  const char *actGadgetName;

  for (unsigned int i=0; i < GUI_WIN_SUM; ++i)
  {
    actGadgetName = guiWindow[i].mouseEvent(mouseAction, (int)rx, (int)ry);
    if (actGadgetName)
    {
      static char buffer[100];
      sprintf(buffer, "%s (%s)",  actGadgetName, guiWindow[i].getName());
      return buffer;
    }
  }

  return 0;
}

///=================================================================================================
/// Send a message to a GuiWindow.
///=================================================================================================
void GuiManager::sendMessage(int window, int message, int element, const char *value)
{
  guiWindow[window].Message(message, element, value);
}
