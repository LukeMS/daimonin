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
#include "gui_element_combobox.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "logger.h"

using namespace Ogre;

// needs to be rewritten!

int GuiElementCombobox::sendMsg(int message, const char *text, uint32 param)
{
    return 0;
}

GuiElementCombobox::GuiElementCombobox(TiXmlElement *xmlElement, void *parent) :GuiElement(xmlElement, parent)
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

GuiElementCombobox::~GuiElementCombobox()
{
    if ( mGfxBuffer )
        delete[] mGfxBuffer;
}

void GuiElementCombobox::draw()
{
}

bool GuiElementCombobox::setState(int state)
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
                case GUI_ELEMENT_COMBOBOX_DDBUTTON:
                    mDispDropdown = true;
                    break;
                case GUI_ELEMENT_COMBOBOX_SCROLL_DOWN:
                    mScrollPos += 5;
                    if ( mScrollPos > mVirtualHeight - mViewport )
                        mScrollPos = mVirtualHeight - mViewport;
                    break;
                case GUI_ELEMENT_COMBOBOX_SCROLL_UP:
                    mScrollPos -= 5;
                    if ( mScrollPos < 0 )
                        mScrollPos = 0;
                    break;
                default:
                    mAction = 1;//GuiWindow::GUI_ACTION_START_TEXT_INPUT;
            }
        }
    }

    return GuiElement::setState(state);
}

void GuiElementCombobox::setText(const char *value)
{
    mvOption[0] = value;
}

const char *GuiElementCombobox::getText()
{
    return mvOption[0].c_str();
}

/*
bool GuiElementCombobox::mouseOver(int x, int y)
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
                    mButton = GUI_ELEMENT_COMBOBOX_SCROLL_UP;
                else if ( y - mPosY - mEntryHeight > mViewport - srcScrollbarUp->height )
                    mButton = GUI_ELEMENT_COMBOBOX_SCROLL_DOWN;
                else
                    mButton = GUI_ELEMENT_COMBOBOX_SCROLL_BAR;
            }
        }
        else
        {
            mActiveDropdownOption = -1;
            if( srcButton && x > mPosX + mWidth - srcButton->width )
                mButton = GUI_ELEMENT_COMBOBOX_DDBUTTON;
            else
                mButton = GUI_ELEMENT_COMBOBOX_NONE;
        }

        return true;
    }
    if ( mDispDropdown )
        mDispDropdown = false;
    return false;
}
*/
