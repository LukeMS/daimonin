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
#include "gui_cursor.h"
#include "gui_textout.h"
#include "gui_manager.h"
#include "gui_gadget.h"
#include "gui_listbox.h"
#include "gui_statusbar.h"
#include "option.h"
#include "logger.h"
#include <Ogre.h>
//#include <OgreFontManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <tinyxml.h>

using namespace Ogre;

const int MIN_GFX_SIZE = 4;
const char XML_BACKGROUND[] = "Background";

///=================================================================================================
/// Init all static Elemnts.
///=================================================================================================
int GuiWindow::msInstanceNr = -1;
int GuiWindow::mMouseDragging = -1;
GuiManager *GuiWindow::mGuiManager = NULL;
std::string GuiWindow::mStrTooltip ="";

///=================================================================================================
/// Constructor.
///=================================================================================================
GuiWindow::GuiWindow()
{
}

///=================================================================================================
/// Destructor.
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
  // Set all shared pointer to null.
  mMaterial.setNull();
  mTexture.setNull();
}

///=================================================================================================
/// Build a window out of a xml description file.
///=================================================================================================
void GuiWindow::Init(TiXmlElement *xmlElem, GuiManager *guiManager)
{
  if (!mGuiManager) mGuiManager = guiManager;
  mSrcPixelBox = guiManager->getTilesetPixelBox();
  mMousePressed  = -1;
  mMouseOver     = -1;
  parseWindowData(xmlElem);
  createWindow();
  drawAll();
}

///=================================================================================================
/// Parse the xml window data..
///=================================================================================================
void GuiWindow::parseWindowData(TiXmlElement *xmlRoot)
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
  { // We will show tooltip only if mouse is over the moving area.
    mStrTooltip = xmlElem->Attribute("text");
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gadgets.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Gadget"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Gadget"))
  {
    /// Find the gfx data in the tileset.
    GuiManager::mSrcEntry *srcEntry = mGuiManager->getStateGfxPositions(xmlElem->Attribute("name"));
    if (srcEntry)
    {
      GuiGadget *gadget = new GuiGadget(xmlElem, srcEntry->width, srcEntry->height, mWidth, mHeight);
      for (unsigned int i = 0; i < srcEntry->state.size(); ++i)
      {
        gadget->setStateImagePos(srcEntry->state[i]->name, srcEntry->state[i]->x, srcEntry->state[i]->y);
      }
      mvGadget.push_back(gadget);
    }
    else
    {
      Logger::log().warning() << xmlElem->Attribute("name") << " was defined in '"
      << FILE_GUI_WINDOWS << "' but the gfx-data in '" << FILE_GUI_IMAGESET << "' is missing.";
    }
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
    if (!stricmp(xmlElem->Attribute("type"), "GFX_FILL"))
    { /// This is a GFX_FILL.
      /// Find the gfx data in the tileset.
      GuiManager::mSrcEntry *srcEntry = mGuiManager->getStateGfxPositions(xmlElem->Attribute("name"));
      if (srcEntry)
      {
        GuiGraphic *graphic = new GuiGraphic(xmlElem, srcEntry->width, srcEntry->height, mWidth, mHeight);
        for (unsigned int i = 0; i < srcEntry->state.size(); ++i)
        {
          graphic->setStateImagePos(srcEntry->state[i]->name, srcEntry->state[i]->x, srcEntry->state[i]->y);
        }
        mvGraphic.push_back(graphic);
      }
      else
      {
        Logger::log().warning() << xmlElem->Attribute("name") << " was defined in '"
        << FILE_GUI_WINDOWS << "' but the gfx-data in '" << FILE_GUI_IMAGESET << "' is missing.";
      }
    }
    else
    { /// This is a COLOR_FILL.
      GuiGraphic *graphic = new GuiGraphic(xmlElem, 0, 0, mWidth, mHeight);
      mvGraphic.push_back(graphic);
    }
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Label.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Label"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Label"))
  {
    TextLine *textline = new TextLine;
    textline->index = -1;
    textline->BG_Backup = 0;
    textline->x1   = atoi(xmlElem->Attribute("x"));
    textline->y1   = atoi(xmlElem->Attribute("y"));
    textline->font = atoi(xmlElem->Attribute("font"));
    textline->text = xmlElem->Attribute("text");
    mvTextline.push_back(textline);
  }

  /////////////////////////////////////////////////////////////////////////
  /// Parse the Textbox.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("TextBox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextBox"))
  {
    TextLine *textline = new TextLine;
    valString = xmlElem->Attribute("name");
    if (valString)
    {
      for (int i = 0; i < GUI_ELEMENTS_SUM; ++i)
      {
        if (GuiManager::GuiElementNames[i].name == valString)
        {
          textline->index = i;
          break;
        }
      }
    }
    else /// Error: No name found. Fallback to label.
    {
      Logger::log().error() << "A Textbox without a name was found.";
      textline->index = -1;
    }
    textline->BG_Backup = 0;
    textline->font = atoi(xmlElem->Attribute("font"));
    textline->x1   = atoi(xmlElem->Attribute("x"));
    textline->y1   = atoi(xmlElem->Attribute("y"));
    textline->width= atoi(xmlElem->Attribute("width"));
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
/// Create the window.
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
/// Returns the gadget under the mousepointer.
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
/// Draw all window elements.
///=================================================================================================
void GuiWindow::drawAll()
{
  /////////////////////////////////////////////////////////////////////////
  /// Draw the background.
  /////////////////////////////////////////////////////////////////////////
  for (unsigned int i = 0; i < mvGraphic.size(); ++i)
  {
    mvGraphic[i]->draw(mSrcPixelBox, mTexture.getPointer());
  }
  /////////////////////////////////////////////////////////////////////////
  /// Draw text.
  /////////////////////////////////////////////////////////////////////////
  for (unsigned int i = 0; i < mvTextline.size() ; ++i)
  {
    ///--------------------------------------------------------------------
    /// Clipping.
    ///--------------------------------------------------------------------
    if (mvTextline[i]->x1 >= (unsigned int) mWidth || mvTextline[i]->y1 >= (unsigned int) mHeight)
    {
      mvTextline[i]->clipped = true;
      continue;
    }
    mvTextline[i]->clipped = false;
    /// Calculate the needed gfx-buffer size for the text.
    mvTextline[i]->x2 = mvTextline[i]->x1 +1;
    mvTextline[i]->y2 = mvTextline[i]->y1 +1;
    GuiTextout::getSingleton().CalcTextSize(
      mvTextline[i]->x2,
      mvTextline[i]->y2,
      mWidth,
      mHeight,
      mvTextline[i]->text.c_str(),
      mvTextline[i]->font);
    /// Fill the BG_Backup buffer with Window background, before printing.
    if (mvTextline[i]->index >= 0)  // Dynamic text.
    {
      mvTextline[i]->x2 = mvTextline[i]->x1 + mvTextline[i]->width;
      if (mvTextline[i]->BG_Backup)  delete[] mvTextline[i]->BG_Backup;
      mvTextline[i]->BG_Backup = new uint32[(mvTextline[i]->x2- mvTextline[i]->x1) * (mvTextline[i]->y2- mvTextline[i]->y1)];
      mTexture.getPointer()->getBuffer()->blitToMemory(Box(
            mvTextline[i]->x1, mvTextline[i]->y1,
            mvTextline[i]->x2, mvTextline[i]->y2),
          PixelBox(
            mvTextline[i]->x2- mvTextline[i]->x1,
            mvTextline[i]->y2- mvTextline[i]->y1,
            1, PF_A8R8G8B8, mvTextline[i]->BG_Backup));
    }
    /// Print.
    GuiTextout::getSingleton().Print(mvTextline[i], mTexture.getPointer(), mvTextline[i]->text.c_str());
  }
  /////////////////////////////////////////////////////////////////////////
  /// Draw gadget.
  /////////////////////////////////////////////////////////////////////////
  for (unsigned int i = 0; i < mvGadget.size() ; ++i)
    mvGadget [i]->draw(mSrcPixelBox, mTexture.getPointer());
  /////////////////////////////////////////////////////////////////////////
  /// Draw statusbar.
  /////////////////////////////////////////////////////////////////////////
  for (unsigned int i = 0; i < mvStatusbar.size() ; ++i)
    mvStatusbar [i]->draw(mSrcPixelBox, mTexture.getPointer(), -1);
  /////////////////////////////////////////////////////////////////////////
  /// Draw listbox.
  /////////////////////////////////////////////////////////////////////////
  // not needed for text-listbox.
}

///=================================================================================================
/// Mouse Event.
///=================================================================================================
const char *GuiWindow::mouseEvent(int MouseAction, int rx, int ry)
{
  int x = rx - mPosX;
  int y = ry - mPosY;

  int gadget;
  const char *actGadgetName = NULL;
  switch (MouseAction)
  {
      //case M_RESIZE:
    case M_PRESSED:
      GuiCursor::getSingleton().setState(mSrcPixelBox, GuiCursor::STATE_BUTTON_DOWN);
      // Mouse over this window?
      if (rx >= mPosX && rx <= mPosX + mWidth && ry >= mPosY && ry <= mPosY + mHeight)
      {
        actGadgetName = mStrName.c_str();
      }
      // Mouse over a gaget?
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
      GuiCursor::getSingleton().setState(mSrcPixelBox, GuiCursor::STATE_STANDARD);
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

          if (!strcmp(actGadgetName, "Button_Close"))
          {
           mOverlay->hide(); // just testing.
           mGuiManager->getSingleton().setTooltip("");
          }
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
            mGuiManager->getSingleton().setTooltip("");
          }
        }
        ////////////////////////////////////////////////////////////
        /// Is mouse over a gadget?
        ////////////////////////////////////////////////////////////
        if (mMousePressed < 0)
        {
          gadget = getGadgetMouseIsOver(x, y);
          if (gadget >=0)
          {
            if ( mvGadget[gadget]->setState(STATE_M_OVER))
            {  // (If not already done) change the gadget state to mouseover.
              mvGadget[gadget]->draw(mSrcPixelBox, mTexture.getPointer());
              mMouseOver = gadget;
              //mStrTooltip = mvGadget[gadget]->getTooltip();
              mGuiManager->getSingleton().setTooltip(mvGadget[gadget]->getTooltip());
            }
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
        if (mvTextline[i]->index != element || mvTextline[i]->clipped) continue;
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
