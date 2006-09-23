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

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_table.h"
#include "logger.h"
#include "gui_window.h"

#include "gui_manager.h"

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
    // ////////////////////////////////////////////////////////////////////
    // Create buffer to hold the pixel information of the listbox.
    // ////////////////////////////////////////////////////////////////////
    mFontHeight = GuiTextout::getSingleton().getFontHeight(mFontNr);
    mSumRows = mHeight / mFontHeight;
    mHeight = mSumRows * mFontHeight;
    --mSumRows; // Reserve space for the borderline.
    mSelectedRow = -1;
    clearBackground();
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
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
bool GuiTable::mouseEvent(int MouseAction, int x, int y)
{
    if (x >= mPosX && x <= mPosX + mWidth && y >= mPosY && y <= mPosY + mHeight)
    {
        if (MouseAction == GuiWindow::BUTTON_PRESSED)
        {
            mSelectedRow = (y-mPosY) / mFontHeight -1;
        }
        return true;
    }
    return false;
}

//================================================================================================
// .
//================================================================================================
void GuiTable::addRow(String textline)
{
    mvRow.push_back(textline+","); // Add the end of col sign (the comma) to the end of the text.
    draw();
}

//================================================================================================
// .
//================================================================================================
int GuiTable::getSelectedRow()
{
    int ret;
    if (mSelectedRow >= (int)mvRow.size())
        ret =-1;
    else
        ret = mSelectedRow;
    mSelectedRow =-1;
    return ret;
}

//================================================================================================
// .
//================================================================================================
void GuiTable::clearBackground()
{
    Texture *texture = ((GuiWindow*) mParent)->getTexture();

    // Draw the column borderlines.
    TextLine textline;
    textline.index = -1;
    textline.BG_Backup = 0;
    textline.font = mFontNr;
    textline.x1   = mPosX;
    textline.y1   = mPosY;
    for (std::vector<TableEntry*>::iterator i = mvColumn.begin(); i < mvColumn.end(); ++i)
    {
        if ((*i)->label!="")
        {
            textline.text = (*i)->label;
            if (GuiTextout::getSingleton().getClippingPos(textline, mPosX + mWidth, mHeight))
            {
                GuiTextout::getSingleton().Print(&textline, texture);
            }
        }
        textline.x1 += (*i)->width;
    }
    // Draw the line background.
    PixelBox pb = texture->getBuffer()->lock (Box(mPosX, mPosY+mFontHeight, mPosX+mWidth, mPosY+mHeight), HardwareBuffer::HBL_DISCARD);
    uint32 *dest_data = (uint32*)pb.data;
    for (int y = 0; y < mSumRows; ++y)
    {
        for (int row =0; row < mFontHeight; ++row)
        {
            for (int x = 0; x < mWidth; ++x)
            {
                dest_data[x] = mColorBack[y&1];
            }
            dest_data+=texture->getWidth();
        }
    }
    texture->getBuffer()->unlock();
    mvRow.clear();
}

//================================================================================================
// .
//================================================================================================
void GuiTable::draw()
{
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    // Draw the column borderlines.
    TextLine textline;
    textline.index = -1;
    textline.BG_Backup = 0;
    textline.font = mFontNr;
    textline.y1   = mPosY+mFontHeight+2;
    std::string::size_type startPos, endPos;

    int index = (int)mvRow.size()-1;
    if (index <0 ||mvRow[index] =="") return;
    textline.y1 += index * mFontHeight;
    {
        textline.x1   = mPosX+3;
        endPos = 0;
        for (std::vector<TableEntry*>::iterator i = mvColumn.begin(); i < mvColumn.end(); ++i)
        {
            startPos = endPos;
            endPos = mvRow[index].find( ',',  startPos);
            if (endPos == std::string::npos) break;
            {
                textline.text = mvRow[index].substr(startPos, endPos-startPos);
                if (GuiTextout::getSingleton().getClippingPos(textline, mPosX + mWidth, mHeight))
                {
                    GuiTextout::getSingleton().Print(&textline, texture);
                }
            }
            ++endPos;
            textline.x1 += (*i)->width;
        }
    }
}
