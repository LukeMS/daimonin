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
#include "gui_graphic.h"
#include "gui_window.h"

using namespace Ogre;

const uint32 SLOT_BUSY_COLOR = 0xbb555555;

//================================================================================================
// .
//================================================================================================
GuiGraphic::GuiGraphic(TiXmlElement *xmlElement, void *parent, bool drawOnInit):GuiElement(xmlElement, parent)
{
    mBuildBuffer = (mHasAlpha) ? new uint32[mWidth*mHeight] : 0;
    if (drawOnInit) draw();
}

//================================================================================================
// .
//================================================================================================
GuiGraphic::~GuiGraphic()
{
    delete[] mBuildBuffer;
}

//================================================================================================
// .
//================================================================================================
inline uint32 GuiGraphic::alphaBlend(const uint32 bg, const uint32 gfx)
{
    uint32 alpha = gfx >> 24;
    if (alpha == 0x00) return bg;
    if (alpha == 0xff) return gfx;
    // We need 1 byte of free space before each color (because of the alpha multiplication),
    // so we need 2 operations on the 3 colors.
    uint32 rb = (((gfx & 0x00ff00ff) * alpha) + ((bg & 0x00ff00ff) * (0xff - alpha))) & 0xff00ff00;
    uint32 g  = (((gfx & 0x0000ff00) * alpha) + ((bg & 0x0000ff00) * (0xff - alpha))) & 0x00ff0000;
    return (bg & 0xff000000) | ((rb | g) >> 8);
}

//================================================================================================
// Draws a graphic.
// If the gfx is bigger than the source image, the source image will be repeated.
//================================================================================================
void GuiGraphic::draw()
{
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    PixelBox src;
    // ////////////////////////////////////////////////////////////////////
    // Fill background rect with a gfx.
    // ////////////////////////////////////////////////////////////////////
    if (mFillType == GuiElement::FILL_GFX)
    {
        // ////////////////////////////////////////////////////////////////////
        // The gfx has alpha.
        // ////////////////////////////////////////////////////////////////////
        if (mHasAlpha)
        {
            src = ((GuiWindow*) mParent)->getPixelBox()->getSubVolume(Box(
                        gfxSrcPos[mState].x,
                        gfxSrcPos[mState].y,
                        gfxSrcPos[mState].x + mSrcWidth,
                        gfxSrcPos[mState].y + mSrcHeight));
            uint32 *LayerState = static_cast<uint32*>(src.data);
            int srcRowSkip = (int) ((GuiWindow*) mParent)->getPixelBox()->getWidth();
            uint32 *dst = LayerWindowBG;
            uint32 *buf = mBuildBuffer;
            int srcX, dSrcY = 0, srcY =0;
            for (int y =0; y < mHeight; ++y)
            {
                srcX = 0;
                for (int x =0; x < mWidth; ++x)
                {
                    *buf++ = alphaBlend(*dst++, LayerState[dSrcY + srcX]);
                    if (++srcX >= mSrcWidth) srcX = 0; // Repeat the image.
                }
                dSrcY+= srcRowSkip;
                if (++srcY >= mSrcHeight){ srcY = 0; dSrcY =0; } // Repeat the image.
            }
            src = PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, mBuildBuffer);
            texture->getBuffer()->blitFromMemory(src, Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
        }
        // ////////////////////////////////////////////////////////////////////
        // The gfx has no alpha.
        // ////////////////////////////////////////////////////////////////////
        else
        {
            int x1, y1, x2, y2;
            bool dirty = true;
            int sumX = (mWidth-1)  / mSrcWidth  + 1;
            int sumY = (mHeight-1) / mSrcHeight + 1;
            y1 = 0; y2 = mSrcHeight;
            PixelBox *mSrcPixelBox = ((GuiWindow*) mParent)->getPixelBox();
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
                    if (y1 >= mHeight) y1 = mHeight-1;
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
                    texture->getBuffer()->blitFromMemory(src, Box(x1 + mPosX, y1 + mPosY, x2 + mPosX, y2 + mPosY));
                    x1 = x2;
                    x2+= mSrcWidth;
                }
                y1 = y2;
                y2+= mSrcHeight;
            }
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Fill background rect with a color.
    // ////////////////////////////////////////////////////////////////////
    else if (mFillType == GuiElement::FILL_COLOR)
    {
        PixelBox pb = texture->getBuffer()->lock (Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight), HardwareBuffer::HBL_DISCARD);
        uint32 *dest_data = (uint32*)pb.data;
        int  posY=0;
        for (int y = mHeight; y; --y)
        {
            for (int x= 0; x < mWidth; ++x)
            {
                dest_data[posY+x] = mFillColor;
            }
            posY+=(int)texture->getWidth();
        }
        texture->getBuffer()->unlock();
    }
}

//================================================================================================
// Draws a slot (including an item and busytime gfx).
//================================================================================================
void GuiGraphic::drawSlot(Ogre::uint32 *srcItemData, int itemSize, int busyTime, int sumItems)
{
    if (!mBuildBuffer)
    {
        // Alpha must be switched on to craete the build buffer.
        Logger::log().error() << "Item-Slots must have Alpha switched on. No drawing will be done.";
        return;
    }
    if (itemSize > mWidth-4) itemSize = mWidth-4;
    if (itemSize > mHeight-4) itemSize = mHeight-4;
    int strtX = mPosX;
    int strtY = mPosY;
    int offX  = (mWidth  - itemSize) /2; // Item needs offset for not to overwrite the slot borders.
    int offY  = (mHeight - itemSize) /2; // Item needs offset for not to overwrite the slot borders.
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    // ////////////////////////////////////////////////////////////////////
    // Slot gfx.
    // ////////////////////////////////////////////////////////////////////
    PixelBox srcSlot = ((GuiWindow*) mParent)->getPixelBox()->getSubVolume(Box(
                           gfxSrcPos[mState].x,
                           gfxSrcPos[mState].y,
                           gfxSrcPos[mState].x + mWidth,
                           gfxSrcPos[mState].y + mHeight));
    uint32 *srcSlotData = static_cast<uint32*>(srcSlot.data);
    int rowSkipSlot = (int)((GuiWindow*) mParent)->getPixelBox()->getWidth();

    // ////////////////////////////////////////////////////////////////////
    // Draw slot + item into the buffer.
    // ////////////////////////////////////////////////////////////////////
    int dSlotY = 0, dItemY = 0, destY =0;
    for (int y = 0; y < mHeight; ++y)
    {
        for (int x =0; x < mWidth; ++x)
        {
            // First check if item has a non transparent pixel to draw.
            if (x >= offX && x < (int)itemSize+offX && y >= offY && y < (int)itemSize+offY)
            {
                if (srcItemData[dItemY + x - offX] > 0x00ffffff)
                {
                    mBuildBuffer[destY + x] = srcItemData[dItemY + x - offX];
                    continue;
                }
            }
            // Now check for the background.
            if (srcSlotData[dSlotY + x] > 0x00ffffff)
                mBuildBuffer[destY + x] = srcSlotData[dSlotY + x];
        }
        if (y >= offY) dItemY+= itemSize;
        dSlotY+= (int)rowSkipSlot;
        destY+= mWidth;
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw busy-gfx into the buffer.
    // ////////////////////////////////////////////////////////////////////
    if (busyTime)
    {
        //mPrevBusyTime = busyTime;
        int x2,x3,y2,dY,dX,xStep,yStep,delta,posY;
        int x = mWidth/2;
        int y = mWidth/2;
        if (busyTime > mWidth *2)  // 180...360°
        {
            if (busyTime <= (int)(mWidth * 2.5))
            {
                // < 225°
                x2 = (int)(mWidth *2.5) - busyTime;
                y2 = mWidth-1;
            }
            else if (busyTime < (int)(mWidth * 3.5))
            {
                // < 325°
                x2 = 0;
                y2 = (int)(mWidth *3.5) - busyTime-1;
            }
            else
            {
                x2 = busyTime - (int)(mWidth *3.5);
                y2 = 0;
            }
            dX = Math::IAbs(x2-x);
            dY = Math::IAbs(y2-y);
            delta= dX - dY;
            xStep= (x2>x)?1:-1;
            yStep= (y2>y)?1:-1;
            x3= (y2<mWidth/2)?mWidth/2:0;
            while (x!=x2)
            {
                if (delta >= 0)
                {
                    x+= xStep;
                    delta-= dY;
                }
                else
                {
                    y+= yStep;
                    delta+= dX;
                    posY = y*mWidth;
                    if (x < x3)
                    {
                        for (int i=posY+x+1; i<posY+x3; ++i)
                            mBuildBuffer[i] = alphaBlend(mBuildBuffer[i], SLOT_BUSY_COLOR);
                    }
                    else
                    {
                        for (int i=posY+x3+1; i<posY+x; ++i)
                            mBuildBuffer[i] = alphaBlend(mBuildBuffer[i], SLOT_BUSY_COLOR);
                    }
                }
            }
            if (y > mWidth/2) y = mWidth/2+1;
            while (--y > 0)
            {
                posY = y*mWidth;
                for (int i=posY+1; i<posY+mWidth/2; ++i)
                {
                    mBuildBuffer[i] = alphaBlend(mBuildBuffer[i], SLOT_BUSY_COLOR);
                }
            }
        }
        else  // 0...180°
        {
            if (busyTime != mWidth*2)
            {
                if (busyTime <= mWidth/2)
                {
                    //  < 45°
                    x2= busyTime + mWidth/2;
                    y2= 0;
                }
                else if (busyTime <= (int)(mWidth * 1.5))
                {
                    // < 135°
                    x2 = mWidth;
                    y2 = busyTime - mWidth /2-1;
                }
                else
                {
                    // < 180°
                    x2 =  (int)(mWidth *2.5) - busyTime;
                    y2 = mWidth-1;
                }
                dX = Math::IAbs(x2-x);
                dY = Math::IAbs(y2-y);
                delta = dX - dY;
                if (y2 >y)
                {
                    yStep = 1;
                    x3= mWidth/2;
                }
                else
                {
                    yStep = -1;
                    x3= mWidth;
                }
                while (x!=x2)
                {
                    if (delta >= 0)
                    {
                        ++x;
                        delta-= dY;
                    }
                    else
                    {
                        y+= yStep;
                        delta+= dX;
                        posY = y*mWidth;
                        if (x < x3)
                        {
                            for (int i=posY+x; i<posY+x3-1; ++i)
                                mBuildBuffer[i] = alphaBlend(mBuildBuffer[i], SLOT_BUSY_COLOR);
                        }
                        else
                        {
                            for (int i=posY+x3; i<posY+x-1; ++i)
                                mBuildBuffer[i] = alphaBlend(mBuildBuffer[i], SLOT_BUSY_COLOR);
                        }
                    }
                }
                // fill lower right quadrant.
                if (yStep <0) y = mWidth/2-1;
                while (++y < mWidth-1)
                {
                    posY = y*mWidth;
                    for (int i=posY+mWidth/2; i<posY+mWidth-1; ++i)
                    {
                        mBuildBuffer[i] = alphaBlend(mBuildBuffer[i], SLOT_BUSY_COLOR);
                    }
                }
            }
            // Fill left half.
            for (y = mWidth; y < (mWidth-1)*mHeight; y+= mWidth)
            {
                for (int i=y+1; i< y+mWidth/2; ++i)
                {
                    mBuildBuffer[i] = alphaBlend(mBuildBuffer[i], SLOT_BUSY_COLOR);
                }
            }
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Blit the buffer.
    // ////////////////////////////////////////////////////////////////////
    texture->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, mBuildBuffer),
        Box(strtX, strtY, strtX + mWidth, strtY + mHeight));
    static GuiTextout::TextLine label;

    if (sumItems > 1)
    {
        //label.text = mItem->d_name;
        label.text = StringConverter::toString(sumItems);
        label.hideText= false;
        label.index= -1;
        label.font = 2;
        label.x1 = strtX + offX+1;
        label.y1 = strtY + offY+1;
        label.x2 = label.x1 + mWidth;
        label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
        label.color= 0x00888888;
        GuiTextout::getSingleton().Print(&label, texture);
    }

    /*
        long time = Root::getSingleton().getTimer()->getMilliseconds();
        for (int z = 0; z < 500; ++z)
        {}
        Logger::log().error() <<"time: "<<  Root::getSingleton().getTimer()->getMilliseconds() - time;
    */

    // unavailable: if ((x&1)!=(y&1)) drawGreyPixel();
}

