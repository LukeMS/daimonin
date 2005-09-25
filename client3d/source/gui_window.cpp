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
#include "gui_gadget.h"
#include "gui_textout.h"
#include "gui_manager.h"
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
unsigned int GuiWindow::msInstanceNr =100;


///=================================================================================================
/// Delete a gadget.
///=================================================================================================
void GuiWindow::delGadget(int pos)
{
  for (vector<GuiGadget*>::iterator i = mvGadget.begin(); i < mvGadget.end(); ++i)
  {
    if (!pos--)
    {
      // Logger::log().info() << "deleting: " << (*i)->getName();
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
  for (unsigned int i = 0; i < mvGadget.size(); ++i)
  {
    delete mvGadget[i];
  }
  mvGadget.clear();
  mTexture.setNull();
}

///=================================================================================================
/// Build a window out of a xml description file.
///=================================================================================================
GuiWindow::GuiWindow(TiXmlElement *xmlElem, GuiManager *guiManager)
{
  mGuiManager= guiManager;
  mGuiManager->getScreenDimension(mScreenWidth, mScreenHeight);
  mSrcPixelBox = mGuiManager->getTilesetPixelBox();
  mMouseDragging = -1;
  mMousePressed  = -1;
  mMouseOver     = -1;
  parseWindowData(xmlElem);
  createWindow();
  drawAll();
}

///=================================================================================================
/// .
///=================================================================================================
void GuiWindow::parseWindowData(TiXmlElement *xmlRoot)
{
  TiXmlElement *xmlElem;
  const char *valString;
  std::string strTmp;
  mStrName = xmlRoot->Attribute("ID");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Coordinates type.
  /////////////////////////////////////////////////////////////////////////
  mSizeRelative = false;
  if ((valString = xmlRoot->Attribute("RelativeCoords")))
  {
    if (!stricmp(valString, "true"))  mSizeRelative = true;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Dragging entries.
  /////////////////////////////////////////////////////////////////////////
  // TODO !!!!
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Position entries.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlElem = xmlRoot->FirstChildElement("Pos")))
  {
    mPosX = atoi(xmlElem->Attribute("X"));
    mPosY = atoi(xmlElem->Attribute("Y"));
    mPosZ = atoi(xmlElem->Attribute("Z-Order"));
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Size entries.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlElem = xmlRoot->FirstChildElement("Size")))
  {
    mWidth  = atoi(xmlElem->Attribute("Width"));
    if (mWidth  < MIN_GFX_SIZE) mWidth  = MIN_GFX_SIZE;
    mHeight = atoi(xmlElem->Attribute("Height"));
    if (mHeight < MIN_GFX_SIZE) mHeight = MIN_GFX_SIZE;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Tooltip entries.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlElem = xmlRoot->FirstChildElement("Tooltip")))
  {
    mStrTooltip = xmlElem->Attribute("Text");
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gadgets.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Gadget"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Gadget"))
  {
    /// Find the gfx data in the tileset.
    GuiManager::mSrcEntry *srcEntry = mGuiManager->getStateGfxPositions(xmlElem->Attribute("ID"));
    if (!srcEntry) continue;
    GuiGadget *gadget = new GuiGadget(xmlElem);
    gadget->setSize(srcEntry->width, srcEntry->height);
    for (unsigned int i = 0; i < srcEntry->state.size(); ++i)
    {
      gadget->setStateImagePos(srcEntry->state[i]->name, srcEntry->state[i]->x, srcEntry->state[i]->y);
    }
    mvGadget.push_back(gadget);
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the graphics.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Graphic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Graphic"))
  {
    /// Find the gfx data in the tileset.
    GuiManager::mSrcEntry *srcEntry = mGuiManager->getStateGfxPositions(xmlElem->Attribute("ID"));
    GuiGraphic *graphic = new GuiGraphic(xmlElem);
    if (srcEntry)
    { /// This is a GFX_FILL.
      graphic->setSize(srcEntry->width, srcEntry->height);
      for (unsigned int i = 0; i < srcEntry->state.size(); ++i)
      {
        graphic->setStateImagePos(srcEntry->state[i]->name, srcEntry->state[i]->x, srcEntry->state[i]->y);
      }
    }
    mvGraphic.push_back(graphic);
  }

  /////////////////////////////////////////////////////////////////////////
  /// Parse the TextOutput.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("TextLine"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextLine"))
  {
    _textLine* TextLine = new  _textLine;
    TextLine->x = atoi(xmlElem->Attribute("X"));
    TextLine->y = atoi(xmlElem->Attribute("Y"));
    TextLine->size = atoi(xmlElem->Attribute("Size"));
    TextLine->text = xmlElem->Attribute("Text");
    mvTextline.push_back(TextLine);
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the TextValue.
  /////////////////////////////////////////////////////////////////////////
  // TODO
}

///=================================================================================================
/// Create the window and delete all gadgets that needs only to be drawn once.
///=================================================================================================
void GuiWindow::createWindow()
{
  mThisWindowNr = msInstanceNr;
  std::string strNum = StringConverter::toString(++msInstanceNr);

  mTexture = TextureManager::getSingleton().createManual("GUI_Texture_" + strNum, "General",
             TEX_TYPE_2D, mWidth, mHeight, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
  Overlay *overlay = OverlayManager::getSingleton().create("GUI_Overlay_"+strNum);
  overlay->setZOrder(msInstanceNr);
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
  overlay->add2D(static_cast<OverlayContainer*>(mElement));
  overlay->show();
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
  // Draw the background.
  for (unsigned int i = 0; i < mvGraphic.size(); ++i)
    mvGraphic[i]->draw(mSrcPixelBox, mTexture.getPointer());
  // Draw Text.
  for (unsigned int i = 0; i < mvTextline.size() ; ++i)
  {
    GuiTextout::getSingleton().Print(mvTextline[i]->x, mvTextline[i]->y, mTexture.getPointer(), mvTextline[i]->text.c_str(), COLOR_BLACK);
    GuiTextout::getSingleton().Print(mvTextline[i]->x, mvTextline[i]->y, mTexture.getPointer(), mvTextline[i]->text.c_str(), COLOR_WHITE);
  }
  //   DrawGadgets.
  for (unsigned int i = 0; i < mvGadget.size() ; ++i)
    mvGadget [i]->draw(mSrcPixelBox, mTexture.getPointer());
}

///=================================================================================================
/// .
///=================================================================================================
const char *GuiWindow::mouseEvent(int MouseAction, Real rx, Real ry)
{
  int x = (int) (rx * mScreenWidth - mPosX);
  int y = (int) (ry * mScreenHeight- mPosY);

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
      break;

    case M_RELEASED:
      ////////////////////////////////////////////////////////////
      /// End dragging.
      ////////////////////////////////////////////////////////////
      if (mMouseDragging >= 0)
      {
        mMouseDragging = -1;
      }
      ////////////////////////////////////////////////////////////
      /// Gadget pressed?
      ////////////////////////////////////////////////////////////
      else if (mMousePressed >= 0)
      {
        gadget = getGadgetMouseIsOver(x, y);
        if (gadget >=0 && gadget == mMousePressed)
        {
          //          actGadgetName = mvSrcEntry[mvGadget[gadget]->getTilsetPos()]->name.c_str();
          mvGadget[mMousePressed]->setState(STATE_STANDARD);
          mvGadget[mMousePressed]->draw(mSrcPixelBox, mTexture.getPointer());
        }
      }
      mMousePressed = -1;
      break;

    case M_MOVED:
      if (mMouseDragging >= 0)
      {
        // give back position change
        break;
      }
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
  return actGadgetName;
}

///=================================================================================================
/// JUST FOR TESTING.
///=================================================================================================
void GuiWindow::keyEvent(int , int , int , int )
{}
