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
#include "gui_gadget_slot.h"

using namespace Ogre;

const uint32 SLOT_BUSY_COLOR     = 0xdd777777;
const uint32 SLOT_QUANTITY_COLOR = 0x00888888;
uint32 buildBuf[GuiGadgetSlot::ITEM_SIZE*GuiGadgetSlot::ITEM_SIZE];

//================================================================================================
// .
//================================================================================================
GuiGraphic::GuiGraphic(TiXmlElement *xmlElement, void *parent, bool drawOnInit):GuiElement(xmlElement, parent)
{
    if (drawOnInit) draw();
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
    Texture *texture = mParent->getTexture();
    // ////////////////////////////////////////////////////////////////////
    // Draw a gfx into the window texture.
    // ////////////////////////////////////////////////////////////////////
    if (mGfxSrc)
    {
        PixelBox src = mParent->getPixelBox()->getSubVolume(Box(
                           mGfxSrc->state[mState].x,
                           mGfxSrc->state[mState].y,
                           mGfxSrc->state[mState].x + mGfxSrc->w,
                           mGfxSrc->state[mState].y + mGfxSrc->h));
        int srcRowSkip = (int) (mParent->getPixelBox()->getWidth());
        uint32 *LayerState = static_cast<uint32*>(src.data);
        int srcX, dSrcY = 0, srcY =0;
        // ////////////////////////////////////////////////////////////////////
        // This gfx is part of the background.
        // (Only used on creating time of the windows)
        // ////////////////////////////////////////////////////////////////////
        if (mBG_Element)
        {
            // Copy the gfx into the background-buffer of the window.
            uint32 *dst = mParent->getLayerBG()+mPosX+mPosY * mParent->getWidth();
            for (int y =0; y < mHeight; ++y)
            {
                srcX = 0;
                for (int x =0; x < mWidth; ++x)
                {
                    dst[x] = alphaBlend(dst[x], LayerState[dSrcY + srcX]);
                    if (++srcX >= mGfxSrc->w) srcX = 0; // Repeat the image.
                }
                dSrcY+= srcRowSkip;
                dst+=mParent->getWidth();
                if (++srcY >= mGfxSrc->h) { srcY = 0; dSrcY =0; } // Repeat the image.
            }
            // Copy the background gfx to the texture.
            PixelBox p1(mParent->getWidth(), mParent->getHeight(), 1, PF_A8R8G8B8, mParent->getLayerBG());
            texture->getBuffer()->blitFromMemory(
                p1.getSubVolume(Box(mPosX, mPosY, mPosX + mWidth, mPosY+ mHeight)),
                Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight));
        }
        // ////////////////////////////////////////////////////////////////////
        // This gfx belongs to an interactive gui element.
        // ////////////////////////////////////////////////////////////////////
        else
        {
            // Draw the gfx together with the sin-background to the texture.
            uint32 *bak = mParent->getLayerBG()+mPosX+mPosY * mParent->getWidth();
            PixelBox pb = texture->getBuffer()->lock(Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight), HardwareBuffer::HBL_DISCARD);
            uint32 *dst = (uint32*)pb.data;
            for (int y =0; y < mHeight; ++y)
            {
                srcX = 0;
                for (int x =0; x < mWidth; ++x)
                {
                    dst[x] = alphaBlend(bak[x], LayerState[dSrcY + srcX]);
                    if (++srcX >= mGfxSrc->w) srcX = 0; // Repeat the image.
                }
                dSrcY+= srcRowSkip;
                dst+=texture->getWidth();
                bak+=mParent->getWidth();
                if (++srcY >= mGfxSrc->h) { srcY = 0; dSrcY =0; } // Repeat the image.
            }
            texture->getBuffer()->unlock();
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Draws a color area to the window texture.
    // ////////////////////////////////////////////////////////////////////
    else
    {
        PixelBox pb = texture->getBuffer()->lock(Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight), HardwareBuffer::HBL_DISCARD);
        uint32 *dst = (uint32*)pb.data;
        // ////////////////////////////////////////////////////////////////////
        // The gfx is part of the background.
        // (Only used on creating time of the windows)
        // ////////////////////////////////////////////////////////////////////
        if (mBG_Element)
        {
            for (int y = mHeight; y; --y)
            {
                for (int x= 0; x < mWidth; ++x)
                    dst[x] = mFillColor;
                dst+=(int)texture->getWidth();
            }
        }
        // ////////////////////////////////////////////////////////////////////
        // This gfx belongs to an interactive gui element.
        // ////////////////////////////////////////////////////////////////////
        else
        {
            uint32 *bak = mParent->getLayerBG()+mPosX+mPosY * mParent->getWidth();
            for (int y = mHeight; y; --y)
            {
                for (int x= 0; x < mWidth; ++x)
                    dst[x] = alphaBlend(bak[x], mFillColor);
                dst+=(int)texture->getWidth();
                bak+=mParent->getWidth();
            }
        }
        texture->getBuffer()->unlock();
    }
}

//================================================================================================
// Draws a slot (including an item and busytime gfx).
//================================================================================================
void GuiGraphic::drawSlot(Ogre::uint32 *srcItemData, int busyTime, int sumItems)
{
    bool inactive = false; // Just for testing!!
    bool useBuildBuffer = (sumItems > 1 || busyTime || inactive)?true:false;
    int itemSize = GuiGadgetSlot::ITEM_SIZE;
    int off  = (mWidth  - itemSize) /2; // Item is always smaller than the slot, so we need an offset
    Texture *texture = mParent->getTexture();
    PixelBox pb = texture->getBuffer()->lock(Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight), HardwareBuffer::HBL_DISCARD);
    uint32 *dst = (uint32*)pb.data;
    // Slot gfx.
    PixelBox pbSlot = mParent->getPixelBox()->getSubVolume(Box(
                          mGfxSrc->state[mState].x,
                          mGfxSrc->state[mState].y,
                          mGfxSrc->state[mState].x + mWidth,
                          mGfxSrc->state[mState].y + mHeight));
    uint32 *srcSlot = static_cast<uint32*>(pbSlot.data);
    // Window background.
    uint32 *bak = mParent->getLayerBG()+mPosX+mPosY * mParent->getWidth();
    int rowSkipSlot = (int) mParent->getPixelBox()->getWidth();
    // ////////////////////////////////////////////////////////////////////
    // Draw the number of items (text) and the busy gfx into a buffer.
    // Draw slot + item + buildbuffer.
    // ////////////////////////////////////////////////////////////////////
    if (useBuildBuffer)
    {
        uint32 *bBuf = buildBuf;
        if (inactive)
        {
            // Draw some gfx to show that this slot is inactive.
            for (int y=0; y < itemSize; ++y)
            {
                for (int x=0; x < itemSize; ++x)
                {
                    if ((x&1)!=(y&1))
                        *bBuf++ = 0xff888888;
                    else
                        *bBuf++ = 0x00000000;
                }
            }
        }
        else
        {
            // Clear the whole buffer.
            memset(bBuf, 0, itemSize*itemSize*sizeof(uint32));
        }
        if (sumItems > 1)
        {
            // Print the number of items.
            GuiTextout::getSingleton().PrintToBuffer(itemSize, itemSize, buildBuf +itemSize+1,
                    StringConverter::toString(sumItems).c_str(), 2, SLOT_QUANTITY_COLOR);
        }
        if (busyTime)
        {
            drawBusyGfx(itemSize, busyTime);
        }
        // Draw it all to the texture.
        bBuf = buildBuf;
        for (int y = 0; y < mHeight; ++y)
        {
            for (int x =0; x < mWidth; ++x)
            {
                if (x >= off && x < itemSize+off && y>= off && y< itemSize+off)
                    dst[x] = alphaBlend(alphaBlend(alphaBlend(bak[x],srcSlot[x]),*srcItemData++),*bBuf++);
                else
                    dst[x] = alphaBlend(bak[x], srcSlot[x]);
            }
            dst+= (int)texture->getWidth();
            bak+= mParent->getWidth();
            srcSlot+= rowSkipSlot;
        }
        texture->getBuffer()->unlock();
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw only slot + item.
    // ////////////////////////////////////////////////////////////////////
    else
    {
        for (int y = 0; y < mHeight; ++y)
        {
            for (int x =0; x < mWidth; ++x)
            {
                if (x>= off && x < itemSize+off && y>= off && y< itemSize+off)
                    dst[x] = alphaBlend(alphaBlend(bak[x],srcSlot[x]), *srcItemData++);//srcItemData[x-off]);
                else
                    dst[x] = alphaBlend(bak[x], srcSlot[x]);
            }
            dst+= (int)texture->getWidth();
            bak+= mParent->getWidth();
            srcSlot+= rowSkipSlot;
        }
        texture->getBuffer()->unlock();
    }
}

//================================================================================================
// Draws the busy gfx into the build buffer..
//================================================================================================
void GuiGraphic::drawBusyGfx(int itemSize, int busyTime)
{
    //mPrevBusyTime = busyTime;
    int x2,x3,y2,dY,dX,xStep,yStep,delta,posY;
    uint32 *bak = buildBuf;
    int x = itemSize/2;
    int y = itemSize/2;
    if (busyTime > itemSize *2)  // 180...360°
    {
        if (busyTime <= (int)(itemSize * 2.5))
        {
            // < 225°
            x2 = (int)(itemSize *2.5) - busyTime;
            y2 = itemSize;
        }
        else if (busyTime < (int)(itemSize * 3.5))
        {
            // < 325°
            x2 = 0;
            y2 = (int)(itemSize *3.5) - busyTime;
        }
        else
        {
            x2 = busyTime - (int)(itemSize *3.5);
            y2 = -1;
        }
        dX = Math::IAbs(x2-x);
        dY = Math::IAbs(y2-y);
        delta= dX - dY;
        xStep= (x2>x)?1:-1;
        yStep= (y2>y)?1:-1;
        x3= (y2<itemSize/2)?itemSize/2:0;
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
                posY = y*itemSize;
                if (x < x3)
                {
                    for (int i=posY+x; i<posY+x3; ++i)
                        bak[i] = alphaBlend(SLOT_BUSY_COLOR, bak[i]);
                }
                else
                {
                    for (int i=posY+x3; i<posY+x; ++i)
                        bak[i] = alphaBlend(SLOT_BUSY_COLOR, bak[i]);
                }
            }
        }
        // Upper left quardant.
        if (y > itemSize/2) y = itemSize/2+1;
        while (--y >= 0)
        {
            posY = y*itemSize;
            for (int i=posY; i<posY+itemSize/2; ++i)
            {
                bak[i] = alphaBlend(SLOT_BUSY_COLOR, bak[i]);
            }
        }
    }
    else  // 0...180°
    {
        if (busyTime != itemSize*2)
        {
            if (busyTime <= itemSize/2)
            {
                //  < 45°
                x2= busyTime + itemSize/2;
                y2= -1;
            }
            else if (busyTime <= (int)(itemSize * 1.5))
            {
                // < 135°
                x2 = itemSize;
                y2 = busyTime - itemSize /2;
            }
            else
            {
                // < 180°
                x2 =  (int)(itemSize *2.5) - busyTime;
                y2 = itemSize;
            }
            dX = Math::IAbs(x2-x);
            dY = Math::IAbs(y2-y);
            delta = dX - dY;
            yStep = (y2 >y)?1:-1;
            x3=(y2 >y)?itemSize/2:itemSize;
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
                    posY = y*itemSize;
                    if (x < x3)
                    {
                        for (int i=posY+x; i<posY+x3; ++i)
                            bak[i] = alphaBlend(SLOT_BUSY_COLOR,bak[i]);
                    }
                    else
                    {
                        for (int i=posY+x3; i<posY+x; ++i)
                            bak[i] = alphaBlend(SLOT_BUSY_COLOR, bak[i]);
                    }
                }
            }
            // fill lower right quadrant.
            if (yStep <0) y = itemSize/2-1;
            while (++y < itemSize)
            {
                posY = y*itemSize;
                for (int i=posY+itemSize/2; i<posY+itemSize; ++i)
                {
                    bak[i] = alphaBlend(SLOT_BUSY_COLOR,bak[i]);
                }
            }
        }
        // Fill the complete left side.
        for (y = 0; y < itemSize*itemSize; y+= itemSize)
        {
            for (int i=y; i< y+itemSize/2; ++i)
            {
                bak[i] = alphaBlend(SLOT_BUSY_COLOR, bak[i]);
            }
        }
    }
}

/*
    long time = Root::getSingleton().getTimer()->getMilliseconds();
    for (int z = 0; z < 500; ++z)
    {}
    Logger::log().error() <<"time: "<<  Root::getSingleton().getTimer()->getMilliseconds() - time;
*/
