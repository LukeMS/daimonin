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
  enum
  {
    STATE_STANDARD, STATE_DRAGGING, STATE_RESIZING, STATE_SUM
  };
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  void setPos(Real x, Real y);
  void setStateImagePos(std::string state, int x, int y);
  void setState(int state);
  void draw(PixelBox &SrcPixelBox);
  void Init(int w, int h, int screenHeight, int screenWidth);
  void freeRecources();
  static GuiCursor &getSingleton()
  {
    static GuiCursor Singleton; return Singleton;
  }

private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  unsigned int mScreenWidth, mScreenHeight;
  struct _pos
  {
    int x, y;
  }
  gfxSrcPos[STATE_SUM];
  unsigned int mState;
  int mWidth, mHeight;
  OverlayElement *mElement;
  TexturePtr mTexture;
  MaterialPtr mMaterial;
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiCursor()
  {}
  ~GuiCursor()
  {}
  GuiCursor(const GuiCursor&); // disable copy-constructor.
};

#endif
