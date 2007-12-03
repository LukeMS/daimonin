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
#include "gui_window.h"
#include "gui_imageset.h"

using namespace Ogre;

const int MAX_CURSOR_SIZE = 64;

//================================================================================================
// Constructor.
//================================================================================================
GuiCursor::GuiCursor()
{}

//================================================================================================
// Destructor.
//================================================================================================
GuiCursor::~GuiCursor()
{}

//================================================================================================
// Create an overlay for the mouse-cursor.
//================================================================================================
void GuiCursor::Init(int w, int h)
{
    mState = GuiImageset::STATE_MOUSE_DEFAULT;
    mWidth = w;
    mHeight= h;
    if (mWidth < 4) mWidth = 4;
    else if (mWidth > MAX_CURSOR_SIZE) mWidth = MAX_CURSOR_SIZE;
    if (mHeight< 4) mHeight= 4;
    else if (mHeight> MAX_CURSOR_SIZE) mHeight= MAX_CURSOR_SIZE;
    mTexture = TextureManager::getSingleton().createManual("GUI_Cursor_Texture", "General",
               TEX_TYPE_2D, MAX_CURSOR_SIZE, MAX_CURSOR_SIZE, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY);
    // We must clear the whole texture (textures have always 2^n size while cursor-gfx can be smaller).
    memset(mTexture->getBuffer()->lock(HardwareBuffer::HBL_DISCARD), 0x00, MAX_CURSOR_SIZE * MAX_CURSOR_SIZE * sizeof(uint32));
    mTexture->getBuffer()->unlock();
    // Create the overlay element.
    Overlay *overlay= OverlayManager::getSingleton().create("GUI_MouseCursor");
    overlay->setZOrder(550);
    mElement = OverlayManager::getSingleton().createOverlayElement(GuiWindow::OVERLAY_ELEMENT_TYPE, "GUI_Cursor");
    mElement->setMetricsMode(GMM_PIXELS);
    mElement->setPosition(0, 0);
    mElement->setDimensions (mTexture->getWidth(), mTexture->getHeight());
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("GUI/Window");
    tmpMaterial = tmpMaterial->clone("GUI_Cursor_Material");
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("GUI_Cursor_Texture");
    tmpMaterial->load();
    mElement->setMaterialName("GUI_Cursor_Material");
    overlay->add2D(static_cast<OverlayContainer*>(mElement));
    overlay->show();
    mState = GuiImageset::STATE_MOUSE_DEFAULT;
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
