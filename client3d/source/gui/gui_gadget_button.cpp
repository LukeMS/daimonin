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

#include "logger.h"
#include "gui_gadget_button.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_manager.h"

using namespace Ogre;

//================================================================================================
// .
//================================================================================================
GuiGadgetButton::GuiGadgetButton(TiXmlElement *xmlElement, void *parent, bool drawOnInit):GuiGraphic(xmlElement, parent, drawOnInit)
{
    mCallFunc = 0;
    mMouseOver = false;
    mMouseButDown = false;
    if (drawOnInit) draw();
}

//================================================================================================
// .
//================================================================================================
GuiGadgetButton::~GuiGadgetButton()
{}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
bool GuiGadgetButton::mouseEvent(int MouseAction, int x, int y)
{
    if (x >= mPosX && x <= mPosX + mWidth && y >= mPosY && y <= mPosY + mHeight)
    {
        if (!mMouseOver)
        {
            mMouseOver = true;
            setState(GuiImageset::STATE_ELEMENT_M_OVER);
            GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            return true;
        }
        if (MouseAction == GuiWindow::BUTTON_PRESSED && !mMouseButDown)
        {
            mMouseButDown = true;
            setState(GuiImageset::STATE_ELEMENT_PUSHED);
            return true;
        }
        if (MouseAction == GuiWindow::BUTTON_RELEASED && mMouseButDown)
        {
            mMouseButDown = false;
            setState(GuiImageset::STATE_ELEMENT_DEFAULT);
            activated();
        }
        return true; // No need to check other gadgets.
    }
    else // Mouse is no longer over the the gadget.
    {
        if (getState() != GuiImageset::STATE_ELEMENT_DEFAULT)
        {
            mMouseOver = false;
            mMouseButDown = false;
            setState(GuiImageset::STATE_ELEMENT_DEFAULT);
            GuiManager::getSingleton().setTooltip("");
            return true; // No need to check other gadgets.
        }
    }
    return false; // No action here, check the other gadgets.
}

//================================================================================================
// Draw the guiElement.
//================================================================================================
void GuiGadgetButton::draw()
{
    GuiGraphic::draw();
    // ////////////////////////////////////////////////////////////////////
    // Draw label.
    // ////////////////////////////////////////////////////////////////////
    if (mStrLabel != "")
    {
        Texture *texture = mParent->getTexture();
        GuiTextout::TextLine label;
        label.hideText= false;
        label.index= -1;
        label.font = mLabelFontNr;
        label.color= 0x00ffffff;
        label.x1 = mPosX+ mLabelPosX;
        label.y1 = mPosY+ mLabelPosY;
        label.x2 = label.x1 + mWidth;
        label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
        if (mState == GuiImageset::STATE_ELEMENT_PUSHED)
        {
            ++label.x1;
            ++label.y1;
        }
        label.text = "";
        for (unsigned int i=0; i < mStrLabel.size(); ++i)
            if (mStrLabel[i] != '~') label.text+=mStrLabel[i];
        label.color= 0;
        GuiTextout::getSingleton().Print(&label, texture);
        --label.x1;
        --label.y1;
        label.text = mStrLabel;
        label.color= 0x00ffffff;
        GuiTextout::getSingleton().Print(&label, texture);
    }
}
