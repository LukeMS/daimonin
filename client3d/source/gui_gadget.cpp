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
GuiGadget::GuiGadget(TiXmlElement *xmlElem, int w, int h, int maxX, int maxY)
{
  TiXmlElement *xmlGadget;
  std::string strValue;
  const char *tmp;
  mState = 0;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gadget.
  /////////////////////////////////////////////////////////////////////////
  mStrName = xmlElem->Attribute("name");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Behavior.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Behavior")))
  {
    mBehavior = xmlGadget->Attribute("type");
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the position.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Pos")))
  {
    mX = atoi(xmlGadget->Attribute("x"));
    mY = atoi(xmlGadget->Attribute("y"));
  }
  if (mX > maxX-2) mX = maxX-2;
  if (mY > maxY-2) mY = maxY-2;
  mWidth = w;
  mHeight= h;
  if (mX + mWidth > maxX) mWidth = maxX-mX-1;
  if (mY + mHeight >maxY) mHeight= maxY-mY-1;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the position.
  /////////////////////////////////////////////////////////////////////////
  mMirrorH = mMirrorV = false;
  if ((xmlGadget = xmlElem->FirstChildElement("Mirror")))
  {
    if (!stricmp(xmlGadget->Attribute("horizontal"), "true")) mMirrorH = true;
    if (!stricmp(xmlGadget->Attribute("vertical"  ), "true")) mMirrorV = true;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the label.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Label")))
  {
    mLabelXPos = atoi(xmlGadget->Attribute("xPos"));
    mLabelYPos = atoi(xmlGadget->Attribute("yPos"));
    mLabelFont = atoi(xmlGadget->Attribute("font"));
    mLabelColor[0]= (unsigned char) atoi(xmlGadget->Attribute("red"));
    mLabelColor[1]= (unsigned char) atoi(xmlGadget->Attribute("green"));
    mLabelColor[2]= (unsigned char) atoi(xmlGadget->Attribute("blue"));
    tmp = xmlGadget->Attribute("text");
    if (tmp) mStrLabel  = xmlGadget->Attribute("text");
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Tooltip entry.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Tooltip")))
  {
    tmp = xmlGadget->Attribute("text");
    if (tmp) mStrTooltip = tmp;
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
  if (mStrLabel != "")
  {
    std::string mStrBgLabel = "~#ff000000"+mStrLabel+"~"; // Black Background for the label.
    TextLine label;
    label.index= -1;
    label.font = mLabelFont;
    label.clipped = false;
    if (mState == STATE_PUSHED)
    {
      label.x1 = mX+ mLabelXPos+1;
      label.x2 = label.x1 + mWidth;
      label.y1 = mY+ mLabelYPos+1;
      label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
    }
    else
    {
      label.x1 = mX+ mLabelXPos;
      label.x2 = label.x1 + mWidth;
      label.y1 = mY+ mLabelYPos;
      label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
    }
    GuiTextout::getSingleton().Print(&label, texture, mStrBgLabel.c_str());
    --label.x1;
    --label.y1;
    GuiTextout::getSingleton().Print(&label, texture, mStrLabel.c_str());
  }
}
