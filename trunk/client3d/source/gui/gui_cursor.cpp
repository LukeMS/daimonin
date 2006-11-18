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

#include <OgreHardwarePixelBuffer.h>
#include <tinyxml.h>
#include "define.h"
#include "gui_cursor.h"
#include "gui_imageset.h"
#include "logger.h"

const int MAX_CURSOR_SIZE = 64;

//================================================================================================
// .
//================================================================================================
GuiCursor::GuiCursor()
{}

//================================================================================================
// .
//================================================================================================
GuiCursor::~GuiCursor()
{}

//================================================================================================
// .
//================================================================================================
void GuiCursor::Init(int w, int h, int screenWidth, int screenHeight)
{
    mState = GuiImageset::STATE_MOUSE_DEFAULT;
    mWidth = w;
    mHeight= h;
    if (mWidth < 4) mWidth = 4;
    if (mHeight< 4) mHeight= 4;
    if (mWidth > MAX_CURSOR_SIZE) mWidth = MAX_CURSOR_SIZE;
    if (mHeight> MAX_CURSOR_SIZE) mHeight= MAX_CURSOR_SIZE;
    // ////////////////////////////////////////////////////////////////////
    // Create the overlay element.
    // ////////////////////////////////////////////////////////////////////
    mTexture = TextureManager::getSingleton().createManual("GUI_Cursor_Texture", "General",
               TEX_TYPE_2D, MAX_CURSOR_SIZE, MAX_CURSOR_SIZE, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY);
    uint32 *dest = (uint32*)mTexture->getBuffer()->lock (0,MAX_CURSOR_SIZE * MAX_CURSOR_SIZE *sizeof(uint32), HardwareBuffer::HBL_DISCARD);
    for (int y = MAX_CURSOR_SIZE * MAX_CURSOR_SIZE; y; --y)  *dest++ = 0;
    mTexture->getBuffer()->unlock();
    mOverlay = OverlayManager::getSingleton().create("GUI_MouseCursor");
    mOverlay->setZOrder(550);
    mElement = OverlayManager::getSingleton().createOverlayElement(OVERLAY_TYPE_NAME, "GUI_Cursor");
    mElement->setMetricsMode(GMM_PIXELS);
    mElement->setPosition(screenWidth/2, screenHeight/2);
    mElement->setDimensions (mTexture->getWidth(), mTexture->getHeight());
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("GUI/Window");
    mMaterial = tmpMaterial->clone("GUI_Cursor_Material");
    mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("GUI_Cursor_Texture");
    mMaterial->load();
    mElement->setMaterialName("GUI_Cursor_Material");
    mOverlay->add2D(static_cast<OverlayContainer*>(mElement));
    mOverlay->show();
}

//================================================================================================
// .
//================================================================================================
void GuiCursor::freeRecources()
{
    mMaterial.setNull();
    mTexture.setNull();
}

//================================================================================================
// .
//================================================================================================
void GuiCursor::setPos(int x, int y)
{
    mElement->setTop (y);
    mElement->setLeft(x);
}

//================================================================================================
// Set the state of the mouse pointer.
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
// .
//================================================================================================
void GuiCursor::setStateImagePos(GuiImageset::gfxPos *Entry)
{
    memcpy(gfxSrcPos, Entry, sizeof(gfxSrcPos));
}

//================================================================================================
// .
//================================================================================================
void GuiCursor::draw()
{
    PixelBox src = GuiImageset::getSingleton().getPixelBox().getSubVolume(Box(
                       gfxSrcPos[mState].x,
                       gfxSrcPos[mState].y,
                       gfxSrcPos[mState].x + mWidth,
                       gfxSrcPos[mState].y + mHeight));
    mTexture->getBuffer()->blitFromMemory(src, Box(0, 0, mWidth, mHeight));
}

