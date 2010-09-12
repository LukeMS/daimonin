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

#include <OgreOverlayElement.h>
#include "gui_imageset.h"

/// @brief This singleton class provides the mouse cursor.
/// @details OgreOverlay is used to display the texture.
class GuiCursor
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /// @brief Returns the reference to this singleton class.
    static GuiCursor &getSingleton()
    {
        static GuiCursor Singleton; return Singleton;
    }

    /// @brief Set the position of the mouse cursor.
    /// @param x The x-pos in pixel.
    /// @param y The y-pos in pixel.
    void setPos(const int x, const int y)
    {
        mElement->setPosition((float)x, (float)y);
    }

    /// @brief Draw the mouse-cursor into its texture.
    void draw();

    /// @brief Set the state of the mouse cursor.
    /// @details The mouse-cursor can have different graphical states.
    /// These are used to show possible actions that can be triggered.
    /// (e.g. a mouth to show the user that he can talk to this npc.)
    /// @param state The state is the look of the mouse cursor.
    /// @see GuiManager for all defined states.
    void setState(Ogre::uchar state);

    /// @brief Init the mouse cursor.
    /// @param resourceName An unique name for creationg the Ogre resources.
    void Init(const char *resourceName);

    /// @brief Free all Ogre resources. Must be called before the destructor.
    void freeRecources();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::uchar mState;
    Ogre::uint16 mWidth, mHeight;
    Ogre::OverlayElement *mElement;
    Ogre::TexturePtr mTexture;
    GuiImageset::gfxSrcMouse gfxSrcPos;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiCursor()  {}
    ~GuiCursor() {}
    GuiCursor(const GuiCursor&);            ///< disable copy-constructor.
    GuiCursor &operator=(const GuiCursor&); ///< disable assignment operator.
};

#endif
