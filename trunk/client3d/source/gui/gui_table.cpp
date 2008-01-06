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
#include <OISKeyboard.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "logger.h"
#include "gui_table.h"
#include "gui_window.h"

using namespace Ogre;

static const unsigned long SCROLL_SPEED = 12;
static const Real CLOSING_SPEED  = 10.0f;  // default: 10.0f

//================================================================================================
// Constructor.
//================================================================================================
GuiTable::GuiTable(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    uint32 *color = 0;
    const char *tmp;
    TiXmlElement *xmlOpt;
    xmlOpt = xmlElement->FirstChildElement("Range");
    if ((tmp = xmlOpt->Attribute("rows"  ))) mSumRows = atoi(tmp);
    for (xmlOpt = xmlElement->FirstChildElement("Color"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Color"))
    {
        if      (!strcmp(xmlOpt->Attribute("type"), "BACK_1")) color = &mColorBack[0];
        else if (!strcmp(xmlOpt->Attribute("type"), "BACK_2")) color = &mColorBack[1];
        else if (!strcmp(xmlOpt->Attribute("type"), "SELECT")) color = &mColorSelect;
        if ((tmp = xmlOpt->Attribute("red"  ))) *color = atoi(tmp) << 16;
        if ((tmp = xmlOpt->Attribute("green"))) *color+= atoi(tmp) <<  8;
        if ((tmp = xmlOpt->Attribute("blue" ))) *color+= atoi(tmp);
        if ((tmp = xmlOpt->Attribute("alpha"))) *color+= atoi(tmp) << 24;
    }
    for (xmlOpt = xmlElement->FirstChildElement("Column"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Column"))
    {
        TableEntry *entry = new TableEntry;
        if ((tmp = xmlOpt->Attribute("width"  ))) entry->width = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("label"  ))) entry->label = tmp;
        mvColumn.push_back(entry );
    }
    mFontHeight = GuiTextout::getSingleton().getFontHeight(mFontNr);
    mSumRows = mHeight / mFontHeight;
    mHeight = mSumRows * mFontHeight;
    --mSumRows; // Reserve space for the headlines.
    mSelectedRow = -1;
    mRowActivated = false;
    mRowChanged = true;
    mUserBreak = false;
    draw();
}

//================================================================================================
// Destructor.
//================================================================================================
GuiTable::~GuiTable()
{
    for (std::vector<TableEntry*>::iterator i = mvColumn.begin(); i < mvColumn.end(); ++i)
    {
        delete (*i);
    }
    mvColumn.clear();
    mvRow.clear();
}

//================================================================================================
// Returns true if the key event was on this gadget (so no need to check the other gadgets).
//================================================================================================
bool GuiTable::keyEvent(const char keyChar, const unsigned char key)
{
    if (key == OIS::KC_UP)
    {
        if (mSelectedRow <= 0)  return true;
        drawSelection(mSelectedRow-1);
        mRowChanged = true;
        return true;
    }
    if (key == OIS::KC_DOWN)
    {
        if (mSelectedRow+1 >= (int)mvRow.size())  return true;
        drawSelection(mSelectedRow+1);
        mRowChanged = true;
        return true;
    }
    if (key == OIS::KC_RETURN) // || key == OIS::KC_NUMPADENTER)
    {
        mRowActivated = true;
        return true;
    }
    if (key == OIS::KC_ESCAPE)
    {
        mUserBreak = true;
        return true;
    }
    return false;
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
bool GuiTable::mouseEvent(int MouseAction, int x, int y)
{
    if (x >= mPosX && x <= mPosX + mWidth && y >= mPosY && y <= mPosY + mHeight)
    {
        static unsigned long time =0;
        int row = (y-mPosY) / mFontHeight -1;
        if (MouseAction == GuiWindow::BUTTON_PRESSED && row >=0 && row < (int)mvRow.size())
        {
            if (mSelectedRow == row)
            {
                if (Root::getSingleton().getTimer()->getMilliseconds()- time < GuiWindow::TIME_DOUBLECLICK)
                    mRowActivated = true;
                else
                    time = Root::getSingleton().getTimer()->getMilliseconds();
            }
            else
            {
                drawSelection(row);
                time = Root::getSingleton().getTimer()->getMilliseconds();
            }
        }
        return true;
    }
    return false;
}

//================================================================================================
// Add a row to the table. Each col is separated by ','.
//================================================================================================
void GuiTable::addRow(String textline)
{
    mvRow.push_back(textline+","); // Add the end of col sign (the comma) to the end of the text.
    drawSelection((int)mvRow.size()-1);
}

//================================================================================================
// Clear all rows.
//================================================================================================
void GuiTable::clearRows()
{
    mvRow.clear();
    mSelectedRow = -1;
    draw();
}

//================================================================================================
// After a row was activated (by dblclick or return key) the row is returned once.
// return value of -1 means no user action was reported.
//================================================================================================
int GuiTable::getActivatedRow()
{
    if (!mRowActivated)
        return -1;
    mRowActivated = false;
    return mSelectedRow;
}

//================================================================================================
// Was a user break detected?
//================================================================================================
bool GuiTable::getUserBreak()
{
    if (!mUserBreak)
        return false;
    mUserBreak = false;
    return true;
}

//================================================================================================
// After a row selection changed, the row is returned once.
// return value of -1 means no user action was reported.
//================================================================================================
int GuiTable::getSelectedRow()
{
    if (!mRowChanged)
        return -1;
    mRowChanged = false;
    return mSelectedRow;
}

//================================================================================================
// Draws the Headlines and Background of the table.
//================================================================================================
void GuiTable::draw()
{
    Texture *texture = mParent->getTexture();
    // Draw the column headlines.
    GuiTextout::TextLine textline;
    textline.index = -1;
    textline.hideText= false;
    textline.LayerWindowBG = 0;
    textline.color =0x00ffffff;
    textline.font = mFontNr;
    textline.x1 = mPosX;
    textline.x2 = textline.x1 + mWidth;
    textline.y1 = mPosY;
    textline.y2 = textline.y1 + mFontHeight;
    for (std::vector<TableEntry*>::iterator i = mvColumn.begin(); i < mvColumn.end(); ++i)
    {
        if ((*i)->label!="")
        {
            textline.text = (*i)->label;
            GuiTextout::getSingleton().Print(&textline, texture);
        }
        textline.x1 += (*i)->width;
    }
    // Draw the line background.
    PixelBox pb = texture->getBuffer()->lock (Box(mPosX, mPosY+mFontHeight, mPosX+mWidth, mPosY+mHeight), HardwareBuffer::HBL_DISCARD);
    uint32 *dest_data = (uint32*)pb.data;
    for (int y = 0; y < mSumRows; ++y)
    {
        for (int line =0; line < mFontHeight; ++line)
        {
            for (int x = 0; x < mWidth; ++x)
            {
                // if (mSelectedRow == y)
                //dest_data[x] = mColorSelect;
                //else
                dest_data[x] = mColorBack[y&1];
            }
            dest_data+=texture->getWidth();
        }
    }
    texture->getBuffer()->unlock();
}

//================================================================================================
// Restore the background of the selected row and draw the selection bar to the new selected row.
//================================================================================================
void GuiTable::drawSelection(int newSelection)
{
    // Restore selection background
    drawRow(mSelectedRow, mColorBack[mSelectedRow&1]);
    // Draw new selection bar.
    drawRow(newSelection, mColorSelect);
    mSelectedRow = newSelection;
}

//================================================================================================
// .
//================================================================================================
void GuiTable::drawRow(int row, uint32 color)
{
    if (row < 0) return;
    Texture *texture = mParent->getTexture();
    GuiTextout::TextLine textline;
    textline.index = -1;
    textline.hideText= false;
    textline.LayerWindowBG = 0;
    textline.font = mFontNr;
    textline.color =0x00ffffff;
    textline.x1 = mPosX +3;
    textline.x2 = textline.x1 + mWidth;

    std::string::size_type startPos, endPos;
    int offset = (row+1) * mFontHeight;
    // Draw the background.
    PixelBox pb = texture->getBuffer()->lock (Box(mPosX, mPosY+offset, mPosX+mWidth, mPosY+offset+mFontHeight), HardwareBuffer::HBL_DISCARD);
    uint32 *dest_data = (uint32*)pb.data;
    for (int line =0; line < mFontHeight; ++line)
    {
        for (int x = 0; x < mWidth; ++x)
            dest_data[x] = color;
        dest_data+=texture->getWidth();
    }
    texture->getBuffer()->unlock();
    // Print the text.
    if (mvRow[row] =="") return;
    textline.y1 = mPosY+mFontHeight+2 + row * mFontHeight;
    textline.y2 = textline.y1 + mFontHeight;
    endPos = 0;
    for (std::vector<TableEntry*>::iterator i = mvColumn.begin(); i < mvColumn.end(); ++i)
    {
        startPos = endPos;
        endPos = mvRow[row].find( ',', startPos);
        if (endPos == std::string::npos) break;
        {
            textline.text = mvRow[row].substr(startPos, endPos-startPos);
            GuiTextout::getSingleton().Print(&textline, texture);
        }
        ++endPos;
        textline.x1 += (*i)->width;
    }
}
