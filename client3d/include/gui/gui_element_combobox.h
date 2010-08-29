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

#ifndef GUI_ELEMENT_COMBOBOX_H
#define GUI_ELEMENT_COMBOBOX_H

#include "gui_element.h"

/**
 ** This class provides an interactive combobox.
 *****************************************************************************/
class GuiElementCombobox : public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiElementCombobox(TiXmlElement *xmlElement, const void *parent);
    ~GuiElementCombobox();
    /// @copydoc GuiElement::sendMsg
    virtual void sendMsg(const int message, Ogre::String &text, Ogre::uint32 &param, const char *text2);
    virtual int mouseEvent(const int mouseAction, int mouseX, int mouseY, int mouseWheel);
    void draw();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {LIST_TEXT_OFFSET = 3, LIST_MAX_HEIGHT = 200};
    std::vector<Ogre::String>mvItem;
    unsigned int mActItem;
    int mListHeight;
    Ogre::uint32 mListColor, mBorderColor, mCursorColor;
    bool mMouseOver, mMouseButDown;
    bool mShowItemList;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /** Select the item holding the given text.
     ** @param item The item to select.
     *****************************************************************************/
    void SetSelection(const Ogre::String item);

    /** Select the item given by a number.
     ** @param item The item to select.
     *****************************************************************************/
    void SetSelection(unsigned int itemNr);

    void calcListHeight();
    GuiElementCombobox(const GuiElementCombobox&);            /**< disable copy-constructor. **/
    GuiElementCombobox &operator=(const GuiElementCombobox&); /**< disable assignment operator. **/
};

#endif
