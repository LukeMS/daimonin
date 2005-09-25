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

#ifndef GUI_TEXTOUT_H
#define GUI_TEXTOUT_H

#include <Ogre.h>
#include <string>

using namespace Ogre;

const uint32 COLOR_BLACK = 0xff000000;
const uint32 COLOR_BLUE  = 0xff0000ff;
const uint32 COLOR_GREEN = 0xff00ff00;
const uint32 COLOR_LBLUE = 0xff00ffff;
const uint32 COLOR_RED   = 0xffff0000;
const uint32 COLOR_PINK  = 0xffff00ff;
const uint32 COLOR_YELLOW= 0xffffff00;
const uint32 COLOR_WHITE = 0xffffffff;

enum
{
  FONT_SMALL, FONT_NORMAL, FONT_BIG, FONT_SUM
};
const int CHARS_IN_FONT =96;

class GuiTextout
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  static GuiTextout &getSingleton()
  {
    static GuiTextout Singleton; return Singleton;
  }
  void Print(int x, int y, Texture *texture, const char *text, uint32 color = COLOR_WHITE);

private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////

  struct
  {
    Image image;
    uint32 *data;
    unsigned int textureWidth;
    unsigned int width;
    unsigned int height;
    char charWidth[CHARS_IN_FONT];
  }
  mFont[FONT_SUM];

  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////

  GuiTextout();
  ~GuiTextout()
  {}
  GuiTextout(const GuiTextout&); // disable copy-constructor.
  void loadFont(const char * filename);
};

#endif


