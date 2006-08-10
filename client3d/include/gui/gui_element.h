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

#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include <tinyxml.h>
#include <Ogre.h>
#include "gui_imageset.h"

using namespace Ogre;

/**
 ** This is the base class for a gui element.
 *****************************************************************************/
class GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiElement(TiXmlElement *xmlElement, void *parent);
    virtual ~GuiElement();
    bool setState(int state);
    int getState()
    {
        return mState;
    }
    const char *getName()
    {
        return mStrName.c_str();
    }
    int getIndex()
    {
        return mIndex;
    }
    void setPosition(int x, int y)
    {
        mX = x;
        mY = y;
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
    // Variables.
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
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    GuiImageset::gfxPos gfxSrcPos[GuiImageset::STATE_ELEMENT_SUM];
    int mX, mY, mMaxX, mMaxY;
    int mWidth, mHeight, mSrcWidth, mSrcHeight;
    int mFontNr;
    int mHasAlpha;
    int mState;
    String mStrType, mStrName, mStrImageName;
    uint32 mFillColor;
    uint32 *BG_Backup;    /**< Backup buffer for alpha blit. **/
    int id;
    int mIndex;
    String mStrLabel, mStrBgLabel, mStrTooltip;
    unsigned char mLabelColor[3];
    int mLabelFont;
    int mLabelXPos, mLabelYPos;
    void *mParent; /**< Pointer to the parent window. **/
};

#endif
