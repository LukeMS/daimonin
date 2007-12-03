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

#include <tinyxml.h>
#include <Ogre.h>
#include "gui_element.h"
#include "gui_gadget_scrollbar.h"

/**
 ** Scrollbar class
 ** which manages the display of scrollable text and/or graphics in a window.
 ** The type can be GFX_FILL or COLOR_FILL.
 *****************************************************************************/
class GuiListbox : public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef void (Callback) (class GuiWindow *parent, int index, int line);

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiListbox(TiXmlElement *xmlElement, void *parent);
    ~GuiListbox();
    void draw();
    void clear();
    bool mouseEvent(int MouseAction, int x, int y, int z);
    int  addTextline(const char *text, Ogre::uint32 color);
    const char *extractFirstLineOfText(const char &text);
    const char *getSelectedKeyword(); /**< Returns the keyword found in the selected line. **/
    void setFunction(Callback *c)
    {
        mCallFunc = c;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {SIZE_STRING_BUFFER = 1 << 7};  /**< MUST be power of 2. **/
    struct _row
    {
        Ogre::String str;
        Ogre::uint32 color;
        int keyword_clipped;
        unsigned char startLine; /**< Offset to the start line of the mulitline text. **/
    }
    row[SIZE_STRING_BUFFER];
    Ogre::Real mClose;                 /**< If closed, only the headline is visible. **/
    Ogre::Real mLastHeight;            /**< The height before window was closed. **/
    Ogre::Real mMinHeight, mMaxHeight;
    Ogre::Real mFirstYPos;
    bool mIsClosing, mIsOpening; /**< User pressed open/close button. **/
    bool mVisible;
    bool mDragging;
    int  mVScrollOffset;         /**< At which amount the scrollbar was scrolled. **/
    int  mPixelScroll;           /**< Number of pixel already scrolled. **/
    int  mRowsToScroll;          /**< Rows left to scroll. **/
    int  mMaxVisibleRows;        /**< Number of rows fitting into the window. **/
    int  mPrintPos;
    int  mBufferPos;
    int  mFontHeight;
    int  mActLines;
    int  mSelectedLine;
    int  mGfxBufferSize;
    int  mKeyStart, mKeyCount;

    Ogre::uint32 *mGfxBuffer;
    unsigned long mTime;
    class GuiGadgetScrollbar *mScrollBarH, *mScrollBarV;
    Callback *mCallFunc;
    int mLine;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void activated(int line)
    {
        if (mCallFunc) mCallFunc((GuiWindow *)mParent, mIndex, line);
    }
    static void scrollbarAction(GuiListbox *me, int index, int scroll);
    void scrollTextVertical(int offset);
    void scrollTextHorizontal(int offset);
};

#endif
