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
    enum
    {
        ELEMENT_COMBOBOX_NONE,
        ELEMENT_COMBOBOX_DDBUTTON,
        ELEMENT_COMBOBOX_SCROLL_UP,
        ELEMENT_COMBOBOX_SCROLL_DOWN,
        ELEMENT_COMBOBOX_SCROLL_BAR,
        ELEMENT_COMBOBOX_SUM
    };

    GuiElementCombobox(TiXmlElement *xmlElement, const void *parent);
    ~GuiElementCombobox() {}
    int sendMsg(const int message, const char *text, Ogre::uint32 param, const char *text2);
    void draw();
    void setText(const char *value);
    bool setState(int state);
    bool mouseOver(int x, int y);
    const char *getText();

private:
    GuiImageset::gfxSrcEntry *srcButton;
    GuiImageset::gfxSrcEntry *srcScrollbarUp;
    GuiImageset::gfxSrcEntry *srcScrollbarDown;
    Ogre::uint32 *mGfxBuffer;
    int mAction;
    int mFontHeight;
    int mMaxChars;
    int mEntryHeight;
    int bw;
    int mMouseX, mMouseY;
    int mActiveDropdownOption;
    int mVirtualHeight;
    int mScrollPos;
    int mViewport;
    int mButton;
    bool mNeedsScroll;
    bool mUseNumbers;
    bool mUseWhitespaces;
    bool mDispDropdown;
    bool mDDButton;
    std::vector<Ogre::String> mvOption;
    std::vector<int> mvValue;

    GuiElementCombobox(const GuiElementCombobox&);            /**< disable copy-constructor. **/
    GuiElementCombobox &operator=(const GuiElementCombobox&); /**< disable assignment operator. **/
};

#endif