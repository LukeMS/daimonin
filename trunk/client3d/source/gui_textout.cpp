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


GuiTextout::GuiTextout()
{
  loadFont("font_12.png");
  loadFont("font_16.png");
  loadFont("font_16.png");
}


void GuiTextout::loadFont(const char * filename)
{
  static int fontNr=-1;
  if (++fontNr >= FONT_SUM) return;
  mFont[fontNr].image.load(filename, "General");
  mFont[fontNr].data = (uint32*)mFont[fontNr].image.getData();
  mFont[fontNr].height = mFont[fontNr].image.getHeight() -1;
  mFont[fontNr].textureWidth = mFont[fontNr].image.getWidth();
  mFont[fontNr].width  = mFont[fontNr].image.getWidth() / CHARS_IN_FONT ;
  unsigned int x;
  for (int i=0; i < CHARS_IN_FONT; ++i)
  {
    for (x=0; x < mFont[fontNr].width; ++x)
    {
      if (mFont[fontNr].data[x+i*mFont[fontNr].width] == 0xff00ff00) break;
    }
    mFont[fontNr].charWidth[i] = x;
  }
}

void GuiTextout::Print(int x, int y, Texture *texture, const char *text, uint32 color)
{
  if (!text || text[0] == 0) return;
  int fontNr = 0;
  long time = clock();
  unsigned int x2 = x + strlen(text) * mFont[fontNr].width;
  uint32 pix;
  int x1;
  PixelBox pb = texture->getBuffer()->lock(Box(x, y, x2, y+mFont[fontNr].height), HardwareBuffer::HBL_READ_ONLY );
  uint32 *dest_data = (uint32*)pb.data;
  for (unsigned int i = 0; i < strlen(text); ++i)
  {
    x1 = (text[i]-32) * mFont[fontNr].width;
    for (unsigned int j =0; j < mFont[fontNr].height; ++j)
    {
      for (int k=0; k < mFont[fontNr].charWidth[text[i]-32]-1; ++k)
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
    dest_data+=mFont[fontNr].charWidth[text[i]-32];
  }
  texture->getBuffer()->unlock();
  Logger::log().info() << "Time to print: " << clock()-time << " ms";
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
