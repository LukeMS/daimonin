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
    /** Set the position of the mouse cursor.
     ** @param x The x-pos in pixel.
     ** @param y The y-pos in pixel.
     *****************************************************************************/
    void setPos(const int x, const int y)
    {
        mElement->setPosition(x, y);
    }

    /** Set the state of the mouse cursor.
     ** @param state The state is the look of the mouse cursor.
     *****************************************************************************/
    void setState(Ogre::uchar state);

    /** Init the mouse cursor.
     ** @param resourceName An unique name for creationg the Ogre resources.
     *****************************************************************************/
    void Init(const char *resourceName);

    /// (Re)load all Ogre resources.
    void loadResources();

    /// Free all Ogre resources. Must be called before the destructor.
    void freeRecources();

    static GuiCursor &getSingleton()
    {
        static GuiCursor Singleton; return Singleton;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::uchar mState;
    Ogre::uint16 mWidth, mHeight;
    Ogre::OverlayElement *mElement;
    Ogre::TexturePtr mTexture;
    Ogre::String mResourceName;
    GuiImageset::gfxSrcMouse gfxSrcPos;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiCursor()  {}
    ~GuiCursor() {}
    /// Disable copy-constructor.
    GuiCursor(const GuiCursor&);
    /// Draws the mouse-cursor into its texture.
    void draw();
};

#endif
