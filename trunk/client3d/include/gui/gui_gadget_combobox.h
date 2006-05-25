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

#ifndef GUI_GADGET_COMBOBOX_H
#define GUI_GADGET_COMBOBOX_H

class GuiGadgetCombobox;

#include <string>
#include <tinyxml.h>
#include <Ogre.h>
#include "gui_imageset.h"
#include "gui_gadget.h"
#include "gui_textout.h"

using namespace Ogre;

enum
{
    GUI_GADGET_COMBOBOX_NONE,
    GUI_GADGET_COMBOBOX_DDBUTTON,
    GUI_GADGET_COMBOBOX_SCROLL_UP,
    GUI_GADGET_COMBOBOX_SCROLL_DOWN,
    GUI_GADGET_COMBOBOX_SCROLL_BAR,
    GUI_GADGET_COMBOBOX_SUM
};

class GuiGadgetCombobox : public GuiGadget
{
public:
    GuiGadgetCombobox(TiXmlElement *xmlElement, int w, int h, int maxX, int maxY);
    ~GuiGadgetCombobox();

    void draw(PixelBox &mSrcPixelBox, Texture *texture);
    void setText(const char *value);
    bool setState(int state);
    bool mouseOver(int x, int y);

    const char *getText();
private:
    GuiSrcEntry *srcButton;
    GuiSrcEntry *srcScrollbarUp;
    GuiSrcEntry *srcScrollbarDown;
    uint32 *mGfxBuffer;
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
    std::vector<std::string> mvOption;
    std::vector<int> mvValue;
};

#endif // GUI_COMBOBOX_H
