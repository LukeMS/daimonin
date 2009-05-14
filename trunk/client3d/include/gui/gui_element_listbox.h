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

#ifndef GUI_LISTBOX_H
#define GUI_LISTBOX_H

#include "gui_element.h"
#include "gui_element_scrollbar.h"

/**
 ** This class handles the display of scrollable text and/or graphics.
 ** It has no own backgroud graphics or color.
 *****************************************************************************/
class GuiListbox : public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiListbox(TiXmlElement *xmlElement, const void *parent);
    ~GuiListbox();
    virtual void update(Ogre::Real dTime);
    virtual void sendMsg(const int message, Ogre::String &text, Ogre::uint32 &param, const char *text2);
    virtual int mouseEvent(const int mouseAction, int mouseX, int mouseY, int mouseWheel);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { SIZE_STRING_BUFFER = 1 << 7 }; /**< MUST be power of 2. **/
    enum { MAX_KEYWORD_LEN    = 255    };

    struct _row
    {
        Ogre::uint32 color;
        Ogre::String text;
        Ogre::String keyword;   /**< All keywords in this row. Separated by "#"
                                     OR "$" followed by a number for an item index. **/
    }
    row[SIZE_STRING_BUFFER];
    static Ogre::String mKeywordPressed;
    int  mVScrollOffset;        /**< At which amount the vertical scrollbar was scrolled. **/
    int  mPixelScroll;          /**< Number of pixel already scrolled. **/
    int  mRowsToPrint;          /**< Rows left to print. **/
    int  mMaxVisibleRows;       /**< Number of rows fitting into the listbox. **/
    int  mPrintPos;             /**< First entry in the ring-buffer to print. **/
    int  mBufferPos;            /**< Next free entry in the ring-buffer. **/
    int  mActLines;             /**< Actual filled entries in the ring-buffer **/
    int  mFontHeight;
    GuiElementScrollbar *mScrollBarV;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    int  addText(const char *text, Ogre::uint32 color);
    int  addItem(const int itemId, Ogre::uint32 color);
    void scrollTextVertical(int offset);
    void clear();
    void draw();
    bool extractKeyword(int x, int y);
};

#endif
