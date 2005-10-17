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

#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <string>
#include <vector>
#include <Ogre.h>
#include <tinyxml.h>
#include "gui_window.h"
#include "gui_cursor.h"

using namespace Ogre;

enum {
  GUI_WIN_STATISTICS,
  GUI_WIN_PLAYERINFO,
  GUI_WIN_TEXTWINDOW,
  GUI_WIN_SUM };

enum {
  GUI_MSG_TXT_CHANGED,
  GUI_MSG_ADD_TEXTLINE,
  GUI_MSG_BUT_PRESSED,
  GUI_MSG_SUM };

enum {
  // Button.
  GUI_BUTTON_CLOSE,
  GUI_BUTTON_OK,
  GUI_BUTTON_CANCEL,
  GUI_BUTTON_MINIMIZE,
  GUI_BUTTON_MAXIMIZE,
  // Listboxes.
  GUI_LIST_TEXTWIN,
  GUI_LIST_CHATWIN,
  GUI_LIST_UP,
  GUI_LIST_DOWN,
  GUI_LIST_LEFT,
  GUI_LIST_RIGHT,
  // TextValues.
  GUI_TEXTVALUE_STAT_CUR_FPS,
  GUI_TEXTVALUE_STAT_BEST_FPS,
  GUI_TEXTVALUE_STAT_WORST_FPS,
  GUI_TEXTVALUE_STAT_SUM_TRIS,
  // Sum of all entries.
  GUI_ELEMENTS_SUM
};

class GuiManager
{
public:
  enum
  {
    MSG_CHANGE_TEXT, MSG_BUTTON_PRESSED, MSG_SUM
  };

  struct _state
  {
    std::string name;
    short x, y;
  };

  struct mSrcEntry
  {
    std::string name;
    int width, height;
    std::vector<_state*>state;
  };

  struct _GuiElementNames
  {
    std::string name;
    unsigned int index;
  }
  static GuiWindowNames[], GuiElementNames[];
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  static GuiManager &getSingleton()
  {
    static GuiManager Singleton; return Singleton;
  }
  struct mSrcEntry *getStateGfxPositions(const char* guiImage);
  PixelBox &getTilesetPixelBox()
  {
    return mSrcPixelBox;
  }
  bool hasFocus()
  {
    return mHasFocus;
  }
  void Init(const char *XML_imageset_file, const char *XML_windows_file, int w, int h);
  void freeRecources();
  void update();
  bool mouseEvent(int MouseAction, Real rx, Real ry);
  void keyEvent(const char keyChar, const unsigned char key);
  void sendMessage(int window, int message, int element, void *value1 = NULL, void *value2 = NULL);
  //void getMessage (int &window, int &message, int &element);

private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  
  
  int mDragSrcWin, mDragDestWin;
  int mDragSrcContainer, mDragDestContainer;
  int mDragSrcItemPosx, mDragSrcItemPosy; // Wird bei drag start gesetzt, um Item bei falschem Drag zurückflutschen zu lassen.
  bool isDragging;
  
  std::string mStrImageSetGfxFile;
  std::vector<mSrcEntry*>mvSrcEntry;
  class GuiWindow *guiWindow;
  unsigned int mScreenWidth, mScreenHeight;
  bool mHasFocus;
  int mFocusedWindow, mFocusedGadget;
  PixelBox mSrcPixelBox;
  Image mImageSetImg;
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiManager()
  {}
  ~GuiManager()
  {}
  bool parseImagesetData(const char *file);
  bool parseWindowsData (const char *file);
};

#endif
