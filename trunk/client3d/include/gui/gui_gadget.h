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

#ifndef GUI_GADGET_H
#define GUI_GADGET_H

class GuiGadget;

#include <tinyxml.h>
#include <Ogre.h>
#include "gui_element.h"

using namespace Ogre;

class GuiGadget: public GuiElement
{

public:
  /// ////////////////////////////////////////////////////////////////////
  /// Functions.
  /// ////////////////////////////////////////////////////////////////////
  GuiGadget(TiXmlElement *xmlElement, int w, int h, int maxX, int maxY);
  virtual ~GuiGadget();

  virtual bool setState(int state);
  virtual void draw(PixelBox &mSrcPixelBox, Texture *texture);
	virtual void addTextline(const char *value);
	virtual void setText(const char *value);
	virtual const char *getText();
	virtual bool mouseOver(int x, int y);

	int getAction();
protected:
	int mAction;
};

#endif
