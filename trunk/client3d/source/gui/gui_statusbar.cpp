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

#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_statusbar.h"
#include "logger.h"


// TODO
// 3d    type hor/vert
// plain type hor/vert
// gfx   type hor/vert

const int  BAR_WIDTH = 16;

///================================================================================================
/// .
///================================================================================================
void GuiStatusbar::draw(PixelBox &, Texture *texture)
{
    /// ////////////////////////////////////////////////////////////////////
    /// Save the original background.
    /// ////////////////////////////////////////////////////////////////////
    if (!mGfxBuffer)
    {
        if (mWidth < BAR_WIDTH) mWidth = BAR_WIDTH;
        mGfxBuffer = new uint32[mWidth * mHeight];
        texture->getBuffer()->blitToMemory(
            Box(mX, mY, mX + mWidth, mY + mHeight),
            PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer));
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Restore background into a tmp buffer.
    /// ////////////////////////////////////////////////////////////////////

    //TODO

    /// ////////////////////////////////////////////////////////////////////
    /// Draw the bar into a temp buffer.
    /// ////////////////////////////////////////////////////////////////////
    int x, y, offset;
    uint32 color;
    uint32 dColor = 0x00000000;
    dColor+=(((mFillColor & 0x00ff0000)/ 6) & 0x00ff0000);
    dColor+=(((mFillColor & 0x0000ff00)/ 6) & 0x0000ff00);
    dColor+=(((mFillColor & 0x000000ff)/ 6) & 0x000000ff);

    /// Draw top of the bar.
    color = mFillColor;
    for (offset =3, y= mValue-5; y < mValue; ++y)
    {
        for (x=offset; x <= BAR_WIDTH-offset; ++x) mGfxBuffer[y*mWidth +x] = color;
        if (y < mValue-3) --offset;
        color+= dColor;
    }

    /// Draw the bar.
    color = 0xff000000;
    for (offset= 3, x=0; x <= BAR_WIDTH/2; ++x)
    {
        if (x == 1 || x == 3) --offset;
        //    for (y = mValue+5-offset; y < mHeight-offset; ++y)
        for (y = mHeight-offset; y > mValue-offset; --y)
        {
            mGfxBuffer[y*mWidth + x] = color;
            mGfxBuffer[y*mWidth + BAR_WIDTH-x] = color;
        }
        color+= dColor;
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Blit buffer into the window-texture.
    /// ////////////////////////////////////////////////////////////////////
    texture->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
        Box(mX, mY, mX + mWidth, mY + mHeight));
}

///================================================================================================
/// .
///================================================================================================
void GuiStatusbar::setValue(Real value)
{
    mValue = (int) (mHeight * (1-value));
    if (mValue > mHeight) mValue = mHeight;
    if (mValue < 5) mValue = 5;
}
