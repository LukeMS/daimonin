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
#include "logger.h"
#include "gui_table.h"
#include "gui_window.h"
#include "gui_manager.h"

using namespace Ogre;

const char SEPARATOR_COL    = ',';
const char SEPARATOR_SUBROW = ';';
const int  TEXT_OFFSET = 2; // Text offset in pixel x/y from the topleft border.

//================================================================================================
// Constructor.
//================================================================================================
GuiTable::GuiTable(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    const char *tmp;
    TiXmlElement *xmlOpt;
    for (xmlOpt = xmlElement->FirstChildElement("Color"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Color"))
    {
        uint32 *color = 0;
        if      (!strcmp(xmlOpt->Attribute("type"), "COLOR_ODD_ROWS"))  color = &mColorRowBG[0]; // Color of odd  rows.
        else if (!strcmp(xmlOpt->Attribute("type"), "COLOR_EVEN_ROWS")) color = &mColorRowBG[1]; // Color of even rows.
        else if (!strcmp(xmlOpt->Attribute("type"), "COLOR_SELECTION")) color = &mColorSelect;  // Color of selection.
        if (!color)
            Logger::log().warning() << "Unknown Color type " << xmlOpt->Attribute("type");
        else
        {
            *color = 0x00000000; // default color.
            if ((tmp = xmlOpt->Attribute("alpha"))) *color+= atoi(tmp) << 24;
            if ((tmp = xmlOpt->Attribute("red")))   *color+= atoi(tmp) << 16;
            if ((tmp = xmlOpt->Attribute("green"))) *color+= atoi(tmp) <<  8;
            if ((tmp = xmlOpt->Attribute("blue")))  *color+= atoi(tmp);
        }
    }
    for (xmlOpt = xmlElement->FirstChildElement("Column"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Column"))
    {
        ColumnEntry *entry = new ColumnEntry;
        entry->width = (tmp = xmlOpt->Attribute("width"))?atoi(tmp):mWidth/4;
        entry->label = (tmp = xmlOpt->Attribute("label"))?tmp:"NOT SET";
        mvColumn.push_back(entry);
    }
    for (xmlOpt = xmlElement->FirstChildElement("SubRow"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("SubRow"))
    {
        SubRowEntry *entry = new SubRowEntry;
        entry->fontNr= ((tmp = xmlOpt->Attribute("font")))?atoi(tmp):0;
        entry->color = 0x00000000; // default color.
        if ((tmp = xmlOpt->Attribute("alpha"))) entry->color = atoi(tmp) << 24;
        if ((tmp = xmlOpt->Attribute("red"  ))) entry->color+= atoi(tmp) << 16;
        if ((tmp = xmlOpt->Attribute("green"))) entry->color+= atoi(tmp) <<  8;
        if ((tmp = xmlOpt->Attribute("blue" ))) entry->color+= atoi(tmp);
        mvSubRow.push_back(entry);
    }
    mUserBreak = false;
    mHeightRow = 0;
    for (std::vector<SubRowEntry*>::iterator i = mvSubRow.begin(); i < mvSubRow.end(); ++i)
        mHeightRow+= GuiTextout::getSingleton().getFontHeight((*i)->fontNr) + TEXT_OFFSET;
    mHeightColumnLabel = GuiTextout::getSingleton().getFontHeight(mFontNr);
    clear();
    draw();
}

//================================================================================================
// Destructor.
//================================================================================================
GuiTable::~GuiTable()
{
    for (std::vector<ColumnEntry*>::iterator i = mvColumn.begin(); i < mvColumn.end(); ++i)
        delete(*i);
    mvColumn.clear();
    for (std::vector<SubRowEntry*>::iterator i = mvSubRow.begin(); i < mvSubRow.end(); ++i)
        delete(*i);
    mvSubRow.clear();
    mvRow.clear();
}

//================================================================================================
//
//================================================================================================
int GuiTable::sendMsg(int message, void *parm1, void *parm2, void *parm3)
{
    switch (message)
    {
        case GuiManager::MSG_ADD_ROW:
            addRow((const char*)parm1);
            return 0;
        case GuiManager::MSG_CLEAR:
            clear();
            return 0;
        case GuiManager::MSG_GET_USERBREAK:
            return getUserBreak();
        case GuiManager::MSG_GET_SELECTION:
            return getSelectedRow();
        case GuiManager::MSG_GET_ACTIVATED:
            return getActivatedRow();
        case GuiManager::MSG_GET_KEY_EVENT:
            return keyEvent((const char*)parm1, (const unsigned char*)parm2);
        case GuiManager::MSG_GET_MOUSE_EVENT:
            return mouseEvent((int*)parm1, (int*)parm2, (int*)parm3);
        default:
            return -1;
    }
}

//================================================================================================
// If a user-break was detected return true once.
//================================================================================================
int GuiTable::getUserBreak()
{
    if (!mUserBreak) return 0;
    mUserBreak = false;
    return -1;
}

//================================================================================================
// Returns EVENT_CHECK_DONE if the key event happened here (so no need to check the other gadgets).
//================================================================================================
int GuiTable::keyEvent(const char *keyChar, const unsigned char *key)
{
    if (*key == OIS::KC_UP)
    {
        if (mSelectedRow > 0)
        {
            drawSelection(mSelectedRow-1);
            mSeletedRowChanged = true;
        }
        return GuiManager::EVENT_CHECK_DONE;
    }
    if (*key == OIS::KC_DOWN)
    {
        if (mSelectedRow+1 < (int)mvRow.size())
        {
            drawSelection(mSelectedRow+1);
            mSeletedRowChanged = true;
        }
        return GuiManager::EVENT_CHECK_DONE;
    }
    if (*key == OIS::KC_RETURN) // || key == OIS::KC_NUMPADENTER)
    {
        mRowActivated = true;
        return GuiManager::EVENT_CHECK_DONE;
    }
    if (*key == OIS::KC_ESCAPE)
    {
        mUserBreak = true;
        return GuiManager::EVENT_CHECK_DONE;
    }
    return GuiManager::EVENT_CHECK_NEXT;
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
int GuiTable::mouseEvent(int *MouseAction, int *x, int *y)
{
    if (!mouseWithin(*x,*y))
        return GuiManager::EVENT_CHECK_NEXT;
    int row = (*y-mPosY-mHeightColumnLabel) / mHeightRow;
    if (row <0 || row >= (int)mvRow.size()) return true;

    if (*MouseAction == GuiWindow::BUTTON_RELEASED)
    {
        static unsigned long time =0;
        {
            if (Root::getSingleton().getTimer()->getMilliseconds()- time < GuiWindow::TIME_DOUBLECLICK)
            {
                mRowActivated = true;
                return GuiManager::EVENT_USER_ACTION;
            }
        }
        time = Root::getSingleton().getTimer()->getMilliseconds();
    }
    if (*MouseAction == GuiWindow::BUTTON_PRESSED)
    {
        if (mSelectedRow != row)
        {
            drawSelection(row);
            mSeletedRowChanged = true;
            return GuiManager::EVENT_CHECK_DONE;
        }
    }
    return GuiManager::EVENT_CHECK_DONE;
}

//================================================================================================
// Change an existing row.
//================================================================================================
void GuiTable::setRow(int row, const char *text)
{
    drawRow(row, (row == mSelectedRow)?mColorSelect:mColorRowBG[row&1]);
}

//================================================================================================
// Clear all rows.
//================================================================================================
void GuiTable::clear()
{
    mvRow.clear();
    mSelectedRow = -1;
    mRowActivated = false;
    mSeletedRowChanged = true;
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
// After a row selection changed, the row is returned once.
// return value of -1 means no user action was reported.
//================================================================================================
int GuiTable::getSelectedRow()
{
    if (!mSeletedRowChanged)
        return -1;
    mSeletedRowChanged = false;
    return mSelectedRow;
}

//================================================================================================
// Add a row to the table. Each col is separated by ',' , each subRow is separated by ';'
//================================================================================================
void GuiTable::addRow(const char *row)
{
    mvRow.push_back(row);
    drawSelection((int)mvRow.size()-1);
}
//================================================================================================
// Draw the Headline and Background of the table.
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
    textline.y2 = textline.y1 + mHeightColumnLabel;
    for (std::vector<ColumnEntry*>::iterator i = mvColumn.begin(); i < mvColumn.end(); ++i)
    {
        if ((*i)->label!="")
        {
            textline.text = (*i)->label;
            GuiTextout::getSingleton().Print(&textline, texture);
        }
        textline.x1 += (*i)->width;
    }
    // Draw the line background.
    PixelBox pb = texture->getBuffer()->lock(Box(mPosX, mPosY+mHeightColumnLabel, mPosX+mWidth, mPosY+mHeight), HardwareBuffer::HBL_DISCARD);
    uint32 *dest_data = (uint32*)pb.data;
    int y = 0, h = mHeightColumnLabel;
    while (h < mHeight)
    {
        for (int line =0; line < mHeightRow && h++ < mHeight; ++line)
        {
            for (int x = 0; x < mWidth; ++x)
                dest_data[x] = mColorRowBG[y&1];
            dest_data+=texture->getWidth();
        }
        ++y;
    }
    texture->getBuffer()->unlock();
}

//================================================================================================
// Restore the background of the selected row and draw the selection bar to the new selected row.
//================================================================================================
void GuiTable::drawSelection(int newSelection)
{
    drawRow(mSelectedRow, mColorRowBG[mSelectedRow&1]); // Restore selection background
    drawRow(newSelection, mColorSelect); // Draw new selection bar.
    mSelectedRow = newSelection;
}

//================================================================================================
// Draw a single row.
//================================================================================================
void GuiTable::drawRow(int row, uint32 bgColor)
{
    if (row < 0) return;
    Texture *texture = mParent->getTexture();
    GuiTextout::TextLine textline;
    textline.index = -1;
    textline.hideText= false;
    textline.LayerWindowBG = 0;
    textline.x2 = mPosX + mWidth;
    int offset = mHeightColumnLabel + row * mHeightRow;
    // Draw the background.
    PixelBox pb = texture->getBuffer()->lock(Box(mPosX, mPosY+offset, mPosX+mWidth, mPosY+offset+mHeightRow), HardwareBuffer::HBL_DISCARD);
    uint32 *dest_data = (uint32*)pb.data;
    for (int line =0; line < mHeightRow; ++line)
    {
        for (int x = 0; x < mWidth; ++x)
            dest_data[x] = bgColor;
        dest_data+=texture->getWidth();
    }
    texture->getBuffer()->unlock();
    // Print the text.
    if (!mvRow[row].size()) return;
    textline.y1 = mPosY+mHeightColumnLabel + row * mHeightRow + TEXT_OFFSET;
    std::string::size_type colStart, colEnd, subRowStrt, subRowEnd = 0;
    for (std::vector<SubRowEntry*>::iterator subRow = mvSubRow.begin(); subRow < mvSubRow.end(); ++subRow)
    {
        int fontHeight = GuiTextout::getSingleton().getFontHeight((*subRow)->fontNr);
        textline.x1 = mPosX + TEXT_OFFSET;
        textline.y2 = textline.y1 + fontHeight + TEXT_OFFSET;
        textline.font = (*subRow)->fontNr;
        textline.color= (*subRow)->color;
        subRowStrt = subRowEnd;
        subRowEnd = mvRow[row].find(SEPARATOR_SUBROW, subRowStrt);
        if (subRowEnd == std::string::npos) subRowEnd = mvRow[row].size();
        colEnd = subRowStrt;
        for (std::vector<ColumnEntry*>::iterator col = mvColumn.begin(); col < mvColumn.end(); ++col)
        {
            colStart = colEnd;
            colEnd = mvRow[row].find(SEPARATOR_COL, colStart);
            if (colEnd > subRowEnd || colEnd == std::string::npos)
                colEnd = subRowEnd;
            textline.text = mvRow[row].substr(colStart, colEnd-colStart);
            GuiTextout::getSingleton().Print(&textline, texture);
            if (++colEnd >= subRowEnd) break;
            textline.x1 += (*col)->width;
        }
        if (++subRowEnd >= mvRow[row].size()) break;
        textline.y1+= fontHeight;
    }
}
