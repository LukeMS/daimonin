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
#include "gui_textout.h"
#include "option.h"
#include "logger.h"
#include <Ogre.h>
#include <OgreFontManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <tinyxml.h>

using namespace Ogre;

GuiManager::GuiManager(const char *XML_imageset_file, const char *XML_windows_file, int w, int h)
{
  mScreenWidth   = w;
  mScreenHeight  = h;
  mMouseDragging = -1;
  mMousePressed  = -1;
  mMouseOver     = -1;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gfx datas from the imageset.
  /////////////////////////////////////////////////////////////////////////
  if (!parseImagesetData(XML_imageset_file)) return;
  mImageSetImg.load(mStrImageSetGfxFile, "General");
  mSrcPixelBox = mImageSetImg.getPixelBox();
  /////////////////////////////////////////////////////////////////////////
  /// Parse the windows datas.
  /////////////////////////////////////////////////////////////////////////
  if (!parseWindowsData( XML_windows_file)) return;
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
  if (!doc.LoadFile(fileImageSet) || !(xmlRoot = doc.RootElement()) || !xmlRoot->Attribute("File"))
  {
    Logger::log().error() << "XML-File '" << fileImageSet << "' is broken or missing.";
    return false;
  }
  mStrImageSetGfxFile = xmlRoot->Attribute("File");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gfx coordinates.
  /////////////////////////////////////////////////////////////////////////
  std::string strTmp;
  for (xmlElem = xmlRoot->FirstChildElement("Image"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Image"))
  {
    mSrcEntry *Entry = new mSrcEntry;
    if (!xmlElem->Attribute("ID"))  continue;
    Entry->name   = xmlElem->Attribute("ID");
    Entry->width  = atoi(xmlElem->Attribute("Width"));
    Entry->height = atoi(xmlElem->Attribute("Height"));
    /////////////////////////////////////////////////////////////////////////
    /// Parse the Position entries.
    /////////////////////////////////////////////////////////////////////////
    for (xmlState = xmlElem->FirstChildElement("State"); xmlState; xmlState = xmlState->NextSiblingElement("State"))
    {
      if (!(xmlState->Attribute("ID"))) continue;
      _state *s = new _state;
      s->name = xmlState->Attribute("ID");
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
  if ((valString = xmlRoot->Attribute("ID")))
    Logger::log().info() << "Parsing '" << valString << "' in file" << fileWindows << ".";
  else
    Logger::log().error() << "File '" << fileWindows << "' has no ID entry.";
  /////////////////////////////////////////////////////////////////////////
  /// Parse the mouse-cursor.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlElem = xmlRoot->FirstChildElement("Cursor")))
  {
    if ((valString = xmlElem->Attribute("ID")))
    {
      mSrcEntry *srcEntry = getStateGfxPositions(valString);
      mMousecursor = new GuiCursor(srcEntry->width, srcEntry->height, mScreenWidth, mScreenHeight, mSrcPixelBox);
      for (unsigned int i=0; i < srcEntry->state.size(); ++i)
      {
        mMousecursor->setStateImagePos(srcEntry->state[i]->name, srcEntry->state[i]->x, srcEntry->state[i]->y);
      }
      mMousecursor->draw();
    }
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the windows.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Window"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Window"))
  {
    if (!xmlElem->Attribute("ID")) continue;
    GuiWindow *window = new GuiWindow(xmlElem, this);
    mvWindow.push_back(window);
  }
  return true;
}

///=================================================================================================
/// .
///=================================================================================================
struct GuiManager::mSrcEntry *GuiManager::getStateGfxPositions(const char* guiImage)
{
  for (unsigned int j = 0; j < mvSrcEntry.size(); ++j)
  {
    if (!stricmp(guiImage, mvSrcEntry[j]->name.c_str())) return mvSrcEntry[j];
  }
  return 0;
}

///=================================================================================================
/// .
///=================================================================================================
const char *GuiManager::mouseEvent(int mouseAction, Real rx, Real ry)
{
  const char *actGadgetName;
  for (unsigned int i=0; i < mvWindow.size(); ++i)
  {
    actGadgetName = mvWindow[i]->mouseEvent(mouseAction, rx, ry);
    if (actGadgetName)
    {
      static char buffer[100];
      sprintf(buffer, "%s (%s)",  actGadgetName, mvWindow[i]->getName());
      return buffer;
    }
  }
  return 0;
}
