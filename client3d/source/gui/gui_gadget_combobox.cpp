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

#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_gadget_combobox.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "logger.h"

GuiGadgetCombobox::GuiGadgetCombobox(TiXmlElement *xmlElement, int w, int h, int maxX, int maxY) : GuiGadget(xmlElement, w, h, maxX, maxY)
{
    /// ////////////////////////////////////////////////////////////////////
    /// .
    /// ////////////////////////////////////////////////////////////////////
    mvValue.push_back(-1);
    mvOption.push_back("");

    TiXmlElement *xmlOpt;
    for (xmlOpt = xmlElement->FirstChildElement("Option"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Option"))
    {
        mvOption.push_back(xmlOpt->Attribute("text"));
    }

    srcButton = GuiImageset::getSingleton().getStateGfxPositions(xmlElement->FirstChildElement("Button")->Attribute("image_name"));
    srcScrollbarDown = GuiImageset::getSingleton().getStateGfxPositions(xmlElement->FirstChildElement("Scroll")->FirstChildElement("Down")->Attribute("image_name"));
    srcScrollbarUp = GuiImageset::getSingleton().getStateGfxPositions(xmlElement->FirstChildElement("Scroll")->FirstChildElement("Up")->Attribute("image_name"));

    mMaxChars = 20;
    mUseNumbers = true;
    mUseWhitespaces = true;
    mDispDropdown = false;
    mGfxBuffer = NULL;
    mEntryHeight = mHeight;
    mVirtualHeight = GuiTextout::getSingleton().getFontHeight(mLabelFont) * (mvOption.size()-1);
    mScrollPos = 0;

    if ( mY + mEntryHeight + mVirtualHeight > maxY )
    {
        printf("The dropdown needs a scroll\n");
        mNeedsScroll = true;
        mViewport = maxY - mY - mEntryHeight;
        mScrollPos = mVirtualHeight - mViewport;
    }
    else
    {
        printf("Noo need for a scroll here... \n");
        mNeedsScroll = false;
    }
}

GuiGadgetCombobox::~GuiGadgetCombobox()
{
    if ( mGfxBuffer )
        delete[] mGfxBuffer;
}

void GuiGadgetCombobox::draw(PixelBox &mSrcPixelBox, Texture *texture)
{
    /// ////////////////////////////////////////////////////////////////////
    /// Save background if needed.
    /// ////////////////////////////////////////////////////////////////////

    if ( mDispDropdown )
    {
        if (!mGfxBuffer)
        {

            bw = mWidth;
            // Bet changing the height like this is gona break something in some future
            if ( mNeedsScroll )
                mHeight += mViewport;
            else
                mHeight += mVirtualHeight;

            mGfxBuffer = new uint32[bw * (mHeight-mEntryHeight)];
            texture->getBuffer()->blitToMemory(
                Box(mX, mY+ mEntryHeight, mX + bw, mY+ mHeight),
                PixelBox(bw, mHeight - mEntryHeight, 1, PF_A8R8G8B8 , mGfxBuffer));
        }
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Draw gaget background.
    /// ////////////////////////////////////////////////////////////////////


    PixelBox src = mSrcPixelBox.getSubVolume(Box(
                       gfxSrcPos[0].x,
                       gfxSrcPos[0].y,
                       gfxSrcPos[0].x + mWidth,
                       gfxSrcPos[0].y + mHeight));
    texture->getBuffer()->blitFromMemory(src, Box(mX, mY, mX + mWidth, mY + mHeight));

    /// ////////////////////////////////////////////////////////////////////
    /// Draw the down button is it is given ( else this will turn into an entry box
    /// ////////////////////////////////////////////////////////////////////

    if (srcButton)
    {
        PixelBox srcbtn = mSrcPixelBox.getSubVolume(Box(
                              srcButton->state[0]->x,
                              srcButton->state[0]->y,
                              srcButton->state[0]->x + srcButton->width,
                              srcButton->state[0]->y + srcButton->height));
        texture->getBuffer()->blitFromMemory(srcbtn, Box(mX + mWidth - srcButton->width, mY, mX + mWidth, mY + mEntryHeight));
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Draw the current line of text
    /// ////////////////////////////////////////////////////////////////////

    TextLine label;
    label.index= -1;
    label.font = mLabelFont;
    label.clipped = false;

    label.x1 = mX + mLabelXPos;
    if ( srcButton )
        label.x2 = label.x1 + mWidth - srcButton->width - mLabelXPos;
    else
        label.x2 = label.x1 + mWidth - mLabelXPos;
    label.y1 = mY+ mLabelYPos;
    label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font) - mLabelYPos;

    GuiTextout::getSingleton().Print(&label, texture, mvOption[0].c_str());

    /// ////////////////////////////////////////////////////////////////////
    /// Draw the dropdown lines, each line will only be the height of the
    /// text no extra spacing at the moment
    /// ////////////////////////////////////////////////////////////////////
    if ( mDispDropdown )
    {
        label.x1 = mX+ mLabelXPos;
        label.x2 = label.x1 + mWidth - mLabelXPos;
        if ( mNeedsScroll )
        {
            /// The up button is deciding the width of the bar at the moment
            if (srcScrollbarUp)
            {
                PixelBox srcbtn = mSrcPixelBox.getSubVolume(Box(
                                      srcScrollbarUp->state[0]->x,
                                      srcScrollbarUp->state[0]->y,
                                      srcScrollbarUp->state[0]->x + srcScrollbarUp->width,
                                      srcScrollbarUp->state[0]->y + srcScrollbarUp->height));
                texture->getBuffer()->blitFromMemory(srcbtn, Box(mX + mWidth - srcScrollbarUp->width, mY + mEntryHeight, mX + mWidth, mY + mEntryHeight + srcScrollbarUp->height));

                label.x2 -= srcScrollbarUp->width;
            }
            if (srcScrollbarDown)
            {
                PixelBox srcbtn = mSrcPixelBox.getSubVolume(Box(
                                      srcScrollbarDown->state[0]->x,
                                      srcScrollbarDown->state[0]->y,
                                      srcScrollbarDown->state[0]->x + srcScrollbarDown->width,
                                      srcScrollbarDown->state[0]->y + srcScrollbarDown->height));
                texture->getBuffer()->blitFromMemory(srcbtn, Box(mX + mWidth - srcScrollbarDown->width, mY + mEntryHeight + mViewport - srcScrollbarDown->height, mX + mWidth, mY + mEntryHeight + mViewport));
            }
        }
        for( unsigned int i = 1 ; i < mvOption.size() ; i++ )
        {
            label.y1 = mY + mEntryHeight + GuiTextout::getSingleton().getFontHeight(label.font) * (i-1) - mScrollPos;
            label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);

            if ( label.y2 < mY + mEntryHeight )
                continue;
            if ( label.y1 < mY + mEntryHeight )
                label.y1 = mY + mEntryHeight;
            // If the text need clipping and if so dont continue throw the list
            if ( label.y2 > texture->getSrcHeight())
            {
                label.y2 = texture->getSrcHeight();
                GuiTextout::getSingleton().Print(&label, texture, mvOption[i].c_str());
                break;
            }

            GuiTextout::getSingleton().Print(&label, texture, mvOption[i].c_str());
        }

    }
    else if ( mGfxBuffer )
    {
        texture->getBuffer()->blitFromMemory(
            PixelBox(bw, mHeight - mEntryHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
            Box(mX, mY+ mEntryHeight, mX + bw, mY+ mHeight));

        delete[] mGfxBuffer;
        mGfxBuffer = NULL;
        mHeight = mEntryHeight;
    }
}

bool GuiGadgetCombobox::setState(int state)
{
    if ( mState != state && mState == STATE_PUSHED)
    {
        if ( mActiveDropdownOption != -1 )
        {
            setText(mvOption[mActiveDropdownOption].c_str());
            mDispDropdown = false;
            mActiveDropdownOption = -1;
        }
        else
        {
            switch ( mButton )
            {
                case GUI_GADGET_COMBOBOX_DDBUTTON:
                mDispDropdown = true;
                break;
                case GUI_GADGET_COMBOBOX_SCROLL_DOWN:
                mScrollPos += 5;
                if ( mScrollPos > mVirtualHeight - mViewport )
                    mScrollPos = mVirtualHeight - mViewport;
                break;
                case GUI_GADGET_COMBOBOX_SCROLL_UP:
                mScrollPos -= 5;
                if ( mScrollPos < 0 )
                    mScrollPos = 0;
                break;
                default:
                mAction = GuiWindow::GUI_ACTION_START_TEXT_INPUT;
            }
        }
    }

    return GuiElement::setState(state);
}

void GuiGadgetCombobox::setText(const char *value)
{
    mvOption[0] = value;
}

const char *GuiGadgetCombobox::getText()
{
    return mvOption[0].c_str();
}

bool GuiGadgetCombobox::mouseOver(int x, int y)
{
    if ( GuiElement::mouseOver(x,y) )
    {
        if ( mDispDropdown )
        {
            if ( y - mY < mEntryHeight )
                mActiveDropdownOption = 0;
            else if ( !srcScrollbarUp || x - mX < mWidth - srcScrollbarUp->width )
            {
                if ( y - mY < mEntryHeight )
                    mActiveDropdownOption = 0;
                else
                    mActiveDropdownOption = (y - mY - mEntryHeight + mScrollPos) / GuiTextout::getSingleton().getFontHeight(mLabelFont) + 1;
                if ( (unsigned int)mActiveDropdownOption > mvOption.size() )
                    mActiveDropdownOption = mvOption.size()-1;
            }
            else
            {
                mActiveDropdownOption = -1;
                if ( y - mY - mEntryHeight < srcScrollbarUp->height )
                    mButton = GUI_GADGET_COMBOBOX_SCROLL_UP;
                else if ( y - mY - mEntryHeight > mViewport - srcScrollbarUp->height )
                    mButton = GUI_GADGET_COMBOBOX_SCROLL_DOWN;
                else
                    mButton = GUI_GADGET_COMBOBOX_SCROLL_BAR;
            }
        }
        else
        {
            mActiveDropdownOption = -1;
            if( srcButton && x > mX + mWidth - srcButton->width )
                mButton = GUI_GADGET_COMBOBOX_DDBUTTON;
            else
                mButton = GUI_GADGET_COMBOBOX_NONE;
        }

        return true;
    }
    if ( mDispDropdown )
        mDispDropdown = false;
    return false;
}
