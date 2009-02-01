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
//
//================================================================================================
int GuiGadgetButton::sendMsg(int message, void *parm1, void *parm2, void *parm3)
{
    switch (message)
    {
        case GuiManager::MSG_GET_KEY_EVENT:
            return keyEvent((const char*)parm1, (const unsigned char*)parm2);
        case GuiManager::MSG_GET_MOUSE_EVENT:
            return mouseEvent((int*)parm1, (int*)parm2, (int*)parm3);
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
GuiGadgetButton::GuiGadgetButton(TiXmlElement *xmlElement, void *parent, bool drawOnInit):GuiElement(xmlElement, parent)
{
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
// .
//================================================================================================
void GuiGadgetButton::setVisible(bool visible)
{
    if (visible == mIsVisible) return;
    mIsVisible = visible;
    draw();
}

//================================================================================================
//
//================================================================================================
int GuiGadgetButton::keyEvent(const char *keyChar, const unsigned char *key)
{
    return 0;
}


//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
int GuiGadgetButton::mouseEvent(int *MouseAction, int *x, int *y)
{
    if (!mouseWithin(*x, *y))
    {
        // Mouse is no longer over the the gadget.
        if (getState() != GuiImageset::STATE_ELEMENT_DEFAULT)
        {
            mMouseOver = false;
            mMouseButDown = false;
            if (setState(GuiImageset::STATE_ELEMENT_DEFAULT)) draw();
            GuiManager::getSingleton().setTooltip("");
            return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
        }
    }
    else
    {
        if (!mMouseOver)
        {
            mMouseOver = true;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (*MouseAction == GuiWindow::BUTTON_PRESSED && !mMouseButDown)
        {
            mMouseButDown = true;
            if (setState(GuiImageset::STATE_ELEMENT_PUSHED)) draw();
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (*MouseAction == GuiWindow::BUTTON_RELEASED && mMouseButDown)
        {
            mMouseButDown = false;
            if (setState(GuiImageset::STATE_ELEMENT_DEFAULT)) draw();
            return GuiManager::EVENT_USER_ACTION;
        }
        return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
    }
    return GuiManager::EVENT_CHECK_NEXT; // No action here, check the other gadgets.
}

//================================================================================================
// Draw the guiElement.
//================================================================================================
void GuiGadgetButton::draw()
{
    GuiElement::draw();
    // ////////////////////////////////////////////////////////////////////
    // Draw label.
    // ////////////////////////////////////////////////////////////////////
    if (!mIsVisible) return;
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
