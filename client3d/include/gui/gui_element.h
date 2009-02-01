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

#include <Ogre.h>
#include <tinyxml.h>
#include "gui_imageset.h"

/**
 ** This is the base class for a gui element.
 *****************************************************************************/
class GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /** All functions that are called from outside the gui are using the Message system. **/
    virtual int sendMsg(int message, void *parm1 =0, void *parm2 =0, void *parm3 =0);

    /** Internal gui functions **/
    GuiElement(TiXmlElement *xmlElement, void *parent);
    virtual ~GuiElement() {};
    virtual void update(Ogre::Real deltaTime) {} /**< Animations, drag'n'drop, etc **/
    virtual void draw();

    bool setState(int state); /**< Returns true if the state was changed. **/
    int getState()
    {
        return mState;
    }
    int getIndex()
    {
        return mIndex;
    }
    void setPosition(int x, int y)
    {
        mPosX = x;
        mPosY = y;
    }
    int getWidth()
    {
        return mWidth;
    }
    int getHeight()
    {
        return mHeight;
    }

    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { BACKGROUND_GFX_ID = -1 };

protected:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    int mIndex;                        /**< Unique number. **/
    int mPosX, mPosY;                  /**< Position of this element. **/
    int mWidth, mHeight;               /**< Dimension of this element. **/
    int mState;                        /**< Actual state of this element. **/
    int mFontNr, mLabelFontNr;
    int mLabelPosX, mLabelPosY;
    bool mIsVisible;
    Ogre::String mStrLabel;

    Ogre::String mStrTooltip; // REMOVE ME: Wrong place - this is not partr of the elment core.

    Ogre::uint32 mFillColor;
    class GuiWindow *mParent;          /**< Pointer to the parent window. **/
    unsigned char mLabelColor[3]; //  Change this to uint32. we must support alpha here.
    GuiImageset::gfxSrcEntry *mGfxSrc; /**< Pointer to the gfx-data structure or 0 for a colorfill. **/
    bool mouseWithin(int x, int y)
    {
        return !(x < mPosX || x > mPosX + mWidth || y < mPosY || y > mPosY + mHeight);
    }
};

#endif
