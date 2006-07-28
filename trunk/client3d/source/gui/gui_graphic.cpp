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
#include "gui_graphic.h"
#include "gui_window.h"

//================================================================================================
// .
//================================================================================================
GuiGraphic::GuiGraphic(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    draw();
}

//================================================================================================
// .
//================================================================================================
GuiGraphic::~GuiGraphic()
{
}

//================================================================================================
// .
//================================================================================================
void GuiGraphic::draw()
{
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    PixelBox *mSrcPixelBox = ((GuiWindow*) mParent)->getPixelBox();
    // ////////////////////////////////////////////////////////////////////
    // Fill background rect with a gfx.
    // ////////////////////////////////////////////////////////////////////
    if (mStrType == "GFX_FILL")
    {
        int x1, y1, x2, y2;
        PixelBox src;
        bool dirty = true;
        int sumX = (mWidth-1)  / mSrcWidth  + 1;
        int sumY = (mHeight-1) / mSrcHeight + 1;
        y1 = 0; y2 = mSrcHeight;

        for (int y = 0; y < sumY; ++y)
        {
            if (dirty)
            {
                src = mSrcPixelBox->getSubVolume(Box(
                                                     gfxSrcPos[mState].x,
                                                     gfxSrcPos[mState].y,
                                                     gfxSrcPos[mState].x + mSrcWidth,
                                                     gfxSrcPos[mState].y + mSrcHeight));
                dirty = false;
            }
            if (y2 > mHeight)
            {
                y2 = mHeight;
                if (y1 > mHeight) y1 = mHeight-1;
                dirty = true;
            }
            x1 = 0; x2 = mSrcWidth;
            for (int x = 0; x < sumX; ++x)
            {
                if (x2 > mWidth)
                {
                    x2 = mWidth;
                    if (x1 >= x2) x1 = x2-1;
                    dirty = true;
                }
                if (dirty)
                {
                    src = mSrcPixelBox->getSubVolume(Box(
                                                         gfxSrcPos[mState].x,
                                                         gfxSrcPos[mState].y,
                                                         gfxSrcPos[mState].x + x2-x1,
                                                         gfxSrcPos[mState].y + y2-y1));
                }
                texture->getBuffer()->blitFromMemory(src, Box(x1 + mX, y1 + mY, x2 + mX, y2 + mY));
                x1 = x2;
                x2+= mSrcWidth;
            }
            y1 = y2;
            y2+= mSrcHeight;
        }
    }

    // ////////////////////////////////////////////////////////////////////
    // Fill background rect without destroying the previrious layer.
    // ////////////////////////////////////////////////////////////////////
    else if (mStrType == "GFX_ALPHA_FILL")
    {}
    // ////////////////////////////////////////////////////////////////////
    // Fill background rect with a color.
    // ////////////////////////////////////////////////////////////////////
    else if (mStrType == "COLOR_FILL")
    {
        PixelBox pb = texture->getBuffer()->lock(Box(mX, mY, mX+mWidth, mY+mHeight), HardwareBuffer::HBL_READ_ONLY );
        uint32 *dest_data = (uint32*)pb.data;
        for (int y = 0; y < mHeight; ++y)
        {
            for (int x= 0; x < mWidth; ++x)
            {
                dest_data[x+y*texture->getWidth()] = mFillColor;
            }
        }
        texture->getBuffer()->unlock();
    }
}
