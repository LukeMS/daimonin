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
#include <OgreFontManager.h>
#include "define.h"
#include "gui_textout.h"
#include "logger.h"
#include <ctime>

const int MAX_TEXTLINE_LEN = 1024;
const int MIN_FONT_SIZE =  4;
const int MAX_FONT_SIZE = 80;
const int MIN_RESO_SIZE = 55;
const int MAX_RESO_SIZE = 96;

const uint32 TXT_COLOR_DEFAULT   = 0x00ffffff;
const uint32 TXT_COLOR_RED       = 0x00ff0000;
const uint32 TXT_COLOR_GREEN     = 0x0000ff00;
const uint32 TXT_COLOR_BLUE      = 0x000000ff;
const uint32 TXT_COLOR_HIGHLIGHT = 0x0000ff00;

const char TXT_CMD_HIGHLIGHT   = '~';
const char TXT_CMD_LOWLIGHT    = '°';
const char TXT_CMD_LINK        = '^';
const char TXT_SUB_CMD_COLOR   = '#'; // followed by 8 chars (atoi -> uint32).
const char TXT_CMD_CHANGE_FONT = '@'; // followed by 2 chars (atoi -> char).

enum
{
  TXT_STATE_HIGHLIGHT =1,
  TXT_STATE_LOWLIGHT,
  TXT_STATE_LINK,
  TXT_STATE_SUM
};

///=================================================================================================
/// Constructor.
///=================================================================================================
GuiTextout::GuiTextout()
{
  mTextGfxBuffer=0;
  maxFontHeight = 0;
}

///=================================================================================================
/// .
///=================================================================================================
void GuiTextout::createBuffer()
{
  if (mTextGfxBuffer) delete[] mTextGfxBuffer;
  mTextGfxBuffer = new uint32[maxFontHeight * MAX_TEXTLINE_LEN];
}

///=================================================================================================
/// Destructor.
///=================================================================================================
GuiTextout::~GuiTextout()
{
  if (mTextGfxBuffer) delete[] mTextGfxBuffer;
  for (std::vector<mFont*>::iterator i = mvFont.begin(); i < mvFont.end(); ++i)
  {
    delete[] (*i)->data;
    delete (*i);
    mvFont.erase(i);
  }
}

///=================================================================================================
/// Load a RAW font into main memory.
///=================================================================================================
void GuiTextout::loadRawFont(const char *filename)
{
  Image image;
  image.load(filename, "General");
  int size = image.getHeight() * image.getWidth();
  mFont *fnt = new mFont;
  mvFont.push_back(fnt);
  fnt->data = new uint32[size];
  memcpy(fnt->data, image.getData(), size * sizeof(uint32));

  fnt->height = image.getHeight();
  if (maxFontHeight < fnt->height)  maxFontHeight = fnt->height;
  fnt->textureWidth = image.getWidth();
  fnt->width  = image.getWidth() / CHARS_IN_FONT;
  // Parse the character width (a vert green line is the end sign).
  unsigned int x;
  for (int i=0; i < CHARS_IN_FONT; ++i)
  {
    for (x=0; x < fnt->width-1; ++x)
    {
      if (fnt->data[x+i*fnt->width] == 0xff00ff00) break;
    }
    fnt->charWidth[i] = x;
  }
  createBuffer();
}

///=================================================================================================
/// Load a TTFont into main memory.
///=================================================================================================
void GuiTextout::loadTTFont(const char *filename, const char *size, const char *reso)
{
  // Load the font.
  Ogre::NameValuePairList pairList;
  int iSize, iReso;
  if (size) iSize = atoi(size);  else iSize = MIN_FONT_SIZE;
  if (iSize < MIN_FONT_SIZE) iSize = MIN_FONT_SIZE;
  if (iSize > MAX_FONT_SIZE) iSize = MAX_FONT_SIZE;
  if (reso) iReso = atoi(reso);  else iReso = MAX_RESO_SIZE;
  if (iReso < MIN_RESO_SIZE) iReso = MIN_RESO_SIZE;
  if (iReso > MAX_RESO_SIZE) iReso = MAX_RESO_SIZE;
  pairList["type"]      = "truetype";
  pairList["source"]    = filename;
  pairList["size"]      = StringConverter::toString(iSize);
  pairList["resolution"]= StringConverter::toString(iReso);
  // pairList["antialias_colour"]= "true"; // doesn't seems to work.
  FontPtr pFont = FontManager::getSingleton().create("tmpFont", "General", false, 0, &pairList);
  pFont->load();
  MaterialPtr pMaterial = pFont.getPointer()->getMaterial();
  String strTexture = pMaterial.getPointer()->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
  TexturePtr pTexture = TextureManager::getSingleton().getByName(strTexture);
  Texture *texture = pTexture.getPointer();
  /////////////////////////////////////////////////////////////////////////
  /// Convert the font to RAW format.
  /////////////////////////////////////////////////////////////////////////
  int texW  = texture->getWidth();
  int texH  = texture->getHeight();

  /// Calculate Size for the RAW buffer.
  mFont *fnt = new mFont;
  mvFont.push_back(fnt);
  fnt->height =0;
  fnt->width  =0;
  fnt->textureWidth = 0;
  unsigned int w, h;
  Real u1,u2, v1, v2;
  int x1, x2, y1, y2;
  for (int i=1; i < CHARS_IN_FONT; ++i)
  {
    pFont->getGlyphTexCoords(i+32, u1, v1, u2, v2);
    w = (int) ((u2-u1)*texW);
    h = (int) ((v2-v1)*texH);
    fnt->charWidth[i] = w;
    fnt->textureWidth+= w;
    if (fnt->width < w)  fnt->width = w;
    if (fnt->height< h)  fnt->height= h;
  }
  if (maxFontHeight < fnt->height)  maxFontHeight = fnt->height;
  fnt->charWidth[0] = fnt->width/2; // ascii(32).
  fnt->textureWidth = fnt->width * CHARS_IN_FONT;
  createBuffer();

  /// Build the RAW datas.
  fnt->data = new uint32[fnt->textureWidth * fnt->height];
  for (unsigned int i=0; i < fnt->textureWidth * fnt->height -1; ++i)
    fnt->data[i] = 0x00ffffff;
  PixelBox pb(fnt->textureWidth, fnt->height, 1, PF_A8R8G8B8 , fnt->data);

  /// Copy all needed chars of the font.
  int xPos = 0;
  for (int i=1; i < CHARS_IN_FONT; ++i)
  {
    xPos+= fnt->width;
    pFont->getGlyphTexCoords(i+32, u1, v1, u2, v2);
    // getGlyphTexCoords gives back values > 1.0f sometimes !?
    x1 = (int)(u1 * texW); if (x1 > texW-2) x1 = texW-2;
    x2 = (int)(u2 * texW); if (x2 > texW-1) x2 = texW-1;
    y1 = (int)(v1 * texH); if (y1 > texH-2) y1 = texH-1;
    y2 = (int)(v2 * texH); if (y2 > texH-1) y2 = texH-1;
    texture->getBuffer()->blitToMemory(
      Box(x1, y1, x2, y1 + fnt->height),
      pb.getSubVolume(Box(xPos, 0, xPos + x2-x1, fnt->height)));
  }

  /// Transparent to color.
  for (unsigned int i=0; i < fnt->textureWidth * fnt->height; ++i)
  {
    if (fnt->data[i] != 0x00ffffff)
    {
      int alpha = (fnt->data[i] >> 24) & 0xff;
      if (alpha < 0x20) fnt->data[i] = 0x00ffffff;
      else              fnt->data[i] = 0xff000000 + (alpha << 16) + (alpha <<8) + alpha;
    }
  }

  fnt->baseline = iSize;

//#define CREATE_SYSTEM_FONT
  #ifdef CREATE_SYSTEM_FONT
  /// ////////////////////////////////////////////////////////////////////
  ///.The first font ever created is our system font.
  /// Run this function only if you are in need of a new system-font!
  /// ////////////////////////////////////////////////////////////////////
  static int sysFont = -1;
  if (!++sysFont)
  {
    Image img;
    /// ////////////////////////////////////////////////////////////////////
    /// This is the Ogre fontdata.
    /// ////////////////////////////////////////////////////////////////////
    /*
    uint32 *sysFontBuf = new uint32[texture->getWidth()*texture->getHeight()];
    texture->getBuffer()->blitToMemory(PixelBox(texture->getWidth(), texture->getHeight(), 1, PF_A8R8G8B8, sysFontBuf));
    img = img.loadDynamicImage((uchar*)sysFontBuf, texture->getWidth(), texture->getHeight(), PF_A8R8G8B8);
    img.save("c:\\OgreFont.png");
    */
    /// ////////////////////////////////////////////////////////////////////
    /// This is the Daimonin fontdata.
    /// ////////////////////////////////////////////////////////////////////
    // draw the cher-end sign to get rid of the monospace.
    uint32 *data = fnt->data;
    for (int i=0; i < CHARS_IN_FONT; ++i)
    {
      for (unsigned int y = 0; y < fnt->height; ++y)
      {
        *(data + fnt->charWidth[i] + y*fnt->textureWidth) = 0xff00ff00;
      }
      data+= fnt->width;
    }
    // write font to disc.
    img = img.loadDynamicImage((uchar*)fnt->data, fnt->textureWidth, fnt->height, PF_A8R8G8B8);
    std::string filename = PATH_TEXTURES;
    filename += FILE_SYSTEM_FONT;
    img.save(filename);
  }
#endif

  //////////////////////////////////////////////////////////////////////////
  ///.Free Memory.
  /////////////////////////////////////////////////////////////////////////
  MaterialManager::getSingleton().remove((ResourcePtr&)pMaterial);
  TextureManager ::getSingleton().remove((ResourcePtr&)pTexture);
  FontManager    ::getSingleton().remove((ResourcePtr&)pFont);

  //////////////////////////////////////////////////////////////////////////
  ///.Create Text Cursor (for text input).
  /////////////////////////////////////////////////////////////////////////

  //TODO.
}

///=================================================================================================
/// .
///=================================================================================================
void GuiTextout::Print(TextLine *line, Texture *texture, const char *text)
{
  /// Restore background.
  if (line->index >= 0)
  { // Dynamic text.
    int y =0;
    for (unsigned int j =0; j < line->y2 - line->y1; ++j)
    {
      for (unsigned int x=0; x < line->width; ++x)
      {
        mTextGfxBuffer[x + y] = line->BG_Backup[x + y];
      }
      y += line->width;
    }
  }
  else
  { // Static text.
    texture->getBuffer()->blitToMemory(
      Box(line->x1, line->y1, line->x2, line->y2),
      PixelBox(line->x2 - line->x1, line->y2 - line->y1, 1, PF_A8R8G8B8, mTextGfxBuffer)
    );
  }
  if (!text || text[0] == 0) return;
  /// draw the text into buffer.
  drawText(line->x2 - line->x1, line->y2 - line->y1, mTextGfxBuffer, text, line->font);
  /// Blit it into the window.
  texture->getBuffer()->blitFromMemory(
    PixelBox(line->x2 - line->x1, line->y2 - line->y1, 1, PF_A8R8G8B8, mTextGfxBuffer),
    Box(line->x1, line->y1, line->x2, line->y2));
}

///=================================================================================================
/// Print to a buffer.
///=================================================================================================
void GuiTextout::PrintToBuffer(int width, uint32 *dest_data, const char*text, int fontNr, uint32 bgColor)
{
  int height = mvFont[fontNr]->height;
  /// Clear the textline.
  for (unsigned int i =0; i < width * mvFont[fontNr]->height; ++i) dest_data[i] = bgColor;
  if (!text || text[0] == 0) return;
  drawText(width, height, dest_data, text, fontNr);
}

///=================================================================================================
/// .
///=================================================================================================
void GuiTextout::drawText(int width, int height, uint32 *dest_data, const char*text, unsigned int fontNr)
{
  int fontPosX, fontPosY=0;
  if (fontNr >= mvFont.size()) fontNr = mvFont.size()-1;
  uint32 pixFont, pixColor;
  uint32 color = TXT_COLOR_DEFAULT;

  //  uint32 *strtX = dest_data;
  uint32 *stopX = dest_data + width;

  while (*text)
  {
    // Parse format commands.
    switch (*text)
    {
      /*
        // atm we draw only 1 line of text in this function!
            case '\n':
              strtX += mvFont[fontNr]->height * width;
              dest_data = strtX;
              stopX = strtX + width;
              ++text;
            break;
      */
      case TXT_CMD_HIGHLIGHT:
      if (!*(++text)) return;
      if (color == TXT_COLOR_DEFAULT)
      {
        /// Parse the highlight color (8 byte hex string to uint32).
        if (*text == TXT_SUB_CMD_COLOR)
        {
          color =0;
          for (int i = 28; i>=0; i-=4)
          {
            color += (*(++text) >='a') ? (*text - 87) <<i : (*text >='A') ? (*text - 55) <<i :(*text -'0') <<i;
          }
          ++text;
        }
        /// Use standard highlight color.
        else color = TXT_COLOR_HIGHLIGHT;
      }
      else color = TXT_COLOR_DEFAULT;
      break;

      case TXT_CMD_CHANGE_FONT:
      if (!*(++text)) return;
      //int base= mvFont[fontNr]->baseline;
      fontNr = (*text >='a') ? (*text - 87) <<4 : (*text >='A') ? (*text - 55) <<4 :(*text -'0') <<4;
      ++text;
      fontNr+= (*text >='a') ? (*text - 87)     : (*text >='A') ? (*text - 55)     :(*text -'0');
      ++text;
      if (fontNr >= mvFont.size()) fontNr = mvFont.size()-1;
      break;

      default:
      unsigned char chr = (*text - 32);
      if (chr > CHARS_IN_FONT) chr = 0;
      fontPosX = chr * mvFont[fontNr]->width;
      for (int y =0; y < (int)mvFont[fontNr]->height && y < height; ++y)
      {
        for (int x=0; x < mvFont[fontNr]->charWidth[chr]; ++x)
        { /// PixelFormat: A8 R8 G8 B8.
          pixFont = mvFont[fontNr]->data[y * mvFont[fontNr]->textureWidth + fontPosX + x];
          if (pixFont <= 0xffffff) continue;
          pixColor = pixFont & 0xff000000;
          pixColor+= ((color&0x0000ff) < (pixFont& 0x0000ff))? color & 0x0000ff : pixFont & 0x0000ff;
          pixColor+= ((color&0x00ff00) < (pixFont& 0x00ff00))? color & 0x00ff00 : pixFont & 0x00ff00;
          pixColor+= ((color&0xff0000) < (pixFont& 0xff0000))? color & 0xff0000 : pixFont & 0xff0000;
          dest_data[(y+fontPosY)*width + x] = pixColor;
        }
      }
      dest_data += mvFont[fontNr]->charWidth[chr] +1;
      if (dest_data + mvFont[fontNr]->charWidth[chr] +1 > stopX) return;
      ++text;
      break;

    }
  }
}

///=================================================================================================
/// Calculate the width and height of the needed pixel-field for the given text.
///=================================================================================================
void GuiTextout::CalcTextSize(unsigned int &x, unsigned int &y, int maxWidth, int maxHeight, const char *text, unsigned int fontNr)
{
  if (fontNr >= mvFont.size()) fontNr = mvFont.size()-1;
  //  int h = mvFont[fontNr]->height;
  while(*text)
  {
    switch (*text)
    {
      case TXT_CMD_HIGHLIGHT:
      if (!*(++text)) break;
      if (*text == TXT_SUB_CMD_COLOR) text+= 8; // format: "#xxxxxxxx".
      break;

      case TXT_CMD_CHANGE_FONT:
      if (!*(++text)) break;
      text+= 2;  // format: "xx".
      break;

      default:
      if (x + mvFont[fontNr]->charWidth[*text - 32] +1  < (unsigned int)maxWidth)
      {
        x+= mvFont[fontNr]->charWidth[*text - 32] +1;
      }
      ++text;
      break;
    }
  }
  y+= mvFont[fontNr]->height;
  if (y > (unsigned int)maxHeight) y = (unsigned int)maxHeight;
}
