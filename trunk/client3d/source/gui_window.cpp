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

#include "define.h"
#include "gui_window.h"
#include "gui_text.h"
#include "gui_manager.h"
#include "gui_gadget.h"
#include "gui_listbox.h"
#include "gui_statusbar.h"
#include "option.h"
#include "logger.h"
#include <Ogre.h>
#include <OgreFontManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <tinyxml.h>

using namespace Ogre;

const int MIN_GFX_SIZE = 4;
const char XML_BACKGROUND[] = "Background";

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
int GuiWindow::msInstanceNr = -1;
int GuiWindow::mMouseDragging = -1;

///=================================================================================================
/// Delete a gadget.
///=================================================================================================
void GuiWindow::delGadget(int pos)
{
  // Delete the gadgets.
  for (vector<GuiGadget*>::iterator i = mvGadget.begin(); i < mvGadget.end(); ++i)
  {
    if (!pos--)
    {
      delete (*i);
      mvGadget.erase(i);
      return;
    }
  }
}

///=================================================================================================
///
///=================================================================================================
GuiWindow::~GuiWindow()
{
  // Delete the gadgets.
  for (vector<GuiGadget*>::iterator i = mvGadget.begin(); i < mvGadget.end(); ++i)
  {
    delete (*i);
    mvGadget.erase(i);
  }
  mvGadget.clear();
  // Delete the listboxes.
  for (vector<GuiListbox*>::iterator i = mvListbox.begin(); i < mvListbox.end(); ++i)
  {
    delete (*i);
    mvListbox.erase(i);
  }
  mvListbox.clear();
  // Delete the graphics.
  for (vector<GuiGraphic*>::iterator i = mvGraphic.begin(); i < mvGraphic.end(); ++i)
  {
    delete (*i);
    mvGraphic.erase(i);
  }
  mvGraphic.clear();
  // Delete the textlines.
  for (vector<TextLine*>::iterator i = mvTextline.begin(); i < mvTextline.end(); ++i)
  {
    delete (*i);
    mvTextline.erase(i);
  }
  mvTextline.clear();

  mTexture.setNull();
}

///=================================================================================================
/// Build a window out of a xml description file.
///=================================================================================================
void GuiWindow::Init(TiXmlElement *xmlElem, GuiManager *guiManager)
{
  mSrcPixelBox = guiManager->getTilesetPixelBox();
  mMousePressed  = -1;
  mMouseOver     = -1;
  parseWindowData(xmlElem, guiManager);
  createWindow();
  drawAll();
}

///=================================================================================================
/// .
///=================================================================================================
void GuiWindow::parseWindowData(TiXmlElement *xmlRoot, GuiManager *guiManager)
{
  TiXmlElement *xmlElem;
  const char *valString;
  std::string strTmp;
  mStrName = xmlRoot->Attribute("name");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Coordinates type.
  /////////////////////////////////////////////////////////////////////////
  mSizeRelative = false;
  if ((valString = xmlRoot->Attribute("relativeCoords")))
  {
    if (!stricmp(valString, "true"))  mSizeRelative = true;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Position entries.
  /////////////////////////////////////////////////////////////////////////
  mPosX = mPosY = mPosZ = 100;
  if ((xmlElem = xmlRoot->FirstChildElement("Pos")))
  {
    mPosX = atoi(xmlElem->Attribute("x"));
    mPosY = atoi(xmlElem->Attribute("y"));
    mPosZ = atoi(xmlElem->Attribute("zOrder"));
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Size entries.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlElem = xmlRoot->FirstChildElement("Size")))
  {
    mWidth  = atoi(xmlElem->Attribute("width"));
    mHeight = atoi(xmlElem->Attribute("height"));
  }
  if (mWidth  < MIN_GFX_SIZE) mWidth  = MIN_GFX_SIZE;
  if (mHeight < MIN_GFX_SIZE) mHeight = MIN_GFX_SIZE;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Dragging entries.
  /////////////////////////////////////////////////////////////////////////
  mDragPosX1 = mDragPosX2 = mDragPosY1 = mDragPosY2 = -100;
  if ((xmlElem = xmlRoot->FirstChildElement("DragArea")))
  {
    mDragPosX1 = atoi(xmlElem->Attribute("x"));
    mDragPosY1 = atoi(xmlElem->Attribute("y"));
    mDragPosX2 = atoi(xmlElem->Attribute("width")) + mDragPosX1;
    mDragPosY2 = atoi(xmlElem->Attribute("height"))+ mDragPosY1;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Tooltip entries.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlElem = xmlRoot->FirstChildElement("Tooltip")))
  {
    mStrTooltip = xmlElem->Attribute("text");
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gadgets.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Gadget"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Gadget"))
  {
    /// Find the gfx data in the tileset.
    GuiManager::mSrcEntry *srcEntry = guiManager->getStateGfxPositions(xmlElem->Attribute("name"));
    if (!srcEntry) continue;
    GuiGadget *gadget = new GuiGadget(xmlElem, srcEntry->width, srcEntry->height, mWidth, mHeight);
    for (unsigned int i = 0; i < srcEntry->state.size(); ++i)
    {
      gadget->setStateImagePos(srcEntry->state[i]->name, srcEntry->state[i]->x, srcEntry->state[i]->y);
    }
    mvGadget.push_back(gadget);
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the listboxes.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Listbox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Listbox"))
  {
    valString = xmlElem->Attribute("name");
    GuiListbox *listbox = new GuiListbox(xmlElem, mWidth, mHeight);
    for (int i = 0; i < GUI_ELEMENTS_SUM; ++i)
    {
      if (GuiManager::GuiElementNames[i].name != valString) continue;
      listbox->setIndex(i);
      break;
    }
    mvListbox.push_back(listbox);
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the graphics.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Graphic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Graphic"))
  {
    /// Find the gfx data in the tileset.
    GuiManager::mSrcEntry *srcEntry = guiManager->getStateGfxPositions(xmlElem->Attribute("name"));
    if (srcEntry)
    { /// This is a GFX_FILL.
      GuiGraphic *graphic = new GuiGraphic(xmlElem, srcEntry->width, srcEntry->height, mWidth, mHeight);
      for (unsigned int i = 0; i < srcEntry->state.size(); ++i)
      {
        graphic->setStateImagePos(srcEntry->state[i]->name, srcEntry->state[i]->x, srcEntry->state[i]->y);
      }
      mvGraphic.push_back(graphic);
    }
    else
    {
      GuiGraphic *graphic = new GuiGraphic(xmlElem, 0, 0, mWidth, mHeight);
      mvGraphic.push_back(graphic);
    }
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the static TextOutput.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("TextStatic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextStatic"))
  {
    TextLine *textline = new TextLine;
    textline->BG_Backup =0;
    textline->x = atoi(xmlElem->Attribute("x"));
    textline->y = atoi(xmlElem->Attribute("y"));
    if (textline->x > mWidth - 2)  textline->x =  mWidth -2;
    if (textline->y > mHeight- 2)  textline->y  = mHeight-2;
    textline->width = atoi(xmlElem->Attribute("width"));
    if (textline->x + textline->width > mWidth) textline->width = mWidth - textline->x -1;
    textline->text = xmlElem->Attribute("text");
    mvTextline.push_back(textline);
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the dynamic TextOutput.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("TextDynamic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextDynamic"))
  {
    valString = xmlElem->Attribute("name");
    if (!valString)
    {
      Logger::log().error() << "DynamicText needs a name value!";
      return;
    }
    TextLine *textline = new TextLine;
    for (int i = 0; i < GUI_ELEMENTS_SUM; ++i)
    {
      if (GuiManager::GuiElementNames[i].name == valString)
      {
        textline->index = i;
        break;
      }
    }
    textline->BG_Backup = new uint32[GuiTextout::getSingleton().getMaxFontHeight() * mWidth];
    textline->x = atoi(xmlElem->Attribute("x"));
    textline->y = atoi(xmlElem->Attribute("y"));
    if (textline->x > mWidth - 2)  textline->x =  mWidth -2;
    if (textline->y > mHeight- 2)  textline->y  = mHeight-2;
    textline->width = atoi(xmlElem->Attribute("width"));
    if (textline->x + textline->width > mWidth) textline->width = mWidth - textline->x -1;
    textline->text = xmlElem->Attribute("text");
    mvTextline.push_back(textline);
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Statusbars.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Statusbar"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Statusbar"))
  {
    GuiStatusbar *statusbar = new GuiStatusbar(xmlElem, mWidth, mHeight);
    mvStatusbar.push_back(statusbar);
  }
}

///=================================================================================================
/// Create the window and delete all gadgets that needs only to be drawn once.
///=================================================================================================
void GuiWindow::createWindow()
{
  mWindowNr = ++msInstanceNr;
  std::string strNum = StringConverter::toString(msInstanceNr);
  mTexture = TextureManager::getSingleton().createManual("GUI_Texture_" + strNum, "General",
             TEX_TYPE_2D, mWidth, mHeight, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
  mOverlay = OverlayManager::getSingleton().create("GUI_Overlay_"+strNum);
  mOverlay->setZOrder(msInstanceNr);
  mElement = OverlayManager::getSingleton().createOverlayElement (OVERLAY_TYPE_NAME, "GUI_Frame_" + strNum);
  mElement->setMetricsMode(GMM_PIXELS);
  // Texture is always a power of 2. set this size also for the overlay.
  mElement->setDimensions (mTexture->getWidth(), mTexture->getHeight());
  mElement->setPosition(mPosX, mPosY);
  MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("GUI/Window");
  mMaterial = tmpMaterial->clone("GUI_Material_"+ strNum);
  mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("GUI_Texture_" + strNum);
  mMaterial->load();
  mElement->setMaterialName("GUI_Material_"+ strNum);
  mOverlay->add2D(static_cast<OverlayContainer*>(mElement));
  mOverlay->show();
  // If the window is smaller then the texture - we have to set the delta-size to transparent.
  PixelBox pb = mTexture->getBuffer()->lock(Box(0,0, mTexture->getWidth(), mTexture->getHeight()), HardwareBuffer::HBL_READ_ONLY );
  uint32 *dest_data = (uint32*)pb.data;
  for (unsigned int y = 0; y < mTexture->getWidth() * mTexture->getHeight(); ++y)  *dest_data++ = 0;
  mTexture->getBuffer()->unlock();
}

///=================================================================================================
/// .
///=================================================================================================
int GuiWindow::getGadgetMouseIsOver(int x, int y)
{
  for (unsigned int i = 0; i < mvGadget.size(); ++i)
  {
    if (mvGadget[i]->mouseOver(x, y)) return i;
  }
  return -1;
}

///=================================================================================================
/// .
///=================================================================================================
void GuiWindow::drawAll()
{
  // Todo delete static stuff after drawing (like bg-gfx, static text, etc).

  /////////////////////////////////////////////////////////////////////////
  /// Draw the background.
  /////////////////////////////////////////////////////////////////////////
  for (unsigned int i = 0; i < mvGraphic.size(); ++i)
    mvGraphic[i]->draw(mSrcPixelBox, mTexture.getPointer());
  /////////////////////////////////////////////////////////////////////////
  /// Draw text.
  /////////////////////////////////////////////////////////////////////////
  for (unsigned int i = 0; i < mvTextline.size() ; ++i)
  {
    // Fill the BG_Backup buffer with Window background, before printing.
    if (mvTextline[i]->BG_Backup)
    {
      PixelBox pb(mvTextline[i]->width, GuiTextout::getSingleton().getMaxFontHeight(), 1, PF_A8R8G8B8 , mvTextline[i]->BG_Backup);
      mTexture.getPointer()->getBuffer()->blitToMemory(Box(
            mvTextline[i]->x,
            mvTextline[i]->y,
            mvTextline[i]->x + mvTextline[i]->width,
            mvTextline[i]->y + GuiTextout::getSingleton().getMaxFontHeight()), pb);
    }
    GuiTextout::getSingleton().Print(mvTextline[i]->x, mvTextline[i]->y, mvTextline[i]->width, mTexture.getPointer(), mvTextline[i]->text.c_str());
    GuiTextout::getSingleton().Print(mvTextline[i]->x, mvTextline[i]->y, mvTextline[i]->width, mTexture.getPointer(), mvTextline[i]->text.c_str());
  }
  /////////////////////////////////////////////////////////////////////////
  /// Draw gadget.
  /////////////////////////////////////////////////////////////////////////
  for (unsigned int i = 0; i < mvGadget.size() ; ++i)
    mvGadget [i]->draw(mSrcPixelBox, mTexture.getPointer());
  /////////////////////////////////////////////////////////////////////////
  /// Draw statusbars.
  /////////////////////////////////////////////////////////////////////////
  for (unsigned int i = 0; i < mvStatusbar.size() ; ++i)
    mvStatusbar [i]->draw(mSrcPixelBox, mTexture.getPointer(), -1);
  /////////////////////////////////////////////////////////////////////////
  /// Draw listbox.
  /////////////////////////////////////////////////////////////////////////
  // not needed for text-listbox.
}

///=================================================================================================
/// .
///=================================================================================================
const char *GuiWindow::mouseEvent(int MouseAction, int rx, int ry)
{
  int x = rx - mPosX;
  int y = ry - mPosY;

  int gadget;
  const char *actGadgetName =0;
  switch (MouseAction)
  {
    case M_PRESSED:
      if (mMouseOver >= 0)
      {
        mMousePressed = mMouseOver;
        mvGadget[mMousePressed]->setState(STATE_PUSHED);
        mvGadget[mMousePressed]->draw(mSrcPixelBox, mTexture.getPointer());
        return mvGadget[mMousePressed]->getName();
      }
      else if (x > mDragPosX1 && x < mDragPosX2 && y > mDragPosY1 && y < mDragPosY2)
      {
        mDragOldMousePosX = rx;
        mDragOldMousePosY = ry;
        mMouseDragging = mWindowNr;
      }
      break;

    case M_RELEASED:
      ////////////////////////////////////////////////////////////
      /// Gadget pressed?
      ////////////////////////////////////////////////////////////
      if (mMousePressed >= 0)
      {
        gadget = getGadgetMouseIsOver(x, y);
        if (gadget >=0 && gadget == mMousePressed)
        {
          //actGadgetName = mvSrcEntry[mvGadget[gadget]->getTilsetPos()]->name.c_str();
          mvGadget[mMousePressed]->setState(STATE_STANDARD);
          mvGadget[mMousePressed]->draw(mSrcPixelBox, mTexture.getPointer());
          actGadgetName = mvGadget[mMousePressed]->getName();


          if (!strcmp(actGadgetName, "Button_Close")) mOverlay->hide(); // just testing.


        }
      }
      mMousePressed = -1;
      mMouseDragging= -1;
      break;

    case M_MOVED:
      ////////////////////////////////////////////////////////////
      /// Dragging.
      ////////////////////////////////////////////////////////////
      if (mMouseDragging == mWindowNr)
      {
        mPosX-= mDragOldMousePosX - rx;
        mPosY-= mDragOldMousePosY - ry;
        mDragOldMousePosX = rx;
        mDragOldMousePosY = ry;
        mElement->setPosition(mPosX, mPosY);
      }
      else if (mMouseDragging < 0)
      {
        ////////////////////////////////////////////////////////////
        /// Is the mouse still over this gadget?
        ////////////////////////////////////////////////////////////
        if (mMouseOver >= 0 && mvGadget[mMouseOver]->mouseOver(x, y) == false)
        {
          if (mvGadget[mMouseOver]->setState(STATE_STANDARD))
          {
            mvGadget[mMouseOver]->draw(mSrcPixelBox, mTexture.getPointer());
            mMouseOver = -1;
          }
        }
        ////////////////////////////////////////////////////////////
        /// Is mouse over a gadget?
        ////////////////////////////////////////////////////////////
        if (mMousePressed < 0)
        {
          gadget = getGadgetMouseIsOver(x, y);
          if (gadget >=0 && mvGadget[gadget]->setState(STATE_M_OVER))
          {  // (If not already done) change the gadget state to mouseover.
            mvGadget[gadget]->draw(mSrcPixelBox, mTexture.getPointer());
            mMouseOver = gadget;
          }
        }
        else
        {
          gadget = getGadgetMouseIsOver(x, y);
          if (gadget >=0 && mvGadget[gadget]->setState(STATE_PUSHED))
          { // (If not already done) change the gadget state to mouseover.
            mvGadget[gadget]->draw(mSrcPixelBox, mTexture.getPointer());
            mMouseOver = gadget;
          }
        }
        break;
      }
  }
  return actGadgetName;
}

///=================================================================================================
/// Parse a message.
///=================================================================================================
void GuiWindow::Message(int message, int element, const char *value)
{
  switch (message)
  {
    case GUI_MSG_ADD_TEXTLINE:
      for (unsigned int i = 0; i < mvListbox.size() ; ++i)
      {
        if (mvListbox[i]->getIndex() != element) continue;
        mvListbox[i]->addTextline(value);
        break;
      }
      break;

    case GUI_MSG_TXT_CHANGED:
      for (unsigned int i = 0; i < mvTextline.size() ; ++i)
      {
        if (mvTextline[i]->index != element) continue;
        GuiTextout::getSingleton().Print(mvTextline[i], mTexture.getPointer(), value);
        break;
      }
      break;

    default:
      break;
  }
}

///=================================================================================================
/// .
///=================================================================================================
void GuiWindow::updateDragAnimation()
{
  ;// "zurückflutschen" bei falschem drag.
}

///=================================================================================================
/// .
///=================================================================================================
void GuiWindow::update2DAnimaton()
{
	;
}

///=================================================================================================
/// .
///=================================================================================================
void GuiWindow::updateListbox()
{
  for (vector<GuiListbox*>::iterator i = mvListbox.begin(); i < mvListbox.end(); ++i)
  {
    (*i)->update( mTexture.getPointer());
  }
}
