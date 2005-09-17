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

#ifndef GUI_CURSOR_H
#define GUI_CURSOR_H
#include <Ogre.h>

using namespace Ogre;

////////////////////////////////////////////////////////////
/// Defines.
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Class.
////////////////////////////////////////////////////////////
class GuiCursor
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiCursor(unsigned int w, unsigned int h, const char* Name);
  ~GuiCursor();
  void setPos(Real x, Real y);

private:
  enum
  {
    STATE_STANDARD, STATE_DRAGGING, STATE_RESIZING, STATE_SUM
  };

  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  unsigned int mScreenWidth, mScreenHeight;
  struct spos
  {
    int x, y,  w, h;
  }
  mSrcPos[STATE_SUM];
  int mWidth, mHeight;
  int mState;
  Image mTileImage;
  TexturePtr mTexture;
  PixelBox mSrcPixelBox;
  OverlayElement *mElement;
  MaterialPtr mMaterial;
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiCursor(const GuiCursor&); // disable copy-constructor.
};

#endif
