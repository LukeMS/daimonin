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

#include "gui_element.h"
#include "logger.h"
#include <Ogre.h>

///================================================================================================
/// Parse a gui element.
///================================================================================================
GuiElement::GuiElement(TiXmlElement *xmlElem, int w, int h, int maxX, int maxY)
{
  TiXmlElement *xmlGadget;
  std::string strValue;
  const char *tmp;
  // Set default values.
  mState = 0;
  mFontNr = 0;
  mWidth = w;
  mHeight= h;

  /// ////////////////////////////////////////////////////////////////////
  /// Parse the element.
  /// ////////////////////////////////////////////////////////////////////
  if ((tmp = xmlElem->Attribute("type"))) mStrType = tmp;
  if ((tmp = xmlElem->Attribute("name"))) mStrName = tmp;
  if ((tmp = xmlElem->Attribute("image_name"))) mStrImageName = tmp;
  if ((tmp = xmlElem->Attribute("font"))) mFontNr  = atoi(tmp);
  /// ////////////////////////////////////////////////////////////////////
  /// Parse the position.
  /// ////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Pos")))
  {
      if ((tmp = xmlGadget->Attribute("x"))) mX = atoi(tmp);
      if ((tmp = xmlGadget->Attribute("y"))) mY = atoi(tmp);
  }
  if (mX > maxX-2) mX = maxX-2;
  if (mY > maxY-2) mY = maxY-2;
  /// ////////////////////////////////////////////////////////////////////
  /// Parse the size (if given).
  /// ////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Range")))
  {
    if ((tmp = xmlGadget->Attribute("width")))  mWidth = atoi(tmp);
    if ((tmp = xmlGadget->Attribute("height"))) mHeight= atoi(tmp);
    mSrcWidth = w;
    mSrcHeight= h;
  }
  if (mX + mWidth > maxX) mWidth = maxX-mX-1;
  if (mY + mHeight >maxY) mHeight= maxY-mY-1;
  /// ////////////////////////////////////////////////////////////////////
  /// Parse the color (if given).
  /// ////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Color")))
  {
    /// PixelFormat: ARGB.
    if ((tmp = xmlGadget->Attribute("red"  ))) mFillColor = atoi(tmp) << 16;
    if ((tmp = xmlGadget->Attribute("green"))) mFillColor+= atoi(tmp) <<  8;
    if ((tmp = xmlGadget->Attribute("blue" ))) mFillColor+= atoi(tmp);
    if ((tmp = xmlGadget->Attribute("alpha"))) mFillColor+= atoi(tmp) << 24;
  }
  /// ////////////////////////////////////////////////////////////////////
  /// Parse the label  (if given).
  /// ////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Label")))
  {
    if ((tmp = xmlGadget->Attribute("xPos")))  mLabelXPos = atoi(tmp);
    if ((tmp = xmlGadget->Attribute("yPos")))  mLabelYPos = atoi(tmp);
    if ((tmp = xmlGadget->Attribute("font")))  mLabelFont = atoi(tmp);
    if ((tmp =  xmlGadget->Attribute("red")))  mLabelColor[0]= (unsigned char) atoi(tmp);
    if ((tmp = xmlGadget->Attribute("green"))) mLabelColor[1]= (unsigned char) atoi(tmp);
    if ((tmp =xmlGadget->Attribute("blue")))   mLabelColor[2]= (unsigned char) atoi(tmp);
    if ((tmp = xmlGadget->Attribute("text")))  mStrLabel = tmp;
  }
  /// ////////////////////////////////////////////////////////////////////
  /// Parse the Tooltip entry.
  /// ////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Tooltip")))
  {
    if ((tmp = xmlGadget->Attribute("text"))) mStrTooltip = tmp;
  }
}

///================================================================================================
/// .
///================================================================================================
void GuiElement::setStateImagePos(std::string name, int x, int y)
{
  int state = -1;
  if      (name == "Standard") state = STATE_STANDARD;
  else if (name == "Passive" ) state = STATE_PASSIVE;
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
