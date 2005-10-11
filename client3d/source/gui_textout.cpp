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

#include <OgreHardwareBuffer.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_textout.h"
#include "logger.h"
#include <ctime>

const int MAX_TEXTLINE_LEN = 1024;

///=================================================================================================
/// Constructor.
///=================================================================================================
GuiTextout::GuiTextout()
{
  maxFontHeight = 0;
  maxFontWidth  = 0;
  loadFont("font_12.png");
  loadFont("font_16.png");
  loadFont("font_16.png");
  TextGfxBuffer = new uint32[maxFontHeight * MAX_TEXTLINE_LEN];
}

///=================================================================================================
/// Destructor.
///=================================================================================================
GuiTextout::~GuiTextout()
{
  delete[] TextGfxBuffer;
}

///=================================================================================================
/// Load a font into main memory.
///=================================================================================================
void GuiTextout::loadFont(const char * filename)
{
  static int fontNr=-1;
  if (++fontNr >= FONT_SUM) return;
  mFont[fontNr].image.load(filename, "General");
  mFont[fontNr].data = (uint32*)mFont[fontNr].image.getData();
  mFont[fontNr].height = mFont[fontNr].image.getHeight() -1;
  if (mFont[fontNr].height > maxFontHeight)  maxFontHeight = mFont[fontNr].height;
  mFont[fontNr].textureWidth = mFont[fontNr].image.getWidth();
  mFont[fontNr].width  = mFont[fontNr].image.getWidth() / CHARS_IN_FONT;
  if (mFont[fontNr].width > maxFontWidth)  maxFontWidth = mFont[fontNr].width;
  unsigned int x;
  for (int i=0; i < CHARS_IN_FONT; ++i)
  {
    for (x=0; x < mFont[fontNr].width-1; ++x)
    {
      if (mFont[fontNr].data[x+i*mFont[fontNr].width] == 0xff00ff00) break;
    }
    mFont[fontNr].charWidth[i] = x;
  }
}

///=================================================================================================
/// Print a dynamic text.
///=================================================================================================
void GuiTextout::Print(TextLine *line, Texture *texture, const char *text, uint32 color)
{
  if (!text || text[0] == 0) return;
  int x, y;
  int fontNr = 0;
//  int x2 = line->x + strlen(text) * (mFont[fontNr].width +1);
//  if (x2 > line->x + line->width) x2 = line->x + line->width;

  // Restore background.
  y =0;
  for (unsigned int j =0; j < mFont[fontNr].height; ++j)
  {
    for (int x=0; x < line->width; ++x)
    {
      TextGfxBuffer[x + y] = line->BG_Backup[x + y];
    }
    y += line->width;
  }

  // Draw the text.
  uint32 pix, *dest = TextGfxBuffer;
  for (unsigned int i = 0; i < strlen(text); ++i)
  {
    x = (text[i]-32) * mFont[fontNr].width;
    y =0;
    for (unsigned int j =0; j < mFont[fontNr].height; ++j)
    {
      for (int k=0; k < mFont[fontNr].charWidth[text[i]-32]; ++k)
      {
        pix = mFont[fontNr].data[x + k +j*mFont[fontNr].textureWidth];
        // PixelFormat: ARGB.
        if (pix !=0xff000000 )
        {
          pix &= color;
          dest[k + y] = pix;
        }
      }
      y += line->width;
    }
    dest += mFont[fontNr].charWidth[text[i]-32] +1;
  }

  // Blit it into the window.
  texture->getBuffer()->blitFromMemory(
    PixelBox(line->width, mFont[fontNr].height, 1, PF_A8R8G8B8 , TextGfxBuffer),
    Box(line->x, line->y, line->x+line->width, line->y+mFont[fontNr].height));
}

///=================================================================================================
/// Print a static text.
///=================================================================================================
void GuiTextout::Print(int x, int y, int gfxLen, Texture *texture, const char *text, uint32 color)
{
  //  long time = clock();

  if (!text || text[0] == 0) return;
  int fontNr = 0;
  int x1, x2 = x + strlen(text) * (mFont[fontNr].width +1);
  if (x2 > x+gfxLen) x2 = x+gfxLen;
  uint32 pix;
  PixelBox pb = texture->getBuffer()->lock(Box(x, y, x2, y+mFont[fontNr].height), HardwareBuffer::HBL_READ_ONLY );
  uint32 *dest_data = (uint32*)pb.data;
  for (unsigned int i = 0; i < strlen(text); ++i)
  {
    x1 = (text[i]-32) * mFont[fontNr].width;
    for (unsigned int j =0; j < mFont[fontNr].height; ++j)
    {
      for (int k=0; k < mFont[fontNr].charWidth[text[i]-32]; ++k)
      {
        pix = mFont[fontNr].data[x1 + k +j*mFont[fontNr].textureWidth];
        // PixelFormat: ARGB.
        if (pix !=0xff000000 )
        {
          pix &= color;
          dest_data[k+j*texture->getWidth()] = pix;
        }
      }
    }
    dest_data += mFont[fontNr].charWidth[text[i]-32] +1;
  }
  texture->getBuffer()->unlock();

  //  Logger::log().info() << "Time to print: " << clock()-time << " ms";
}

/*
    FontPtr testFont = FontManager::getSingleton().getByName(mStrFont);
    Material *material = testFont.getPointer()->getMaterial().getPointer();
    std::string strTexture = material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
    Logger::log().error() << strTexture;
    TexturePtr ptexture = TextureManager::getSingleton().getByName(strTexture);
    Texture *texture = ptexture.getPointer();
    Logger::log().error() << texture->getHeight() << " " << texture->getWidth();
*/
