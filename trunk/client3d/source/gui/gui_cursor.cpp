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

#include <OgreHardwarePixelBuffer.h>
#include "gui_cursor.h"
#include "gui_imageset.h"

using namespace Ogre;


const int MIN_CURSOR_SIZE =  4;
const int MAX_CURSOR_SIZE = 64;

//================================================================================================
// Create an overlay for the mouse-cursor.
//================================================================================================
void GuiCursor::Init(int w, int h, const char *resourceName)
{
    mResourceName = resourceName;
    mState = GuiImageset::STATE_MOUSE_DEFAULT;
    mWidth = w;
    mHeight= h;
    if (mWidth < MIN_CURSOR_SIZE) mWidth = MIN_CURSOR_SIZE; else
        if (mWidth > MAX_CURSOR_SIZE) mWidth = MAX_CURSOR_SIZE;
    if (mHeight< MIN_CURSOR_SIZE) mHeight= MIN_CURSOR_SIZE; else
        if (mHeight> MAX_CURSOR_SIZE) mHeight= MAX_CURSOR_SIZE;
    loadResources();
    mElement->setPosition(0, 0);
}

//================================================================================================
// (Re)loads the material and texture or creates them if they dont exist.
//================================================================================================
void GuiCursor::loadResources()
{
    Overlay *overlay = GuiImageset::getSingleton().loadResources(MAX_CURSOR_SIZE, mResourceName, mTexture);
    mElement = overlay->getChild(mResourceName + GuiImageset::ELEMENT_RESOURCE_NAME);
    overlay->setZOrder(550);
    overlay->show();
    draw();
}

//================================================================================================
// Free all used memory (must be called before destructor).
//================================================================================================
void GuiCursor::freeRecources()
{
    mTexture.setNull();
}

//================================================================================================
// Copy the state informations to the private part of the class.
//================================================================================================
void GuiCursor::setStateImagePos(GuiImageset::gfxPos *Entry)
{
    memcpy(gfxSrcPos, Entry, sizeof(gfxSrcPos));
    draw();
}

//================================================================================================
// Set the state of the mouse-cursor.
//================================================================================================
void GuiCursor::setState(unsigned int state)
{
    if (state < GuiImageset::STATE_MOUSE_SUM && mState != state)
    {
        mState = state;
        draw();
    }
}

//================================================================================================
// Draw a new state into the mouse-cursor texture.
//================================================================================================
void GuiCursor::draw()
{
    mTexture->getBuffer()->blitFromMemory(GuiImageset::getSingleton().getPixelBox().getSubVolume(
                                              Box(gfxSrcPos[mState].x,
                                                  gfxSrcPos[mState].y,
                                                  gfxSrcPos[mState].x + mWidth,
                                                  gfxSrcPos[mState].y + mHeight)),
                                          Box(0, 0, mWidth, mHeight));
}
