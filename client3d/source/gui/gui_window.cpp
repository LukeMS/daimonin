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

#include <OgreTextureManager.h>
#include <OgreStringConverter.h>
#include "logger.h"
#include "profiler.h"
#include "gui/gui_window.h"
#include "gui/gui_cursor.h"
#include "gui/gui_imageset.h"
#include "gui/gui_element_slot.h"
#include "gui/gui_element_table.h"
#include "gui/gui_element_textbox.h"
#include "gui/gui_element_listbox.h"
#include "gui/gui_element_button.h"
#include "gui/gui_element_combobox.h"
#include "gui/gui_element_statusbar.h"

using namespace Ogre;

int GuiWindow::mDragOffsetX = -1;
int GuiWindow::mDragOffsetY = -1;
int GuiWindow::mDragElement = -1;
int GuiWindow::mDragWindowNr= -1;
int GuiWindow::mElementClicked = -1;
static const int MAX_WINDOW_SIZE = 1 << 10;

//================================================================================================
// Destructor.
//================================================================================================
void GuiWindow::freeRecources()
{
    PROFILE()
    for (std::vector<GuiElement*>::iterator i = mvElement.begin(); i < mvElement.end(); ++i)
        delete (*i);
    mvElement.clear();
    delete[] mWinLayerBG;
    mTexture.setNull();
}

//================================================================================================
// Build a window out of a xml description file.
//================================================================================================
void GuiWindow::Init(TiXmlElement *xmlRoot, const char *resourceWin, int winNr, uchar defaultZPos)
{
    PROFILE()
    mWinLayerBG = 0;
    mLastMouseOverElement = -1;
    mHeight = GuiElement::MIN_SIZE;
    mWidth  = GuiElement::MIN_SIZE;
    mWindowNr = winNr;
    TiXmlElement *xmlElem;
    int screenH = GuiManager::getSingleton().getScreenHeight();
    int screenW = GuiManager::getSingleton().getScreenWidth();
    const char *strTmp = xmlRoot->Attribute("name");
    if (strTmp && GuiManager::getSingleton().getPrintInfo())
        Logger::log().info() << "Parsing window: " << strTmp;
    // ////////////////////////////////////////////////////////////////////
    // Parse the Size entries.
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElem = xmlRoot->FirstChildElement("Size")))
    {
        if ((strTmp = xmlElem->Attribute("width")))  mWidth = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("height"))) mHeight= atoi(strTmp);
        if (mWidth < GuiElement::MIN_SIZE) mWidth  = GuiElement::MIN_SIZE;
        if (mWidth > screenW) mWidth = screenW;
        if (mHeight< GuiElement::MIN_SIZE) mHeight = GuiElement::MIN_SIZE;
        if (mHeight > screenH) mHeight = screenH;
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Alignment entries.
    // ////////////////////////////////////////////////////////////////////
    int aX =1, aY =1;
    if ((xmlElem = xmlRoot->FirstChildElement("Alignment")))
    {
        if ((strTmp = xmlElem->Attribute("x")))
        {
            if      (!stricmp(strTmp, "center")) aX = 0;
            else if (!stricmp(strTmp, "right"))  aX =-1;
        }
        if ((strTmp = xmlElem->Attribute("y")))
        {
            if      (!stricmp(strTmp, "center")) aY = 0;
            else if (!stricmp(strTmp, "bottom")) aY =-1;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Position entries.
    // ////////////////////////////////////////////////////////////////////
    mPosX = mPosY = 0;
    if ((xmlElem = xmlRoot->FirstChildElement("Pos")))
    {
        if ((strTmp = xmlElem->Attribute("x")))
        {
            if (aX <0) mPosX = screenW+1 - atoi(strTmp);
            else if (aX==0) mPosX =(screenW- mWidth) /2 + atoi(strTmp);
            else mPosX = atoi(strTmp);
        }
        if ((strTmp = xmlElem->Attribute("y")))
        {
            if (aY <0) mPosY = screenH+1 - atoi(strTmp);
            else if (aY==0) mPosY =(screenH- mHeight) /2 + atoi(strTmp);
            else mPosY = atoi(strTmp);
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Now we have all data to create the window.
    // ////////////////////////////////////////////////////////////////////
    mWinLayerBG = new uint32[mWidth * mHeight];
    if ((xmlElem = xmlRoot->FirstChildElement("Color")))
    {
        // PixelFormat: ARGB.
        uint32 color = 0;
        if ((strTmp = xmlElem->Attribute("red"  ))) color+= atoi(strTmp) << 16;
        if ((strTmp = xmlElem->Attribute("green"))) color+= atoi(strTmp) <<  8;
        if ((strTmp = xmlElem->Attribute("blue" ))) color+= atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("alpha"))) color+= atoi(strTmp) << 24;
        uint32 *dst = mWinLayerBG;
        for (int i=0; i < mWidth*mHeight; ++i)
            *dst++ = color;
    }
    else
    {
        memset(mWinLayerBG, 0x00, mWidth * mHeight * sizeof(uint32));
    }
    int textureSize = GuiElement::MIN_SIZE;
    int psize = (mWidth > mHeight)?mWidth:mHeight;
    while (textureSize < psize && textureSize < MAX_WINDOW_SIZE) textureSize <<= 1; // Make the size a power of 2.
    String strTexture = StringConverter::toString(textureSize) + "_";
    strTexture+= resourceWin + StringConverter::toString(mWindowNr) +GuiManager::TEXTURE_RESOURCE_NAME;
    mTexture = GuiManager::getSingleton().createTexture(strTexture);
    mElement = GuiManager::getSingleton().createOverlay(resourceWin + StringConverter::toString(mWindowNr), strTexture, mOverlay);
    mElement->setPosition(mPosX, mPosY);
    setZPos(defaultZPos);
    // ////////////////////////////////////////////////////////////////////
    // Parse the STATIC child elements.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Graphic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Graphic"))
    {
        GuiElement *gfx = new GuiElement(xmlElem, this);
        gfx->draw(true);
        delete gfx;
    }
    for (xmlElem = xmlRoot->FirstChildElement("String"); xmlElem; xmlElem = xmlElem->NextSiblingElement("String"))
    {
        GuiElementTextbox *str = new GuiElementTextbox(xmlElem, this);
        delete str;
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Dynamic child elements.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("TextBox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextBox"))
    {
        if (xmlElem->Attribute("name"))
            mvElement.push_back(new GuiElementTextbox(xmlElem, this));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Table"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Table"))
    {
        if (xmlElem->Attribute("name"))
            mvElement.push_back(new GuiTable(xmlElem, this));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Listbox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Listbox"))
    {
        if (xmlElem->Attribute("name"))
            mvElement.push_back(new GuiListbox(xmlElem, this));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Button"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Button"))
    {
        if (xmlElem->Attribute("name"))
            mvElement.push_back(new GuiElementButton(xmlElem, this));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Slot"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Slot"))
    {
        if (xmlElem->Attribute("name"))
            mvElement.push_back(new GuiElementSlot(xmlElem, this));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Slotgroup"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Slotgroup"))
    {
        if (xmlElem->Attribute("name"))
            mvElement.push_back(new GuiElementSlotGroup(xmlElem, this));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Statusbar"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Statusbar"))
    {
        if (xmlElem->Attribute("name"))
            mvElement.push_back(new GuiStatusbar(xmlElem, this));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Combobox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Combobox"))
    {
        if (xmlElem->Attribute("name"))
            mvElement.push_back(new GuiElementCombobox(xmlElem, this));
    }
}

//================================================================================================
// Check if an interavtive element overlaps another to prevent strange behavior of the gui
// like ignoring a button or activate 2 buttons at the same time.
//================================================================================================
void GuiWindow::checkForOverlappingElements()
{
    PROFILE()
    // Todo
    //Logger::log().warning() << "Element " << xyz << " opverlaps element " << xyz2;
}

//================================================================================================
// Centres the window on the mousecursor.
//================================================================================================
void GuiWindow::centerWindowOnMouse(int x, int y)
{
    PROFILE()
    if (!isVisible()) return;
    mPosX = (int)(x-mTexture->getWidth())/2;
    mPosY = (int)(y-mTexture->getHeight())/2 - 50;
    mElement->setPosition(mPosX, mPosY);
}

//================================================================================================
// Key event.
//================================================================================================
int GuiWindow::keyEvent(const int keyChar, const unsigned int key)
{
    PROFILE()
    if (!isVisible()) return GuiManager::EVENT_CHECK_NEXT;
    for (unsigned int i = 0; i < mvElement.size(); ++i)
    {
        if (mvElement[i]->keyEvent(keyChar, key) == GuiManager::EVENT_CHECK_DONE)
            return GuiManager::EVENT_CHECK_DONE;
    }
    return GuiManager::EVENT_CHECK_NEXT;
}

//================================================================================================
// Mouse Event.
//================================================================================================
int GuiWindow::mouseEvent(const int mouseAction, Vector3 &mouse)
{
    PROFILE()
    // ////////////////////////////////////////////////////////////////////
    // User is moving the window.
    // ////////////////////////////////////////////////////////////////////
    if (mDragWindowNr >= 0)
    {
        if (mDragWindowNr != mWindowNr) // User moves another window.
            return GuiManager::EVENT_CHECK_NEXT;
        if (mouseAction == GuiManager::BUTTON_RELEASED)
        {
            //GuiCursor::getSingleton().setState(GuiImageset::STATE_MOUSE_DEFAULT);
            mDragWindowNr= -1;
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (mouseAction == GuiManager::MOUSE_MOVEMENT)
        {
            mPosX = (int)mouse.x - mDragOffsetX;
            mPosY = (int)mouse.y - mDragOffsetY;
            mElement->setPosition(mPosX, mPosY);
            return GuiManager::EVENT_CHECK_DONE;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Is the mouse inside the window?
    // ////////////////////////////////////////////////////////////////////
    if (!mouseWithin((int)mouse.x, (int)mouse.y))
    {
        mouseLeftWindow();
        return GuiManager::EVENT_OUTSIDE_WIN;
    }
    // ////////////////////////////////////////////////////////////////////
    // Look for a mouse event in a child element.
    // ////////////////////////////////////////////////////////////////////
    int x = (int)mouse.x - mPosX;
    int y = (int)mouse.y - mPosY;
    for (unsigned int i = 0; i < mvElement.size(); ++i)
    {
        int event = mvElement[i]->mouseEvent(mouseAction, x, y, (int)mouse.z);
        if (event != GuiManager::EVENT_CHECK_NEXT)
        {
            mLastMouseOverElement = i;
            if (event == GuiManager::EVENT_USER_ACTION)
            {
                mElementClicked = mvElement[i]->getIndex();
                if (mElementClicked == GuiManager::BUTTON_CLOSE)
                {
                    setVisible(false);
                    return GuiManager::EVENT_CHECK_DONE;
                }
            }
            else if (event == GuiManager::EVENT_DRAG_STRT)
                mDragElement = i; // Drag from a slot.
            return event;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // When the mouse is not over a child element, window can be moved.
    // ////////////////////////////////////////////////////////////////////
    if (mouseAction == GuiManager::BUTTON_PRESSED)
    {
        //GuiCursor::getSingleton().setState(GuiImageset::STATE_MOUSE_PUSHED);
        GuiManager::getSingleton().windowToFront(mWindowNr);
        mDragOffsetX = x;
        mDragOffsetY = y;
        mDragWindowNr = mWindowNr;
    }
    mLastMouseOverElement = -1;
    return GuiManager::EVENT_CHECK_DONE;
}

//================================================================================================
// On overlapping windows with overlapping elements: When the mouse leaves a window
// we need to set back the default state of the "mouse-over-element".
//================================================================================================
void GuiWindow::mouseLeftWindow()
{
    PROFILE()
    if (mLastMouseOverElement >= 0)
    {
        mvElement[mLastMouseOverElement]->mouseEvent(-1, -1, -1, -1);
        mLastMouseOverElement = -1;
    }
    /*
        if (mDragElement >= 0)
        {
            mvElement[mDragElement]->mouseEvent(-1, -1, -1, -1);
            GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, StringConverter::toString(mDragElement).c_str());
        }
    */
}

//================================================================================================
// Show/hide the window.
//================================================================================================
void GuiWindow::setVisible(bool visible)
{
    PROFILE()
    if (!mOverlay) return;
    if (!visible) mOverlay->hide();
    else mOverlay->show();
}

//================================================================================================
// Update the window.
//================================================================================================
void GuiWindow::update(Ogre::Real timeSinceLastFrame)
{
    PROFILE()
    if (!isVisible()) return;
    // ToDo. Update drag animation (move back on wrong drag).
    for (unsigned int i = 0; i < mvElement.size(); ++i)
        mvElement[i]->update(timeSinceLastFrame);
}

//================================================================================================
//
//================================================================================================
void GuiWindow::sendMsg(int elementNr, int message, String &text, uint32 &param, const char *text2)
{
    PROFILE()
    if (!mvElement.empty())
    {
        if (elementNr <0 ||elementNr >= (int)mvElement.size())
        {
            Logger::log().error() << "Critical: GuiWindow::sendMsg: elementNr out of Range! (MessageID = " << message <<")";
            //Logger::log().error() << "Ignoring Message: (text)" << text.c_str() << " (text2) " << text2;
            return;
        }
        mvElement[elementNr]->sendMsg((int)message, text, param, text2);
    }
}
