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

/**
 ** This class provides a text printing on gui elements.
 *****************************************************************************/
class GuiTextout
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        unsigned int x1, y1, x2, y2; /**< Area for printing the text. **/
        int font;                    /**< Font number. **/
        int index;                   /**< Unique number. **/
        bool hideText;               /**< Hide the text e.g. for password input. **/
        String text;
        uint32 *BG_Backup;           /**< Backup buffer for dynamic text. **/
        uint32 color;
    }
    TextLine;

    enum { MAX_TEXTLINE_LEN = 1024 };

    enum
    {
        FONT_SYSTEM, FONT_SMALL, FONT_NORMAL, FONT_BIG, FONT_SUM
    };

    enum
    {
        STANDARD_CHARS_IN_FONT = 96,
        SPECIAL_CHARS_IN_FONT  = 127,
        CHARS_IN_FONT          = STANDARD_CHARS_IN_FONT+SPECIAL_CHARS_IN_FONT,
    };
    static const uint32 COLOR_BLACK;
    static const uint32 COLOR_BLUE;
    static const uint32 COLOR_GREEN;
    static const uint32 COLOR_LBLUE;
    static const uint32 COLOR_RED;
    static const uint32 COLOR_PINK;
    static const uint32 COLOR_YELLOW;
    static const uint32 COLOR_WHITE;
    static const char TXT_CMD_HIGHLIGHT;
    static const char TXT_CMD_LOWLIGHT;
    static const char TXT_CMD_LINK;
    static const char TXT_CMD_SOUND;
    static const char TXT_SUB_CMD_COLOR; // followed by 8 chars (atoi -> uint32).
    static const char TXT_CMD_CHANGE_FONT; // followed by 2 chars (atoi -> char).
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
    void Print(TextLine *line, Texture *texture);
    void PrintToBuffer(int width, int height, uint32 *dest_data, const char*text, unsigned int font, uint32 color = COLOR_WHITE);
    int CalcTextWidth(const char *text, unsigned int fontNr = 0);
    int getFontHeight(int fontNr)
    {
        return mvFont[fontNr]->height;
    }
    int getMaxFontHeight()
    {
        return mMaxFontHeight;
    }
    int getCharWidth(int fontNr, int Char);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        TXT_STATE_HIGHLIGHT =1,
        TXT_STATE_LOWLIGHT,
        TXT_STATE_LINK,
        TXT_STATE_SUM
    };
    enum
    {
        MIN_FONT_SIZE =  4, MAX_FONT_SIZE = 80,
        MIN_RESO_SIZE = 55, MAX_RESO_SIZE = 96
    };

    typedef struct
    {
        uint32 *data;
        unsigned short textureWidth;
        unsigned short height;
        unsigned short baseline;
        unsigned short charStart[CHARS_IN_FONT];
        unsigned char  charWidth[CHARS_IN_FONT];
    }
    mFont;
    typedef struct
    {
        int x, y, w, h;
        String strGfxCode;
    }
    mSpecialChar;

    std::vector<mFont*>mvFont;
    std::vector<mSpecialChar*>mvSpecialChar;
    PixelBox *mPb;
    uint32 *mTextGfxBuffer;
    unsigned int mMaxFontHeight;
    static const uint32 TXT_COLOR_DEFAULT;
    static const uint32 TXT_COLOR_LOWLIGHT;
    static const uint32 TXT_COLOR_HIGHLIGHT;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiTextout();
    ~GuiTextout();
    GuiTextout(const GuiTextout&); // disable copy-constructor.
    void drawText(int width, int height, uint32 *dest_data, String text, bool hideText, unsigned int fontNr = 0, uint32 color = 0x00ffffff);
};

#endif
