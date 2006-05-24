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

#ifndef GUI_LISTBOX_H
#define GUI_LISTBOX_H

#include <string>
#include <tinyxml.h>
#include <Ogre.h>
#include "gui_element.h"
#include "gui_textout.h"

using namespace Ogre;

static const int  SIZE_STRING_BUFFER = 128; // MUST be 2^X.

class GuiListbox : public GuiElement
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    GuiListbox(TiXmlElement *xmlElement, int w, int h, int maxX, int maxY):GuiElement(xmlElement, w, h, maxX, maxY)
    {
        /// ////////////////////////////////////////////////////////////////////
        /// Create buffer to hold the pixel information of the listbox.
        /// ////////////////////////////////////////////////////////////////////
        mFontHeight = GuiTextout::getSingleton().getFontHeight(mFontNr);
        int size = mWidth * mHeight + mWidth * (mFontHeight+1);
        mGfxBuffer = new uint32[size];
        for (int i =0; i < size; ++i) mGfxBuffer[i] = mFillColor;
        /// ////////////////////////////////////////////////////////////////////
        /// Set defaults.
        /// ////////////////////////////////////////////////////////////////////
        mIsClosing    = false;
        mIsOpening    = false;
        mDragging     = false;
        mBufferPos    = 0;
        mPrintPos     = 0;
        mRowsToScroll = 0;
        mRowsToPrint  = mHeight / mFontHeight;
        mScroll       = 0;
    }
    ~GuiListbox();
    void draw(PixelBox &, Texture *texture);
    const char *extractFirstLineOfText(const char &text);
    void addTextline(const char *text);
    int getIndex()
    {
        return mIndex;
    }
    void setIndex(int index)
    {
        mIndex = index;
    }
private:

    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    struct _row
    {
        std::string str;
        //    ColourValue colorTop;
        //    ColourValue colorBottom;
    }
    row[SIZE_STRING_BUFFER];

    Real mClose;                 // Only Headline visible.
    Real mLastHeight;            // The height before window was closed.
    Real mMinHeight, mMaxHeight;
    Real mFirstYPos;
    bool mIsClosing, mIsOpening; // User pressed open/close button.
    bool mVisible;
    bool mDragging;
    int  mScroll;
    uint32 *mGfxBuffer;
    int  mRowsToScroll, mRowsToPrint;
    int  mSumRows;
    int  mPrintPos;
    int  mBufferPos;
    int  mIndex;
    int  mFontHeight;
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
};

#endif
