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
#include "gui_element_button.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_manager.h"

using namespace Ogre;

//================================================================================================
//
//================================================================================================
int GuiElementButton::sendMsg(int message, void *parm1, void *parm2, void *parm3)
{
    switch (message)
    {
        case GuiManager::MSG_SET_VISIBLE:
            setVisible((bool*)parm1);
            return 0;
        default:
            return -1;
    }
}

//================================================================================================
// .
//================================================================================================
GuiElementButton::GuiElementButton(TiXmlElement *xmlElement, void *parent, bool drawOnInit):GuiElement(xmlElement, parent)
{
    mMouseOver = false;
    mMouseButDown = false;
    if (drawOnInit) draw();
}

//================================================================================================
// .
//================================================================================================
GuiElementButton::~GuiElementButton()
{}

//================================================================================================
// .
//================================================================================================
void GuiElementButton::setVisible(bool visible)
{
    if (visible == mIsVisible) return;
    mIsVisible = visible;
    draw();
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
int GuiElementButton::mouseEvent(int MouseAction, int x, int y, int z)
{
    if (!mouseWithin(x, y))
    {
        // Mouse is no longer over the the gadget.
        if (getState() != GuiImageset::STATE_ELEMENT_DEFAULT)
        {
            mMouseOver = false;
            mMouseButDown = false;
            if (setState(GuiImageset::STATE_ELEMENT_DEFAULT)) draw();
            GuiManager::getSingleton().setTooltip("");
            return GuiManager::EVENT_CHECK_NEXT;
        }
    }
    else // Mousecursor is over the button.
    {
        if (!mMouseOver)
        {
            mMouseOver = true;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            //GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            GuiManager::getSingleton().setTooltip("Testing!");
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (MouseAction == GuiManager::BUTTON_PRESSED && !mMouseButDown)
        {
            mMouseButDown = true;
            if (setState(GuiImageset::STATE_ELEMENT_PUSHED)) draw();
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (MouseAction == GuiManager::BUTTON_RELEASED && mMouseButDown)
        {
            mMouseButDown = false;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            return GuiManager::EVENT_USER_ACTION;
        }
        return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
    }
    return GuiManager::EVENT_CHECK_NEXT; // No action here, check the other gadgets.
}

//================================================================================================
// Draw the guiElement.
//================================================================================================
void GuiElementButton::draw()
{
    GuiElement::draw();
    // Draw label.
    if (!mIsVisible || !mStrLabel.size()) return;
    int offX = 0, offY = 0;
    if (mState == GuiImageset::STATE_ELEMENT_PUSHED)
    {
        ++offX;
        ++offY;
    }
    Texture *texture = mParent->getTexture();
    PixelBox pb = texture->getBuffer()->lock(Box(mPosX+mLabelPosX+offX, mPosY+mLabelPosY+offY, mWidth+mPosX-offX, mHeight+mPosY-offY), HardwareBuffer::HBL_NO_OVERWRITE);
    GuiTextout::getSingleton().printText(mWidth-mLabelPosX-2*offX, mHeight-mLabelPosY-2*offY, (uint32*)pb.data, pb.rowPitch,
                                         mParent->getLayerBG() + mPosX+mLabelPosX+offX + (mPosY+mLabelPosY+offY)*mParent->getWidth(), mParent->getWidth(),
                                         mStrLabel.c_str(), mLabelFontNr, 0x00ffffff);
    texture->getBuffer()->unlock();
}
