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

#ifndef DIALOG_H
#define DIALOG_H

#include <Ogre.h>
#include "textwindow.h"

using namespace Ogre;

const unsigned int DIALOG_TXT_LINES = 12;
const unsigned int DIALOG_INFO_LINES = 4;

enum _dialog_stage
{
  DIALOG_STAGE_LOGIN_GET_NAME,
  DIALOG_STAGE_LOGIN_GET_PASSWD,
  DIALOG_STAGE_LOGIN_GET_PASSWD_AGAIN,
  DIALOG_STAGE_GET_META_SERVER,
  DIALOG_STAGE_SUM
};

enum _dialog_warning
{
  DIALOG_WARNING_NONE,
  DIALOG_WARNING_LOGIN_WRONG_NAME,
  DIALOG_WARNING_SUM
};

class Dialog
{
public:
  static Dialog &getSingleton()
  {
    static Dialog Singleton; return Singleton;
  }
  bool Init();
  void setVisible(bool vis);
  bool isVisible()
  {
    return mVisible;
  }
  void UpdateLogin(unsigned int stage);
  bool UpdateNewChar();
  void setWarning(int warning);
  void setSelText (unsigned int pos, const char *text, ColourValue = TXT_WHITE);
  void setInfoText(unsigned int pos, const char *text, ColourValue = TXT_WHITE);
  void clearInfoText();

private:
  Dialog()
  {}
  ~Dialog()
  {}
  Dialog(const Dialog&); // disable copy-constructor.
  Overlay *mLoginOverlay;
  OverlayContainer *mDialogSelPanel, *mDialogInfoPanel;
  OverlayElement *mPlayerName, *mPlayerPasswd, *mPlayerRePasswd;
  OverlayElement *mPanelPlayerName, *mPanelPlayerPasswd, *mPanelPlayerRePasswd;
  OverlayElement *mElementLine[DIALOG_TXT_LINES], *mElementSelectionBar;
  OverlayElement *mElementInfo[DIALOG_INFO_LINES];
  bool mVisible;
};

#endif
