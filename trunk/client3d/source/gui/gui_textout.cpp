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
#include "gui_graphic.h"
#include "gui_textout.h"
#include "gui_imageset.h"
#include "logger.h"

using namespace Ogre;

#define INC_ALPHA

const char GuiTextout::TXT_CMD_HIGHLIGHT   = '~';
const char GuiTextout::TXT_CMD_LOWLIGHT    = -80; // prevent anjuta and codeblocks problems with the degree character.
const char GuiTextout::TXT_CMD_LINK        = '^';
const char GuiTextout::TXT_CMD_SOUND       = '§';
const char GuiTextout::TXT_SUB_CMD_COLOR   = '#'; // followed by 8 chars (atoi -> uint32).
const char GuiTextout::TXT_CMD_CHANGE_FONT = '@'; // followed by 2 chars (atoi -> char).
const char GuiTextout::CURSOR[] = { GuiTextout::STANDARD_CHARS_IN_FONT+31, 0 };
const uint32 GuiTextout::TXT_COLOR_HIGHLIGHT = 0x0000ff00;
const uint32 GuiTextout::TXT_COLOR_LOWLIGHT  = 0x00ffffff;
const uint32 FONT_ENDX_SIGN = 0xff00ff00;

//================================================================================================
// Constructor.
//================================================================================================
GuiTextout::GuiTextout()
{
    // ////////////////////////////////////////////////////////////////////
    // Parse the font extensions.
    // ////////////////////////////////////////////////////////////////////
    TiXmlElement *xmlRoot, *xmlElem;
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
}

//================================================================================================
// Load a RAW font into main memory.
//================================================================================================
void GuiTextout::loadRawFont(const char *filename)
{
    Image image;
    image.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    size_t size = image.getHeight() * image.getWidth();
    mFont *fnt = new mFont;
    mvFont.push_back(fnt);
    fnt->data = new uint32[size];
    uint32 *src = (uint32*)image.getData();
    uint32 *dst = fnt->data;
    for (size_t i=0; i < size; ++i)
    {
        *dst++ = (*src == FONT_ENDX_SIGN)?FONT_ENDX_SIGN:*src & 0xff000000;
        ++src;
    }
    fnt->height = (int) image.getHeight();
    fnt->textureWidth = (int)image.getWidth();
    fnt->charStart[0]=0;
    // Parse the character width (a vert green line is the end sign).
    unsigned int i=0, w=0;
    for (unsigned int x=0; x < fnt->textureWidth; ++x)
    {
        ++w;
        if (fnt->data[x] == FONT_ENDX_SIGN)
        {
            fnt->charWidth[i] =w;
            w =0;
            if (i) fnt->charStart[i] = fnt->charStart[i-1] + fnt->charWidth[i-1];
            if (++i >= CHARS_IN_FONT) break;
        }
    }
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
    FontPtr pFont = FontManager::getSingleton().create("tmpFont", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, false, 0, &pairList);
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
    memset(fnt->data, 0x00,(fnt->textureWidth+1) * fnt->height * sizeof(uint32));
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
                fnt->data[fnt->charStart[i]+x + yPos] = ttfData[x1+x + y*texture->getWidth()]& 0xff000000;
#ifdef INC_ALPHA
                if (fnt->data[fnt->charStart[i]+x + yPos])
                    fnt->data[fnt->charStart[i]+x + yPos]|= 0x07000000;
#endif
                yPos+= fnt->textureWidth;
            }
        }
    }
    delete[] ttfData;
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
                fnt->data[x1] = FONT_ENDX_SIGN;
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
}

//================================================================================================
// Calculate the gfx-width for the given text.
//================================================================================================
int GuiTextout::calcTextWidth(const unsigned char *text, unsigned int fontNr)
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
                if (!*(++text)) break;
                fontNr+= (*text >='a') ? (*text - 87)     : (*text >='A') ? (*text - 55)     :(*text -'0');
                ++text;
                if (fontNr >= (unsigned int)mvFont.size()) fontNr = 0;
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
// The caller function must check fontNr to be valid [< mvFont.size()].
//================================================================================================
int GuiTextout::getCharWidth(int fontNr, unsigned char Char)
{
    return (Char<32)?0:mvFont[fontNr]->charWidth[Char-32]-1;
}

//================================================================================================
// Replace xml-keycodes by char number in a text.
//================================================================================================
void GuiTextout::parseUserDefinedChars(String &txt)
{
    size_t found;
    char replacement[] = {(char)(STANDARD_CHARS_IN_FONT+32),0};
    for (unsigned int i=0; i < mvSpecialChar.size();++i)
    {
        while ((found = txt.find(mvSpecialChar[i]->strGfxCode))!= String::npos)
            txt.replace(found, mvSpecialChar[i]->strGfxCode.size(), replacement);
        ++replacement[0];
    }
}

//================================================================================================
// Alphablend a text with a gfx/color into a given background. Clipping is performed.
// @param bakLineSkip : When 0 the background will be filled with the color from *bak
//================================================================================================
void GuiTextout::printText(int width, int height, uint32 *dst, int dstLineSkip, uint32 *bak, int bakLineSkip, const char *txt, unsigned int fontNr, uint32 fontColor, bool hideText)
{
    const char *text = (!txt)?"":txt; // Prevent trouble when txt is NULL.
    if (fontNr >= (unsigned int)mvFont.size()) fontNr = 0;

    int srcRow, bakRow, dstRow, stopX, clipX=0;
    unsigned char chr;
    uint32 aktColor, color = fontColor & 0x00ffffff; // Alpha part comes from the font.

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
                if (!*(++text)) goto textDone;
                if (color!= TXT_COLOR_LOWLIGHT)
                    color = TXT_COLOR_LOWLIGHT;
                else
                    color = fontColor  & 0x00ffffff;
                break;
            case TXT_CMD_HIGHLIGHT:
                if (!*(++text)) goto textDone;
                if (color != TXT_COLOR_HIGHLIGHT)
                {
                    // Parse the highlight color (8 byte hex string to uint32).
                    if (*text == TXT_SUB_CMD_COLOR)
                    {
                        fontColor =0;
                        for (int i = 28; i>=0; i-=4)
                        {
                            if (!*(++text)) goto textDone;
                            fontColor += (*text >='a') ? (*text - 87) <<i : (*text >='A') ? (*text - 55) <<i :(*text -'0') <<i;
                        }
                        if (!*(++text)) goto textDone;
                        fontColor&= 0x00ffffff;
                        color = fontColor;
                    }
                    // Use standard highlight color.
                    else color = TXT_COLOR_HIGHLIGHT;
                }
                else color = fontColor & 0x00ffffff;
                break;
            case TXT_CMD_CHANGE_FONT:
                if (!*(++text)) goto textDone;
                //int base= mvFont[fontNr]->baseline;
                fontNr = (*text >='a') ? (*text - 87) <<4 : (*text >='A') ? (*text - 55) <<4 :(*text -'0') <<4;
                if (!*(++text)) goto textDone;
                fontNr+= (*text >='a') ? (*text - 87)     : (*text >='A') ? (*text - 55)     :(*text -'0');
                if (!*(++text)) goto textDone;
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
                // Font alignment = bottom.
                int deltaHeight = height - (int)mvFont[fontNr]->height;
                if (deltaHeight < 0)
                {
                    // Todo: erase all empty lines below the font (in the font loader)
                    //       Only that way we can have a good looking bottom centered font.
                    //srcRow-= deltaHeight * mvFont[fontNr]->textureWidth;
                    srcRow-= deltaHeight/2 * mvFont[fontNr]->textureWidth; // just a hack
                    deltaHeight = 0;
                }
                aktColor = (chr < STANDARD_CHARS_IN_FONT)?color:0; // Special chars always keep their own color.
                if (!bakLineSkip) // Background is a simple colorfill.
                {
                    for (int y =0; y < deltaHeight; ++y)
                    {
                        for (int x = 0; x < stopX; ++x)
                             dst[dstRow + x] = *bak;
                        dstRow+= dstLineSkip;
                    }
                    //for (int y =(int)mvFont[fontNr]->height < height?(int)mvFont[fontNr]->height:height; y; --y)
                    for (int y =deltaHeight; y < height; ++y)
                    {
                        for (int x = 0; x < stopX; ++x)
                            dst[dstRow + x] = GuiGraphic::getSingleton().alphaBlend(*bak, aktColor + mvFont[fontNr]->data[srcRow + x]);
                        srcRow+= mvFont[fontNr]->textureWidth;
                        dstRow+= dstLineSkip;
                    }
                }
                else // Background is a graphic.
                {
                    bakRow = 0;
                    for (int y =0; y < deltaHeight; ++y)
                    {
                        for (int x = 0; x < stopX; ++x)
                            dst[dstRow + x] = bak[bakRow + x];
                        dstRow+= dstLineSkip;
                        bakRow+= bakLineSkip;
                    }
                    //for (int y =(int)mvFont[fontNr]->height < height?(int)mvFont[fontNr]->height:height; y; --y)
                    for (int y =deltaHeight; y < height; ++y)
                    {
                        for (int x = 0; x < stopX; ++x)
                            dst[dstRow + x] = GuiGraphic::getSingleton().alphaBlend(bak[bakRow + x], aktColor + mvFont[fontNr]->data[srcRow + x]);
                        srcRow+= mvFont[fontNr]->textureWidth;
                        dstRow+= dstLineSkip;
                        bakRow+= bakLineSkip;
                    }
                    bak+= stopX;
                }
                clipX+= stopX;
                if (clipX >= width) goto textDone;
                dst+= stopX;
                ++text;
                break;
        }
    }
textDone: // Fill the space right of the text with the background.
    if (!bakLineSkip) // Background is a simple colorfill.
    {
        for (int y =0; y < height; ++y)
        {
            for (int x = 0; x < width-clipX; ++x)
                *(dst+x) = *bak;
            dst+= dstLineSkip;
        }
    }
    else
    {
        for (int y =0; y < height; ++y)
        {
            for (int x = 0; x < width-clipX; ++x)
                *(dst+x) = *(bak+x);
            dst+= dstLineSkip;
            bak+= bakLineSkip;
        }
    }
}
