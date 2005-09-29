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

#include <ctime>
#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_graphic.h"
#include "gui_manager.h"
#include "gui_cursor.h"
#include "logger.h"

///=================================================================================================
/// Parse a gadget entry.
///=================================================================================================
GuiGraphic::GuiGraphic(TiXmlElement *xmlElem, int w, int h, int maxX, int maxY)
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
  mBehavior = xmlElem->Attribute("Type");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the position.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Pos")))
  {
    mX = atoi(xmlGadget->Attribute("X"));
    mY = atoi(xmlGadget->Attribute("Y"));
  }
  if (mX > maxX-2) mX = maxX-2;
  if (mY > maxY-2) mY = maxY-2;
  mSrcWidth = w;
  mSrcHeight= h;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the fill color.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Color")))
  {
    // PixelFormat: ARGB.
    mFillColor = atoi(xmlGadget->Attribute("Blue" ));
    mFillColor+= atoi(xmlGadget->Attribute("Green")) << 8;
    mFillColor+= atoi(xmlGadget->Attribute("Red"  )) << 16;
    mFillColor+= atoi(xmlGadget->Attribute("Alpha")) << 24;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the dimension.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Range")))
  {
    mDestWidth = atoi(xmlGadget->Attribute("Width"));
    mDestHeight= atoi(xmlGadget->Attribute("Height"));
  }
  if (mX + mDestWidth > maxX) mDestWidth = maxX-mX-1;
  if (mY + mDestHeight >maxY) mDestHeight= maxY-mY-1;
}

///=================================================================================================
/// .
///=================================================================================================
void GuiGraphic::setStateImagePos(std::string name, int x, int y)
{
  if (name == "Standard")
  {
    gfxSrcPos.x = x;
    gfxSrcPos.y = y;
  }
  else
  {
    Logger::log().error() << "Graphic '" << mStrName << "' has no State '" << "' " << name;
  }
}

///=================================================================================================
/// .
///=================================================================================================
void GuiGraphic::draw(PixelBox &mSrcPixelBox, Texture *texture)
{
  std::string strID, strTemp;
  int x1, y1, x2, y2;
  bool color_fill;
  if (mBehavior == "GFX_FILL")
    color_fill = false;
  else
    color_fill = true;
  /////////////////////////////////////////////////////////////////////////
  /// Fill background rect with a color.
  /////////////////////////////////////////////////////////////////////////
  if (color_fill)
  {
    long time = clock();
    PixelBox pb = texture->getBuffer()->lock(Box(mX, mY, mX+mDestWidth, mY+mDestHeight), HardwareBuffer::HBL_READ_ONLY );
    uint32 *dest_data = (uint32*)pb.data;
    for (int y = 0; y < mDestHeight; ++y)
    {
      for (int x= 0; x < mDestWidth; ++x)
      {
        dest_data[x+y*texture->getWidth()] = mFillColor;
      }
    }
    texture->getBuffer()->unlock();
    Logger::log().info() << "Time to fill fill: " << clock()-time << " ms";
  }
  /////////////////////////////////////////////////////////////////////////
  /// Fill background rect with a gfx.
  /////////////////////////////////////////////////////////////////////////
  else
  {
    PixelBox src;
    bool dirty = true;
    int sumX = (mDestWidth-1)  / mSrcWidth  + 1;
    int sumY = (mDestHeight-1) / mSrcHeight + 1;

    y1 = 0; y2 = mSrcHeight;
    for (int y = 0; y < sumY; ++y)
    {
      if (dirty)
      {
        src = mSrcPixelBox.getSubVolume(Box(
                                          gfxSrcPos.x,
                                          gfxSrcPos.y,
                                          gfxSrcPos.x + mSrcWidth,
                                          gfxSrcPos.y + mSrcHeight));
        dirty = false;
      }
      if (y2 > mDestHeight)
      {
        y2 = mDestHeight;
        if (y1 > mDestHeight) y1 = mDestHeight-1;
        dirty = true;
      }
      x1 = mX; x2 = mSrcWidth;
      for (int x = 0; x < sumX; ++x)
      {
        if (x2 > mDestWidth)
        {
          x2 = mDestWidth;
          if (x1 > mDestWidth) x1 = mDestWidth-1;
          dirty = true;
        }
        if (dirty)
        {
          src = mSrcPixelBox.getSubVolume(Box(
                                            gfxSrcPos.x,
                                            gfxSrcPos.y,
                                            gfxSrcPos.x + x2-x1,
                                            gfxSrcPos.y + y2-y1));
        }
        texture->getBuffer()->blitFromMemory(src, Box(x1 + mX, y1 + mY, x2 + mX, y2 + mY));
        x1 = x2;
        x2+= mSrcWidth;
      }
      y1 = y2;
      y2+= mSrcHeight;
    }
  }
}

