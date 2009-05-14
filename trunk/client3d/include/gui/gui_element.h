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

/**
 ** This is the base class for a gui element.
 *****************************************************************************/
class GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { MIN_SIZE = 1 << 2 }; /**< Minimal graphic size of an element. **/
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiElement(TiXmlElement *xmlElement, const void *parent);
    virtual ~GuiElement() {}

    /** Send a message from GuiManager with max 2 parameters and returns max. 2 values.
     ** @param message The message/command for the element.
     ** @param text    parameter and/or return string value.
     ** @param param   parameter and/or return integer value.
     ** @param text2   Additional text parameter.
     *****************************************************************************/
    virtual void sendMsg(const int message, Ogre::String &text, Ogre::uint32 &param, const char *text2) {}

    virtual int keyEvent(const int keyChar, const unsigned int key);
    virtual int mouseEvent(const int mouseAction, int mouseX, int mouseY, int mouseWheel);

    /** Update an element. Used for drag'n'drop, animations, etc.
     ** @param deltaTime The time since the last frame.
     *****************************************************************************/
    virtual void update(Ogre::Real deltaTime) {}

    /** Draw an element into the parent window.
     ** @param uploadToTexture When false the gfx is not yet copied to the texture.
     **                        This way texture write access can be optimized for
     **                        multi layer graphics.
     *****************************************************************************/
    virtual void draw(bool uploadToTexture = true);
    Ogre::uint16 getWidth()  const { return mWidth;  }
    Ogre::uint16 getHeight() const { return mHeight; }

    /** Set the state of the element.
     ** @param  state The state is the look of the element.
     ** @return Returns true if the state was changed.
     *****************************************************************************/
    bool setState(const Ogre::uchar state);

    /** Set the position of the element.
     ** @param x The x-pos in pixel from the top-left pos of the parent window.
     ** @param y The y-pos in pixel from the top-left pos of the parent window.
     *****************************************************************************/
    void setPosition(const int x, const int y)
    {
        mPosX = x;
        mPosY = y;
    }

    /** Returns true if the mouse is within the borders of this element.
     ** @param x The mouse x-pos in pixel.
     ** @param y The mouse y-pos in pixel.
     *****************************************************************************/
    bool mouseWithin(const int x, const int y) const
    {
        return !(!mVisible || x < mPosX || x > mPosX + mWidth || y < mPosY || y > mPosY + mHeight);
    }
    /// Returns the unique numer of this element.
    int getIndex() const { return mIndex;  }

protected:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    int mIndex;                          /**< Unique number. -1 means its a background-gfx (no interaction) **/
    bool mVisible;
    GuiWindow *mParent;                  /**< Pointer to the parent window. **/
    Ogre::uchar mState;                  /**< Actual state of this element. **/
    Ogre::uint16 mPosX, mPosY;           /**< Pixeloffset from the upper-left corner of the window. **/
    Ogre::uint16 mWidth, mHeight;        /**< Dimension of this element. **/
    Ogre::uint16 mFontNr, mLabelFontNr;
    Ogre::uint16 mLabelPosX, mLabelPosY;
    Ogre::String mLabelString;
    Ogre::uint32 mLabelColor;            /**< The textcolor of the label. **/
    Ogre::uint32 mFillColor;             /**< The fill color of the element. Only used when mGfxSrc is null. **/
    GuiImageset::gfxSrcEntry *mGfxSrc;   /**< Pointer to the gfx-data structure or 0 for a colorfill. **/

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    bool isVisible() const { return mVisible; }
    void setVisible(bool visible);
};

#endif
