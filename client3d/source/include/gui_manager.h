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
#include "gui_imageset.h"
#include "gui_cursor.h"
#include "logger.h"

using namespace Ogre;

enum
{
  GUI_WIN_STATISTICS,
  GUI_WIN_PLAYERINFO,
  GUI_WIN_TEXTWINDOW,
//  GUI_WIN_CREATION,
  GUI_WIN_SUM
};

enum
{
  GUI_MSG_TXT_GET,
  GUI_MSG_TXT_CHANGED,
  GUI_MSG_ADD_TEXTLINE,
  GUI_MSG_BUT_PRESSED,
  GUI_MSG_SUM
};

typedef struct
{
  char *name;
  unsigned int index;
}
GuiWinNam;

class GuiManager
{
public:
  enum
  {
    MSG_CHANGE_TEXT, MSG_BUTTON_PRESSED, MSG_SUM
  };

  /// ////////////////////////////////////////////////////////////////////
  /// Functions.
  /// ////////////////////////////////////////////////////////////////////
  static GuiManager &getSingleton()
  {
    static GuiManager singleton; return singleton;
  }
  void freeRecources();
  void Init(int w, int h);
  void parseImageset(const char *XML_imageset_file);
  void parseWindows (const char *XML_windows_file);
  void update();
  bool mouseEvent(int MouseAction, Real rx, Real ry);
  bool keyEvent(const char keyChar, const unsigned char key);
  const char *sendMessage(int window, int message, int element, void *value1 = NULL, void *value2 = NULL);
  void setTooltip(const char*text);
  void displaySystemMessage(const char*text);
  void startTextInput(int window, int winElement, int maxChars, bool useNumbers, bool useWhitespaces);

private:
  /// ////////////////////////////////////////////////////////////////////
  /// Variables.
  /// ////////////////////////////////////////////////////////////////////
  static GuiWinNam mGuiWindowNames[GUI_WIN_SUM];
  int mDragSrcWin, mDragDestWin;
  int mDragSrcContainer, mDragDestContainer;
  int mDragSrcItemPosx, mDragSrcItemPosy; // Wird bei drag start gesetzt, um Item bei falschem Drag zurückflutschen zu lassen.
  int mProcessingTextInput;
  int mActiveWindow, mActiveElement;
  int mMouseX, mMouseY, mHotSpotX, mHotSpotY;
  bool mTooltipRefresh;
  bool isDragging;
  std::string  mStrTooltip, mBackupTextInputString;
  class GuiWindow *guiWindow;
  unsigned int mScreenWidth, mScreenHeight;
  clock_t mTooltipDelay;
  Overlay *mOverlay;
  OverlayElement *mElement;
  MaterialPtr mMaterial;
  TexturePtr mTexture;
  /// ////////////////////////////////////////////////////////////////////
  /// Functions.
  /// ////////////////////////////////////////////////////////////////////
  GuiManager()
  {
    guiWindow = NULL;
  }
  ~GuiManager()
  {
  }
  GuiManager(const GuiManager&); // disable copy-constructor.
  //GuiManager& operator=(GuiManager const&);

  bool parseWindowsData (const char *file);
  void clearTooltip();
};

#endif
