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

#ifndef GUI_ELEMENT_SCROLLBAR_H
#define GUI_ELEMENT_SCROLLBAR_H

#include "gui_element.h"
#include "gui_element_button.h"

/**
 ** This class provides an interactive scrollbar.
 ** Its a helper class for scrolling graphical stuff within gui elements.
 *****************************************************************************/
class GuiElementScrollbar : public GuiElement
{

public:
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
    GuiElementScrollbar(TiXmlElement *xmlElement, void *parent, void *parentElement);
    ~GuiElementScrollbar();
    //int sendMsg(int message, const char *text, Ogre::uint32 param);
    int getScrollOffset();
    int mouseEvent(int MouseAction, int x, int y, int mouseWheel);
    void stopDragging() { mDragging = false; mMouseButDown = false; }
    void resize(int newWidth, int newHeight);
    void updateSliderSize(int actPos, int scrollOffset, int maxVisPos, int maxPos = -1);
    void draw();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    int mSliderPos, mSliderSize, mMaxSliderSize;
    int mStartX, mStopX, mStartY, mStopY;
    int mLastScrollAmount;
    bool mHorizontal, mDragging;
    bool mMouseOver, mMouseButDown;
    float mPixelScrollToLineScroll; /**< When slider scrolls 1 pixel, the parent element must scroll x lines. **/
    Ogre::uint32 mColorBackground, mColorBorderline, mColorBarPassive, mColorBarM_Over, mColorBarActive;
    class GuiElementButton *mButScrollUp, *mButScrollDown;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void arrangeButtons(int x1, int y1, int x2, int y2);
    bool mouseOverSlider(int x, int y);
};

#endif
