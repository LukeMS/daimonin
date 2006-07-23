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

#ifndef GUI_LISTBOX_H
#define GUI_LISTBOX_H

#include <tinyxml.h>
#include <Ogre.h>
#include "gui_element.h"
#include "gui_textout.h"
#include "gui_gadget_button.h"

using namespace Ogre;

class GuiListbox : public GuiElement
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    GuiListbox(TiXmlElement *xmlElement, void *parent);
    ~GuiListbox();
    void draw();
    bool mouseEvent(int MouseAction, int x, int y);
    const char *extractFirstLineOfText(const char &text);
    void addTextline(const char *text);

private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    enum { SIZE_STRING_BUFFER = 128 }; // MUST be 2^X.

    struct _row
    {
        String str;
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
    int mButScrollWidth, mButScrollHeight;
    uint32 *mGfxBuffer;
    int  mRowsToScroll, mRowsToPrint;
    int  mSumRows;
    int  mPrintPos;
    int  mBufferPos;
    int  mFontHeight;
    class GuiGadgetButton *mButScrollUp, *mButScrollDown;
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    void drawScrollbar();
};

#endif
