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
#include "gui_listbox.h"
#include "gui_textout.h"
#include "logger.h"

#include <ctime>

///=================================================================================================
/// Parse a gadget entry.
///=================================================================================================
GuiListbox::GuiListbox(TiXmlElement *xmlElem, int maxX, int maxY)
{
  TiXmlElement *xmlGadget;
  std::string strValue;
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
  /////////////////////////////////////////////////////////////////////////
  /// Parse the size.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Range")))
  {
    mWidth = atoi(xmlGadget->Attribute("width"));
    mHeight= atoi(xmlGadget->Attribute("height"));
  }
  if (mX + mWidth > maxX) mWidth = maxX-mX-1;
  if (mY + mHeight >maxY) mHeight= maxY-mY-1;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the fill color.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Color")))
  {
    // PixelFormat: ARGB.
    mFillColor = atoi(xmlGadget->Attribute("blue" ));
    mFillColor+= atoi(xmlGadget->Attribute("green")) << 8;
    mFillColor+= atoi(xmlGadget->Attribute("red"  )) << 16;
    mFillColor+= atoi(xmlGadget->Attribute("alpha")) << 24;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Set defaults.
  /////////////////////////////////////////////////////////////////////////
  mIsClosing    = false;
  mIsOpening    = false;
  mBufferPos    = 0;
  mPrintPos     = 0;
  mRowsToScroll = 0;
  mRowsToPrint  = 10; // mHeight / fontHeight;
  mScroll       = 0.0f;
}

///=================================================================================================
/// .
///=================================================================================================
void GuiListbox::draw(PixelBox &mSrcPixelBox, Texture *texture)
{
  long time = clock();
  //  Logger::log().info() << "listbox: " << "x: " << mWidth << " y: " << mHeight;

  for (int test =0; test < 20; ++test)
    for (int r = 0; r < mRowsToPrint; ++r)
    {
      GuiTextout::getSingleton().Print(mX, mY+r*16, mWidth, texture, "textline...W");
    }
  Logger::log().info() << "Time to print: " << clock()-time << " ms";
}

//=================================================================================================
// Add a line of text.
//=================================================================================================
void GuiListbox::print(const char *text)
{
  row[mBufferPos & (SIZE_STRING_BUFFER-1)].str   = text;
  //  row[mBufferPos & (SIZE_STRING_BUFFER-1)].colorTop = color;
  //  row[mBufferPos & (SIZE_STRING_BUFFER-1)].colorBottom = color/1.5;
  ++mBufferPos;
  ++mRowsToScroll;
}

