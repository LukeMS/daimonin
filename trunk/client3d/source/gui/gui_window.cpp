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

#include <Ogre.h>
#include <tinyxml.h>
#include "define.h"
#include "logger.h"
#include "gui_window.h"
#include "gui_cursor.h"
#include "gui_manager.h"
#include "gui_imageset.h"
#include "gui_element_table.h"
#include "gui_element_textbox.h"
#include "gui_element_listbox.h"
#include "gui_element_button.h"
#include "option.h"
#include "sound.h"
#include "events.h"
#include "resourceloader.h"

using namespace Ogre;

const int MIN_GFX_SIZE = 1 << 2;
int GuiWindow::mMouseDragging = -1;
int GuiWindow::mElementClicked = -1;
GuiElementSlot *GuiWindow::mSlotReference = 0;

//================================================================================================
// Destructor.
//================================================================================================
void GuiWindow::freeRecources()
{
    // Delete the statusbars. WILL BE REPLACED...
    for (std::vector<GuiStatusbar*>::iterator i = mvStatusbar.begin(); i < mvStatusbar.end(); ++i)
        delete (*i);
    mvStatusbar.clear();

    for (std::vector<GuiElement*>::iterator i = mvElement.begin(); i < mvElement.end(); ++i)
        delete (*i);
    mvElement.clear();
    delete[] mWinLayerBG;
    mTexture.setNull();
    mInit = false;
}

//================================================================================================
// Build a window out of a xml description file.
//================================================================================================
void GuiWindow::Init(TiXmlElement *xmlElem, const char *resourceWin, const char *resourceDnD, int winNr, unsigned char defaultZPos)
{
    mOverlay = 0;
    mWinLayerBG = 0;
    mElementDrag  =-1;
    mSumUsedSlots= 0;
    mMouseOver     =-1;
    mHeight = MIN_GFX_SIZE;
    mWidth  = MIN_GFX_SIZE;
    mWindowNr = winNr;
    mResourceName = resourceWin;
    mResourceName+= "#" + StringConverter::toString(mWindowNr, GuiManager::SUM_WIN_DIGITS, '0');
    mSrcPixelBox = GuiImageset::getSingleton().getPixelBox();
    mInit = true;
    parseWindowData(xmlElem, resourceDnD, defaultZPos);
}

//================================================================================================
// Parse the xml window data..
//================================================================================================
void GuiWindow::parseWindowData(TiXmlElement *xmlRoot, const char *resourceDnD, unsigned char defaultZPos)
{
    TiXmlElement *xmlElem;
    const char *strTmp;
    int screenH = GuiManager::getSingleton().getScreenHeight();
    int screenW = GuiManager::getSingleton().getScreenWidth();
    if ((strTmp = xmlRoot->Attribute("name")))
        Logger::log().info() << "Parsing window: " << strTmp;
    // ////////////////////////////////////////////////////////////////////
    // Parse the Coordinates type.
    // ////////////////////////////////////////////////////////////////////
    mSizeRelative = ((strTmp = xmlRoot->Attribute("relativeCoords")) && !stricmp(strTmp, "true"));
    // ////////////////////////////////////////////////////////////////////
    // Parse the Size entries.
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElem = xmlRoot->FirstChildElement("Size")))
    {
        if ((strTmp = xmlElem->Attribute("width")))  mWidth = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("height"))) mHeight= atoi(strTmp);
        if (mWidth < MIN_GFX_SIZE) mWidth  = MIN_GFX_SIZE;
        if (mWidth > screenW) mWidth = screenW;
        if (mHeight< MIN_GFX_SIZE) mHeight = MIN_GFX_SIZE;
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
            if (!stricmp(strTmp, "center")) aX = 0;
            else if (!stricmp(strTmp, "right"))  aX =-1;
        }
        if ((strTmp = xmlElem->Attribute("y")))
        {
            if (!stricmp(strTmp, "center")) aY = 0;
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
    // Parse the Dragging entry.
    // ////////////////////////////////////////////////////////////////////
    mDragPosX1 = mDragPosX2 = mDragPosY1 = mDragPosY2 = -100;
    if ((xmlElem = xmlRoot->FirstChildElement("DragArea")))
    {
        if ((strTmp = xmlElem->Attribute("x")))      mDragPosX1 = atoi(strTmp);
        if (mDragPosX1 > mWidth) mDragPosX1 = mWidth-1;
        if ((strTmp = xmlElem->Attribute("y")))      mDragPosY1 = atoi(strTmp);
        if (mDragPosY1 > mHeight) mDragPosY1 = mHeight-1;
        if ((strTmp = xmlElem->Attribute("width")))  mDragPosX2 = mDragPosX1 + atoi(strTmp);
        if (mDragPosX2 > mWidth) mDragPosX2 = mWidth;
        if ((strTmp = xmlElem->Attribute("height"))) mDragPosY2 = mDragPosY1 + atoi(strTmp);
        if (mDragPosY2 > mHeight) mDragPosY2 = mHeight;
    }
    // ////////////////////////////////////////////////////////////////////
    // Now we have all data to create the window..
    // ////////////////////////////////////////////////////////////////////
    mWinLayerBG = new uint32[mWidth * mHeight];
    if ((xmlElem = xmlRoot->FirstChildElement("Color")))
    {
        // PixelFormat: ARGB.
        uint32 color;
        if ((strTmp = xmlElem->Attribute("red"  ))) color = atoi(strTmp) << 16;
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
    mTexture = TextureManager::getSingleton().createManual(mResourceName + GuiManager::TEXTURE_RESOURCE_NAME,
               ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, mWidth, mHeight, 0, PF_A8R8G8B8,
               TU_STATIC_WRITE_ONLY, ManResourceLoader::getSingleton().getLoader());
    mTexture->load();
    mOverlay->hide();
    mElement->setPosition(mPosX, mPosY);
    setZPos(defaultZPos);
    // ////////////////////////////////////////////////////////////////////
    // Parse the child elements.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Graphic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Graphic"))
    {
        GuiElement *gfx = new GuiElement(xmlElem, this);
        gfx->draw();
        delete gfx;
    }
    for (xmlElem = xmlRoot->FirstChildElement("TextBox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextBox"))
    {
        int index = GuiManager::getSingleton().getElementIndex(xmlElem->Attribute("name"));
        GuiElement *element = new GuiElementTextbox(xmlElem, this);
        if (index <0)
            delete element;
        else
            mvElement.push_back(element);
    }
    for (xmlElem = xmlRoot->FirstChildElement("Table"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Table"))
    {
        if (!(strTmp = xmlElem->Attribute("name"))) continue;
        mvElement.push_back(new GuiTable(xmlElem, this));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Listbox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Listbox"))
    {
        if (!(strTmp = xmlElem->Attribute("name"))) continue;
        mvElement.push_back(new GuiListbox(xmlElem, this));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Button"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Button"))
    {
        if (!(strTmp = xmlElem->Attribute("name"))) continue;
        mvElement.push_back(new GuiElementButton(xmlElem, this, true));
    }
    for (xmlElem = xmlRoot->FirstChildElement("Slot"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Slot"))
    {
        String resName = mResourceName+"_"; resName+= resourceDnD;
        GuiElementSlot *slot = new GuiElementSlot(xmlElem, this, resName.c_str());
        if (!mSlotReference) mSlotReference = slot;
        mvElement.push_back(slot);
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Statusbars.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Statusbar"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Statusbar"))
    {
        if (!(strTmp = xmlElem->Attribute("image_name"))) continue;
        mvStatusbar.push_back(new GuiStatusbar(xmlElem, this));
    }
}

//================================================================================================
// (Re)loads the material and texture or creates them if they dont exist.
//================================================================================================
void GuiWindow::loadResources(bool slot)
{
    if (slot)
    {
        mSlotReference->loadResources();
        return;
    }
    mOverlay = GuiManager::getSingleton().loadResources(mWidth, mHeight, mResourceName);
    if (mOverlay)
        mElement = mOverlay->getChild(mResourceName + GuiManager::ELEMENT_RESOURCE_NAME);
    else
        Logger::log().error() << "Critical: Error on creating the overlay.";
}

//================================================================================================
// Check if an interavtive element overlaps another to prevent strange behavior of the gui like
// ignoring a button or activate 2 buttons at the same time.
//================================================================================================
void checkForOverlappingElements()
{
    // Todo
    //Logger::log().warning() << "Element " << xyz << " opverlaps element " << xyz2;
}

//================================================================================================
// Centres the window on the mousecursor.
//================================================================================================
void GuiWindow::centerWindowOnMouse(int x, int y)
{
    if (!mInit) return;
    mPosX = (int)(x-mTexture->getWidth())/2;
    mPosY = (int)(y-mTexture->getHeight())/2 - 50;
    mElement->setPosition(mPosX, mPosY);
}

//================================================================================================
// Key event.
//================================================================================================
int GuiWindow::keyEvent(const int keyChar, const unsigned int key)
{
    if (!mInit || !mOverlay->isVisible()) return GuiManager::EVENT_CHECK_NEXT;
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
int GuiWindow::mouseEvent(int MouseAction, Vector3 &mouse)
{
    if (!mInit || !mOverlay->isVisible()) return GuiManager::EVENT_CHECK_NEXT;
    // ////////////////////////////////////////////////////////////////////
    // user is moving the window.
    // ////////////////////////////////////////////////////////////////////
    if (mMouseDragging >= 0)
    {
        if (mMouseDragging != mWindowNr) // User moves another window.
            return GuiManager::EVENT_CHECK_NEXT;
        if (MouseAction == GuiManager::BUTTON_RELEASED)
        {
            //GuiCursor::getSingleton().setState(GuiImageset::STATE_MOUSE_DEFAULT);
            mMouseDragging= -1;
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (MouseAction == GuiManager::MOUSE_MOVEMENT)
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
    if ((int)mouse.x < mPosX || (int)mouse.x > mPosX + mWidth || (int)mouse.y < mPosY || (int)mouse.y > mPosY + mHeight)
        return GuiManager::EVENT_CHECK_NEXT;
    // ////////////////////////////////////////////////////////////////////
    // Is the mouse within the "drag to move window" area?
    // ////////////////////////////////////////////////////////////////////
    int x = (int)mouse.x - mPosX;
    int y = (int)mouse.y - mPosY;
    if (MouseAction == GuiManager::BUTTON_PRESSED)
    {
        GuiManager::getSingleton().windowToFront(mWindowNr);
        if (x > mDragPosX1 && x < mDragPosX2 && y > mDragPosY1 && y < mDragPosY2)
        {
            //GuiCursor::getSingleton().setState(GuiImageset::STATE_MOUSE_PUSHED);
            mDragOffsetX = x;
            mDragOffsetY = y;
            mMouseDragging = mWindowNr;
            return GuiManager::EVENT_CHECK_DONE;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Look for a mouse event in a child element.
    // ////////////////////////////////////////////////////////////////////
    for (unsigned int i = 0; i < mvElement.size(); ++i)
    {
        int event = mvElement[i]->mouseEvent(MouseAction, x, y, (int)mouse.z);
        if (event != GuiManager::EVENT_CHECK_NEXT)
        {
            if (event == GuiManager::EVENT_USER_ACTION)
            {
                mElementClicked = mvElement[i]->getIndex();
                if (mElementClicked == GuiManager::GUI_BUTTON_CLOSE)
                {
                    setVisible(false);
                    return GuiManager::EVENT_CHECK_DONE;
                }
            }
            if (event == GuiManager::EVENT_DRAG_STRT)
                mElementDrag = i;
            return event;
        }
    }
    return GuiManager::EVENT_CHECK_DONE;
}

//================================================================================================
// Show/hide the window.
//================================================================================================
void GuiWindow::setVisible(bool visible)
{
    if (!mInit) return;
    if (!visible) mOverlay->hide(); else mOverlay->show();
}

//================================================================================================
// Change the value of the statusbar.
//================================================================================================
void GuiWindow::setStatusbarValue(int element, Real value)
{
    for (unsigned int i = 0; i < mvStatusbar.size() ; ++i)
    {
        if (mvStatusbar[i]->getIndex() == element)
            mvStatusbar[i]->setValue(value);
    }
}

//================================================================================================
// Change the height of this window.
//================================================================================================
void GuiWindow::setHeight(int newHeight)
{
    if (mHeight == newHeight) return;
    if (newHeight < mDragPosY2)
        newHeight = mDragPosY2;
    if (newHeight > (int)mTexture->getHeight())
        newHeight = (int)mTexture->getHeight();
    mElement->setHeight(newHeight);
}

//================================================================================================
// Update the window.
//================================================================================================
void GuiWindow::update(Real timeSinceLastFrame)
{
    if (!mInit || !mOverlay->isVisible()) return;
    // ToDo. Update drag animation (move back on wrong drag).
    for (unsigned int i = 0; i < mvElement.size(); ++i)
        mvElement[i]->update(timeSinceLastFrame);
}

//================================================================================================
//
//================================================================================================
int GuiWindow::sendMsg(int elementNr, int message, const char *text, Ogre::uint32 param)
{
    return mvElement[elementNr]->sendMsg(message, text, param);
}

//================================================================================================
//
//================================================================================================
const char *GuiWindow::getInfo(int elementNr, int info)
{
    return mvElement[elementNr]->getInfo(info);
}
