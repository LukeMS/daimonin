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

#ifndef GUI_CURSOR_H
#define GUI_CURSOR_H
#include <Ogre.h>

using namespace Ogre;

/**
 ** This singleton class provides the mouse cursor.
 *****************************************************************************/
class GuiCursor
{
public:
    /** Actual state of the mouse cursor: **/
    enum
    {
        STATE_STANDARD,  /**< Standard. **/
        STATE_PRESSED,   /**< Any button down. **/
        STATE_DRAGGING,  /**< Dragging in action. **/
        STATE_RESIZING,  /**< Resizing a window. **/
        STATE_SUM
    };
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void setPos(Real x, Real y);
    void setStateImagePos(std::string state, int x, int y);
    void setState(int state);
    void draw();
    void Init(int w, int h, int screenHeight, int screenWidth, int scale);
    void freeRecources();
    static GuiCursor &getSingleton()
    {
        static GuiCursor Singleton; return Singleton;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    unsigned int mScreenWidth, mScreenHeight;
    struct
    {
        int x, y;
    }
    gfxSrcPos[STATE_SUM];
    unsigned int mState;
    int mWidth, mHeight;
    Overlay *mOverlay;
    OverlayElement *mElement;
    TexturePtr mTexture;
    MaterialPtr mMaterial;
    int mScale;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiCursor();
    ~GuiCursor();
    GuiCursor(const GuiCursor&); // disable copy-constructor.
};

#endif
