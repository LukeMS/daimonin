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

#ifndef GUI_TABLE_H
#define GUI_TABLE_H

#include <vector>
#include <Ogre.h>
#include <tinyxml.h>
#include "gui_element.h"

/** --------------------------------------------------------------------------
 ** This class handles an interactive table.
 ** --------------------------------------------------------------------------
 ** SubRows are separated by ';' in the row-text.
 ** Columns are separated by ',' in the row-text.
 ** @todo add gfx-columnn support.
 ** @todo add scrollbars.
 ** @todo change the seletionbar to a gfx (alpha) and draw it on top of the row..
 *****************************************************************************/
class GuiTable : public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiTable(TiXmlElement *xmlElement, void *parent);
    ~GuiTable();
    int sendMsg(int element, void *parm1 =0, void *parm2 =0, void *parm3 =0);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        int width;          /**< Width of the column. **/
        Ogre::String label; /**< Label of the column. **/
    } ColumnEntry;
    std::vector<ColumnEntry*>mvColumn; /**< Columns in a single row. **/
    typedef struct
    {
        Ogre::uint32 color; /**< Default color of the subrow. **/
        int fontNr;         /**< Default font  of the subrow. **/
    } SubRowEntry;
    std::vector<SubRowEntry*>mvSubRow;
    std::vector<Ogre::String>mvRow;
    bool mUserBreak;
    bool mRowActivated;                /**< A row was activated by the user.(double-click lmb or return key) **/
    bool mSeletedRowChanged;           /**< The selected row has changed by the user (crsr up/down or lmb). **/
    int  mHeightColumnLabel;
    int  mHeightRow;
    int  mSelectedRow;                 /**< The actual selected row. **/
    Ogre::uint32 mColorRowBG[2];
    Ogre::uint32 mColorSelect;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    inline int getUserBreak();
    inline int getSelectedRow();
    inline int getActivatedRow();
    int keyEvent(const int keyChar, const unsigned int key);
    int mouseEvent(int MouseAction, int x, int y, int z);
    inline void clear();                             /**< Clear the whole table.   **/
    inline void addRow(const char *row);
    inline void setRow(int row, const char *rowTxt); /**< Set new values to a row. **/
    void draw();                                     /**< Draw the background of the table. **/
    void drawSelection(int newSelection);
    void drawRow(int row, Ogre::uint32 color);
};

#endif
