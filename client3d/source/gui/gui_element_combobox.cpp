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

#include <OgreHardwarePixelBuffer.h>
#include "logger.h"
#include "profiler.h"
#include "gui/gui_graphic.h"
#include "gui/gui_textout.h"
#include "gui/gui_element_combobox.h"

using namespace Ogre;

//================================================================================================
// Constructor.
//================================================================================================
GuiElementCombobox::GuiElementCombobox(TiXmlElement *xmlRoot, const void *parent):GuiElement(xmlRoot, parent)
{
    PROFILE()
    mListColor  = 0xff00ff00;
    mBorderColor= 0xff000000;
    mCursorColor= 0xff888888;
    if ((xmlRoot = xmlRoot->FirstChildElement("Itemlist")))
    {
        TiXmlElement *xmlElem;
        const char *strTmp;
        for (xmlElem = xmlRoot->FirstChildElement("Item"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Item"))
        {
            if ((strTmp = xmlElem->Attribute("text"))) mvItem.push_back(strTmp);
        }
        if ((xmlElem = xmlRoot->FirstChildElement("Size")))
        {
            if ((strTmp = xmlElem->Attribute("height"))) mListHeight = atoi(strTmp);
        }
        if ((xmlElem = xmlRoot->FirstChildElement("BackColor")))
        {
            if ((strTmp = xmlElem->Attribute("alpha"))) mListColor = atoi(strTmp) << 24;
            if ((strTmp = xmlElem->Attribute("red"  ))) mListColor+= atoi(strTmp) << 16;
            if ((strTmp = xmlElem->Attribute("green"))) mListColor+= atoi(strTmp) <<  8;
            if ((strTmp = xmlElem->Attribute("blue" ))) mListColor+= atoi(strTmp);
        }
        if ((xmlElem = xmlRoot->FirstChildElement("BorderColor")))
        {
            if ((strTmp = xmlElem->Attribute("alpha"))) mBorderColor = atoi(strTmp) << 24;
            if ((strTmp = xmlElem->Attribute("red"  ))) mBorderColor+= atoi(strTmp) << 16;
            if ((strTmp = xmlElem->Attribute("green"))) mBorderColor+= atoi(strTmp) <<  8;
            if ((strTmp = xmlElem->Attribute("blue" ))) mBorderColor+= atoi(strTmp);
        }
        if ((xmlElem = xmlRoot->FirstChildElement("CursorColor")))
        {
            if ((strTmp = xmlElem->Attribute("alpha"))) mCursorColor = atoi(strTmp) << 24;
            if ((strTmp = xmlElem->Attribute("red"  ))) mCursorColor+= atoi(strTmp) << 16;
            if ((strTmp = xmlElem->Attribute("green"))) mCursorColor+= atoi(strTmp) <<  8;
            if ((strTmp = xmlElem->Attribute("blue" ))) mCursorColor+= atoi(strTmp);
        }
    }
    mActItem = 0;
    mMouseOver = false;
    mMouseButDown = false;
    mShowItemList = false;
    calcListHeight();
    draw();
}

//================================================================================================
// Destructor.
//================================================================================================
GuiElementCombobox::~GuiElementCombobox()
{
    PROFILE()
    mvItem.clear();
}
//================================================================================================
// Message handling.
//================================================================================================
void GuiElementCombobox::sendMsg(const int message, String &text, uint32 &param, const char * /*text2*/)
{
    PROFILE()
    switch (message)
    {
        case GuiManager::MSG_SET_VISIBLE:
            if (setHidden(param?false:true)) draw();
            return;
        case GuiManager::MSG_SET_TEXT:
            //setLabel(text);
            return;
    }
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
int GuiElementCombobox::mouseEvent(const int mouseAction, int mouseX, int mouseY, int /*mouseWheel*/)
{
    PROFILE()
    if (!mouseWithin(mouseX, mouseY))
    {
        // Mouse is no longer over the gadget.
        if (setState(GuiImageset::STATE_ELEMENT_DEFAULT))
        {
            /*
            mMouseOver = false;
            mMouseButDown = false;
            draw();
            GuiManager::getSingleton().setTooltip("");
            */
            return GuiManager::EVENT_CHECK_NEXT;
        }
    }
    else // Mousecursor is over the button.
    {
        /*
                if (!mMouseOver)
                {
                    mMouseOver = true;
                    if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
                    //GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
                    return GuiManager::EVENT_CHECK_DONE;
                }
        */
        if (mouseAction == GuiManager::BUTTON_PRESSED)
        {
            mShowItemList = !mShowItemList;
            mMouseButDown = mShowItemList;
            setState(mShowItemList?GuiImageset::STATE_ELEMENT_PUSHED:GuiImageset::STATE_ELEMENT_DEFAULT);
            draw();
            return GuiManager::EVENT_CHECK_DONE;
        }
        // if (itemWasPressed) ... return GuiManager::EVENT_USER_ACTION;
        /*
        if (mouseAction == GuiManager::BUTTON_RELEASED && mMouseButDown)
        {
            mMouseButDown = false;
            //if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            //mShowItemList = !mShowItemList;
            //return GuiManager::EVENT_USER_ACTION;
        }
        */
        return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
    }
    return GuiManager::EVENT_CHECK_NEXT; // No action here, check the other gadgets.
}

//================================================================================================
// Draw the guiElement.
//================================================================================================
void GuiElementCombobox::draw()
{
    PROFILE()
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    if (mHidden)
    {
        GuiGraphic::getSingleton().restoreWindowBG(mWidth, mHeight+mListHeight,
                mParent->getLayerBG()+mPosX+mPosY*mParent->getWidth(), dst, mParent->getWidth(), mWidth);
    }
    else
    {
        const int LIST_BORDER_SIZE = 1;
        GuiElement::draw(false); // Draw the combobox background.
        GuiGraphic::getSingleton().restoreWindowBG(mWidth, mListHeight,
                mParent->getLayerBG()+mPosX+(mPosY+mHeight)*mParent->getWidth(),
                dst+mHeight*mWidth , mParent->getWidth(), mWidth);
        int fontHeight = GuiTextout::getSingleton().getFontHeight(mLabelFontNr);
        GuiTextout::getSingleton().printText(mWidth, fontHeight, dst+mLabelPosX+(mLabelPosY*mWidth), mWidth,
                                             mvItem[mActItem].c_str(), mLabelFontNr);
        if (mShowItemList && !mvItem.empty() && mListHeight)
        {
            uint32 *buf = dst+mHeight*mWidth;
            // Background itemlist.
            GuiGraphic::getSingleton().drawColorBorder(mWidth, mListHeight, mListColor, mBorderColor, dst+mHeight*mWidth, mWidth, LIST_BORDER_SIZE);
            // Background active item.
            GuiGraphic::getSingleton().blendColorToBuffer(mWidth-LIST_BORDER_SIZE*2, fontHeight, mCursorColor,
                                                            dst+(fontHeight*mActItem+mHeight+LIST_BORDER_SIZE)*mWidth+LIST_BORDER_SIZE, mWidth);
            buf = dst+mHeight*mWidth;
            for (std::vector<String>::iterator i = mvItem.begin(); i < mvItem.end(); ++i)
            {
                GuiTextout::getSingleton().printText(mWidth-LIST_TEXT_OFFSET, fontHeight,
                                                     buf+LIST_TEXT_OFFSET+(LIST_TEXT_OFFSET*mWidth), mWidth, (*i).c_str(), mLabelFontNr);
                buf+= fontHeight * mWidth;
            }
        }
    }
    // Blit to the texture.
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight+mListHeight, 1, PF_A8R8G8B8, dst),
            Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight+mListHeight));
}

//================================================================================================
// Calc the height of the itemlist.
//================================================================================================
void GuiElementCombobox::calcListHeight()
{
    int fontHeight = GuiTextout::getSingleton().getFontHeight(mLabelFontNr);
    mListHeight = fontHeight * mvItem.size() + LIST_TEXT_OFFSET;
    while (mListHeight > LIST_MAX_HEIGHT || mPosY+mHeight+mListHeight+LIST_TEXT_OFFSET > mParent->getHeight())
        mListHeight-= fontHeight;
    if (mListHeight < 0) mListHeight = 0;
}
