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

#ifndef GUI_TEXTOUT_H
#define GUI_TEXTOUT_H

#include <Ogre.h>

using namespace Ogre;

const uint32 COLOR_BLACK = 0xff000000;
const uint32 COLOR_BLUE  = 0xff0000ff;
const uint32 COLOR_GREEN = 0xff00ff00;
const uint32 COLOR_LBLUE = 0xff00ffff;
const uint32 COLOR_RED   = 0xffff0000;
const uint32 COLOR_PINK  = 0xffff00ff;
const uint32 COLOR_YELLOW= 0xffffff00;
const uint32 COLOR_WHITE = 0xffffffff;

const char TXT_CMD_HIGHLIGHT   = '~';
const char TXT_CMD_LOWLIGHT    = -80; // prevent anjuta and codeblocks problems with the degree character.
const char TXT_CMD_LINK        = '^';
const char TXT_CMD_SOUND       = '�';
const char TXT_SUB_CMD_COLOR   = '#'; // followed by 8 chars (atoi -> uint32).
const char TXT_CMD_CHANGE_FONT = '@'; // followed by 2 chars (atoi -> char).



const int MAX_TEXTLINE_LEN = 1024;

typedef struct TextLine
{
    unsigned int x1, y1, x2, y2; /**< Area for printing the text. **/
    unsigned int width;          /**< Width of the parent window. **/
    int font;
    int index;                   /**< Unique number. **/
    bool hideText;               /**< Hide the text e.g. for password input. **/
    String text;
    uint32 *BG_Backup;           /**< Backup buffer for dynamic text. **/
    uint32 color;
}
TextLine;

enum
{
    FONT_SYSTEM, FONT_SMALL, FONT_NORMAL, FONT_BIG, FONT_SUM
};

const unsigned int CHARS_IN_FONT =96;

/**
 ** This class provides a text printing on gui elements.
 *****************************************************************************/
class GuiTextout
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiTextout &getSingleton()
    {
        static GuiTextout Singleton;
        return Singleton;
    }
    void loadRawFont(const char *filename);
    void loadTTFont (const char *filename, const char *size, const char *resolution);
    void createBuffer(int width = MAX_TEXTLINE_LEN);
    bool getClippingPos(TextLine &textline, int maxWidth, int maxHeight);
    void Print(TextLine *line, Texture *texture);
    void PrintToBuffer(int width, int height, uint32 *dest_data, const char*text, unsigned int font, uint32 color = COLOR_WHITE);
    int CalcTextWidth(const char *text, unsigned int fontNr = 1);
    int getFontHeight(int fontNr)
    {
        return mvFont[fontNr]->height;
    }
    int getMaxFontHeight()
    {
        return maxFontHeight;
    }
    int getCharWidth(int fontNr, int Char)
    {
        if (Char < 32) return 0;
        return mvFont[fontNr]->charWidth[Char-32]+1;
    }
private:
    // ////////////////////////////////////////////////////////////////////
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    struct mFont
    {
        uint32 *data;
        unsigned int textureWidth;
        unsigned int width;
        unsigned int height;
        unsigned int baseline;
        char charWidth[CHARS_IN_FONT];
    };
    std::vector<mFont*>mvFont;
    uint32 *mTextGfxBuffer;
    PixelBox *mPb;
    unsigned int maxFontHeight;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiTextout();
    ~GuiTextout();
    GuiTextout(const GuiTextout&); // disable copy-constructor.
    void drawText(int width, int height, uint32 *dest_data, const char*text, bool hideText, unsigned int fontNr = 0, uint32 color = 0x00ffffff);
};

#endif
