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

using namespace Ogre;

static const unsigned long SCROLL_SPEED = 12;
static const Real CLOSING_SPEED  = 10.0f;  // default: 10.0f
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
        uint32 *color;
        if      (!strcmp(xmlOpt->Attribute("type"), "COLOR_ODD_ROWS"))  { color = &mColorBack[0]; *color =0; } // Color of odd  rows.
        else if (!strcmp(xmlOpt->Attribute("type"), "COLOR_EVEN_ROWS")) { color = &mColorBack[1]; *color =0; } // Color of even rows.
        else if (!strcmp(xmlOpt->Attribute("type"), "COLOR_SELECTION")) { color = &mColorSelect;  *color =0; } // Color of selection.
        if ((tmp = xmlOpt->Attribute("alpha"))) *color+= atoi(tmp) << 24;
        if ((tmp = xmlOpt->Attribute("red")))   *color+= atoi(tmp) << 16;
        if ((tmp = xmlOpt->Attribute("green"))) *color+= atoi(tmp) <<  8;
        if ((tmp = xmlOpt->Attribute("blue")))  *color+= atoi(tmp);
    }
    for (xmlOpt = xmlElement->FirstChildElement("Column"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Column"))
    {
        ColumnEntry *entry = new ColumnEntry;
        if ((tmp = xmlOpt->Attribute("width"))) entry->width = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("label"))) entry->label = tmp;
        mvColumn.push_back(entry);
    }
    for (xmlOpt = xmlElement->FirstChildElement("SubRow"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("SubRow"))
    {
        SubRowEntry *entry = new SubRowEntry;
        entry->color = 0;
        if ((tmp = xmlOpt->Attribute("font")))  entry->fontNr= atoi(tmp);
        if ((tmp = xmlOpt->Attribute("alpha"))) entry->color+= atoi(tmp) << 24;
        if ((tmp = xmlOpt->Attribute("red"  ))) entry->color+= atoi(tmp) << 16;
        if ((tmp = xmlOpt->Attribute("green"))) entry->color+= atoi(tmp) <<  8;
        if ((tmp = xmlOpt->Attribute("blue" ))) entry->color+= atoi(tmp);
        mvSubRow.push_back(entry);
    }
    mUserBreak = false;
    mRowHeight = 0;
    for (std::vector<SubRowEntry*>::iterator i = mvSubRow.begin(); i < mvSubRow.end(); ++i)
        mRowHeight+= GuiTextout::getSingleton().getFontHeight((*i)->fontNr) + TEXT_OFFSET;
    mHeightBorderline = GuiTextout::getSingleton().getFontHeight(mFontNr);
    clearRows();
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
    if (x < mPosX || x > mPosX + mWidth || y < mPosY || y > mPosY + mHeight)
        return false;
    int row = (y-mPosY) / mRowHeight -1;
    if (MouseAction == GuiWindow::BUTTON_PRESSED && row >=0 && row < (int)mvRow.size())
    {
        static unsigned long time =0;
        if (mSelectedRow == row)
        {
            if (Root::getSingleton().getTimer()->getMilliseconds()- time < GuiWindow::TIME_DOUBLECLICK)
                return (mRowActivated = true);
        }
        else
        {
            drawSelection(row);
        }
        time = Root::getSingleton().getTimer()->getMilliseconds();
    }
    return true;
}

//================================================================================================
// If a user-break was detected return true once.
//================================================================================================
bool GuiTable::getUserBreak()
{
    if (!mUserBreak)
        return false;
    mUserBreak = false;
    return true;
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
// Clear all rows.
//================================================================================================
void GuiTable::clearRows()
{
    mvRow.clear();
    mSelectedRow = -1;
    mRowActivated = false;
    mRowChanged = true;
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
    if (!mRowChanged)
        return -1;
    mRowChanged = false;
    return mSelectedRow;
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
    textline.y2 = textline.y1 + mHeightBorderline;
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
    PixelBox pb = texture->getBuffer()->lock(Box(mPosX, mPosY+mHeightBorderline, mPosX+mWidth, mPosY+mHeight), HardwareBuffer::HBL_DISCARD);
    uint32 *dest_data = (uint32*)pb.data;
    int y = 0, h = mHeightBorderline;
    while (h < mHeight)
    {
        for (int line =0; line < mRowHeight && h++ < mHeight; ++line)
        {
            for (int x = 0; x < mWidth; ++x)
                dest_data[x] = mColorBack[y&1];
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
    drawRow(mSelectedRow, mColorBack[mSelectedRow&1]); // Restore selection background
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
    int offset = mHeightBorderline + row * mRowHeight;
    // Draw the background.
    PixelBox pb = texture->getBuffer()->lock(Box(mPosX, mPosY+offset, mPosX+mWidth, mPosY+offset+mRowHeight), HardwareBuffer::HBL_DISCARD);
    uint32 *dest_data = (uint32*)pb.data;
    for (int line =0; line < mRowHeight; ++line)
    {
        for (int x = 0; x < mWidth; ++x)
            dest_data[x] = bgColor;
        dest_data+=texture->getWidth();
    }
    texture->getBuffer()->unlock();
    // Print the text.
    if (!mvRow[row].size()) return;
    textline.y1 = mPosY+mHeightBorderline + row * mRowHeight + TEXT_OFFSET;
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
            if (colEnd > subRowEnd) colEnd = subRowEnd;
            if (colEnd == std::string::npos) colEnd = subRowEnd;
            textline.text = mvRow[row].substr(colStart, colEnd-colStart);
            GuiTextout::getSingleton().Print(&textline, texture);
            if (++colEnd >= subRowEnd) break;
            textline.x1 += (*col)->width;
        }
        if (++subRowEnd >= mvRow[row].size()) break;
        textline.y1+= fontHeight;
    }
}
/*
    for (int subRow = 0; subRow < 5; ++subRow)
    {
        subRowStrt = subRowEnd;
        subRowEnd = str.find(';', subRowStrt);
        if (subRowEnd == std::string::npos) subRowEnd = str.size();
        colEnd = subRowStrt;
        for (int i = 0; i < 5; ++i)
        {
            colStrt = colEnd;
            colEnd = str.find(',', colStrt);
            if (colEnd == std::string::npos) colEnd = subRowEnd;
            cout << str.substr(colStrt, colEnd-colStrt) << " ";
            if (++colEnd >= subRowEnd) break;
        }
        if (++subRowEnd >= str.size()) break;
        cout << endl;
    }
*/
