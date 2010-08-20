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
        if ((xmlElem = xmlRoot->FirstChildElement("Color")))
        {
            // PixelFormat: ARGB.
            if ((strTmp = xmlElem->Attribute("red"  ))) mListColor = atoi(strTmp) << 16;
            if ((strTmp = xmlElem->Attribute("green"))) mListColor+= atoi(strTmp) <<  8;
            if ((strTmp = xmlElem->Attribute("blue" ))) mListColor+= atoi(strTmp);
            if ((strTmp = xmlElem->Attribute("alpha"))) mListColor+= atoi(strTmp) << 24;
        }
    }
    else // Default values.
    {
        mListHeight = 200;
        mListColor = 0xff00ff00;
    }
    mActItem = 0;
    mMouseOver = false;
    mMouseButDown = false;
    mShowItemList = false;
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
            setHidden(param?false:true);
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
    GuiElement::draw(false);
    if (!mHidden && !mvItem.empty())
    {
        // The combobox gfx was drawn by GuiElement::draw() into the build-buffer, now blend
        // the text for the actice item over it. Then blit the result into the window-texture.
        //int fontHeight = GuiTextout::getSingleton().getFontHeight(mLabelFontNr);
        uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
        mLabelPosX = 2; // Just for testing!!!!
        GuiTextout::getSingleton().printText(mWidth-mLabelPosX, mHeight, dst+mLabelPosX,
                                             mWidth, mvItem[mActItem].c_str(), mLabelFontNr);
        mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst),
                Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight));
    }
    drawItemList();
}

//================================================================================================
// Draw the guiElement.
//================================================================================================
void GuiElementCombobox::drawItemList()
{
    PROFILE()
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    if (mHidden || !mShowItemList || mvItem.empty())
    {
        uint32 *bak = mParent->getLayerBG() + mPosX + (mPosY+mHeight)*mParent->getWidth();
        GuiGraphic::getSingleton().restoreWindowBG(mWidth, mListHeight, bak, dst, mParent->getWidth(), mWidth);
    }
    else
    {
        int fontHeight = GuiTextout::getSingleton().getFontHeight(mLabelFontNr);
        uint32 *buf = dst;
        // Draw all items
        for (unsigned int i = 0; i < mWidth * mListHeight; ++i) buf[i] = mListColor;
        for (std::vector<String>::iterator i = mvItem.begin(); i < mvItem.end(); ++i)
        {
            GuiTextout::getSingleton().printText(mWidth, fontHeight, buf, mWidth, (*i).c_str(), mLabelFontNr);
            buf+= fontHeight * mWidth;
        }
    }
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mListHeight, 1, PF_A8R8G8B8, dst), Box(mPosX, mPosY+mHeight, mPosX+mWidth, mPosY+mHeight+mListHeight));
}
