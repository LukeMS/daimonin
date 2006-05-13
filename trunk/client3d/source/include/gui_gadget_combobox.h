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
		uint32 *mGfxBuffer;
    int  mFontHeight;
		int mMaxChars;
		int mEntryHeight;
		int bw;
		int mMouseX, mMouseY;
		int mActiveDropdownOption;
    bool mUseNumbers;
    bool mUseWhitespaces;
		bool mDispDropdown;
		bool mDDButton;
    std::vector<std::string> mvOption;
    std::vector<int> mvValue;
    
};


#endif // GUI_COMBOBOX_H

