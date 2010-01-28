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

#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreStringConverter.h>
#include <OgreOverlayManager.h>
#include <OgreHardwarePixelBuffer.h>
#include "gui_cursor.h"

using namespace Ogre;

const int MIN_CURSOR_SIZE = 1 << 4;
const int MAX_CURSOR_SIZE = 1 << 7;

//================================================================================================
// Create an overlay for the mouse-cursor.
//================================================================================================
void GuiCursor::Init(const char *resourceName)
{
    GuiImageset::gfxSrcMouse *srcEntry = GuiImageset::getSingleton().getStateGfxPosMouse();
    mWidth = srcEntry->w;
    mHeight = srcEntry->h;
    String resName = resourceName;
    mState = GuiManager::STATE_MOUSE_DEFAULT;
    int textureSize = MIN_CURSOR_SIZE;
    int psize = (mWidth > mHeight)?mWidth:mHeight;
    while (textureSize < psize && textureSize < MAX_CURSOR_SIZE) textureSize <<= 1; // Make the size a power of 2.
    String strTexture = StringConverter::toString(textureSize) + "_" + resName + GuiManager::TEXTURE_RESOURCE_NAME;
    Overlay *overlay;
    mTexture = GuiManager::getSingleton().createTexture(strTexture);
    mElement = GuiManager::getSingleton().createOverlay(resName, strTexture, overlay);
    mElement->setPosition(0, 0);
    overlay->setZOrder(GuiManager::MAX_OVERLAY_ZPOS);
    overlay->show();
}

//================================================================================================
// Free all used memory (must be called before destructor).
//================================================================================================
void GuiCursor::freeRecources()
{
    mTexture.setNull();
}

//================================================================================================
// Set the state of the mouse-cursor.
//================================================================================================
void GuiCursor::setState(uchar state)
{
    if (mState == state || state >= GuiManager::STATE_MOUSE_SUM) return;
    mState = state;
    draw();
}

//================================================================================================
// Draw a new state into the mouse-cursor texture.
//================================================================================================
void GuiCursor::draw()
{
    GuiImageset::gfxSrcMouse *gfxSrcPos = GuiImageset::getSingleton().getStateGfxPosMouse();
    mTexture->getBuffer()->blitFromMemory(GuiImageset::getSingleton().getPixelBox().getSubVolume(
            Box(gfxSrcPos->state[mState].x,
                gfxSrcPos->state[mState].y,
                gfxSrcPos->state[mState].x + mWidth,
                gfxSrcPos->state[mState].y + mHeight)),
                                          Box(0, 0, mWidth, mHeight));
}
