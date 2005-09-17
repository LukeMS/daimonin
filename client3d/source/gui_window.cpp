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
/// Add a gagdet to the window.
///=================================================================================================
void GuiWindow::addGadget()
{
  GuiGadget *gadget = new GuiGadget(mStrXMLFile.c_str(), mvGadget.size());
  mvGadget.push_back(gadget);
}

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
/// Read the gfx-data for the given gadget.
///=================================================================================================
bool GuiWindow::parseImagesetData()
{
  /////////////////////////////////////////////////////////////////////////
  /// Check for a working description file.
  /////////////////////////////////////////////////////////////////////////
  TiXmlElement *xmlRoot, *xmlElem, *xmlGadget;
  TiXmlDocument doc(mStrImageSet.c_str());
  if (!doc.LoadFile(mStrImageSet.c_str()) || !(xmlRoot = doc.RootElement()) || !xmlRoot->Attribute("File"))
  {
    Logger::log().error() << "XML-File '" << mStrImageSet << "' is broken or missing.";
    return false;
  }
  mStrImageSetGfxFile = xmlRoot->Attribute("File");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Font entries.
  /////////////////////////////////////////////////////////////////////////
  if (xmlRoot->Attribute("Font"))
  {
    mStrFont = xmlRoot->Attribute("Font");
  }
  else
  {
    Logger::log().error() << "No font entry found in '"<< mStrImageSetGfxFile<< "' Using default Font 'TrebuchetMSBold'.";
    mStrFont = "TrebuchetMSBold";
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gfx coordinates.
  /////////////////////////////////////////////////////////////////////////
  std::string strTmp;
  for (xmlElem = xmlRoot->FirstChildElement("Image"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Image"))
  {
    mSrcEntry *Entry = new mSrcEntry;
    if (!xmlElem->Attribute("ID"))  continue;
    Entry->name   = xmlElem->Attribute("ID");
    Entry->width  = atoi(xmlElem->Attribute("Width"));
    Entry->height = atoi(xmlElem->Attribute("Height"));
    for (int i=0; i < STATE_SUM; ++i) Entry->pos[i].x = -1;
    /////////////////////////////////////////////////////////////////////////
    /// Parse the Position entries.
    /////////////////////////////////////////////////////////////////////////
    for (xmlGadget = xmlElem->FirstChildElement("State"); xmlGadget; xmlGadget = xmlGadget->NextSiblingElement("State"))
    {
      if (!(xmlGadget->Attribute("ID"))) continue;
      Entry->drawAndForget = false; /// This Gadget needs event handling.
      strTmp = xmlGadget->Attribute("ID");
      int state = -1;
      if      (strTmp == "Standard") state = STATE_STANDARD;
      else if (strTmp == "Pushed"  ) state = STATE_PUSHED;
      else if (strTmp == "M_Over"  ) state = STATE_M_OVER;
      Entry->pos[state].x = atoi(xmlGadget->Attribute("posX"));
      Entry->pos[state].y = atoi(xmlGadget->Attribute("posY"));
      if (state < 0)
      {
        Logger::log().error() << "Gadget '" << Entry->name << "' has no 'Standard' State ID";
      }
    }
    // If a state does not exist clone the standard state.
    for (int i=1; i < STATE_SUM; ++i)
    {
      if (Entry->pos[i].x < 0)
      {
        Entry->pos[i].x = Entry->pos[STATE_STANDARD].x;
        Entry->pos[i].y = Entry->pos[STATE_STANDARD].y;
      }
    }
    mvSrcEntry.push_back(Entry);
  }
  return true;
}

///=================================================================================================
/// Build a window out of a xml description file.
///=================================================================================================
GuiWindow::GuiWindow(unsigned int w, unsigned int h, const char* windowFile)
{
  mScreenWidth   = w;
  mScreenHeight  = h;
  mMouseDragging = -1;
  mMousePressed  = -1;
  mMouseOver     = -1;
  parseWindowData(windowFile);
  createWindow();
}

///=================================================================================================
/// .
///=================================================================================================
void GuiWindow::parseWindowData(const char *windowFile)
{
  TiXmlElement *xmlRoot, *xmlElem;
  TiXmlDocument doc(windowFile);
  std::string strValue;
  mStrXMLFile = windowFile;
  /////////////////////////////////////////////////////////////////////////
  /// Check for a working window description.
  /////////////////////////////////////////////////////////////////////////
  if ( !doc.LoadFile(windowFile) || !(xmlRoot = doc.RootElement()) )
  {
    Logger::log().error() << "XML-File '" << windowFile << "' is missing or broken.";
    return;
  }
  if (xmlRoot->Attribute("ID"))
    Logger::log().info() << "Parsing '" << xmlRoot->Attribute("ID") << "' in file" << windowFile << ".";
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Coordinates type.
  /////////////////////////////////////////////////////////////////////////
  mSizeRelative = false;
  if (xmlRoot->Attribute("RelativeCoords"))
  {
    strValue = xmlRoot->Attribute("RelativeCoords");
    if (strValue == "true" || strValue == "True" || strValue == "TRUE")  mSizeRelative = true;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the ImageSet.
  /////////////////////////////////////////////////////////////////////////
  xmlElem = xmlRoot->FirstChildElement("ImageSet");
  if (!xmlElem || !xmlElem->Attribute("File"))
  {
    Logger::log().error() << "Can't find an ImageSet in '" << windowFile << "'.";
    return;
  }
  mStrImageSet = xmlElem->Attribute("File");
  parseImagesetData();
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Dragging entries.
  /////////////////////////////////////////////////////////////////////////
  // TODO !!!!
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Position entries.
  /////////////////////////////////////////////////////////////////////////
  xmlElem = xmlRoot->FirstChildElement("Pos");
  if (xmlElem)
  {
    /*
         if (mPosRelative)
        {
          // TODO !!
        }
        else
    */
    {
      mPosX = StringConverter::parseInt(xmlElem->Attribute("X"));
      mPosY = StringConverter::parseInt(xmlElem->Attribute("Y"));
      mPosZ = StringConverter::parseInt(xmlElem->Attribute("Z-Order"));
    }
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Size entries.
  /////////////////////////////////////////////////////////////////////////
  xmlElem = xmlRoot->FirstChildElement("Size");
  if (xmlElem)
  {
    if (mSizeRelative)
    {
      // TODO !!
    }
    else
    {
      mWidth  = (int) StringConverter::parseInt(xmlElem->Attribute("Width"));
      if (mWidth  < MIN_GFX_SIZE) mWidth  = MIN_GFX_SIZE;
      mHeight = (int) StringConverter::parseInt(xmlElem->Attribute("Height"));
      if (mHeight < MIN_GFX_SIZE) mHeight = MIN_GFX_SIZE;
    }
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the Tooltip entries.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlElem = xmlRoot->FirstChildElement("Tooltip"))) mStrTooltip = xmlElem->Attribute("Text");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gadgets.
  /////////////////////////////////////////////////////////////////////////
  for (xmlElem = xmlRoot->FirstChildElement("Gadget"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Gadget"))
  {
    if (xmlElem->Attribute("ID")) addGadget();
    /// Find the pos in the inmageSet for this gadget.
    for (unsigned int j = 0; j < mvSrcEntry.size(); ++j)
    {
      if (mvGadget[mvGadget.size()-1]->getName() != mvSrcEntry[j]->name) continue;
      mvGadget[mvGadget.size()-1]->setTilsetPos(j);
      break;
    }
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the TextOutput.
  /////////////////////////////////////////////////////////////////////////
  // TODO
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
  // Texture is always a power of 2. to if the overlay size is not a power of 2 we need to scale.
  mRatioW = (Real)mTexture->getWidth() / (Real)mWidth;
  mRatioH = (Real)mTexture->getHeight()/ (Real)mHeight;



  Overlay *overlay = OverlayManager::getSingleton().create("GUI_Overlay_"+strNum);
  overlay->setZOrder(msInstanceNr);
  mElement = OverlayManager::getSingleton().createOverlayElement (OVERLAY_TYPE_NAME, "GUI_Frame_" + strNum);
  mElement->setMetricsMode(GMM_PIXELS);
  mElement->setDimensions (mWidth, mHeight);
  mElement->setPosition(mPosX, mPosY);

  MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("GUI/Window");
  mMaterial = tmpMaterial->clone("GUI_Material_"+ strNum);
  mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("GUI_Texture_" + strNum);
  mMaterial->load();
  mElement->setMaterialName("GUI_Material_"+ strNum);
  Logger::log().info() << "Used ImageSet:" << mStrImageSetGfxFile;
  mTileImage.load(mStrImageSetGfxFile, "General");
  mSrcPixelBox = mTileImage.getPixelBox();
  overlay->add2D(static_cast<OverlayContainer*>(mElement));
  overlay->show();

  unsigned int x1, x2, y1, y2, i;
  /////////////////////////////////////////////////////////////////////////
  /// Draw Background.
  /////////////////////////////////////////////////////////////////////////
  PixelBox src;
  bool dirty = true;
  for (unsigned int j = 0; j < mvGadget.size(); ++j)
  {
    if (mvGadget[j]->getName() == XML_BACKGROUND)
    {
      i = mvGadget[j]->getTilsetPos();
      int sumX = (mWidth-1)  / mvSrcEntry[i]->width;
      int sumY = (mHeight-1) / mvSrcEntry[i]->height;
      y1 = 0; y2 = mvSrcEntry[i]->height;
      for (int y = 0; y < sumY+1; ++y)
      {
        if (dirty)
        {
          src = mSrcPixelBox.getSubVolume(Box(
                                            mvSrcEntry[i]->pos[0].x,
                                            mvSrcEntry[i]->pos[0].y,
                                            mvSrcEntry[i]->pos[0].x + mvSrcEntry[i]->width,
                                            mvSrcEntry[i]->pos[0].y + mvSrcEntry[i]->height));
          dirty = false;
        }
        if (y2 > mHeight)
        {
          y2 = mHeight;
          if (y1 > mHeight) y1 = mHeight-1;
          dirty = true;
        }
        x1 = 0; x2 = mvSrcEntry[i]->width;
        for (int x = 0; x < sumX+1; ++x)
        {
          if (x2 > mWidth)
          {
            x2 = mWidth;
            if (x1 > mWidth) x1 = mWidth-1;
            dirty = true;
          }
          if (dirty)
          {
            src = mSrcPixelBox.getSubVolume(Box(
                                              mvSrcEntry[i]->pos[0].x,
                                              mvSrcEntry[i]->pos[0].y,
                                              mvSrcEntry[i]->pos[0].x + x2-x1,
                                              mvSrcEntry[i]->pos[0].y + y2-y1));
          }
          mTexture->getBuffer()->blitFromMemory(src, Box(
                                                  (int)(x1*mRatioW),
                                                  (int)(y1*mRatioH),
                                                  (int)(x2*mRatioW),
                                                  (int)(y2*mRatioH)));
          x1 = x2;
          x2+= mvSrcEntry[i]->width;
        }
        y1 = y2;
        y2+= mvSrcEntry[i]->height;
      }
      delGadget(j);
    }
  }
  drawAll();
}


///=================================================================================================
/// .
///=================================================================================================
void GuiWindow::drawGadget(int gadgetNr)
{
  static int x1, x2, y1, y2, srcPos, state;
  srcPos = mvGadget[gadgetNr]->getTilsetPos();
  state  = mvGadget[gadgetNr]->getState();
  x1 = mvSrcEntry[srcPos]->pos[state].x;
  x2 = mvSrcEntry[srcPos]->pos[state].x + mvSrcEntry[srcPos]->width;
  y1 = mvSrcEntry[srcPos]->pos[state].y;
  y2 = mvSrcEntry[srcPos]->pos[state].y + mvSrcEntry[srcPos]->height;
  PixelBox src = mSrcPixelBox.getSubVolume(Box(x1, y1, x2, y2));
  mvGadget[gadgetNr]->getPos(x1, y1, x2, y2);
  //Logger::log().info() << "dest: " << x1 << " "<< y1 << " "<< x2 << " "<< y2;
  mTexture->getBuffer()->blitFromMemory(src, Box(
                                          (int)(x1*mRatioW),
                                          (int)(y1*mRatioH),
                                          (int)(x2*mRatioW),
                                          (int)(y2*mRatioH)));
}

///=================================================================================================
/// .
///=================================================================================================
int GuiWindow::getGadgetMouseIsOver(int x, int y)
{
  for (unsigned int i = 0; i < mvGadget.size(); ++i)
  {
    if (mvGadget[i]->mouseOver(x-mPosX, y-mPosY)) return i;
  }
  return -1;
}


///=================================================================================================
/// .
///=================================================================================================
void GuiWindow::drawAll()
{
  for (unsigned int i = 0; i < mvGadget.size(); ++i) drawGadget(i);
}

///=================================================================================================
/// .
///=================================================================================================
const char *GuiWindow::mouseEvent(int MouseAction, Real x, Real y)
{
  x*= mScreenWidth;
  y*= mScreenHeight;
  int gadget;
  const char *actGadgetName =0;
  switch (MouseAction)
  {
    case M_PRESSED:
      if (mMouseOver >= 0)
      {
        mMousePressed = mMouseOver;
        mvGadget[mMousePressed]->setState(STATE_PUSHED);
        drawGadget(mMousePressed);
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
        gadget = getGadgetMouseIsOver((int)x, (int)y);
        if (gadget >=0 && gadget == mMousePressed)
        {
          Logger::log().info() << "button released: "<< mvSrcEntry[mvGadget[gadget]->getTilsetPos()]->name;
          actGadgetName = mvSrcEntry[mvGadget[gadget]->getTilsetPos()]->name.c_str();
          mvGadget[mMousePressed]->setState(STATE_STANDARD);
          drawGadget(mMousePressed);
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
      if (mMouseOver >= 0 && !mvGadget[mMouseOver]->mouseOver((int)x, (int)y))
      {
        mvGadget[mMouseOver]->setState(STATE_STANDARD);
        drawGadget(mMouseOver);
        mMouseOver = -1;
      }
      ////////////////////////////////////////////////////////////
      /// Is mouse over a gadget?
      ////////////////////////////////////////////////////////////
      if (mMousePressed < 0)
      {
        gadget = getGadgetMouseIsOver((int)x, (int)y);
        if (gadget >=0 && mvGadget[gadget]->setState(STATE_M_OVER))
        { // (If not already done) change the gadget state to mouseover.
          drawGadget(gadget);
          mMouseOver = gadget;
        }
      }
      else
      {
        gadget = getGadgetMouseIsOver((int)x, (int)y);
        if (gadget >=0 && mvGadget[gadget]->setState(STATE_PUSHED))
        { // (If not already done) change the gadget state to mouseover.
          drawGadget(gadget);
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
