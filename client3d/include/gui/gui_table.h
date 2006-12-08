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

#ifndef GUI_TABLE_H
#define GUI_TABLE_H

#include <tinyxml.h>
#include <vector>
#include <Ogre.h>
#include "gui_element.h"
#include "gui_textout.h"

using namespace Ogre;

/**
 **
 *****************************************************************************/
class GuiTable : public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiTable(TiXmlElement *xmlElement, void *parent);
    ~GuiTable();
    void draw();
    void clearRows();
    void drawSelection(int newSelection);
    void addRow(String textline);
    bool getUserBreak();
    bool mouseEvent(int MouseAction, int x, int y);
    bool keyEvent(const char keyChar, const unsigned char key);
    int  getSelectedRow();
    int  getActivatedRow();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        int width;
        String label;
    }
    TableEntry;
    std::vector<TableEntry*>mvColumn;
    std::vector<String>mvRow;
    Real mMinHeight, mMaxHeight;
    bool mVisible;
    bool mRowActivated;
    bool mRowChanged;
    bool mUserBreak;
    int  mSumRows;
    int  mSelectedRow;
    int  mFontHeight;
    uint32 *mGfxBuffer;
    uint32 mColorBack[2], mColorSelect;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void drawSelection();
    void drawRow(int row, uint32 color);
};

#endif
