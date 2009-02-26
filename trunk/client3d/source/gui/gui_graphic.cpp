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

#include "gui_graphic.h"

using namespace Ogre;

//================================================================================================
// .
//================================================================================================
void GuiGraphic::drawGfxToBuffer(int w, int h, int srcW, int srcH, uint32 *src, uint32 *bak, uint32 *dst, int srcRowSkip, int bakRowSkip, int dstRowSkip)
{
    int srcY = 0;
    for (; h; --h)
    {
        int srcX = 0;
        for (int x = 0; x < w; ++x)
        {
            dst[x] = alphaBlend(bak[x], src[srcY*srcRowSkip + srcX]);
            if (++srcX >= srcW) srcX = 0; // Repeat the source image.
        }
        bak+=bakRowSkip;
        dst+=dstRowSkip;
        if (++srcY >= srcH) srcY = 0; // Repeat the source image.
    }
}

//================================================================================================
// .
//================================================================================================
void GuiGraphic::drawColorToBuffer(int w, int h, uint32 color, uint32 *bak, uint32 *dst, int bakRowSkip, int dstRowSkip)
{
    for (; h; --h)
    {
        for (int x = 0; x < w; ++x)
            dst[x] = alphaBlend(bak[x], color);
        bak+=bakRowSkip;
        dst+=dstRowSkip;
    }
}

//================================================================================================
// .
//================================================================================================
void GuiGraphic::drawColorToBuffer(int w, int h, uint32 color, uint32 *dst, int dstRowSkip)
{
    for (; h; --h)
    {
        for (int x = 0; x < w; ++x)
            dst[x] = alphaBlend(dst[x], color);
        dst+=dstRowSkip;
    }
}

//================================================================================================
// .
//================================================================================================
void GuiGraphic::restoreWindowBG(int w, int h, uint32 *src, uint32 *dst, int srcRowSkip, int dstRowSkip)
{
    for (; h; --h)
    {
        for (int x = 0; x < w; ++x)
            dst[x] = src[x];
        src+=srcRowSkip;
        dst+=dstRowSkip;
    }
}

//================================================================================================
// .
//================================================================================================
uint32 GuiGraphic::alphaBlend(const uint32 bg, const uint32 gfx)
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
