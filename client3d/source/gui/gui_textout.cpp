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
In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.
You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include <OgreHardwareBuffer.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreFontManager.h>
#include "define.h"
#include "option.h"
#include "gui_textout.h"
#include "logger.h"

const int MIN_FONT_SIZE =  4;
const int MAX_FONT_SIZE = 80;
const int MIN_RESO_SIZE = 55;
const int MAX_RESO_SIZE = 96;
const uint32 TXT_COLOR_DEFAULT   = 0x00ffffff;
const uint32 TXT_COLOR_RED       = 0x00ff0000;
const uint32 TXT_COLOR_GREEN     = 0x0000ff00;
const uint32 TXT_COLOR_BLUE      = 0x000000ff;
const uint32 TXT_COLOR_YELLOW    = 0x00ffff00;
const uint32 TXT_COLOR_HIGHLIGHT = TXT_COLOR_GREEN;
const uint32 TXT_COLOR_LOWLIGHT  = TXT_COLOR_YELLOW;
enum
{
    TXT_STATE_HIGHLIGHT =1,
    TXT_STATE_LOWLIGHT,
    TXT_STATE_LINK,
    TXT_STATE_SUM
};

//================================================================================================
// Constructor.
//================================================================================================
GuiTextout::GuiTextout()
{
    mTextGfxBuffer= 0;
    mMaxFontHeight= 0;
}

//================================================================================================
// Destructor.
//================================================================================================
GuiTextout::~GuiTextout()
{
    for (std::vector<mFont*>::iterator i = mvFont.begin(); i < mvFont.end(); ++i)
    {
        delete[] (*i)->data;
        delete (*i);
    }
    mvFont.clear();
    if (mTextGfxBuffer) delete[] mTextGfxBuffer;
}

//================================================================================================
// Create a buffer to save the background (for dynamic text).
//================================================================================================
void GuiTextout::createBuffer(int width)
{
    if (mTextGfxBuffer) delete[] mTextGfxBuffer;
    if (width < MAX_TEXTLINE_LEN) width = MAX_TEXTLINE_LEN;
    mTextGfxBuffer = new uint32[mMaxFontHeight * width];
}

//================================================================================================
// Load a RAW font into main memory.
//================================================================================================
void GuiTextout::loadRawFont(const char *filename)
{
    Image image;
    image.load(filename, "General");
    int size = (int) (image.getHeight() * image.getWidth());
    mFont *fnt = new mFont;
    mvFont.push_back(fnt);
    fnt->data = new uint32[size];
    memcpy(fnt->data, image.getData(), size * sizeof(uint32));
    fnt->height = (int) image.getHeight();
    if (mMaxFontHeight < fnt->height)  mMaxFontHeight = fnt->height;
    fnt->textureWidth = (int)image.getWidth();
    fnt->charStart[0]=0;
    // Parse the character width (a vert green line is the end sign).
    unsigned int i=0, w=0;
    for (unsigned int x=0; x < fnt->textureWidth; ++x)
    {
        ++w;
        if (fnt->data[x] == 0xff00ff00)
        {
            fnt->charWidth[i] =w;
            w =0;
            if (i) fnt->charStart[i] = fnt->charStart[i-1] + fnt->charWidth[i-1];
            if (++i >= CHARS_IN_FONT) break;
        }
    }
    createBuffer((int)image.getWidth());
    Logger::log().info() << "System-Font (" << image.getWidth() << "x" << image. getHeight() <<") was created.";
}

//================================================================================================
// Load a TTFont into main memory.
//================================================================================================
void GuiTextout::loadTTFont(const char *filename, const char *size, const char *reso)
{
    // Load the font.
    if (!filename) return;
    Ogre::NameValuePairList pairList;
    int iSize, iReso;
    if (size) iSize = atoi(size);
    else iSize = MIN_FONT_SIZE;
    if (iSize < MIN_FONT_SIZE) iSize = MIN_FONT_SIZE;
    if (iSize > MAX_FONT_SIZE) iSize = MAX_FONT_SIZE;
    if (reso) iReso = atoi(reso);
    else iReso = MAX_RESO_SIZE;
    if (iReso < MIN_RESO_SIZE) iReso = MIN_RESO_SIZE;
    if (iReso > MAX_RESO_SIZE) iReso = MAX_RESO_SIZE;
    pairList["type"]      = "truetype";
    pairList["source"]    = filename;
    pairList["size"]      = StringConverter::toString(iSize);
    pairList["resolution"]= StringConverter::toString(iReso);
    // pairList["antialias_colour"]= "true"; // doesn't seems to work.
    if (iSize > 16 && !Option::getSingleton().getIntValue(Option::HIGH_TEXTURE_DETAILS))
    {
        Logger::log().warning() << "Can't load TTF's with size > 16 on this gfx-card. You must use raw-fonts instead!";
        return;
    }
    FontPtr pFont = FontManager::getSingleton().create("tmpFont", "General", false, 0, &pairList);
    pFont->load();
    MaterialPtr pMaterial = pFont.getPointer()->getMaterial();
    String strTexture = pMaterial.getPointer()->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
    TexturePtr pTexture = TextureManager::getSingleton().getByName(strTexture);
    Texture *texture = pTexture.getPointer();
    // ////////////////////////////////////////////////////////////////////
    // Calculate the Char position and size.
    // ////////////////////////////////////////////////////////////////////
    int texW  = texture->getWidth();
    int texH  = texture->getHeight();
    // Calculate Size for the RAW buffer.
    mFont *fnt = new mFont;
    mvFont.push_back(fnt);
    Real u1,u2, v1, v2;
    // Space char.
    pFont->getGlyphTexCoords(33, u1, v1, u2, v2);
    fnt->height =0;
    fnt->charWidth[0] = (unsigned char)((u2-u1)*texW)+1;
    fnt->charStart[0] = 0;
    // Now we look for the other chars.
    for (unsigned int i=1; i < CHARS_IN_FONT-1; ++i)
    {
        pFont->getGlyphTexCoords(32+i, u1, v1, u2, v2);
        fnt->charWidth[i]= (unsigned char) ((u2-u1)*texW)+1; // 1 pisxel for the endOfChar sign.
        fnt->charStart[i] = fnt->charStart[i-1] + fnt->charWidth[i-1];
        if (fnt->height< (unsigned int) ((v2-v1)*texH))
            fnt->height= (unsigned int) ((v2-v1)*texH);
    }
    // TextCursour char.
    fnt->charWidth[CHARS_IN_FONT-1] = fnt->charWidth[0];
    fnt->charStart[CHARS_IN_FONT-1] = fnt->charStart[CHARS_IN_FONT-2] + fnt->charWidth[CHARS_IN_FONT-2];
    if (mMaxFontHeight < fnt->height)  mMaxFontHeight = fnt->height;
    fnt->textureWidth = fnt->charStart[CHARS_IN_FONT-1] + fnt->charWidth[CHARS_IN_FONT-1];
    // ////////////////////////////////////////////////////////////////////
    // blit the whole texture to memory.
    // ////////////////////////////////////////////////////////////////////
    uint32 *ttfData = new uint32[texture->getHeight() * texture->getWidth()];
    PixelBox pb(texture->getWidth(), texture->getHeight(), 1, PF_A8R8G8B8, ttfData);
    texture->getBuffer()->blitToMemory(Box(1, 1, texture->getWidth(), texture->getHeight()),pb);
    // ////////////////////////////////////////////////////////////////////
    // Build the RAW datas.
    // ////////////////////////////////////////////////////////////////////
    // Clear the background.
    fnt->data = new uint32[fnt->textureWidth * fnt->height];
    for (int i=0; i < fnt->textureWidth * fnt->height; ++i)
        fnt->data[i] = 0x00ffffff;
    // Copy all needed chars of the font.
    int x1, x2, y1, y2, yPos;
    for (unsigned int i=1; i < CHARS_IN_FONT-1; ++i)
    {
        pFont->getGlyphTexCoords(i+32, u1, v1, u2, v2);
        // getGlyphTexCoords gives back values > 1.0f sometimes !?
        x1 = (int)(u1 * texW);
        if (x1 > texW) x1 = texW;
        x2 = (int)(u2 * texW);
        if (x2 > texW) x2 = texW;
        y1 = (int)(v1 * texH);
        if (y1 > texH) y1 = texH;
        y2 = (int)(v2 * texH);
        if (y2 > texH) y2 = texH;
        for (int x = 0; x < fnt->charWidth[i]; ++x)
        {
            yPos=0;
            for (int y = y1; y < y2; ++y)
            {
                fnt->data[fnt->charStart[i]+x + yPos] = ttfData[x1+x + y*texture->getWidth()];
                yPos+= fnt->textureWidth;
            }
        }
    }
    delete[] ttfData;
    // ////////////////////////////////////////////////////////////////////
    // Transparent to color.
    // ////////////////////////////////////////////////////////////////////
    for (int i=0; i < fnt->textureWidth * fnt->height; ++i)
    {
        if (fnt->data[i] != 0x00ffffff)
        {
            int alpha = (fnt->data[i] >> 24) & 0xff;
            alpha+=0x40;
            if (alpha > 0xff) alpha = 0xff;
            if (alpha < 0x70) fnt->data[i] = 0x00ffffff;
            else              fnt->data[i] = 0xff000000 + (alpha<<16) + (alpha <<8) + alpha;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Create Text Cursor (for text input).
    // ////////////////////////////////////////////////////////////////////
    for (unsigned int x = 0; x < fnt->charWidth[CHARS_IN_FONT-1]; ++x)
        fnt->data[x + fnt->charStart[CHARS_IN_FONT-1] + (fnt->height-2)*fnt->textureWidth] = 0xffffffff;
    fnt->baseline = iSize;
    // ////////////////////////////////////////////////////////////////////
    // Create a raw font.
    // ////////////////////////////////////////////////////////////////////
    if (Option::getSingleton().getIntValue(Option::CMDLINE_CREATE_RAW_FONTS))
    {
        Image img;
        // ////////////////////////////////////////////////////////////////////
        // This is the Ogre fontdata.
        // ////////////////////////////////////////////////////////////////////
        /*
        uint32 *sysFontBuf = new uint32[texture->getWidth()*texture->getHeight()];
        texture->getBuffer()->blitToMemory(PixelBox(texture->getWidth(), texture->getHeight(), 1, PF_A8R8G8B8, sysFontBuf));
        img = img.loadDynamicImage((uchar*)sysFontBuf, texture->getWidth(), texture->getHeight(), PF_A8R8G8B8);
        img.save("c:\\OgreFont.png");
        */
        // ////////////////////////////////////////////////////////////////////
        // This is the Daimonin fontdata.
        // ////////////////////////////////////////////////////////////////////
        // draw the endOfchar sign. we dont want a monospace font.
        for (unsigned int i=0; i < CHARS_IN_FONT; ++i)
        {
            x1 = fnt->charStart[i] + fnt->charWidth[i]-1;
            for (unsigned int y = 0; y < fnt->height; ++y)
            {
                fnt->data[x1] = 0xff00ff00;
                x1 +=fnt->textureWidth;
            }
        }
        // write font to disc.
        // This is broken in the codeblocks sdk version of ogre1.2.3, use GNU/Linux or VC to get it done.
        img = img.loadDynamicImage((uchar*)fnt->data, fnt->textureWidth, fnt->height, PF_A8R8G8B8);
        std::string rawFilename = PATH_TEXTURES;
        rawFilename += "NoLonger";
        rawFilename += filename;
        rawFilename.resize(rawFilename.size()-4);
        rawFilename += '_'+ StringConverter::toString(iSize, 3, '0');
        rawFilename += '_'+ StringConverter::toString(iReso, 3, '0');
        rawFilename += ".png";
        img.save(rawFilename);
    }
    // ////////////////////////////////////////////////////////////////////
    // Free Memory.
    // ////////////////////////////////////////////////////////////////////
    TextureManager ::getSingleton().remove(pTexture->getName());
    MaterialManager::getSingleton().remove(pMaterial->getName());
    FontManager    ::getSingleton().remove(pFont->getName());
    createBuffer(MAX_TEXTLINE_LEN); // Set standard buffer size.
}

//================================================================================================
// Prepare the background and print the text.
//================================================================================================
void GuiTextout::Print(TextLine *line, Texture *texture)
{
    // Restore background.
    if (line->index >= 0)
    { // Dynamic text.
        int y =0;
        for (unsigned int j =0; j < line->y2 - line->y1; ++j)
        {
            for (unsigned int x=0; x < line->x2- line->x1; ++x)
            {
                mTextGfxBuffer[x + y] = line->BG_Backup[x + y];
            }
            y += line->x2- line->x1;
        }
    }
    else
    { // Static text.
        if (!line->text.size()) return;
        texture->getBuffer()->blitToMemory(
            Box(line->x1, line->y1, line->x2, line->y2),
            PixelBox(line->x2 - line->x1, line->y2 - line->y1, 1, PF_A8R8G8B8, mTextGfxBuffer)
        );
    }
    // draw the text into buffer.
    if (line->text.size())
    {
        drawText(line->x2 - line->x1, line->y2 - line->y1, mTextGfxBuffer, line->text.c_str(), line->hideText, line->font, line->color);
    }
    // Blit it into the window.
    texture->getBuffer()->blitFromMemory(
        PixelBox(line->x2 - line->x1, line->y2 - line->y1, 1, PF_A8R8G8B8, mTextGfxBuffer),
        Box(line->x1, line->y1, line->x2, line->y2));
}

//================================================================================================
// Print to a given background.
//================================================================================================
void GuiTextout::PrintToBuffer(int width, int height, uint32 *dest_data, const char *text, unsigned int fontNr, uint32 color)
{
    if (fontNr >= (unsigned int)mvFont.size()) fontNr = 0;
    int h = mvFont[fontNr]->height;
    if (h > height) h = height;
    if (!text || text[0] == 0) return;
    drawText(width, h, dest_data, text, false, fontNr, color);
}

//================================================================================================
// Print text into a given background. All stuff beyond width/height will be clipped.
//================================================================================================
void GuiTextout::drawText(int width, int height, uint32 *dest_data, const char*text, bool hideText, unsigned int fontNr, uint32 color)
{
    if (fontNr >= (unsigned int)mvFont.size()) fontNr = 0;
    uint32 pixFont, pixColor;
    int srcRow, dstRow, stopX, clipX=0;
    unsigned char chr;
    while (*text)
    {
        // Parse format commands.
        switch (*text)
        {
            case TXT_CMD_LOWLIGHT:
                if (!*(++text)) return;
                if (color== TXT_COLOR_DEFAULT)
                    color = TXT_COLOR_LOWLIGHT;
                else
                    color = TXT_COLOR_DEFAULT;
                break;
            case TXT_CMD_HIGHLIGHT:
                if (!*(++text)) return;
                if (color == TXT_COLOR_DEFAULT)
                {
                    // Parse the highlight color (8 byte hex string to uint32).
                    if (*text == TXT_SUB_CMD_COLOR)
                    {
                        color =0;
                        for (int i = 28; i>=0; i-=4)
                        {
                            color += (*(++text) >='a') ? (*text - 87) <<i : (*text >='A') ? (*text - 55) <<i :(*text -'0') <<i;
                        }
                        ++text;
                    }
                    // Use standard highlight color.
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
                if (fontNr >= (unsigned int)mvFont.size()) fontNr = 0;
                break;

            default:
                chr = (*text - 32);
                if (chr > CHARS_IN_FONT) chr = 0; // Unknown chars will be displayed as space.
                if (hideText && chr != CHARS_IN_FONT-1) chr = '*'-32;
                dstRow = 0;
                srcRow = mvFont[fontNr]->charStart[chr];
                stopX  = mvFont[fontNr]->charWidth[chr]-1;
                if (clipX + stopX > width)
                    stopX = width - clipX;
                for (int y =(int)mvFont[fontNr]->height < height?(int)mvFont[fontNr]->height:height; y; --y)
                {
                    for (int x = 0; x < stopX; ++x)
                    {   // PixelFormat: A8 R8 G8 B8.
                        pixFont = mvFont[fontNr]->data[srcRow + x];
                        if (pixFont <= 0xffffff) continue;
                        pixColor = pixFont & 0xff000000;
                        pixColor+= ((color&0x0000ff) < (pixFont& 0x0000ff))? color & 0x0000ff : pixFont & 0x0000ff;
                        pixColor+= ((color&0x00ff00) < (pixFont& 0x00ff00))? color & 0x00ff00 : pixFont & 0x00ff00;
                        pixColor+= ((color&0xff0000) < (pixFont& 0xff0000))? color & 0xff0000 : pixFont & 0xff0000;
                        dest_data[dstRow + x] = pixColor;
                    }
                    srcRow+= mvFont[fontNr]->textureWidth;
                    dstRow+= width;
                }
                clipX+= stopX;
                if (clipX >= width) return;
                dest_data+= stopX;
                ++text;
                break;
        }
    }
}

//================================================================================================
// Calculate the gfx-width for the given text.
//================================================================================================
int GuiTextout::CalcTextWidth(const char *text, unsigned int fontNr)
{
    int x =0;
    if (fontNr >= (unsigned int)mvFont.size()) fontNr = 0;
    while (*text)
    {
        switch (*text)
        {
            case TXT_CMD_LOWLIGHT:
                ++text;
                break;
            case TXT_CMD_HIGHLIGHT:
                if (!*(++text)) break;
                if (*text == TXT_SUB_CMD_COLOR) text+= 8; // format: "#xxxxxxxx".
                break;
            case TXT_CMD_CHANGE_FONT:
                if (!*(++text)) break;
                fontNr = (*text >='a') ? (*text - 87) <<4 : (*text >='A') ? (*text - 55) <<4 :(*text -'0') <<4;
                ++text;
                fontNr+= (*text >='a') ? (*text - 87)     : (*text >='A') ? (*text - 55)     :(*text -'0');
                ++text;
                break;
            default:
                x+= getCharWidth(fontNr, *text);
                ++text;
                break;
        }
    }
    return x;
}

//================================================================================================
// Calculate the gfx-width for the given text.
//================================================================================================
int GuiTextout::getCharWidth(int fontNr, int Char)
{
    if (Char < 32) return 0;
    if (fontNr >= (int)mvFont.size()) fontNr = 0;
    return mvFont[fontNr]->charWidth[Char-32];
}
