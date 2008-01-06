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

#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_gadget_combobox.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "logger.h"

using namespace Ogre;

// needs to be rewritten!

GuiGadgetCombobox::GuiGadgetCombobox(TiXmlElement *xmlElement, void *parent) :GuiElement(xmlElement, parent)
{
    /*
    // ////////////////////////////////////////////////////////////////////
    // .
    // ////////////////////////////////////////////////////////////////////
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
    mVirtualHeight = GuiTextout::getSingleton().getFontHeight(mLabelFontNr) * ((int)mvOption.size()-1);
    mScrollPos = 0;

    int mMaxY = 0;

    if ( mPosY + mEntryHeight + mVirtualHeight > mMaxY )
    {
        printf("The dropdown needs a scroll\n");
        mNeedsScroll = true;
        mViewport = mMaxY - mPosY - mEntryHeight;
        mScrollPos = mVirtualHeight - mViewport;
    }
    else
    {
        printf("Noo need for a scroll here... \n");
        mNeedsScroll = false;
    }
    */
}

GuiGadgetCombobox::~GuiGadgetCombobox()
{
    if ( mGfxBuffer )
        delete[] mGfxBuffer;
}

void GuiGadgetCombobox::draw()
{
    // ////////////////////////////////////////////////////////////////////
    // Save background if needed.
    // ////////////////////////////////////////////////////////////////////
    Texture *texture = mParent->getTexture();
    PixelBox *mSrcPixelBox = mParent->getPixelBox();

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
                Box(mPosX, mPosY+ mEntryHeight, mPosX + bw, mPosY+ mHeight),
                PixelBox(bw, mHeight - mEntryHeight, 1, PF_A8R8G8B8 , mGfxBuffer));
        }
    }

    // ////////////////////////////////////////////////////////////////////
    // Draw gaget background.
    // ////////////////////////////////////////////////////////////////////
    PixelBox src = mSrcPixelBox->getSubVolume(Box(
                       mGfxSrc->state[0].x,
                       mGfxSrc->state[0].y,
                       mGfxSrc->state[0].x + mWidth,
                       mGfxSrc->state[0].y + mHeight));
    texture->getBuffer()->blitFromMemory(src, Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));

    // ////////////////////////////////////////////////////////////////////
    // Draw the down button is it is given ( else this will turn into an entry box
    // ////////////////////////////////////////////////////////////////////
    if (srcButton)
    {
        PixelBox srcbtn = mSrcPixelBox->getSubVolume(Box(
                              srcButton->state[0].x,
                              srcButton->state[0].y,
                              srcButton->state[0].x + srcButton->w,
                              srcButton->state[0].y + srcButton->h));
        texture->getBuffer()->blitFromMemory(srcbtn, Box(mPosX + mWidth - srcButton->w, mPosY, mPosX + mWidth, mPosY + mEntryHeight));
    }

    // ////////////////////////////////////////////////////////////////////
    // Draw the current line of text
    // ////////////////////////////////////////////////////////////////////
    GuiTextout::TextLine label;
    label.index= -1;
    label.hideText= false;
    label.font = mLabelFontNr;
    label.x1 = mPosX + mLabelPosX;
    if ( srcButton )
        label.x2 = label.x1 + mWidth - srcButton->w - mLabelPosX;
    else
        label.x2 = label.x1 + mWidth - mLabelPosX;
    label.y1 = mPosY+ mLabelPosY;
    label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font) - mLabelPosY;
    label.text = mvOption[0];
    GuiTextout::getSingleton().Print(&label, texture);

    // ////////////////////////////////////////////////////////////////////
    // Draw the dropdown lines, each line will only be the height of the
    // text no extra spacing at the moment
    // ////////////////////////////////////////////////////////////////////
    if ( mDispDropdown )
    {
        label.x1 = mPosX+ mLabelPosX;
        label.x2 = label.x1 + mWidth - mLabelPosX;
        if ( mNeedsScroll )
        {
            // The up button is deciding the width of the bar at the moment
            if (srcScrollbarUp)
            {
                PixelBox srcbtn = mSrcPixelBox->getSubVolume(Box(
                                      srcScrollbarUp->state[0].x,
                                      srcScrollbarUp->state[0].y,
                                      srcScrollbarUp->state[0].x + srcScrollbarUp->w,
                                      srcScrollbarUp->state[0].y + srcScrollbarUp->h));
                texture->getBuffer()->blitFromMemory(srcbtn, Box(mPosX + mWidth - srcScrollbarUp->w, mPosY + mEntryHeight, mPosX + mWidth, mPosY + mEntryHeight + srcScrollbarUp->h));
                label.x2 -= srcScrollbarUp->w;
            }
            if (srcScrollbarDown)
            {
                PixelBox srcbtn = mSrcPixelBox->getSubVolume(Box(
                                      srcScrollbarDown->state[0].x,
                                      srcScrollbarDown->state[0].y,
                                      srcScrollbarDown->state[0].x + srcScrollbarDown->w,
                                      srcScrollbarDown->state[0].y + srcScrollbarDown->h));
                texture->getBuffer()->blitFromMemory(srcbtn, Box(mPosX + mWidth - srcScrollbarDown->w, mPosY + mEntryHeight + mViewport - srcScrollbarDown->h, mPosX + mWidth, mPosY + mEntryHeight + mViewport));
            }
        }
        for ( unsigned int i = 1 ; i < mvOption.size() ; i++ )
        {
            label.y1 = mPosY + mEntryHeight + GuiTextout::getSingleton().getFontHeight(label.font) * (i-1) - mScrollPos;
            label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
            label.text = mvOption[i];
            if ( label.y2 < (unsigned int) mPosY + mEntryHeight )
                continue;
            if ( label.y1 < (unsigned int) mPosY + mEntryHeight )
                label.y1 = mPosY + mEntryHeight;
            // If the text need clipping and if so dont continue throw the list
            if ( label.y2 > texture->getSrcHeight())
            {
                label.y2 = (int)texture->getSrcHeight();
                GuiTextout::getSingleton().Print(&label, texture);
                break;
            }

            GuiTextout::getSingleton().Print(&label, texture);
        }

    }
    else if ( mGfxBuffer )
    {
        texture->getBuffer()->blitFromMemory(
            PixelBox(bw, mHeight - mEntryHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
            Box(mPosX, mPosY+ mEntryHeight, mPosX + bw, mPosY+ mHeight));

        delete[] mGfxBuffer;
        mGfxBuffer = NULL;
        mHeight = mEntryHeight;
    }
}

bool GuiGadgetCombobox::setState(int state)
{
    if ( mState != state && mState == GuiImageset::STATE_ELEMENT_PUSHED)
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

/*
bool GuiGadgetCombobox::mouseOver(int x, int y)
{
    if ( GuiElement::mouseOver(x,y) )
    {
        if ( mDispDropdown )
        {
            if ( y - mPosY < mEntryHeight )
                mActiveDropdownOption = 0;
            else if ( !srcScrollbarUp || x - mPosX < mWidth - srcScrollbarUp->width )
            {
                if ( y - mPosY < mEntryHeight )
                    mActiveDropdownOption = 0;
                else
                    mActiveDropdownOption = (y - mPosY - mEntryHeight + mScrollPos) / GuiTextout::getSingleton().getFontHeight(mLabelFontNr) + 1;
                if ( (unsigned int)mActiveDropdownOption > mvOption.size() )
                    mActiveDropdownOption = mvOption.size()-1;
            }
            else
            {
                mActiveDropdownOption = -1;
                if ( y - mPosY - mEntryHeight < srcScrollbarUp->height )
                    mButton = GUI_GADGET_COMBOBOX_SCROLL_UP;
                else if ( y - mPosY - mEntryHeight > mViewport - srcScrollbarUp->height )
                    mButton = GUI_GADGET_COMBOBOX_SCROLL_DOWN;
                else
                    mButton = GUI_GADGET_COMBOBOX_SCROLL_BAR;
            }
        }
        else
        {
            mActiveDropdownOption = -1;
            if( srcButton && x > mPosX + mWidth - srcButton->width )
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
*/
