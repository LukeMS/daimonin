/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include "dialog.h"
#include "textwindow.h"
#include "textinput.h"

static std::string mStrPlayerName;
//=================================================================================================
// Constructor.
//=================================================================================================
Dialog::Dialog()
{
}

//=================================================================================================
// Destructor.
//=================================================================================================
Dialog::~Dialog()
{
}

//=================================================================================================
// Return the instance.
//=================================================================================================
Dialog &Dialog::getSingelton()
{
   static Dialog singelton;
   return singelton;
}

//=================================================================================================
// Init all elements.
//=================================================================================================
bool Dialog::Init()
{
    mLoginOverlay   = OverlayManager::getSingleton().getByName("DialogOverlay");
	mPlayerName     = OverlayManager::getSingleton().getOverlayElement("Dialog/Login/Playername/Text");
	mPlayerPasswd   = OverlayManager::getSingleton().getOverlayElement("Dialog/Login/Password/Text");
	mPlayerRePasswd = OverlayManager::getSingleton().getOverlayElement("Dialog/Login/RePassword/Text");

	mPanelPlayerName     = OverlayManager::getSingleton().getOverlayElement("Dialog/Login/Playername");
	mPanelPlayerPasswd   = OverlayManager::getSingleton().getOverlayElement("Dialog/Login/Password");
	mPanelPlayerRePasswd = OverlayManager::getSingleton().getOverlayElement("Dialog/Login/RePassword");


	mVisible = false;
    return true;
}

//=================================================================================================
// Show/Hide the Overlay.
//=================================================================================================
void Dialog::visible(bool vis)
{
	if (vis == true)
	{
		mVisible = true;
		mPanelPlayerName    ->hide();
		mPanelPlayerPasswd  ->hide();
		mPanelPlayerRePasswd->hide();
		mLoginOverlay->show();
	}
    else
	{
		mVisible = false;
		mLoginOverlay->hide();
	}
}

//=================================================================================================
// Show a warning.
//=================================================================================================
void  Dialog::setWarning(int warning)
{
	switch(warning)
	{
		case DIALOG_WARNING_NONE:
			break;
		case DIALOG_WARNING_LOGIN_WRONG_NAME:
			break;
		default:
			return;
	}
}

static std::string mStrPassword;
static std::string mStrRePasswd;
//=================================================================================================
// Login Overlay.
//=================================================================================================
void Dialog::UpdateLogin(unsigned int stage)
{

	switch(stage)
	{
		case DIALOG_STAGE_LOGIN_GET_NAME:
			mPanelPlayerName    ->show();
			mPanelPlayerPasswd  ->hide();
			mPanelPlayerRePasswd->hide();
			mStrPlayerName = TextInput::getSingleton().getString();
			mPlayerName->setCaption(mStrPlayerName);
			break;
		case DIALOG_STAGE_LOGIN_GET_PASSWD:
			mPanelPlayerPasswd->show();
			{
			   mStrPassword ="**********************************";
			   mStrPassword.resize(TextInput::getSingleton().size());
			}
			mPlayerPasswd->setCaption(mStrPassword.c_str());
			break;
		case DIALOG_STAGE_LOGIN_GET_PASSWD_AGAIN:
			mPanelPlayerRePasswd->show();
			{
			   mStrRePasswd ="**********************************";
			   mStrRePasswd.resize(TextInput::getSingleton().size());
			}
			mPlayerRePasswd->setCaption(mStrRePasswd.c_str());
			break;
		default:
			return;
	}
}
