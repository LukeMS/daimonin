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

#include "logger.h"
#include "gui_gadget_button.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_manager.h"

//================================================================================================
// .
//================================================================================================
GuiGadgetButton::GuiGadgetButton(TiXmlElement *xmlElement, void *parent, bool drawOnInit):GuiElement(xmlElement, parent)
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
            draw();
            GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
        }
        if (MouseAction == GuiWindow::BUTTON_PRESSED && !mMouseButDown)
        {
            mMouseButDown = true;
            setState(GuiImageset::STATE_ELEMENT_PUSHED);
            draw();
        }
        if (MouseAction == GuiWindow::BUTTON_RELEASED && mMouseButDown)
        {
            mMouseButDown = false;
            setState(GuiImageset::STATE_ELEMENT_DEFAULT);
            activated();
        }
        return true; // No need to check other gadgets.
    }
    else  // Mouse is no longer over the the gadget.
    {
        if (mMouseOver)
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
    // ////////////////////////////////////////////////////////////////////
    // Draw gaget.
    // ////////////////////////////////////////////////////////////////////
    PixelBox src;
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    if (mHasAlpha)
    {
        src = ((GuiWindow*) mParent)->getPixelBox()->getSubVolume(Box(
                    gfxSrcPos[mState].x,
                    gfxSrcPos[mState].y,
                    gfxSrcPos[mState].x + mWidth,
                    gfxSrcPos[mState].y + mHeight));
        uint32 *srcData = static_cast<uint32*>(src.data);
        size_t rowSkip = ((GuiWindow*) mParent)->getPixelBox()->getWidth();
        int dSrcY = 0, dDstY =0;
        for (int y =0; y < mHeight; ++y)
        {
            for (int x =0; x < mWidth; ++x)
            {
                if (srcData[dSrcY + x] <= 0xffffff) continue;
                BG_Backup[dDstY + x] = srcData[dSrcY + x];
            }
            dSrcY+= (int)rowSkip;
            dDstY+= mWidth;
        }


        src = PixelBox(mWidth, mHeight, 1, PF_A8B8G8R8, BG_Backup);
        texture->getBuffer()->blitFromMemory(src, Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
    }
    else
    {
        src = ((GuiWindow*) mParent)->getPixelBox()->getSubVolume(Box(
                    gfxSrcPos[mState].x,
                    gfxSrcPos[mState].y,
                    gfxSrcPos[mState].x + mWidth,
                    gfxSrcPos[mState].y + mHeight));
        texture->getBuffer()->blitFromMemory(src, Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
    }

    // ////////////////////////////////////////////////////////////////////
    // Draw label.
    // ////////////////////////////////////////////////////////////////////
    if (mStrLabel != "")
    {
        std::string strBgLabel = "~#ff000000"+mStrLabel+"~"; // Black Background for the label.
        TextLine label;
        label.hideText= false;
        label.index= -1;
        label.font = mLabelFontNr;
        label.x1 = mPosX+ mLabelPosX;
        label.y1 = mPosY+ mLabelPosY;
        label.x2 = label.x1 + mWidth;
        label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
        if (mState == GuiImageset::STATE_ELEMENT_PUSHED)
        {
            ++label.x1;
            ++label.y1;
        }
        label.text = strBgLabel;
        GuiTextout::getSingleton().Print(&label, texture);
        --label.x1;
        --label.y1;
        label.text = mStrLabel;
        GuiTextout::getSingleton().Print(&label, texture);
    }
}
