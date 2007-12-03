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

#ifndef GUI_CURSOR_H
#define GUI_CURSOR_H

#include <Ogre.h>
#include "gui_imageset.h"

/**
 ** This singleton class provides the mouse cursor (as an ogre3d overlay).
 *****************************************************************************/
class GuiCursor
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void setPos(int x, int y)
    {
        mElement->setPosition(x, y);
    }
    void getPos(Ogre::Real &x, Ogre::Real &y)
    {
        y = mElement->getTop () - mHeight/2;
        x = mElement->getLeft() - mWidth/2;
    }
    void setState(unsigned int state);
    int  getState()
    {
        return mState;
    }
    void Init(int w, int h);
    void freeRecources();
    static GuiCursor &getSingleton()
    {
        static GuiCursor Singleton; return Singleton;
    }
    void setStateImagePos(GuiImageset::gfxPos *Entry);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    int mWidth, mHeight;
    unsigned int mState;
    GuiImageset::gfxPos gfxSrcPos[GuiImageset::STATE_MOUSE_SUM];
    Ogre::OverlayElement *mElement;
    Ogre::TexturePtr mTexture;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiCursor();
    ~GuiCursor();
    GuiCursor(const GuiCursor&); // disable copy-constructor.
    void draw();
};

#endif
