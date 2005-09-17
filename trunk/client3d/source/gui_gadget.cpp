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

#include <tinyxml.h>
#include "define.h"
#include "gui_gadget.h"
#include "logger.h"

///=================================================================================================
/// .
///=================================================================================================
void GuiGadget::getPos(int &x1, int &y1, int &x2, int &y2)
{
  x1 = mX1;
  y1 = mY1;
  x2 = mX2;
  y2 = mY2;
}

///=================================================================================================
/// Parse the nth gadget entry.
///=================================================================================================
GuiGadget::GuiGadget(const char *descFile, int pos)
{
  TiXmlElement *xmlRoot, *xmlElem, *xmlGadget;
  TiXmlDocument doc(descFile);
  std::string strValue;
  mState = 0;
  /////////////////////////////////////////////////////////////////////////
  /// Check for a working window description.
  /////////////////////////////////////////////////////////////////////////
  if ( !doc.LoadFile(descFile) || !(xmlRoot = doc.RootElement()) )
  {
    Logger::log().error() << "GUIGadget: XML-File '" << descFile << "' is missing or broken.";
    return;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Coordinates.
  /////////////////////////////////////////////////////////////////////////
  xmlElem = xmlRoot->FirstChildElement("Gadget");
  while (--pos > -1) xmlElem = xmlElem->NextSiblingElement("Gadget");
  mName = xmlElem->Attribute("ID");
  xmlGadget = xmlElem->FirstChildElement("Dimension");
  if (xmlGadget)
  {
    mX1 = atoi(xmlGadget->Attribute("X"));
    mY1 = atoi(xmlGadget->Attribute("Y"));
    mX2 = atoi(xmlGadget->Attribute("Width" )) + mX1;
    mY2 = atoi(xmlGadget->Attribute("Height")) + mY1;
  }
}
