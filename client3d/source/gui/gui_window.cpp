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
#include "gui_window_dialog.h"
#include "option.h"
#include "sound.h"
#include "events.h"
#include "resourceloader.h"

using namespace Ogre;

const int MIN_GFX_SIZE = 1 << 2;
int GuiWindow::mMouseDragging = -1;
String GuiWindow::mStrTooltip ="";

//================================================================================================
// Destructor.
//================================================================================================
void GuiWindow::freeRecources()
{
    // Delete the slots.
    for (std::vector<GuiGadgetSlot*>::iterator i = mvSlot.begin(); i < mvSlot.end(); ++i)
        delete (*i);
    mvSlot.clear();
    // Delete the buttons.
    for (std::vector<GuiGadgetButton*>::iterator i = mvGadgetButton.begin(); i < mvGadgetButton.end(); ++i)
        delete (*i);
    mvGadgetButton.clear();
    // Delete the comboboxes.
    for (std::vector<GuiGadgetCombobox*>::iterator i = mvGadgetCombobox.begin(); i < mvGadgetCombobox.end(); ++i)
        delete (*i);
    mvGadgetCombobox.clear();
    // Delete the listboxes.
    for (std::vector<GuiListbox*>::iterator i = mvListbox.begin(); i < mvListbox.end(); ++i)
        delete (*i);
    mvListbox.clear();
    // Delete the graphics.
    for (std::vector<GuiElement*>::iterator i = mvGraphic.begin(); i < mvGraphic.end(); ++i)
        delete (*i);
    mvGraphic.clear();
    // Delete the statusbars.
    for (std::vector<GuiStatusbar*>::iterator i = mvStatusbar.begin(); i < mvStatusbar.end(); ++i)
        delete (*i);
    mvStatusbar.clear();
    // Delete the tables.
    for (std::vector<GuiTable*>::iterator i = mvTable.begin(); i < mvTable.end(); ++i)
        delete (*i);
    mvTable.clear();
    // Delete the textlines.
    for (std::vector<GuiTextout::TextLine*>::iterator i = mvTextline.begin(); i < mvTextline.end(); ++i)
    {
        if ((*i)->index >= 0) delete[] (*i)->LayerWindowBG;
        delete (*i);
    }
    mvTextline.clear();
    // Set all shared pointer to null.
    delete[] mWinLayerBG;
    mTexture.setNull();
}

//================================================================================================
// Build a window out of a xml description file.
//================================================================================================
void GuiWindow::Init(TiXmlElement *xmlElem, const char *resourceWin, const char *resourceDnD, int winNr, unsigned char defaultZPos)
{
    mOverlay = 0;
    mWinLayerBG = 0;
    mGadgetDrag  =-1;
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
    // Parse the Dragging entries.
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
    // Parse the Tooltip entries.
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElem = xmlRoot->FirstChildElement("Tooltip")))
    { // We will show tooltip only if mouse is over the moving area.
        if ((strTmp = xmlElem->Attribute("text"))) mStrTooltip = strTmp;
    }
    // ////////////////////////////////////////////////////////////////////
    // Now we have all data to create the window..
    // ////////////////////////////////////////////////////////////////////
    mWinLayerBG = new uint32[mWidth * mHeight];
    memset(mWinLayerBG, 0x00, mWidth * mHeight * sizeof(uint32));
    mTexture = TextureManager::getSingleton().createManual(mResourceName + GuiManager::TEXTURE_RESOURCE_NAME,
               ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, mWidth, mHeight, 0, PF_A8R8G8B8,
               TU_STATIC_WRITE_ONLY, ManResourceLoader::getSingleton().getLoader());
    mTexture->load();
    mOverlay->hide();
    mElement->setPosition(mPosX, mPosY);
    setZPos(defaultZPos);



    // ////////////////////////////////////////////////////////////////////
    // Parse the graphics.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Graphic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Graphic"))
    {
        GuiElement *gfx = new GuiElement(xmlElem, this);
        gfx->draw();
        if (gfx->getIndex() == GuiElement::BACKGROUND_GFX_ID)
            delete gfx;
        else
            mvGraphic.push_back(gfx);
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Label.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Label"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Label"))
    {
        printParsedTextline(xmlElem);
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Textbox.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("TextBox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextBox"))
    {
        // Error: No name found. Fallback to label.
        if (!(strTmp = xmlElem->Attribute("name")))
        {
            Logger::log().warning() << "A Textbox without a name was found. Will be handled as static text.";
            printParsedTextline(xmlElem);
            continue;
        }
        String strName = strTmp;
        int index = -1;
        for (int i = 0; i < GuiImageset::GUI_ELEMENTS_SUM; ++i)
        {
            if (!stricmp(GuiImageset::getSingleton().getElementName(i), strTmp))
            {
                index = GuiImageset::getSingleton().getElementIndex(i);
                break;
            }
        }
        if (index <0)
        {
            Logger::log().warning() << "TextBox name " << strTmp << " is unknown. Will be handled as static text.";
            printParsedTextline(xmlElem);
            continue;
        }
        GuiTextout::TextLine *textline = new GuiTextout::TextLine;
        textline->index = index;
        textline->hideText= false;
        textline->color = 0x00ffffff;
        if ((strTmp = xmlElem->Attribute("font")))  textline->font = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("x")))     textline->x1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("y")))     textline->y1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("width"))) textline->x2   = atoi(strTmp) + textline->x1;
        textline->y2 = textline->y1 + GuiTextout::getSingleton().getFontHeight(textline->font);
        if (textline->x1 > (unsigned int) mWidth)   textline->x1 = 0; // If pos is out of window set it to leftmost pos.
        if (textline->x2 > (unsigned int) mWidth)   textline->x2 = mWidth-1;
        if (textline->y1 > (unsigned int) mHeight)  textline->y1 = 0; // If pos is out of window set it to topmost pos.
        if (textline->y2 > (unsigned int) mHeight)  textline->y1 = mHeight-1;
        if ((strTmp = xmlElem->Attribute("text")))   textline->text = strTmp;
        if ((strTmp = xmlElem->Attribute("hide")))   textline->hideText= (atoi(strTmp)==1);

        // Fill the LayerWindowBG buffer with Window background, before printing.
        mvTextline.push_back(textline);
        textline->LayerWindowBG = new uint32[(textline->x2- textline->x1) * (textline->y2- textline->y1)];
        mTexture.getPointer()->getBuffer()->blitToMemory(Box(
                    textline->x1, textline->y1,
                    textline->x2, textline->y2),
                PixelBox(
                    textline->x2- textline->x1,
                    textline->y2- textline->y1,
                    1, PF_A8R8G8B8, textline->LayerWindowBG));
        GuiTextout::getSingleton().Print(textline, mTexture.getPointer());
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the listboxes.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Listbox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Listbox"))
    {
        if (!(strTmp = xmlElem->Attribute("name"))) continue;
        GuiListbox *listbox = new GuiListbox(xmlElem, this);
        listbox->setFunction(this->listboxPressed);
        mvListbox.push_back(listbox);
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the tables.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Table"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Table"))
    {
        if (!(strTmp = xmlElem->Attribute("name"))) continue;
        mvTable.push_back(new GuiTable(xmlElem, this));
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Statusbars.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Statusbar"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Statusbar"))
    {
        if (!(strTmp = xmlElem->Attribute("image_name"))) continue;
        mvStatusbar.push_back(new GuiStatusbar(xmlElem, this));
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the gadgets.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Gadget"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Gadget"))
    {
        if ( !strcmp(xmlElem->Attribute("type"), "BUTTON"))
        {
            GuiGadgetButton *button = new GuiGadgetButton(xmlElem, this);
            button->setFunction(this->buttonPressed);
            button->draw();
            mvGadgetButton.push_back(button);
        }
        else if ( !strcmp(xmlElem->Attribute("type"), "SLOT"))
        {
            String resName = mResourceName+"_"; resName+= resourceDnD;
            mvSlot.push_back(new GuiGadgetSlot(xmlElem, this, resName.c_str()));
        }
        else if ( !strcmp(xmlElem->Attribute("type"), "COMBOBOX"))
        {
            GuiGadgetCombobox *combobox = new GuiGadgetCombobox(xmlElem, this);
            mvGadgetCombobox.push_back(combobox);
        }
        else
            Logger::log().warning() << xmlElem->Attribute("type") << " is not a defined gadget type.";
    }
}

//================================================================================================
// (Re)loads the material and texture or creates them if they dont exist.
//================================================================================================
void GuiWindow::loadResources()
{
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
// Parse and print a text (label element).
//================================================================================================
inline void GuiWindow::printParsedTextline(TiXmlElement *xmlElem)
{
    const char *strTmp;
    GuiTextout::TextLine textline;
    textline.index = -1;
    textline.hideText= false;
    textline.LayerWindowBG = 0;
    textline.color = 0x00ffffff;
    if ((strTmp = xmlElem->Attribute("x")))    textline.x1   = atoi(strTmp);
    if ((strTmp = xmlElem->Attribute("y")))    textline.y1   = atoi(strTmp);
    if ((strTmp = xmlElem->Attribute("font"))) textline.font = atoi(strTmp);
    if ((strTmp = xmlElem->Attribute("text"))) textline.text = strTmp;
    if (textline.x1 > (unsigned int) mWidth)  textline.x1 = 0; // If pos is out of window set it to leftmost pos.
    if (textline.y1 > (unsigned int) mHeight) textline.y1 = 0; // If pos is out of window set it to topmost pos.
    textline.x2 = mWidth - textline.x1;
    textline.y2 = textline.y1 + GuiTextout::getSingleton().getFontHeight(textline.font);
    GuiTextout::getSingleton().Print(&textline, mTexture.getPointer());
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
bool GuiWindow::keyEvent(const char keyChar, const unsigned char key)
{
    for (unsigned int i = 0; i < mvTable.size(); ++i)
    {
        if (mvTable[i]->keyEvent(keyChar, key))
            return true;
    }
    return false;
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
        if (MouseAction == BUTTON_RELEASED)
        {
            //GuiCursor::getSingleton().setState(GuiImageset::STATE_MOUSE_DEFAULT);
            mMouseDragging= -1;
            return GuiManager::EVENT_CHECK_DONE;
        }
        if (MouseAction == MOUSE_MOVEMENT)
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
    if (x > mDragPosX1 && x < mDragPosX2 && y > mDragPosY1 && y < mDragPosY2 && MouseAction == BUTTON_PRESSED)
    {
        //GuiCursor::getSingleton().setState(GuiImageset::STATE_MOUSE_PUSHED);
        GuiManager::getSingleton().windowToFront(mWindowNr);
        mDragOffsetX = x;
        mDragOffsetY = y;
        mMouseDragging = mWindowNr;
        return GuiManager::EVENT_CHECK_DONE;
    }
    // ////////////////////////////////////////////////////////////////////
    // Look for a mouse event in a child element.
    // ////////////////////////////////////////////////////////////////////
    int sumPressed = 0;
    for (unsigned int i = 0; i < mvGadgetButton.size(); ++i)
    {   // We dont return on a true return value - to avoid 2 buttons selected at same time..
        // This will still happen when their gfx is overlapping.
        if (mvGadgetButton[i]->mouseEvent(MouseAction, x, y)) ++sumPressed;
    }
    if (sumPressed) return GuiManager::EVENT_CHECK_DONE;

    for (unsigned int i = 0; i < mvSlot.size(); ++i)
    {
        if (mvSlot[i]->mouseEvent(MouseAction, x, y) == GuiManager::EVENT_DRAG_STRT)
        {
            mGadgetDrag = i;
            return GuiManager::EVENT_DRAG_STRT;
        }
    }
    for (unsigned int i = 0; i < mvGadgetCombobox.size(); ++i)
    {
//        if (mvGadgetCombobox[i]->mouseEvent(MouseAction, x, y))
//            return true;
    }
    for (unsigned int i = 0; i < mvListbox.size(); ++i)
    {
        if (mvListbox[i]->mouseEvent(MouseAction, x, y, (int) mouse.z))
            return GuiManager::EVENT_CHECK_DONE;
    }
    for (unsigned int i = 0; i < mvTable.size(); ++i)
    {
        if (mvTable[i]->mouseEvent(MouseAction, x, y))
            return GuiManager::EVENT_CHECK_DONE;
    }
    return GuiManager::EVENT_CHECK_DONE;
}

//================================================================================================
// Returns the buttonhandle, or 0 if the button was not found.
//================================================================================================
class GuiGadgetButton *GuiWindow::getButtonHandle(int element)
{
    for (unsigned int i = 0; i < mvGadgetButton.size() ; ++i)
    {
        if (mvGadgetButton[i]->getIndex() == element)
            return mvGadgetButton[i];
    }
    return 0;
}

//================================================================================================
// Add a row to the table.
//================================================================================================
void GuiWindow::addTableRow(int element, const char *text)
{
    for (unsigned int i = 0; i < mvTable.size() ; ++i)
    {
        if (mvTable[i]->getIndex() == element)
        {
            mvTable[i]->addRow(text);
            return;
        }
    }
}

//================================================================================================
// Returns the selected element of a table, or -1 when no entry was selected.
//================================================================================================
int GuiWindow::getTableSelection(int element)
{
    for (unsigned int i = 0; i < mvTable.size() ; ++i)
    {
        if (mvTable[i]->getIndex() == element)
            return mvTable[i]->getSelectedRow();
    }
    return -1;
}

//================================================================================================
// Returns the activated element (dblclk or return) of a table, or -1 when no entry was activeted.
//================================================================================================
int GuiWindow::getTableActivated(int element)
{
    for (unsigned int i = 0; i < mvTable.size() ; ++i)
    {
        if (mvTable[i]->getIndex() == element)
            return mvTable[i]->getActivatedRow();
    }
    return -1;
}

//================================================================================================
// User pressed ESC.
//================================================================================================
bool GuiWindow::getTableUserBreak(int element)
{
    for (unsigned int i = 0; i < mvTable.size() ; ++i)
    {
        if (mvTable[i]->getIndex() == element)
            return mvTable[i]->getUserBreak();
    }
    return -1;
}

//================================================================================================
// Clear the whole table..
//================================================================================================
void GuiWindow::clearTable(int element)
{
    for (unsigned int i = 0; i < mvTable.size() ; ++i)
    {
        if (mvTable[i]->getIndex() == element)
        {
            mvTable[i]->clear();
            return;
        }
    }
}

//================================================================================================
// Sets the time for a slot to be busy.
//================================================================================================
void GuiWindow::setSlotBusyTime(int element, Real busyTime)
{
    if (element < (int)mvSlot.size())
        mvSlot[element]->setBusyTime(busyTime);
}

//================================================================================================
// Set the slot to busy.
//================================================================================================
void GuiWindow::setSlotBusy(int element)
{
    if (element < (int)mvSlot.size())
        mvSlot[element]->setBusy();
}

//================================================================================================
// Add a textline to a listbox.
//================================================================================================
int GuiWindow::addTextline(int element, const char *text, uint32 color)
{
    for (unsigned int i = 0; i < mvListbox.size() ; ++i)
    {
        if (mvListbox[i]->getIndex() == element)
            return mvListbox[i]->addTextline(text, color);
    }
    return 0;
}

//================================================================================================
// Clear the whole list.
//================================================================================================
void GuiWindow::clearListbox(int element)
{
    for (unsigned int i = 0; i < mvListbox.size() ; ++i)
    {
        if (mvListbox[i]->getIndex() == element)
        {
            mvListbox[i]->clear();
            return ;
        }
    }
}

//================================================================================================
// Returns the slot number, or -1 if the mouse is not within a slot.
//================================================================================================
int GuiWindow::getMouseOverSlot(int x, int y)
{
    for (unsigned int i = 0; i < mvSlot.size(); ++i)
    {
        if (mvSlot[i]->mouseWithin(x - mPosX, y - mPosY))
            return i;
    }
    return -1;
}

//================================================================================================
// Adds an item to the slots.
//================================================================================================
void GuiWindow::addItem(Item::sItem *item)
{
    if (mSumUsedSlots < mvSlot.size())
        mvSlot[mSumUsedSlots++]->setItem(item);
}

//================================================================================================
// Delete an item from a slot.
//================================================================================================
void GuiWindow::delItem(Item::sItem *item)
{
    for (unsigned int i = 0; i < mSumUsedSlots; ++i)
    {
        if (item == mvSlot[i]->getItem())
        {
            if (i < mSumUsedSlots-1)
            {
                // Found the item that will be deleted.
                for (; i < mSumUsedSlots; ++i)
                    mvSlot[i]->setItem(mvSlot[i+1]->getItem());
            }
            mvSlot[--mSumUsedSlots]->setItem(0);
            return;
        }
    }
    Logger::log().error() << "GuiWindow::delItem(): cant find item " << item->d_name;
}

//================================================================================================
// Set all slots to empty.
//================================================================================================
void GuiWindow::clrItem()
{
    for (unsigned int i = 0; i < mvSlot.size(); ++i)
        mvSlot[i]->setItem(0);
    mSumUsedSlots = 0;
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
// Show/hide a child element.
//================================================================================================
void GuiWindow::setVisible(int element, bool visible)
{
    for (unsigned int i = 0; i < mvGadgetButton.size() ; ++i)
    {
        if (mvGadgetButton[i]->getIndex() == element)
            mvGadgetButton[i]->setVisible(visible);
    }
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
// Return the text of a gui element.
//================================================================================================
const String &GuiWindow::getElementText(int element)
{
    for (unsigned int i = 0; i < mvTextline.size() ; ++i)
    {
        if (mvTextline[i]->index == element)
            return mvTextline[i]->text;
    }
    for (unsigned int i = 0; i < mvGadgetCombobox.size() ; ++i)
    {
        //        if (mvGadgetCombobox[i]->getIndex() == element)
        //            return mvGadgetCombobox[i]->getText();
    }
    // Only reached if a false elemnt was given as parameter.
    // To make the compiler happy, we give back a "random" string.
    return mStrTooltip;
}

//================================================================================================
// Set the text of a gui element.
//================================================================================================
void GuiWindow::setElementText(int element, const char *text)
{
    for (unsigned int i = 0; i < mvTextline.size() ; ++i)
    {
        if (mvTextline[i]->index == element)
        {
            mvTextline[i]->text = text;
            GuiTextout::getSingleton().Print(mvTextline[i], mTexture.getPointer());
            return;
        }
    }
    for (unsigned int i = 0; i < mvGadgetCombobox.size() ; ++i)
    {
        if (mvGadgetCombobox[i]->getIndex() == element)
        {
            mvGadgetCombobox[i]->setText(text);
            mvGadgetCombobox[i]->draw();
            return;
        }
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
    for (unsigned int i = 0; i < mvSlot.size(); ++i)
        mvSlot[i]->update(timeSinceLastFrame);
    for (unsigned int i = 0; i < mvListbox.size(); ++i)
        mvListbox[i]->update(timeSinceLastFrame);
}

//================================================================================================
// Button event inside a listbox.
//================================================================================================
void GuiWindow::listboxPressed(GuiWindow *me, int index, int line)
{
    Sound::getSingleton().playStream(Sound::BUTTON_CLICK);
    switch (index)
    {
        case GuiImageset::GUI_LIST_NPC:
            GuiDialog::getSingleton().mouseEvent(line);
            return;
    }
}

//================================================================================================
// Button event.
//================================================================================================
void GuiWindow::buttonPressed(GuiWindow *me, int index)
{
    Sound::getSingleton().playStream(Sound::BUTTON_CLICK);
    switch (index)
    {
            // Standard buttons.
        case GuiImageset::GUI_BUTTON_CLOSE:
            me->setVisible(false);
            return;
            // Unique buttons.
        case GuiImageset::GUI_BUTTON_NPC_ACCEPT:
            GuiDialog::getSingleton().buttonEvent(0);
            return;
        case GuiImageset::GUI_BUTTON_NPC_DECLINE:
            GuiDialog::getSingleton().buttonEvent(1);
            return;
    }
    // Not yet supported elements...
    GuiManager::getSingleton().addTextline(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "button event from <anonymous> element.");
}
