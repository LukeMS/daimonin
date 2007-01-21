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

#ifndef GUI_GADGET_SCROLLBAR_H
#define GUI_GADGET_SCROLLBAR_H

#include <tinyxml.h>
#include <Ogre.h>
#include "gui_element.h"
#include "gui_listbox.h"
#include "gui_gadget_button.h"

/**
 ** This class provides an interactive scrollbar.
 ** Its a helper class for scrolling graphical stuff within gui elements.
 *****************************************************************************/
class GuiGadgetScrollbar : public GuiElement
{

public:
    typedef void (Callback) (class GuiListbox *parentElement, int index, int value);
    enum
    {
        // Horizontal Elements.
        BUTTON_H_ADD,
        BUTTON_H_SUB,
        SLIDER_H,
        // Vertical Elements.
        BUTTON_V_ADD,
        BUTTON_V_SUB,
        SLIDER_V
    };
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiGadgetScrollbar(TiXmlElement *xmlElement, void *parent, void *parentElement);
    ~GuiGadgetScrollbar();
    void resize(int newWidth, int newHeight);
    void updateSliderSize(int maxPos, int maxVisPos, int actPos);
    void setFunction(Callback *c)
    {
        mCallFunc = c;
    }
    bool mouseEvent(int MouseAction, int x, int y);
    void draw();
private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    int mSliderPos,  mMaxSliderPos;
    int mSliderSize, mMaxSliderSize;
    int mStartX, mStopX, mStartY, mStopY;
    bool mHorizontal, mDragging;
    bool mMouseOver, mMouseButDown;
    Ogre::uint32 *mGfxBuffer;
    Ogre::uint32 mColorBackground, mColorBorderline, mColorBarPassive, mColorBarM_Over, mColorBarActive;
    float mSingleLineSize;
    class GuiGadgetButton *mButScrollUp, *mButScrollDown;
    void *mParent, *mParentElement;
    Callback *mCallFunc;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void updateSliderPos(int type, int offset);
    void activated(int index, int value)
    {
        if (mCallFunc) mCallFunc((GuiListbox *)mParentElement, index, value);
    }
};

#endif
