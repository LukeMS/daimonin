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

#ifndef GUI_STATUSBAR_H
#define GUI_STATUSBAR_H

#include <tinyxml.h>
#include <Ogre.h>
#include "gui_element.h"

using namespace Ogre;

class GuiStatusbar : public GuiElement
{
public:
  GuiStatusbar(TiXmlElement *xmlElement, int w, int h, int maxX, int maxY):GuiElement(xmlElement, w, h, maxX, maxY)
  {
    mGfxBuffer = 0;
    setValue(1.0); // default: 100%
  }
  ~GuiStatusbar()
  {
    delete[] mGfxBuffer;
  }
  void draw(PixelBox &mSrcPixelBox, Texture *texture);
  void setValue(Real value);

private:
  bool mHorizontal;
  int mValue;
  uint32 *mGfxBuffer;
};

#endif
