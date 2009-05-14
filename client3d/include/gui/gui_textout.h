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
 ** This class provides printing abilities for all gui elements.
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
    /// Special chars to access gui functions.
    /// The link- and the info-cmd are clickable with the mouse in a listbox-element.
    static const char TXT_CMD_SEPARATOR;   /**< Used for various separator functions. **/
    static const char TXT_CMD_LINK;        /**< Used to encode a link-command in the text. **/
    static const char TXT_CMD_INFO;        /**< Used to encode an info-command in the text. **/
    static const char TXT_CMD_LOWLIGHT;    /**< Used to encode a color toggle in the text. **/
    static const char TXT_CMD_HIGHLIGHT;   /**< Used to encode a color toggle. Can be followed by a TXT_SUB_CMD_COLOR **/
    static const char TXT_SUB_CMD_COLOR;   /**< Followed by 8 hex-chars to encode a color toggle in the text. **/
    static const char TXT_CMD_CHANGE_FONT; /**< Followed by 2 hex-chars to encode a font change in the text. **/
    static const char TXT_CMD_SOUND;       /**< Followed by a filename to encode a sound in the text. **/
    static const char CURSOR[];
    static const Ogre::uint32 TXT_COLOR_DEFAULT;
    static const Ogre::uint32 TXT_COLOR_LOWLIGHT;
    static const Ogre::uint32 TXT_COLOR_HIGHLIGHT;
    static const Ogre::uint32 TXT_COLOR_LINK;
    static const Ogre::uint32 TXT_COLOR_INFO;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiTextout &getSingleton()
    {
        static GuiTextout Singleton; return Singleton;
    }
    /** Loads an internal raw format font.
     ** @param filename The filename of the font.
     *****************************************************************************/
     void loadRawFont(const char *filename);

     /** Loads a TTFont and converts it to the internal format.
      ** @param filename      The filename of the font.
      ** @param size          The point size of the font.
      ** @param resolution    The resolution (dpi) of the font.
      ** @param createRawFont Save the font as internal raw format font.
      *****************************************************************************/
    void loadTTFont (const char *filename, const char *size, const char *resolution, bool createRawFont);

    /** Replace the key-code of user defined chars by their "ascii"-code in a given text.
     ** @param text The text to process.
     ** @note  These chars are, of course, not part of the ascii-code, but you get the point.
     *****************************************************************************/
    void parseUserDefinedChars(Ogre::String &text);

    /** Returns the color-code for the last char in the given text.
     ** @param text The text to check.
     *****************************************************************************/
    const char *getTextendColor(const Ogre::String &text);

    /** Alphablends a text with a single background color into a buffer.
     ** @param width       The width of the textfield. All beyond this with will be clipped.
     ** @param height      The height of the textfield. All beyond this height will be clipped.
     ** @param dst         A pointer to the destination buffer.
     ** @param dstLineSkip The lineskip of the destination buffer.
     ** @param colorBG     The color of the background to draw.
     ** @param text        The text to print.
     ** @param fontNr      The font-number of the text.
     ** @param fontColor   The color of the text.
     ** @param hideText    If true the text is hidden e.g. for printing passwords.
     ** @param borderColor If set, the text will be printed with a colored border.
     *****************************************************************************/
    void printText(int width, int height,
                   Ogre::uint32 *dst, int dstLineSkip,
                   Ogre::uint32 colorBG,
                   const char *text, unsigned int fontNr,
                   Ogre::uint32 fontColor = 0xffffffff, bool hideText = false, Ogre::uint32 borderColor = 0)
    {
        printText(width, height, dst, dstLineSkip, &colorBG, 0, text, fontNr, fontColor, hideText, borderColor);
    }

    /** Alphablends a text with the given buffer.
     ** @param width       The width of the textfield. All beyond this with will be clipped.
     ** @param height      The height of the textfield. All beyond this height will be clipped.
     ** @param dst         A pointer to the buffer.
     ** @param dstLineSkip The lineskip of the buffer.
     ** @param text        The text to print.
     ** @param fontNr      The font-number of the text.
     ** @param fontColor   The color of the text.
     ** @param hideText    If true the text is hidden e.g. for printing passwords.
     ** @param borderColor If set, the text will be printed with a colored border.
     *****************************************************************************/
    void printText(int width, int height,
                   Ogre::uint32 *dst, int dstLineSkip,
                   const char *text, unsigned int fontNr,
                   Ogre::uint32 fontColor = 0xffffffff, bool hideText = false, Ogre::uint32 borderColor = 0)
    {
        printText(width, height, dst, dstLineSkip, dst, dstLineSkip, text, fontNr, fontColor, hideText, borderColor);
    }

    /** Alphablends a text with a graphical background into a buffer.
     ** @param width       The width of the textfield. All beyond this with will be clipped.
     ** @param height      The height of the textfield. All beyond this height will be clipped.
     ** @param dst         A pointer to the destination buffer.
     ** @param dstLineSkip The lineskip of the destination buffer.
     ** @param bak         A pointer to the background buffer.
     ** @param bakLineSkip The lineskip of the background buffer.
     ** @param text        The text to print.
     ** @param fontNr      The font-number of the text.
     ** @param fontColor   The color of the text.
     ** @param hideText    If true the text is hidden e.g. for printing passwords.
     ** @param borderColor If set, the text will be printed with a colored border.
     *****************************************************************************/
    void printText(int width, int height,
                   Ogre::uint32 *dst, int dstLineSkip,
                   Ogre::uint32 *bak, int bakLineSkip,
                   const char *text, unsigned int fontNr,
                   Ogre::uint32 fontColor = 0xffffffff, bool hideText = false, Ogre::uint32 borderColor = 0);

    /** Returns the height (in pixel) for the given font-number.
     ** @param fontNr The font-number.
     ** @warning No bound check for the font-number is done.
     *****************************************************************************/
    int getFontHeight(int fontNr)
    {
        return mvFont[fontNr]->height;
    }
    /** Returns the number of the last char that fits into a textfield with the given width.
     ** @param text   The text to check.
     ** @param fontNr The font-number of the text.
     ** @param width  The width of the textfield in pixel.
     *****************************************************************************/
    int getLastCharPosition(const char *text, unsigned int fontNr, int width);

    /** Returns the width of the textfield needed for the given text.
     ** @param text   The text to check.
     ** @param fontNr The font-number of the text.
     *****************************************************************************/
    int calcTextWidth(const char *text, unsigned int fontNr = 0);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
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
        Ogre::uchar  charWidth[CHARS_IN_FONT];
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

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiTextout();
    ~GuiTextout();
    /// Disable copy-constructor.
    GuiTextout(const GuiTextout&);

    /** Returns the pixel width of a char within a font.
     ** @param fontNr The number of the font.
     ** @param char   The character to check.
     ** @warning No bound check for the font-number is done.
     *****************************************************************************/
    int getCharWidth(int fontNr, Ogre::uchar Char);

    /** Converts a text with a hexadecimal value into a uint32 value.
     ** @param  text  The text to convert.
     ** @param  len   The length of the text to convert in bytes.
     ** @param  value A container to store the result.
     ** @return Returns the number of the converted chars.
     *****************************************************************************/
    int hexToInt(const char *text, int len, Ogre::uint32 &result);
};

#endif
