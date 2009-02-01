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

#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_element_statusbar.h"
#include "logger.h"
#include "gui_window.h"

using namespace Ogre;

// TODO
// 3d    type hor/vert
// plain type hor/vert
// gfx   type hor/vert

const int  BAR_WIDTH = 16;


int GuiStatusbar::sendMsg(int element, void *parm1, void *parm2, void *parm3)
{
    return 0;
}

//================================================================================================
// .
//================================================================================================
GuiStatusbar::GuiStatusbar(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    mGfxBuffer = 0;
    mValue = -1;
    setValue(1.0); // default: 100%
}

//================================================================================================
// .
//================================================================================================
GuiStatusbar::~GuiStatusbar()
{
    delete[] mGfxBuffer;
}

//================================================================================================
// Todo: Replace blit by direct writing to the texture.
//================================================================================================
void GuiStatusbar::draw()
{
    Texture *texture = mParent->getTexture();
    PixelBox *pb = mParent->getPixelBox();
    // ////////////////////////////////////////////////////////////////////
    // Save the original background.
    // ////////////////////////////////////////////////////////////////////
    if (!mGfxBuffer)
    {
        mGfxBuffer = new uint32[mWidth * mHeight];
        texture->getBuffer()->blitToMemory(
            Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight),
            PixelBox(mWidth, mHeight, 1, PF_A8B8G8R8, mGfxBuffer));
    }
    // ////////////////////////////////////////////////////////////////////
    // Restore background into a tmp buffer.
    // ////////////////////////////////////////////////////////////////////
    //TODO

    // ////////////////////////////////////////////////////////////////////
    // Draw the bar into a temp buffer.
    // ////////////////////////////////////////////////////////////////////
    if (mGfxSrc)
    {
        // ////////////////////////////////////////////////////////////////////
        // Background part.
        // ////////////////////////////////////////////////////////////////////
        uint32 *src = (uint32*)pb->data;
        uint32 color, pixColor;
        for (int y = 0; y < mHeight; ++y)
        {
            for (int x= 0; x < mWidth; ++x)
            {
                color = src[(y+mGfxSrc->state[0].y) * pb->getWidth() + x+mGfxSrc->state[0].x];
                if (color & 0xff000000)
                {
                    mGfxBuffer[y*mWidth + x] = color;
                }
            }
        }
        // ////////////////////////////////////////////////////////////////////
        // The dynamic part.
        // ////////////////////////////////////////////////////////////////////
        uint32 barColor=  0xff000000+ (mLabelColor[2] << 16) + (mLabelColor[1] << 8) +mLabelColor[0];
        for (int y = mLabelPosY + mValue; y < mHeight-mLabelPosY; ++y)
        {
            for (int x= mLabelPosX; x < mWidth-mLabelPosX; ++x)
            {
                color = src[(y+mGfxSrc->state[0].y) * pb->getWidth() + x+mGfxSrc->state[0].x];
                if (color & 0xff000000)
                {
                    pixColor = color & 0xff000000;
                    pixColor+= ((barColor&0x0000ff) < (color& 0x0000ff))? barColor & 0x0000ff : color & 0x0000ff;
                    pixColor+= ((barColor&0x00ff00) < (color& 0x00ff00))? barColor & 0x00ff00 : color & 0x00ff00;
                    pixColor+= ((barColor&0xff0000) < (color& 0xff0000))? barColor & 0xff0000 : color & 0xff0000;
                    mGfxBuffer[y*mWidth + x] = pixColor;
                }
            }
        }
    }
    else
    {
        // ////////////////////////////////////////////////////////////////////
        // Draw the bar into a temp buffer.
        // ////////////////////////////////////////////////////////////////////
        int x, y, offset;
        uint32 color;
        uint32 dColor = 0x00000000;
        dColor+=(((mFillColor & 0x00ff0000)/ 6) & 0x00ff0000);
        dColor+=(((mFillColor & 0x0000ff00)/ 6) & 0x0000ff00);
        dColor+=(((mFillColor & 0x000000ff)/ 6) & 0x000000ff);

        // Draw top of the bar.
        color = mFillColor;
        for (offset =3, y= mValue-5; y < mValue; ++y)
        {
            for (x=offset; x <= BAR_WIDTH-offset; ++x) mGfxBuffer[y*mWidth +x] = color;
            if (y < mValue-3) --offset;
            color+= dColor;
        }

        // Draw the bar.
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
    }
    // ////////////////////////////////////////////////////////////////////
    // Blit buffer into the window-texture.
    // ////////////////////////////////////////////////////////////////////
    texture->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8B8G8R8, mGfxBuffer),
        Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
}

//================================================================================================
// .
//================================================================================================
void GuiStatusbar::setValue(Real value)
{
    int val = (int) ((mHeight-2*mLabelPosY) * (1-value));
    if (val < 0) val = 0;
    if (val > mHeight) val = mHeight;
    if (mValue != val)
    {
        mValue = val;
        draw();
    }
}
