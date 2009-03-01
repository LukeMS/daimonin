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
#ifndef GUI_TEXTOUT_H
#define GUI_TEXTOUT_H

#include <Ogre.h>

/**
 ** This class provides a text printing on gui elements.
 *****************************************************************************/
class GuiTextout
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
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
    static const char TXT_CMD_HIGHLIGHT;
    static const char TXT_CMD_LOWLIGHT;
    static const char TXT_CMD_LINK;
    static const char TXT_CMD_SOUND;
    static const char TXT_SUB_CMD_COLOR; // followed by 8 chars (atoi -> uint32).
    static const char TXT_CMD_CHANGE_FONT; // followed by 2 chars (atoi -> char).
    static const char CURSOR[];
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
    void printText(int width, int height, Ogre::uint32 *dst, int dstLineSkip,
        Ogre::uint32 *bak, int bakLineSkip, const char *txt, unsigned int fontNr,
        Ogre::uint32 color = 0xffffffff, bool hideText = false);
    int calcTextWidth(const unsigned char *text, unsigned int fontNr = 0);
    int getFontHeight(int fontNr)
    {
        return mvFont[fontNr]->height;
    }
    int getCharWidth(int fontNr, unsigned char Char);
    void parseUserDefinedChars(Ogre::String &txt);

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
        Ogre::uint32 *data;
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
        Ogre::String strGfxCode;
    }
    mSpecialChar;

    std::vector<mFont*>mvFont;
    std::vector<mSpecialChar*>mvSpecialChar;
    static const Ogre::uint32 TXT_COLOR_DEFAULT;
    static const Ogre::uint32 TXT_COLOR_LOWLIGHT;
    static const Ogre::uint32 TXT_COLOR_HIGHLIGHT;
    static const Ogre::uint32 TXT_COLOR_LINK;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiTextout();
    ~GuiTextout();
    GuiTextout(const GuiTextout&); // disable copy-constructor.
};

#endif
