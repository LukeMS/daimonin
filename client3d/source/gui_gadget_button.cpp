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

#include <OgreHardwarePixelBuffer.h>
#include "gui_gadget_button.h"
#include "gui_textout.h"
#include "logger.h"

GuiGadgetButton::GuiGadgetButton(TiXmlElement *xmlElement, int w, int h, int maxX, int maxY) : GuiGadget(xmlElement, w, h, maxX, maxY)
{
}

GuiGadgetButton::~GuiGadgetButton()
{
}

///================================================================================================
/// .
///================================================================================================
void GuiGadgetButton::draw(PixelBox &mSrcPixelBox, Texture *texture)
{
    /// ////////////////////////////////////////////////////////////////////
    /// Draw gaget.
    /// ////////////////////////////////////////////////////////////////////

    PixelBox src = mSrcPixelBox.getSubVolume(Box(
                       gfxSrcPos[mState].x,
                       gfxSrcPos[mState].y,
                       gfxSrcPos[mState].x + mWidth,
                       gfxSrcPos[mState].y + mHeight));
    texture->getBuffer()->blitFromMemory(src, Box(mX, mY, mX + mWidth, mY + mHeight));
    /// ////////////////////////////////////////////////////////////////////
    /// Draw label.
    /// ////////////////////////////////////////////////////////////////////
    if (mStrLabel != "")
    {
        std::string mStrBgLabel = "~#ff000000"+mStrLabel+"~"; // Black Background for the label.
        TextLine label;
        label.index= -1;
        label.font = mLabelFont;
        label.clipped = false;
        if (mState == STATE_PUSHED)
        {
            label.x1 = mX+ mLabelXPos+1;
            label.x2 = label.x1 + mWidth;
            label.y1 = mY+ mLabelYPos+1;
            label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
        }
        else
        {
            label.x1 = mX+ mLabelXPos;
            label.x2 = label.x1 + mWidth;
            label.y1 = mY+ mLabelYPos;
            label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
        }
        GuiTextout::getSingleton().Print(&label, texture, mStrBgLabel.c_str());
        --label.x1;
        --label.y1;
        GuiTextout::getSingleton().Print(&label, texture, mStrLabel.c_str());
    }
}
