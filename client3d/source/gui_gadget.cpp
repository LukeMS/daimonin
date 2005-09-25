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
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_gadget.h"
#include "gui_textout.h"
#include "logger.h"

///=================================================================================================
/// Parse a gadget entry.
///=================================================================================================
GuiGadget::GuiGadget(TiXmlElement *xmlElem)
{
  TiXmlElement *xmlGadget;
  std::string strValue;
  mState = 0;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gadget.
  /////////////////////////////////////////////////////////////////////////
  mStrName = xmlElem->Attribute("ID");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Behavior.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Behavior")))
  {
    mBehavior = xmlGadget->Attribute("Type");
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the position.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Pos")))
  {
    mX = atoi(xmlGadget->Attribute("X"));
    mY = atoi(xmlGadget->Attribute("Y"));
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the label.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Label")))
  {
    mLabelXPos = atoi(xmlGadget->Attribute("XPos"));
    mLabelYPos = atoi(xmlGadget->Attribute("YPos"));
    mLabelColor[0]= (unsigned char) atoi(xmlGadget->Attribute("Red"));
    mLabelColor[1]= (unsigned char) atoi(xmlGadget->Attribute("Green"));
    mLabelColor[2]= (unsigned char) atoi(xmlGadget->Attribute("Blue"));
    mStrLabel  = xmlGadget->Attribute("Text");
  }
}

///=================================================================================================
/// .
///=================================================================================================
void GuiGadget::setStateImagePos(std::string name, int x, int y)
{
  int state = -1;
  if      (name == "Standard") state = STATE_STANDARD;
  else if (name == "Pushed"  ) state = STATE_PUSHED;
  else if (name == "M_Over"  ) state = STATE_M_OVER;
  if (state < 0)
  {
    Logger::log().error() << "Gadget '" << mStrName << "' has no State '" << "' " << name;
    return;
  }
  gfxSrcPos[state].x = x;
  gfxSrcPos[state].y = y;
  //  Logger::log().info() << "2: " << gfxSrcPos[state].x << " "<< gfxSrcPos[state].y << " "<<mWidth << " "<< mHeight;
}

///=================================================================================================
/// .
///=================================================================================================
void GuiGadget::draw(PixelBox &mSrcPixelBox, Texture *texture)
{
  /////////////////////////////////////////////////////////////////////////
  /// Draw gaget.
  /////////////////////////////////////////////////////////////////////////
  PixelBox src = mSrcPixelBox.getSubVolume(Box(
                   gfxSrcPos[mState].x,
                   gfxSrcPos[mState].y,
                   gfxSrcPos[mState].x + mWidth,
                   gfxSrcPos[mState].y + mHeight));
  //  Logger::log().info() << "dest: " << gfxSrcPos[mState].x << " "<< gfxSrcPos[mState].y << " "<<mWidth << " "<< mHeight;
  texture->getBuffer()->blitFromMemory(src, Box(mX, mY, mX + mWidth, mY + mHeight));
  /////////////////////////////////////////////////////////////////////////
  /// Draw label.
  /////////////////////////////////////////////////////////////////////////
  if (mState == STATE_PUSHED)
  {
    GuiTextout::getSingleton().Print(mX+mLabelXPos+2, mY+mLabelYPos+2, texture, mStrLabel.c_str(), COLOR_BLACK);
    GuiTextout::getSingleton().Print(mX+mLabelXPos+1, mY+mLabelYPos+1, texture, mStrLabel.c_str(), COLOR_WHITE);
  }
  else
  {
    GuiTextout::getSingleton().Print(mX+mLabelXPos+1, mY+mLabelYPos+1, texture, mStrLabel.c_str(), COLOR_BLACK);
    GuiTextout::getSingleton().Print(mX+mLabelXPos  , mY+mLabelYPos  , texture, mStrLabel.c_str(), COLOR_WHITE);
  }
}

