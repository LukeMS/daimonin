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
#include "logfile.h"

//=================================================================================================
// Init all elements.
//=================================================================================================
bool Dialog::Init()
{
	mLoginOverlay					= OverlayManager::getSingleton().getByName			("DialogOverlay");
	mPanelPlayerName			= OverlayManager::getSingleton().getOverlayElement	("Dialog/Login/Playername");
	mPlayerName					= OverlayManager::getSingleton().getOverlayElement	("Dialog/Login/Playername/Text");
	mPanelPlayerPasswd		= OverlayManager::getSingleton().getOverlayElement	("Dialog/Login/Password");
	mPlayerPasswd				= OverlayManager::getSingleton().getOverlayElement	("Dialog/Login/Password/Text");
	mPlayerRePasswd			= OverlayManager::getSingleton().getOverlayElement	("Dialog/Login/RePassword/Text");
	mPanelPlayerRePasswd	= OverlayManager::getSingleton().getOverlayElement	("Dialog/Login/RePassword");
	mElementSelectionBar		= OverlayManager::getSingleton().getOverlayElement	("Dialog/MetaSelect/select");
	mDialogSelPanel= static_cast<OverlayContainer*>(OverlayManager::getSingleton().getOverlayElement("Dialog/MetaSelect/Back"));
	mDialogInfoPanel=static_cast<OverlayContainer*>(OverlayManager::getSingleton().getOverlayElement("Dialog/Info/Panel"));
	// Selection textareas.
	std::string name= "Dialog/Text/";
	for (unsigned int i=0; i < DIALOG_TXT_LINES; i++)
	{
		mElementLine[i]= OverlayManager::getSingleton().
			cloneOverlayElementFromTemplate("Dialog/TextRow",name+"Line_"+ StringConverter::toString(i));
		mElementLine[i]->setTop(i*12+1);
		mDialogSelPanel->addChild(mElementLine[i]);
	}
	// Info textareas.
	name= "Dialog/Info/";
	for (unsigned int i=0; i < DIALOG_INFO_LINES; i++)
	{
		mElementInfo[i]= OverlayManager::getSingleton().
			cloneOverlayElementFromTemplate("Dialog/TextRow",name+"Line_"+ StringConverter::toString(i));
		mElementInfo[i]->setTop(i*12+1);
		mElementInfo[i]->setCaption(name+"Line_"+ StringConverter::toString(i));
		mDialogInfoPanel->addChild(mElementInfo[i]);
	}

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
		mPanelPlayerName	->hide();
		mPanelPlayerPasswd	->hide();
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
void Dialog::setWarning(int warning)
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

//=================================================================================================
// Fill a textarea of selectable text.
//=================================================================================================
void Dialog::setSelText(unsigned int pos, const char *text, ColourValue color)
{
	if (pos > DIALOG_TXT_LINES) { pos = DIALOG_TXT_LINES; }
	mElementLine[pos]->setCaption(text);
	mElementLine[pos]->setColour(color);
}

//=================================================================================================
// Clear all selectable text.
//=================================================================================================



//=================================================================================================
// Fill the textarea of Info text.
//=================================================================================================
void Dialog::setInfoText(unsigned int pos, const char *text, ColourValue color)
{
	if (pos > DIALOG_INFO_LINES) { pos = DIALOG_INFO_LINES; }
	mElementInfo[pos]->setCaption(text);
	mElementInfo[pos]->setColour(color);
}

//=================================================================================================
// Clear all Info text.
//=================================================================================================
void Dialog::clearInfoText()
{
	for (unsigned int i=0; i < DIALOG_INFO_LINES; ++i) { mElementInfo[i]->setCaption(""); }
}

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
			mDialogSelPanel		->hide();
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
		case DIALOG_STAGE_GET_META_SERVER:
			mDialogSelPanel->show();
			mElementSelectionBar->setTop(12*TextInput::getSingleton().getSelCursorPos()+1);
		default:
			return;
	}
}
