/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/
#include <OgreHardwareBuffer.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreFontManager.h>
#include "define.h"
#include "option.h"
#include "gui_textout.h"
#include "gui_imageset.h"
#include "logger.h"

using namespace Ogre;

const char GuiTextout::TXT_CMD_HIGHLIGHT   = '~';
const char GuiTextout::TXT_CMD_LOWLIGHT    = -80; // prevent anjuta and codeblocks problems with the degree character.
const char GuiTextout::TXT_CMD_LINK        = '^';
const char GuiTextout::TXT_CMD_SOUND       = '§';
const char GuiTextout::TXT_SUB_CMD_COLOR   = '#'; // followed by 8 chars (atoi -> uint32).
const char GuiTextout::TXT_CMD_CHANGE_FONT = '@'; // followed by 2 chars (atoi -> char).
const char GuiTextout::CURSOR[] = { GuiTextout::STANDARD_CHARS_IN_FONT+31, 0 };

const uint32 GuiTextout::COLOR_BLACK = 0xff000000;
const uint32 GuiTextout::COLOR_BLUE  = 0xff0000ff;
const uint32 GuiTextout::COLOR_GREEN = 0xff00ff00;
const uint32 GuiTextout::COLOR_LBLUE = 0xff00ffff;
const uint32 GuiTextout::COLOR_RED   = 0xffff0000;
const uint32 GuiTextout::COLOR_PINK  = 0xffff00ff;
const uint32 GuiTextout::COLOR_YELLOW= 0xffffff00;
const uint32 GuiTextout::COLOR_WHITE = 0xffffffff;
const uint32 GuiTextout::TXT_COLOR_DEFAULT   = COLOR_WHITE;
const uint32 GuiTextout::TXT_COLOR_HIGHLIGHT = COLOR_GREEN;
const uint32 GuiTextout::TXT_COLOR_LOWLIGHT  = COLOR_WHITE;

//================================================================================================
// Constructor.
//================================================================================================
GuiTextout::GuiTextout()
{
    mTextGfxBuffer= 0;
    mMaxFontHeight= 0;
    mTextGfxBuffer= 0;
    TiXmlElement *xmlRoot, *xmlElem;

    // ////////////////////////////////////////////////////////////////////
    // Parse the font extensions.
    // ////////////////////////////////////////////////////////////////////
    TiXmlDocument doc(FILE_GUI_IMAGESET);
    const char *strTemp;
    if (!doc.LoadFile() || !(xmlRoot = doc.RootElement()) || !(strTemp = xmlRoot->Attribute("file")))
    {
        Logger::log().error() << "XML-File '" << FILE_GUI_IMAGESET << "' is broken or missing.";
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the gfx coordinates.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("ImageFntExt"); xmlElem; xmlElem = xmlElem->NextSiblingElement("ImageFntExt"))
    {
        if (!(strTemp = xmlElem->Attribute("name")) || stricmp(strTemp, "FontExtensions")) continue;
        for (TiXmlElement *xmlState = xmlElem->FirstChildElement("State"); xmlState; xmlState = xmlState->NextSiblingElement("State"))
        {
            if (!(xmlState->Attribute("name"))) continue;
            mSpecialChar *Entry = new mSpecialChar;
            mvSpecialChar.push_back(Entry);
            if ((strTemp = xmlState->Attribute("posX"  ))) Entry->x = atoi(strTemp);
            if ((strTemp = xmlState->Attribute("posY"  ))) Entry->y = atoi(strTemp);
            if ((strTemp = xmlState->Attribute("width" ))) Entry->w = atoi(strTemp);
            if ((strTemp = xmlState->Attribute("height"))) Entry->h = atoi(strTemp);
            if ((strTemp = xmlState->Attribute("name"  )))
            {
                for (unsigned int i=0; i < strlen(strTemp); ++i)
                {
                    // Numers enclosed in [] will be comverted to an ascii char.
                    if (strTemp[i] == '[')
                    {
                        int j =0;
                        char number[4];
                        while (j < 3)
                        {
                            if (strTemp[++i] == ']') break;
                            number[j] = strTemp[i];
                            ++j;
                        }
                        number[j] = 0;
                        Entry->strGfxCode+= (char) atoi(number);
                    }
                    else
                        Entry->strGfxCode+= strTemp[i];
                }
            }
            if (mvSpecialChar.size() == SPECIAL_CHARS_IN_FONT-1)
            {
                Logger::log().warning() << "Maximum of user defined chars was reached.";
                Logger::log().warning() << "You can't define more than " << (int)SPECIAL_CHARS_IN_FONT << " chars.";
                break;
            }
        }
        break;
    }
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
    for (std::vector<mSpecialChar*>::iterator i = mvSpecialChar.begin(); i < mvSpecialChar.end(); ++i)
        delete (*i);
    mvSpecialChar.clear();
    if (mTextGfxBuffer) delete[] mTextGfxBuffer;
}

//================================================================================================
// Create a buffer to save the background (for dynamic text).
//================================================================================================
void GuiTextout::createBuffer(int width)
{
    delete[] mTextGfxBuffer;
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
    size_t size = image.getHeight() * image.getWidth();
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
    int iSize = size?atoi(size):MIN_FONT_SIZE;
    if (iSize < MIN_FONT_SIZE) iSize = MIN_FONT_SIZE;
    else if (iSize > MAX_FONT_SIZE) iSize = MAX_FONT_SIZE;
    int iReso = reso?atoi(reso):MAX_RESO_SIZE;
    if (iReso < MIN_RESO_SIZE) iReso = MIN_RESO_SIZE;
    else if (iReso > MAX_RESO_SIZE) iReso = MAX_RESO_SIZE;
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
    int texW = (int)texture->getWidth();
    int texH = (int)texture->getHeight();
    // Calculate Size for the RAW buffer.
    mFont *fnt = new mFont;
    mvFont.push_back(fnt);
    // Space char.
    FloatRect rect = pFont->getGlyphTexCoords(33);
    fnt->height =0;
    fnt->charWidth[0] = (unsigned char)((rect.right-rect.left)*texW)+1; // 1 extra pixel for the endOfChar sign.
    fnt->charStart[0] = 0;
    // Standard chars.
    for (unsigned int i=1; i < STANDARD_CHARS_IN_FONT-1; ++i)
    {
        rect = pFont->getGlyphTexCoords(32+i);
        fnt->charWidth[i]= (unsigned char) ((rect.right-rect.left)*texW)+1; // 1 extra pixel for the endOfChar sign.
        fnt->charStart[i] = fnt->charStart[i-1] + fnt->charWidth[i-1];
        if (fnt->height< (unsigned int) ((rect.bottom-rect.top)*texH))
            fnt->height= (unsigned int) ((rect.bottom-rect.top)*texH);
    }
    // TextCursour char.
    fnt->charWidth[STANDARD_CHARS_IN_FONT-1] = fnt->charWidth[0];
    fnt->charStart[STANDARD_CHARS_IN_FONT-1] = fnt->charStart[STANDARD_CHARS_IN_FONT-2] + fnt->charWidth[STANDARD_CHARS_IN_FONT-2];
    if (mMaxFontHeight < fnt->height)  mMaxFontHeight = fnt->height;
    // Special chars.
    unsigned int i, j =0;
    for (i = STANDARD_CHARS_IN_FONT; i < STANDARD_CHARS_IN_FONT + mvSpecialChar.size(); ++i)
    {
        fnt->charWidth[i] = mvSpecialChar[j]->w+1;
        fnt->charStart[i] = fnt->charStart[i-1] + fnt->charWidth[i-1];
        //if (mvSpecialChar[j]->h > fnt->height) fnt->height = mvSpecialChar[j]->h;
        ++j;
    }
    for (;i < CHARS_IN_FONT; ++i)
    {
        fnt->charStart[i] = fnt->charStart[i-1] + fnt->charWidth[i-1];
        fnt->charWidth[i] = 2;
    }

    // ////////////////////////////////////////////////////////////////////
    // blit the whole font texture to memory.
    // ////////////////////////////////////////////////////////////////////
    uint32 *ttfData = new uint32[texture->getHeight() * texture->getWidth()];
    PixelBox pb(texture->getWidth(), texture->getHeight(), 1, PF_A8R8G8B8, ttfData);
    texture->getBuffer()->blitToMemory(Box(1, 1, texture->getWidth(), texture->getHeight()),pb);
    // ////////////////////////////////////////////////////////////////////
    // Build the RAW datas.
    // ////////////////////////////////////////////////////////////////////
    // Clear the background.
    fnt->textureWidth = fnt->charStart[CHARS_IN_FONT-1] + fnt->charWidth[CHARS_IN_FONT-1];
    fnt->data = new uint32[(fnt->textureWidth+1) * fnt->height];
    for (register int i=0; i < (fnt->textureWidth+1) * fnt->height; ++i)
        fnt->data[i] = 0x00ffffff;
    // Copy all needed chars of the font.
    int x1, y1, yPos;
    for (unsigned int i=1; i < STANDARD_CHARS_IN_FONT-1; ++i)
    {
        rect = pFont->getGlyphTexCoords(i+32);
        x1 = (unsigned int)(rect.left * texW)-1;
        y1 = (unsigned int)(rect.top * texH);
        for (int x = 0; x <= fnt->charWidth[i]; ++x)
        {
            yPos=0;
            for (int y = y1; y < y1+fnt->height; ++y)
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
            else              fnt->data[i] = 0xff000000 + ((alpha<<16)&0xff0000) + ((alpha <<8)&0xff00) + alpha;
        }
    }

    // Copy all special chars.
    PixelBox srcPixelBox = GuiImageset::getSingleton().getPixelBox();
    PixelBox src;
    int k = 0;
    for (std::vector<mSpecialChar*>::iterator i = mvSpecialChar.begin(); i < mvSpecialChar.end(); ++i)
    {
        src = srcPixelBox.getSubVolume(Box(mvSpecialChar[k]->x,
                                           mvSpecialChar[k]->y,
                                           mvSpecialChar[k]->x + mvSpecialChar[k]->w,
                                           mvSpecialChar[k]->y + mvSpecialChar[k]->h));
        uint32 *srcData = static_cast<uint32*>(src.data);
        int rowSkip = (int) srcPixelBox.getWidth();
        int dSrcY = 0, dDstY =0;

        for (int y =0; y < mvSpecialChar[k]->h; ++y)
        {
            for (int x =0; x < mvSpecialChar[k]->w; ++x)
            {
                fnt->data[fnt->charStart[STANDARD_CHARS_IN_FONT+k]+x + dDstY] =  srcData[dSrcY + x];
            }
            dSrcY+= rowSkip;
            dDstY+= fnt->textureWidth;
        }
        ++k;
    }

    // ////////////////////////////////////////////////////////////////////
    // Create Text Cursor (for text input).
    // ////////////////////////////////////////////////////////////////////
    for (unsigned int x = 0; x < fnt->charWidth[STANDARD_CHARS_IN_FONT-1]; ++x)
        fnt->data[x + fnt->charStart[STANDARD_CHARS_IN_FONT-1] + (fnt->height-2)*fnt->textureWidth] = 0xffffffff;
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
        static int fontNr = -1;
        uint32 *sysFontBuf = new uint32[texture->getWidth()*texture->getHeight()];
        texture->getBuffer()->blitToMemory(PixelBox(texture->getWidth(), texture->getHeight(), 1, PF_A8R8G8B8, sysFontBuf));
        img = img.loadDynamicImage((unsigned char*)sysFontBuf, texture->getWidth(), texture->getHeight(), PF_A8R8G8B8);
        img.save("./OgreFont"+StringConverter::toString(++fontNr,3,'0')+".png");
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
                x1+= fnt->textureWidth;
            }
        }
        // write font to disc.
        // This is broken in the codeblocks sdk version of ogre1.2.x (width > 1024 will be clipped).
        // Use GNU/Linux or VC to get it done.
        img = img.loadDynamicImage((unsigned char*)fnt->data, fnt->textureWidth, fnt->height, 1, PF_A8R8G8B8);
        String rawFilename = "./NoLonger";
        rawFilename+= filename;
        rawFilename.resize(rawFilename.size()-4); // Cut extension.
        rawFilename+= '_'+ StringConverter::toString(iSize, 3, '0');
        rawFilename+= '_'+ StringConverter::toString(iReso, 3, '0');
        rawFilename+= ".png";
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
                mTextGfxBuffer[x + y] = line->LayerWindowBG[x + y];
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
void GuiTextout::drawText(int width, int height, uint32 *dest_data, const char *text, bool hideText, unsigned int fontNr, uint32 color)
{
    if (fontNr >= mvFont.size()) fontNr = 0;
    uint32 pixFont, pixColor;
    uint32 colorBack = color;
    int srcRow, dstRow, stopX, clipX=0;
    unsigned char chr;

    while (*text)
    {
        if ((unsigned char) *text < 32)
        {
            ++text;
            continue;
        }
        // Parse format commands.
        switch (*text)
        {
            case TXT_CMD_LOWLIGHT:
                if (!*(++text)) return;
                if (color!= TXT_COLOR_LOWLIGHT)
                    color = TXT_COLOR_LOWLIGHT;
                else
                    color = colorBack;
                break;
            case TXT_CMD_HIGHLIGHT:
                if (!*(++text)) return;
                if (color != TXT_COLOR_HIGHLIGHT)
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
                else color = colorBack;
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
                if (hideText && chr != STANDARD_CHARS_IN_FONT-1) chr = '*'-32;
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
int GuiTextout::CalcTextWidth(unsigned char *text, unsigned int fontNr)
{
    int x =0;
    if (fontNr >= mvFont.size()) fontNr = 0;

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
int GuiTextout::getCharWidth(int fontNr, char Char)
{
    if ((unsigned char) Char < 32) return 0;
    if (fontNr >= (int)mvFont.size()) fontNr = 0;
    return mvFont[fontNr]->charWidth[(unsigned char)(Char-32)]-1;
}

//================================================================================================
// Calculate the gfx-width for the given text.
//================================================================================================
const char *GuiTextout::showUserDefinedChars(const char *XmlUserChars)
{
    static String txt;
    txt = XmlUserChars;
    // Look for userdefined chars in the text.
    size_t found;
    char replacement[] = {(char)(STANDARD_CHARS_IN_FONT+32),0};
    for (unsigned int i=0; i < mvSpecialChar.size();++i)
    {
        while ((found = txt.find(mvSpecialChar[i]->strGfxCode))!= String::npos)
            txt.replace(found, mvSpecialChar[i]->strGfxCode.size(), replacement);
        ++replacement[0];
    }
    return txt.c_str();
}
