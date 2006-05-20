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
#include "gui_imageset.h"
#include "gui_listbox.h"
#include "gui_statusbar.h"
#include "option.h"
#include "sound.h"
#include "events.h"
#include "logger.h"
#include <Ogre.h>
#include <OgreHardwarePixelBuffer.h>
#include <tinyxml.h>

using namespace Ogre;

const int MIN_GFX_SIZE = 4;
const char XML_BACKGROUND[] = "Background";

///================================================================================================
/// Init all static Elemnts.
///================================================================================================
int GuiWindow::msInstanceNr = -1;
int GuiWindow::mMouseDragging = -1;
std::string GuiWindow::mStrTooltip ="";

///================================================================================================
/// Constructor.
///================================================================================================
GuiWindow::GuiWindow()
{
    isInit = false;
}

///================================================================================================
/// Destructor.
///================================================================================================
void GuiWindow::freeRecources()
{
    // Delete the gadgets.
    for (vector<GuiGadget*>::iterator i = mvGadget.begin(); i < mvGadget.end(); ++i)
    {
        delete (*i)
        ;
    }
    mvGadget.clear();
    // Delete the listboxes.
    for (vector<GuiListbox*>::iterator i = mvListbox.begin(); i < mvListbox.end(); ++i)
    {
        delete (*i)
        ;
    }
    mvListbox.clear();
    // Delete the graphics.
    for (vector<GuiGraphic*>::iterator i = mvGraphic.begin(); i < mvGraphic.end(); ++i)
    {
        delete (*i)
        ;
    }
    mvGraphic.clear();
    // Delete the textlines.
    for (vector<TextLine*>::iterator i = mvTextline.begin(); i < mvTextline.end(); ++i)
    {
        if ((*i)->
                index >= 0) delete[] (*i)->BG_Backup;
        delete (*i);
    }
    mvTextline.clear();
    // Delete the statusbars.
    for (vector<GuiStatusbar*>::iterator i = mvStatusbar.begin(); i < mvStatusbar.end(); ++i)
    {
        delete (*i);
    }
    mvStatusbar.clear();
    // Set all shared pointer to null.
    mMaterial.setNull();
    mTexture.setNull();
}

///================================================================================================
/// Build a window out of a xml description file.
///================================================================================================
void GuiWindow::Init(TiXmlElement *xmlElem)
{
    mSrcPixelBox = GuiImageset::getSingleton().getPixelBox();
    mMousePressed  = -1;
    mMouseOver     = -1;
    mSpeakAnimState= 0;
    parseWindowData(xmlElem);
    createWindow();
    drawAll();
    isInit = true;
}

///================================================================================================
/// Parse the xml window data..
///================================================================================================
void GuiWindow::parseWindowData(TiXmlElement *xmlRoot)
{
    TiXmlElement *xmlElem;
    const char *strTmp;

    if ((strTmp = xmlRoot->Attribute("name")))
        mStrName = strTmp;
    Logger::log().info () << "Parsing window: " << mStrName;
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Coordinates type.
    /// ////////////////////////////////////////////////////////////////////
    mSizeRelative = false;
    if ((strTmp = xmlRoot->Attribute("relativeCoords")))
    {
        if (!stricmp(strTmp, "true"))
            mSizeRelative = true;
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Position entries.
    /// ////////////////////////////////////////////////////////////////////
    mPosX = mPosY = mPosZ = 100;
    if ((xmlElem = xmlRoot->FirstChildElement("Pos")))
    {
        if ((strTmp = xmlElem->Attribute("x")))
            mPosX = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("y")))
            mPosY = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("zOrder")))
            mPosZ = atoi(strTmp);
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Size entries.
    /// ////////////////////////////////////////////////////////////////////
    if ((xmlElem = xmlRoot->FirstChildElement("Size")))
    {
        if ((strTmp = xmlElem->Attribute("width")))
            mWidth = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("height")))
            mHeight = atoi(strTmp);
    }
    if (mWidth  < MIN_GFX_SIZE)
        mWidth  = MIN_GFX_SIZE;
    if (mHeight < MIN_GFX_SIZE)
        mHeight = MIN_GFX_SIZE;
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Dragging entries.
    /// ////////////////////////////////////////////////////////////////////
    mDragPosX1 = mDragPosX2 = mDragPosY1 = mDragPosY2 = -100;
    if ((xmlElem = xmlRoot->FirstChildElement("DragArea")))
    {
        if ((strTmp = xmlElem->Attribute("x")))
            mDragPosX1 = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("y")))
            mDragPosY1 = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("width")))
            mDragPosX2 = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("height")))
            mDragPosY2 = atoi(strTmp);
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Tooltip entries.
    /// ////////////////////////////////////////////////////////////////////
    if ((xmlElem = xmlRoot->FirstChildElement("Tooltip")))
    { /// We will show tooltip only if mouse is over the moving area.
        if ((strTmp = xmlElem->Attribute("text")))
            mStrTooltip = strTmp;
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the gadgets.
    /// ////////////////////////////////////////////////////////////////////
    GuiSrcEntry *srcEntry;
    for (xmlElem = xmlRoot->FirstChildElement("Gadget"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Gadget"))
    {
        int w = 0;
        int h = 0;


        if ((strTmp = xmlElem->Attribute("image_name")))
        {
            if (( srcEntry = GuiImageset::getSingleton().getStateGfxPositions(strTmp)))
            {
                w = srcEntry->width;
                h = srcEntry->height;
            }
        }

        if ( !strcmp(xmlElem->Attribute("type"), "BUTTON"))
        {
            GuiGadgetButton *button = new GuiGadgetButton(xmlElem, w, h, mWidth, mHeight);
            mvGadget.push_back(button);
            mvButton.push_back(button);
        }
        else if ( !strcmp(xmlElem->Attribute("type"), "COMBOBOX"))
        {
            GuiGadgetCombobox *combobox = new GuiGadgetCombobox(xmlElem, w, h, mWidth, mHeight);
            mvCombobox.push_back(combobox);
            mvGadget.push_back(combobox);
        }
        else
            Logger::log().warning() << xmlElem->Attribute("type") << " is not a defined gadget type.";

    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the graphics.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Graphic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Graphic"))
    {
        if (!(strTmp = xmlElem->Attribute("type")))
            continue;
        if (!stricmp(strTmp, "GFX_FILL"))
        { /// This is a GFX_FILL.
            /// Find the gfx data in the tileset.
            if (!(strTmp = xmlElem->Attribute("image_name")))
                continue;
            srcEntry = GuiImageset::getSingleton().getStateGfxPositions(strTmp);
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
                Logger::log().warning() << strTmp << " was defined in '" << FILE_GUI_WINDOWS
                << "' but the gfx-data in '" << FILE_GUI_IMAGESET << "' is missing.";
            }
        }
        else
        { /// This is a COLOR_FILL.
            GuiGraphic *graphic = new GuiGraphic(xmlElem, 0, 0, mWidth, mHeight);
            mvGraphic.push_back(graphic);
        }
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the listboxes.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Listbox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Listbox"))
    {
        if (!(strTmp = xmlElem->Attribute("name")))
            continue;
        GuiListbox *listbox = new GuiListbox(xmlElem, 0, 0, mWidth, mHeight);
        for (int i = 0; i < GUI_ELEMENTS_SUM; ++i)
        {
            if (stricmp(GuiImageset::getSingleton().getElementName(i), strTmp))
                continue;
            listbox->setIndex(i);
            break;
        }
        mvListbox.push_back(listbox);
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Label.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Label"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Label"))
    {
        TextLine *textline = new TextLine;
        textline->index = -1; /// Value < 0 => Can be deleted after drawing.
        textline->BG_Backup = 0;
        if ((strTmp = xmlElem->Attribute("x")))
            textline->x1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("y")))
            textline->y1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("font")))
            textline->font = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("text")))
            textline->text = strTmp;
        mvTextline.push_back(textline);
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Textbox.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("TextBox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextBox"))
    {
        TextLine *textline = new TextLine;
        if ((strTmp = xmlElem->Attribute("name")))
        {
            for (int i = 0; i < GUI_ELEMENTS_SUM; ++i)
            {
                if (!stricmp(GuiImageset::getSingleton().getElementName(i), strTmp))
                {
                    textline->index = GuiImageset::getSingleton().getElementIndex(i);
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
        if ((strTmp = xmlElem->Attribute("font")))
            textline->font = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("x")))
            textline->x1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("y")))
            textline->y1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("width")))
            textline->width= atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("text")))
            textline->text = strTmp;
        mvTextline.push_back(textline);
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Statusbars.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Statusbar"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Statusbar"))
    {
        GuiStatusbar *statusbar = new GuiStatusbar(xmlElem, 0,0, mWidth, mHeight);
        mvStatusbar.push_back(statusbar);
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Parse the "Talking Head".
    /// ////////////////////////////////////////////////////////////////////
    mSceneNode = 0;
    /// Currently we are using only 1 head !
    for (xmlElem = xmlRoot->FirstChildElement("NPC_Head"); xmlElem; xmlElem = xmlElem->NextSiblingElement("NPC_Head"))
    {
        if ((strTmp = xmlElem->Attribute("x")))
            mHeadPosX = atoi(strTmp);
        else
            mHeadPosX =0;
        if ((strTmp = xmlElem->Attribute("y")))
            mHeadPosY = atoi(strTmp);
        else
            mHeadPosY =0;
        if (!(strTmp = xmlElem->Attribute("mesh")))
            continue;

        MeshPtr mesh = MeshManager::getSingleton().load(strTmp, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Animation* anim = mesh->createAnimation("manual", 0);
        VertexAnimationTrack* track = anim->createVertexTrack(4, VAT_POSE);
        VertexPoseKeyFrame *manualKeyFrame = track->createVertexPoseKeyFrame(0);
        // create pose references, initially zero
        const int SI_COUNT = 18;
        unsigned short poseIndexes[SI_COUNT] =
            {
                1, 2, 3, 4, 7, 8, 6, 5, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
            };
        for (int i = 0; i < SI_COUNT; ++i)
        {
            manualKeyFrame->addPoseReference(poseIndexes[i], 0.0f);
        }
        Entity* head = Event->GetSceneManager()->createEntity("Head", strTmp);
        //        mSceneNode = Event->GetSceneManager()->createSceneNode();
        mSceneNode = new SceneNode(0);
        mSceneNode->attachObject(head);


        Real px, py;
        px = (Event->getCamCornerX()/ GuiManager::getSingleton().getScreenWidth() )*2
             *(mPosX+ mHeadPosX) - Event->getCamCornerX();
        py = (Event->getCamCornerY()/ GuiManager::getSingleton().getScreenHeight())*2
             *(mPosY+ mHeadPosY) - Event->getCamCornerY();
        mSceneNode->setPosition(px, py, -200);


        mSceneNode->scale(.5, .5, .5); // testing

        mSpeakAnimState = head->getAnimationState("Speak");
        mSpeakAnimState->setEnabled(true);
        mManualAnimState = head->getAnimationState("manual");
        mManualAnimState->setTimePosition(0);
    }


}

///================================================================================================
/// Create the window.
///================================================================================================
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
    ///
    if (mSceneNode)
    {
        mNPC_HeadOverlay = OverlayManager::getSingleton().create("GUI_Overlay_Head");
        mNPC_HeadOverlay->setZOrder(500);
        mNPC_HeadOverlay->add3D(mSceneNode);
        mNPC_HeadOverlay->show();
    }

    // If the window is smaller then the texture - we have to set the delta-size to transparent.
    PixelBox pb = mTexture->getBuffer()->lock(Box(0,0, mTexture->getWidth(), mTexture->getHeight()), HardwareBuffer::HBL_READ_ONLY )
                  ;
    uint32 *dest_data = (uint32*)pb.data;
    for (unsigned int y = 0; y < mTexture->getWidth() * mTexture->getHeight(); ++y)
        *dest_data++ = 0;
    mTexture->getBuffer()->unlock();
}

///================================================================================================
/// Returns the gadget under the mousepointer.
///================================================================================================
int GuiWindow::getGadgetMouseIsOver(int x, int y)
{
    for (unsigned int i = 0; i < mvGadget.size(); ++i)
    {
        if (mvGadget[i]->mouseOver(x, y))
            return i;
    }
    return -1;
}

///================================================================================================
/// Draw all window elements.
///================================================================================================
void GuiWindow::drawAll()
{
    /// ////////////////////////////////////////////////////////////////////
    /// Draw the background.
    /// ////////////////////////////////////////////////////////////////////
    for (unsigned int i = 0; i < mvGraphic.size(); ++i)
    {
        mvGraphic[i]->draw(mSrcPixelBox, mTexture.getPointer());
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Draw text.
    /// ////////////////////////////////////////////////////////////////////
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
        GuiTextout::getSingleton().getClippingPos(
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
            if (mvTextline[i]->BG_Backup)
                delete[] mvTextline[i]->BG_Backup;
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
    /// Now delete all TextLines with index < 0.
    for (vector<TextLine*>::iterator i = mvTextline.end(); i< mvTextline.begin(); --i)
    {
        if ((*i)->index < 0) mvTextline.erase(i);
        if (mvTextline.empty()) break;
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Draw gadget.
    /// ////////////////////////////////////////////////////////////////////
    for (unsigned int i = 0; i < mvGadget.size() ; ++i)
        mvGadget [i]->draw(mSrcPixelBox, mTexture.getPointer());
    /// ////////////////////////////////////////////////////////////////////
    /// Draw statusbar.
    /// ////////////////////////////////////////////////////////////////////
    for (unsigned int i = 0; i < mvStatusbar.size() ; ++i)
        mvStatusbar [i]->draw(mSrcPixelBox, mTexture.getPointer());
    /// ////////////////////////////////////////////////////////////////////
    /// Draw listbox.
    /// ////////////////////////////////////////////////////////////////////
    // not needed for text-listbox.

}

///================================================================================================
/// Mouse Event.
///================================================================================================
const char *GuiWindow::mouseEvent(int MouseAction, int rx, int ry)
{
    int x = rx - mPosX;
    int y = ry - mPosY;

    int gadget;
    const char *actGadgetName = 0;

    // Dont pass the action throw this window
    if (rx >= mPosX && rx <= mPosX + mWidth && ry >= mPosY && ry <= mPosY + mHeight)
        actGadgetName = "";
    switch (MouseAction)
    {
        //case M_RESIZE:
        case M_PRESSED:
        {
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
                mvGadget[mMousePressed]->setState(GuiElement::STATE_PUSHED);
                mvGadget[mMousePressed]->draw(mSrcPixelBox, mTexture.getPointer());
                return mvGadget[mMousePressed]->getName();
            }
            else if (x > mDragPosX1 && x < mDragPosX2 && y > mDragPosY1 && y < mDragPosY2)
            {
                mDragOldMousePosX = rx;
                mDragOldMousePosY = ry;
                mMouseDragging = mWindowNr;
            }
        }
        break;

        case M_RELEASED:
        {
            GuiCursor::getSingleton().setState(mSrcPixelBox, GuiCursor::STATE_STANDARD);
            /// ////////////////////////////////////////////////////////////////////
            /// Gadget pressed?
            /// ////////////////////////////////////////////////////////////////////
            if (mMousePressed >= 0)
            {
                gadget = getGadgetMouseIsOver(x, y);
                if (gadget >=0 && gadget == mMousePressed)
                {
                    //actGadgetName = mvSrcEntry[mvGadget[gadget]->getTilsetPos()]->name.c_str();
                    mvGadget[mMousePressed]->setState(GuiElement::STATE_STANDARD);
                    mvGadget[mMousePressed]->draw(mSrcPixelBox, mTexture.getPointer());
                    actGadgetName = mvGadget[mMousePressed]->getName();

                    if (!stricmp(GuiImageset::getSingleton().getElementName(GUI_BUTTON_CLOSE), actGadgetName))
                    {
                        GuiManager::getSingleton().setTooltip(0);
                        Sound::getSingleton().playStream(Sound::BUTTON_CLICK);
                        mOverlay->hide();
                        if (mSceneNode) mNPC_HeadOverlay->hide();
                    }
                    else if (!stricmp(GuiImageset::getSingleton().getElementName(GUI_BUTTON_MAXIMIZE), actGadgetName))
                    {}
                    else if (!stricmp(GuiImageset::getSingleton().getElementName(GUI_BUTTON_MINIMIZE), actGadgetName))
                    {
                        mDefaultHeight = mHeight;
                        setHeight(0);
                    }
                    else if (!stricmp(GuiImageset::getSingleton().getElementName(GUI_BUTTON_RESIZE), actGadgetName))
                    {
                        setHeight(mDefaultHeight);
                    }
                }
            }
            mMousePressed = -1;
            mMouseDragging= -1;
        }
        break;

        case M_MOVED:
        /// ////////////////////////////////////////////////////////////////////
        /// Dragging.
        /// ////////////////////////////////////////////////////////////////////
        if (mMouseDragging == mWindowNr)
        {
            mPosX-= mDragOldMousePosX - rx;
            mPosY-= mDragOldMousePosY - ry;
            mDragOldMousePosX = rx;
            mDragOldMousePosY = ry;
            mElement->setPosition(mPosX, mPosY);
            if (mSceneNode)
            {
                Real px, py;
                px = (Event->getCamCornerX()/ GuiManager::getSingleton().getScreenWidth() )*2
                     *(mPosX+ mHeadPosX) - Event->getCamCornerX();
                py = (Event->getCamCornerY()/ GuiManager::getSingleton().getScreenHeight())*2
                     *(mPosY+ mHeadPosY) - Event->getCamCornerY();
                mSceneNode->setPosition(px, py, -200);
            }
        }
        else if (mMouseDragging < 0)
        {
            /// ////////////////////////////////////////////////////////////////////
            /// Is the mouse still over this gadget?
            /// ////////////////////////////////////////////////////////////////////
            if (mMouseOver >= 0 && mvGadget[mMouseOver]->mouseOver(x, y) == false)
            {
                if (mvGadget[mMouseOver]->setState(GuiElement::STATE_STANDARD))
                {
                    mvGadget[mMouseOver]->draw(mSrcPixelBox, mTexture.getPointer());
                    mMouseOver = -1;
                    GuiManager::getSingleton().setTooltip(0);
                }
            }
            /// ////////////////////////////////////////////////////////////////////
            /// Is mouse over a gadget?
            /// ////////////////////////////////////////////////////////////////////
            if (mMousePressed < 0)
            {
                gadget = getGadgetMouseIsOver(x, y);
                if (gadget >=0)
                {
                    if ( mvGadget[gadget]->setState(GuiElement::STATE_M_OVER))
                    {  // (If not already done) change the gadget state to mouseover.
                        mvGadget[gadget]->draw(mSrcPixelBox, mTexture.getPointer());
                        mMouseOver = gadget;
                        GuiManager::getSingleton().setTooltip(mvGadget[gadget]->getTooltip());
                    }
                }
            }
            else
            {
                gadget = getGadgetMouseIsOver(x, y);
                if (gadget >=0 && mvGadget[gadget]->setState(GuiElement::STATE_PUSHED))
                { // (If not already done) change the gadget state to mouseover.
                    mvGadget[gadget]->draw(mSrcPixelBox, mTexture.getPointer());
                    mMouseOver = gadget;
                }
            }
            break;
        }
    }

    PreformActions();
    return actGadgetName;
}

void GuiWindow::PreformActions()
{
    for( unsigned int i = 0 ; i < mvGadget.size() ; i++ )
    {
        switch( mvGadget[i]->getAction() )
        {
            case GUI_ACTION_START_TEXT_INPUT:
            GuiManager::getSingleton().startTextInput(mWindowNr, mvGadget[i]->index, 20, true, true);
            mvGadget[i]->draw(mSrcPixelBox, mTexture.getPointer());
            break;
        }
    }

}

///================================================================================================
/// Parse a message.
///================================================================================================
const char *GuiWindow::Message(int message, int element, const char *value)
{
    switch (message)
    {
        case GUI_MSG_ADD_TEXTLINE:
        for (unsigned int i = 0; i < mvListbox.size() ; ++i)
        {
            if (mvListbox[i]->getIndex() != element)
                continue;
            mvListbox[i]->addTextline(value);
            break;
        }
        break;

        case GUI_MSG_TXT_CHANGED:
        for (unsigned int i = 0; i < mvTextline.size() ; ++i)
        {
            if (mvTextline[i]->index != element || mvTextline[i]->clipped)
                continue;
            mvTextline[i]->text = value;
            GuiTextout::getSingleton().Print(mvTextline[i], mTexture.getPointer(), value);
            return 0;
            break;
        }
        for (unsigned int i = 0; i < mvGadget.size() ; ++i)
        {
            if (mvGadget[i]->index != element)
                continue;
            mvGadget[i]->setText(value);
            mvGadget[i]->draw(mSrcPixelBox, mTexture.getPointer());
            return 0;
            break;
        }
        break;

        case GUI_MSG_TXT_GET:
        for (unsigned int i = 0; i < mvTextline.size() ; ++i)
        {
            if (mvTextline[i]->index != element || mvTextline[i]->clipped)
                continue;
            return mvTextline[i]->text.c_str();
            break;
        }
        for (unsigned int i = 0; i < mvGadget.size() ; ++i)
        {
            if (mvGadget[i]->index != element)
                continue;
            return mvGadget[i]->getText();
            break;
        }
        break;

        default:
        break;
    }
    return 0;
}

///================================================================================================
/// .
///================================================================================================
void GuiWindow::setHeight(int newHeight)
{
    if (mHeight == newHeight)
        return;
    if (newHeight < mDragPosY2)
        newHeight = mDragPosY2;
    if (newHeight > (int)mTexture->getHeight())
        newHeight = mTexture->getHeight();
    mElement->setHeight(newHeight);
}

///================================================================================================
/// .
///================================================================================================
void GuiWindow::updateDragAnimation()
{
    ;// move back on wrong drag
}

///================================================================================================
/// Update all animation stuff.
///================================================================================================
void GuiWindow::updateAnimaton(Real timeSinceLastFrame)
{
    /// ////////////////////////////////////////////////////////////////////
    /// Speak Animation.
    /// ////////////////////////////////////////////////////////////////////
    if (mSpeakAnimState)
        mSpeakAnimState->addTime(timeSinceLastFrame);

    /// ////////////////////////////////////////////////////////////////////
    /// xyz Animations
    /// ////////////////////////////////////////////////////////////////////
}

///================================================================================================
/// .
///================================================================================================
void GuiWindow::updateListbox()
{
    for (vector<GuiListbox*>::iterator i = mvListbox.begin(); i < mvListbox.end(); ++i)
    {
        (*i)->draw(mSrcPixelBox /* just a dummy */, mTexture.getPointer());
    }
}

