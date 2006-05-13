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
#include "gui_manager.h"
#include "logger.h"

#include <ctime>


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
        mvValue.push_back(atol(xmlOpt->Attribute("value")));
        mvOption.push_back(xmlOpt->Attribute("text"));
    }

		srcButton = GuiImageset::getSingleton().getStateGfxPositions(xmlElement->FirstChildElement("Button")->Attribute("image_name"));
		
    mMaxChars = 20;
    mUseNumbers = true;
    mUseWhitespaces = true;
		mDispDropdown = false;
		mGfxBuffer = NULL;
		mEntryHeight = mHeight;
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
				// Bet this is gona break something in some future
				mHeight += GuiTextout::getSingleton().getFontHeight(mLabelFont) * (int) (mvOption.size()-1);
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

    label.x1 = mX+ mLabelXPos;
		if ( srcButton )
			label.x2 = label.x1 + mWidth - srcButton->width;
		else
			label.x2 = label.x1 + mWidth;
    label.y1 = mY+ mLabelYPos;
    label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);

    GuiTextout::getSingleton().Print(&label, texture, mvOption[0].c_str());
		
		/// ////////////////////////////////////////////////////////////////////
    /// Draw the dropdown lines
    /// ////////////////////////////////////////////////////////////////////
		if ( mDispDropdown )
		{
			for( unsigned int i = 1 ; i < mvOption.size() ; i++ )
			{
				label.x1 = mX+ mLabelXPos;
				label.x2 = label.x1 + mWidth;
				label.y1 = mY+ mLabelYPos + GuiTextout::getSingleton().getFontHeight(label.font) * i;
				label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font) * (i+1);
			
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
			else if ( mDDButton ) // if clicked on the entry part
			{
				mDispDropdown = true;
			}
			else
			{
				mAction = GUI_ACTION_START_TEXT_INPUT;
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
			else
				mActiveDropdownOption = (y - mY - mEntryHeight) / GuiTextout::getSingleton().getFontHeight(mLabelFont) + 1;
			if ( (unsigned int)mActiveDropdownOption > mvOption.size() )
				mActiveDropdownOption = (int) mvOption.size()-1;
		}
		else
		{
			mActiveDropdownOption = -1;
			if( srcButton && x > mX + mWidth - srcButton->width )
				mDDButton = true;
			else
				mDDButton = false;
		}
			
		return true;
	}
	if ( mDispDropdown )
		mDispDropdown = false;
	return false;
}
