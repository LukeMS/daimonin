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

#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include "gui_window.h"
#include "gui_imageset.h"

/// @brief This is the base class for a gui element.
/// @details
class GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { MIN_SIZE = 1 << 2  /**< Minimal graphic size of an element. **/ };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiElement(TiXmlElement *xmlElement, const void *parent); ///< Default constructor.
    virtual ~GuiElement() {} ///< Default destructor.

    /// @brief This function is called to interact with this element.
    /// @param message The message/command for this element.
    /// @param string  parameter and/or return string value.
    /// @param param   parameter and/or return integer value.
    /// @param text    Additional text parameter.
    virtual void sendMsg(const int /*message*/, Ogre::String &/*text*/, Ogre::uint32 &/*param*/, const char * /*text2*/) {}

    /// @brief This function is called when a key event has happened.
    /// @param  keyChar The key character.
    /// @param  key     The key value.
    /// @return The status of event.
    /// @see GuiManager for all defined status types.
    virtual int keyEvent(const int keyChar, const unsigned int key);

    /// @brief This function is called when a mouse event has happened.
    /// @param action The mouse action (button pressed, etc).
    /// @param posX   The x-pos of the mouse cursor.
    /// @param posY   The y-pos of the mouse cursor.
    /// @param wheel  The pos of the mouse wheel.
    virtual int mouseEvent(const int action, int posX, int posY, int wheel);

    /// @brief Update this element. Used for drag'n'drop, animations, etc.
    /// @param deltaTime The time since the last frame.
    virtual void update(Ogre::Real /*deltaTime*/) {}

    /// @brief Draw an element into the parent window.
    /// @param uploadToTexture When false the gfx is not yet copied to the texture.
    ///        This way texture write access can be optimized for multi layer graphics.
    virtual void draw(bool uploadToTexture = true);

    /// @brief Returns the width of this element.
    Ogre::uint16 getWidth() const  { return mWidth;  }

    /// @brief Returns the height of this element.
    Ogre::uint16 getHeight() const { return mHeight; }

    /// @brief Returns the unique numer of this element.
    int getIndex() const { return mIndex; }

    /// @brief Set the state of this element.
    /// @param  state The state is the look of this element.
    /// @return Returns true if the state has changed.
    bool setState(const Ogre::uchar state);

    /// @brief Set the position of this element.
    /// @param x The x-pos in pixel from the top-left pos of the parent window.
    /// @param y The y-pos in pixel from the top-left pos of the parent window.
    void setPosition(const int x, const int y)
    {
        mPosX = x;
        mPosY = y;
    }

    /// @brief Check if the Mouse-Pointer is within this element.
    /// @param x The mouse x-pos in pixel.
    /// @param y The mouse y-pos in pixel.
    /// @return Returns true if the mouse is within the borders of this element.
    bool mouseWithin(const int x, const int y) const
    {
        return !(mHidden || x < mPosX || x > mPosX + mWidth || y < mPosY || y > mPosY + mHeight);
    }

protected:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    int mIndex;                          ///< Unique number. -1 means its a background-gfx (no interaction)
    bool mHidden;                        ///< Is this element currently hidden?
    GuiWindow *mParent;                  ///< Pointer to the parent window.
    Ogre::uchar mState;                  ///< Actual state of this element.
    Ogre::uint16 mPosX, mPosY;           ///< Pixeloffset from the upper-left corner of the window.
    Ogre::uint16 mWidth, mHeight;        ///< Dimension of this element.
    Ogre::uint16 mFontNr, mLabelFontNr;
    Ogre::uint16 mLabelPosX, mLabelPosY;
    Ogre::String mLabelString;           ///< The text of the label.
    Ogre::uint32 mLabelColor;            ///< The textcolor of the label.
    Ogre::uint32 mFillColor;             ///< The fill color of this element. Only used when mGfxSrc is null.
    GuiImageset::gfxSrcEntry *mGfxSrc;   ///< Pointer to the gfx-data structure or 0 for a colorfill.

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /// @return Returns true if this element is currently hidden.
    bool isHidden() const { return mHidden; }

    /// @brief Hiding/Unhiding an element.
    /// @param hidden Set the hidden status of this element.
    /// @return Returns true if the derived class needs a redraw.
    bool setHidden(bool hidden);

private:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiElement(const GuiElement&);            ///< disable copy-constructor.
    GuiElement &operator=(const GuiElement&); ///< disable assignment operator.
};

#endif
