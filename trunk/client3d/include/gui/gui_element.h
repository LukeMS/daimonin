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

#include <tinyxml.h>
#include <Ogre.h>
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
    GuiElement(TiXmlElement *xmlElement, void *parent, bool forceAlpha = false);
    virtual ~GuiElement();
    bool setState(int state);
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
    virtual void draw() =0;
    const char *getTooltip()
    {
        return mStrTooltip.c_str();
    }

    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        FILL_GFX,
        FILL_COLOR,
        FILL_NONE,
        FILL_SUM
    };
    enum
    {
        TYPE_GFX,
        TYPE_BUTTON,
        TYPE_BUTTON_CHECK,
        TYPE_BUTTON_RADIO,
        TYPE_SLIDER,
        TYPE_SUM
    };

protected:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    int mIndex;                /**< Unique number. **/
    int mPosX, mPosY;          /**< Position of this element. **/
    int mSrcWidth, mSrcHeight; /**< Dimension of the graphics source. **/
    int mWidth, mHeight;       /**< Dimension of this element. **/
    int mState;                /**< Actual state of this element. **/
    int mFillType;             /**< The way the background is drawn. **/
    int mHasAlpha;             /**< Graphics is using alpha values. **/
    int mFontNr, mLabelFontNr;
    int mLabelPosX, mLabelPosY;
    Ogre::String mStrLabel, mStrTooltip;
    Ogre::uint32 mFillColor;
    Ogre::uint32 *LayerWindowBG;         /**< Backup buffer for alpha blit. **/
    void *mParent;             /**< Pointer to the parent window. **/
    unsigned char mLabelColor[3];
    GuiImageset::gfxPos gfxSrcPos[GuiImageset::STATE_ELEMENT_SUM];
};

#endif
