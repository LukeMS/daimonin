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
#include "gui_element_table.h"
#include "gui_window.h"
#include "gui_manager.h"
#include "gui_textout.h"

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
        entry->label = (tmp = xmlOpt->Attribute("label"))?tmp:"";
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
int GuiTable::keyEvent(const int keyChar, const unsigned int key)
{
    if (key == OIS::KC_UP)
    {
        if (mSelectedRow > 0)
        {
            drawSelection(mSelectedRow-1);
            mSeletedRowChanged = true;
        }
        return GuiManager::EVENT_CHECK_DONE;
    }
    if (key == OIS::KC_DOWN)
    {
        if (mSelectedRow+1 < (int)mvRow.size())
        {
            drawSelection(mSelectedRow+1);
            mSeletedRowChanged = true;
        }
        return GuiManager::EVENT_CHECK_DONE;
    }
    if (key == OIS::KC_RETURN) // || key == OIS::KC_NUMPADENTER)
    {
        mRowActivated = true;
        return GuiManager::EVENT_CHECK_DONE;
    }
    if (key == OIS::KC_ESCAPE)
    {
        mUserBreak = true;
        return GuiManager::EVENT_CHECK_DONE;
    }
    return GuiManager::EVENT_CHECK_NEXT;
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
int GuiTable::mouseEvent(int MouseAction, int x, int y, int z)
{
    if (!mouseWithin(x,y))
        return GuiManager::EVENT_CHECK_NEXT;
    int row = (y-mPosY-mHeightColumnLabel) / mHeightRow;
    if (row <0 || row >= (int)mvRow.size()) return true;

    if (MouseAction == GuiManager::BUTTON_RELEASED)
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
    if (MouseAction == GuiManager::BUTTON_PRESSED)
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
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    uint32 *bak = mParent->getLayerBG() + mPosX + mPosY*mParent->getWidth();
    // Draws a gfx into the window texture.
    int startX = 0;
    for (std::vector<ColumnEntry*>::iterator i = mvColumn.begin(); i < mvColumn.end(); ++i)
    {
        if ((*i)->label.size())
        {
            GuiTextout::getSingleton().printText((*i)->width, mHeightColumnLabel, dst + startX, mWidth,
                                                 bak + startX, mParent->getWidth(),
                                                 (*i)->label.c_str(), mFontNr, 0x00ffffff);
        }
        startX+= (*i)->width;
    }
    // Draw the line background.
    uint32 *buf = dst + mHeightColumnLabel * mWidth;
    int y = 0, h = mHeightColumnLabel;
    while (h < mHeight)
    {
        for (int line =0; line < mHeightRow && h++ < mHeight; ++line)
            for (int x = 0; x < mWidth; ++x)
                *buf++ = mColorRowBG[y&1];
        ++y;
    }
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst), Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight));
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
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    // Draw the background.
    uint32 *buf = dst;
    for (int i =0; i < mHeightRow*mWidth; ++i)
        *buf++ = bgColor;
    // Print the text.
    buf = dst + TEXT_OFFSET + TEXT_OFFSET*mWidth;
    if (mvRow[row].size())
    {
        int offX, fontHeight;
        std::string::size_type colStart, colEnd, subRowStrt, subRowEnd = 0;
        for (std::vector<SubRowEntry*>::iterator subRow = mvSubRow.begin(); subRow < mvSubRow.end(); ++subRow)
        {
            offX = 0;
            fontHeight = GuiTextout::getSingleton().getFontHeight((*subRow)->fontNr);
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
                GuiTextout::getSingleton().printText((*col)->width, fontHeight, buf + offX, mWidth,
                                                     &bgColor, 0, mvRow[row].substr(colStart, colEnd-colStart).c_str(), (*subRow)->fontNr, (*subRow)->color);
                if (++colEnd >= subRowEnd) break;
                offX += (*col)->width;
            }
            if (++subRowEnd >= mvRow[row].size()) break;
            buf+= fontHeight*mWidth;
        }
    }
    int offY = mPosY+mHeightColumnLabel + row * mHeightRow;
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeightRow, 1, PF_A8R8G8B8, dst), Box(mPosX, offY, mPosX+mWidth, offY+mHeightRow));
}
