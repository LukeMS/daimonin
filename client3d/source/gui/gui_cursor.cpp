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

const int MAX_CURSOR_SIZE = 128;

//================================================================================================
// .
//================================================================================================
GuiCursor::GuiCursor()
{
}

//================================================================================================
// .
//================================================================================================
GuiCursor::~GuiCursor()
{
}

//================================================================================================
// .
//================================================================================================
void GuiCursor::Init(int w, int h, int screenWidth, int screenHeight, int scale)
{
    mState = STATE_STANDARD;
    mWidth = w;
    mHeight= h;
    mScale = scale;
    if (mWidth <   4) mWidth =   4;
    if (mHeight<   4) mHeight=   4;
    if (mWidth > MAX_CURSOR_SIZE) mWidth = MAX_CURSOR_SIZE;
    if (mHeight> MAX_CURSOR_SIZE) mHeight= MAX_CURSOR_SIZE;
    // ////////////////////////////////////////////////////////////////////
    // Create the overlay element.
    // ////////////////////////////////////////////////////////////////////
    mTexture = TextureManager::getSingleton().createManual("GUI_Cursor_Texture", "General",
               TEX_TYPE_2D, MAX_CURSOR_SIZE, MAX_CURSOR_SIZE, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY);
    uint32 *dest = (uint32*)mTexture->getBuffer()->lock(0,MAX_CURSOR_SIZE * MAX_CURSOR_SIZE *sizeof(uint32), HardwareBuffer::HBL_DISCARD);
    for (int y = 0; y < MAX_CURSOR_SIZE * MAX_CURSOR_SIZE; ++y)  *dest++ = 0;
    mTexture->getBuffer()->unlock();
    mOverlay = OverlayManager::getSingleton().create("GUI_MouseCursor");
    mOverlay->setZOrder(550);
    mElement = OverlayManager::getSingleton().createOverlayElement(OVERLAY_TYPE_NAME, "GUI_Cursor");
    //  mElement->setMetricsMode(GMM_PIXELS);
    mElement->setHeight((Real)mHeight / (Real)screenHeight);
    mElement->setWidth ((Real)mWidth  / (Real)screenWidth );
    mElement->setTop (0.5);
    mElement->setLeft(0.5);
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
void GuiCursor::setPos(Real x, Real y)
{
    mElement->setTop (y);
    mElement->setLeft(x);
}
//================================================================================================
// .
//================================================================================================
void GuiCursor::setState(int state)
{
    if (state < STATE_SUM)
    {
        mState = state;
        draw();
    }
}

//================================================================================================
// .
//================================================================================================
void GuiCursor::setStateImagePos(std::string name, int x, int y)
{
    int state = -1;
    if      (name == "Standard")   state = STATE_STANDARD;
    else if (name == "Pressed")    state = STATE_PRESSED;
    else if (name == "Dragging")   state = STATE_DRAGGING;
    else if (name == "Resizing")   state = STATE_RESIZING;
    if (state < 0)
    {
        Logger::log().error() << "MouseCursor has no State '" << name << "!";
        return;
    }
    gfxSrcPos[state].x = x;
    gfxSrcPos[state].y = y;
}

//================================================================================================
// .
//================================================================================================
void GuiCursor::draw()
{
    // Scaling, done by blitFromMemory, seems to fail under OpenGL sometimes. (Ogre3D 1.2.2)
    // So we need to do it by hand.
    PixelBox src = GuiImageset::getSingleton().getPixelBox().getSubVolume(Box(
                                                gfxSrcPos[mState].x,
                                                gfxSrcPos[mState].y,
                                                gfxSrcPos[mState].x + mWidth,
                                                gfxSrcPos[mState].y + mHeight));
    uint32 buffer[MAX_CURSOR_SIZE * MAX_CURSOR_SIZE];
    int w = mWidth* mScale;
    int h = mHeight* mScale;
    if (w > MAX_CURSOR_SIZE) w = MAX_CURSOR_SIZE;
    if (h > MAX_CURSOR_SIZE) h = MAX_CURSOR_SIZE;
    PixelBox scaled = PixelBox(w, h, src.getDepth(), src.format, buffer);
    Image::scale(src, scaled);
    mTexture->getBuffer()->blitFromMemory(scaled, Box(0, 0, w, h));
}
