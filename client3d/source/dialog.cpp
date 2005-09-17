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

#include "dialog.h"
#include "textwindow.h"
#include "textinput.h"
#include "logger.h"
#include "event.h"
#include "tinyxml.h"

//=================================================================================================
// Init all elements.
//=================================================================================================
bool Dialog::Init(SceneManager *SceneMgr)
{
  static int INr =0;
/*  
  Entity *mEntityNPC = SceneMgr->createEntity("GUI_"+StringConverter::toString(++INr), "tree01.mesh" );
  SceneNode *Node = SceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(200, 200, 200));
  Node->attachObject(mEntityNPC);
*/








  TiXmlElement *rootElem, *typeElem, *sectElem;
  int intValue;
  double doubleValue;
  bool boolValue;

  TiXmlDocument doc("./media/xml/GUI_ItemDisplay.xml");
  if (!doc.LoadFile())
  {
    return false;
  }

/*
  rootElem = doc.RootElement();
  if (!rootElem) return false;
  screen = root->FirstChildElement();
  if (!screen) return false;
  Logger::log().info() << "XML " << "Value: "     << screen->Value();
  screen = screen->NextSiblingElement();
  Logger::log().info() << "XML " << "Value: "     << screen->Value();
  screen = screen->NextSiblingElement();
  Logger::log().info() << "XML " << "Value: "     << screen->Value();
*/

  rootElem = doc.RootElement();
  if (!rootElem) return false;
  typeElem = rootElem->FirstChildElement();
  if (!typeElem) return false;
  Logger::log().info() << "XML-Type " << "Value: "     << typeElem->Value();
   sectElem = typeElem->FirstChildElement();
    Logger::log().info() << "XML-Section " << "Value: "     << sectElem->Value();
    sectElem = sectElem->NextSiblingElement();
    Logger::log().info() << "child " << "Value: "     << sectElem->Value();
    sectElem = sectElem->NextSiblingElement();
    Logger::log().info() << "child " << "Value: "     << sectElem->Value();
    sectElem = sectElem->NextSiblingElement();
    Logger::log().info() << "child " << "Value: "     << sectElem->Value();



/*
  rootElem = doc.RootElement();
  if (!rootElem) return false;
  typeElem = rootElem->FirstChildElement("Screen");
  if (!typeElem) return false;
  Logger::log().info() << typeElem->Value() << " = "  << typeElem->Attribute("Item");
  sectElem = typeElem->FirstChildElement("Dimension");
  Logger::log().info() << "Value: " << sectElem ->Value() << " = "  << sectElem->Attribute("Relative");



  sectElem = sectElem->NextSiblingElement();

  Logger::log().info() << "XML-Section " << "Value: "     << sectElem->Value();
  sectElem = sectElem->NextSiblingElement();
*/


/*
  Logger::log().info() << "XML " << "Attribute: " << screen->Attribute("Test", &intValue);
  Logger::log().info() << "XML " << "2 " << intValue;
*/

/*
  child = screen->FirstChildElement("Test");
  Logger::log().info() << "XML " << "Value: " << child->Value();
  Logger::log().info() << "XML " << "Attribute: " << child->Attribute("Test", &intValue);
//  Logger::log().info() << child->Attribute("RelativePosition", &boolValue);
  Logger::log().info() << "XML " << "2 " << intValue;
*/

/*
  Logger::log().info() << child->Attribute("RelativePosition", &boolValue);
  Logger::log().info() << "XML " << "2 " << boolValue;
  Logger::log().info() << "XML " << "3 " << true;
*/
//  Logger::log().info() << child->Attribute("Test", &intValue);
//  Logger::log().info() << "XML " << "2 " << intValue;



//  screen = child->FirstChildElement("RelativePosition");
//  Logger::log().info() << "XML " << "Value: " << screen->Value();



//        for (TiXmlElement* smElem = mSubmeshesNode->FirstChildElement();
//            smElem != 0; smElem = smElem->NextSiblingElement())





  Logger::log().headline("Init Dialog-System");
  mLoginOverlay       = OverlayManager::getSingleton().getByName("DialogOverlay");
  mPanelPlayerName    = OverlayManager::getSingleton().getOverlayElement("Dialog/Login/Playername");
  mPanelPlayerPasswd  = OverlayManager::getSingleton().getOverlayElement("Dialog/Login/Password");
  mPanelPlayerRePasswd= OverlayManager::getSingleton().getOverlayElement("Dialog/Login/RePassword");
  mElementSelectionBar= OverlayManager::getSingleton().getOverlayElement("Dialog/MetaSelect/select");
  mPlayerName         = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("Dialog/Login/Playername/Text"));
  mPlayerPasswd       = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("Dialog/Login/Password/Text"));
  mPlayerRePasswd     = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("Dialog/Login/RePassword/Text"));
  mDialogSelPanel     = static_cast<OverlayContainer*>(OverlayManager::getSingleton().getOverlayElement("Dialog/MetaSelect/Back"));
  mDialogInfoPanel    = static_cast<OverlayContainer*>(OverlayManager::getSingleton().getOverlayElement("Dialog/Info/Panel"));

  // Selection textAreas.
  std::string name= "Dialog/Text/";
  for (unsigned int i=0; i < DIALOG_TXT_LINES; i++)
  {
    mElementLine[i]= OverlayManager::getSingleton().
                     cloneOverlayElementFromTemplate("Dialog/TextRow",name+"Line_"+ StringConverter::toString(i));
    mElementLine[i]->setTop(i*12+1);
    mDialogSelPanel->addChild(mElementLine[i]);
  }
  // Info textAreas.
  name= "Dialog/Info/";
  for (unsigned int j=0; j < DIALOG_INFO_LINES; j++)
  {
    mElementInfo[j]= OverlayManager::getSingleton().
                     cloneOverlayElementFromTemplate("Dialog/TextRow",name+"Line_"+ StringConverter::toString(j));
    mElementInfo[j]->setTop(j*12+1);
    mElementInfo[j]->setCaption(name+"Line_"+ StringConverter::toString(j));
    mDialogInfoPanel->addChild(mElementInfo[j]);
  }





  // mPlayerName->setSpaceWidth(11.5);
  Logger::log().info() << "hier: " << mPlayerName->getSpaceWidth();
  //  static_cast<TextAreaOverlayElement*>(mPanelPlayerName)->setSpaceWidth(0.0);
  //  mPlayerName->CmdSpaceWidth::doSet(mPlayerName, "|");
  //  static_cast<TextAreaOverlayElement*>(mPlayerName)->setSpaceWidth(0.0);








  setVisible(false);
  return true;
}

//=================================================================================================
// Show/Hide the Overlay.
//=================================================================================================
void Dialog::setVisible(bool vis)
{
  if (vis == mVisible)
  {
    return;
  }
  if (vis == true)
  {
    mPanelPlayerName    ->hide();
    mPanelPlayerPasswd  ->hide();
    mPanelPlayerRePasswd->hide();
    mLoginOverlay       ->show();
  }
  else
  {
    mLoginOverlay->hide();
  }
  mVisible = vis;
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
  if (pos > DIALOG_TXT_LINES)
  {
    pos = DIALOG_TXT_LINES;
  }
  mElementLine[pos]->setCaption(text);
  mElementLine[pos]->setColour(color);
}

//=================================================================================================
// Fill the textarea of Info text.
//=================================================================================================
void Dialog::setInfoText(unsigned int pos, const char *text, ColourValue color)
{
  if (pos > DIALOG_INFO_LINES)
  {
    pos = DIALOG_INFO_LINES;
  }
  mElementInfo[pos]->setCaption(text);
  mElementInfo[pos]->setColour(color);
}

//=================================================================================================
// Clear all selectable text.
//=================================================================================================


//=================================================================================================
// Clear all Info text.
//=================================================================================================
void Dialog::clearInfoText()
{
  for (unsigned int i=0; i < DIALOG_INFO_LINES; ++i)
  {
    mElementInfo[i]->setCaption("");
  }
}

//=================================================================================================
// Login Overlay.
//=================================================================================================
void Dialog::UpdateLogin(unsigned int stage)
{
  static std::string mStrPlayerName, mStrPassword, mStrRePasswd;
  switch(stage)
  {
    case DIALOG_STAGE_LOGIN_GET_NAME:
      mPanelPlayerName    ->show();
      mPanelPlayerPasswd  ->hide();
      mPanelPlayerRePasswd->hide();
      mDialogSelPanel     ->hide();
      mStrPlayerName = TextInput::getSingleton().getText(true);
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

//=================================================================================================
// Login Overlay.
//=================================================================================================
bool Dialog::UpdateNewChar()
{

  return true; // Character was build, send it to server.
}
