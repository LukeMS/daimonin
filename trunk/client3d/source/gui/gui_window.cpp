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

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include <Ogre.h>
#include <tinyxml.h>
#include "define.h"
#include "gui_window.h"
#include "gui_cursor.h"
#include "gui_textout.h"
#include "gui_manager.h"
#include "gui_imageset.h"
#include "gui_listbox.h"
#include "gui_statusbar.h"
#include "option.h"
#include "sound.h"
#include "events.h"
#include "logger.h"

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
    mGadgetDrag = false;
}

///================================================================================================
/// Destructor.
///================================================================================================
void GuiWindow::freeRecources()
{
    // Delete the buttons.
    for (vector<GuiGadgetButton*>::iterator i = mvGadgetButton.begin(); i < mvGadgetButton.end(); ++i)
        delete (*i);
    mvGadgetButton.clear();

    // Delete the comboboxes.
    for (vector<GuiGadgetCombobox*>::iterator i = mvGadgetCombobox.begin(); i < mvGadgetCombobox.end(); ++i)
        delete (*i);
    mvGadgetCombobox.clear();

    // Delete the listboxes.
    for (vector<GuiListbox*>::iterator i = mvListbox.begin(); i < mvListbox.end(); ++i)
        delete (*i);
    mvListbox.clear();

    // Delete the graphics.
    for (vector<GuiGraphic*>::iterator i = mvGraphic.begin(); i < mvGraphic.end(); ++i)
        delete (*i);
    mvGraphic.clear();

    // Delete the textlines.
    for (vector<TextLine*>::iterator i = mvTextline.begin(); i < mvTextline.end(); ++i)
    {
        if ((*i)->index >= 0) delete[] (*i)->BG_Backup;
        delete (*i);
    }
    mvTextline.clear();

    // Delete the statusbars.
    for (vector<GuiStatusbar*>::iterator i = mvStatusbar.begin(); i < mvStatusbar.end(); ++i)
        delete (*i);
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
    mMousePressed  =-1;
    mMouseOver     =-1;
    mSpeakAnimState= 0;
    mSrcPixelBox = GuiImageset::getSingleton().getPixelBox();
    parseWindowData(xmlElem);
    isInit = true;
}

///================================================================================================
/// Parse the xml window data..
///================================================================================================
void GuiWindow::parseWindowData(TiXmlElement *xmlRoot)
{
    TiXmlElement *xmlElem;
    const char *strTmp;

    if ((strTmp = xmlRoot->Attribute("name"))) mStrName = strTmp;
    Logger::log().info () << "Parsing window: " << mStrName;
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Coordinates type.
    /// ////////////////////////////////////////////////////////////////////
    mSizeRelative = false;
    if ((strTmp = xmlRoot->Attribute("relativeCoords")))
    {
        if (!stricmp(strTmp, "true")) mSizeRelative = true;
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
    if (mWidth  < MIN_GFX_SIZE) mWidth  = MIN_GFX_SIZE;
    if (mHeight < MIN_GFX_SIZE) mHeight = MIN_GFX_SIZE;

    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Position entries.
    /// ////////////////////////////////////////////////////////////////////
    mPosX = mPosY = mPosZ = 100;
    if ((xmlElem = xmlRoot->FirstChildElement("Pos")))
    {
        if ((strTmp = xmlElem->Attribute("x")))
        {
            mPosX = atoi(strTmp);
            if (mPosX <0) mPosX += GuiManager::getSingleton().getScreenWidth()+1;
        }
        else // centred.
        {
            mPosX = (GuiManager::getSingleton().getScreenWidth() - mWidth)/2;
        }
        if ((strTmp = xmlElem->Attribute("y")))
        {
            mPosY = atoi(strTmp);
            if (mPosY <0) mPosY += GuiManager::getSingleton().getScreenHeight()+1;
        }
        else // centred.
        {
            mPosY = (GuiManager::getSingleton().getScreenHeight() - mHeight)/2;
        }
        if ((strTmp = xmlElem->Attribute("zOrder")))
            mPosZ = atoi(strTmp);
    }

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
    /// Now we have all datas to create the window..
    /// ////////////////////////////////////////////////////////////////////
    createWindow();

    /// ////////////////////////////////////////////////////////////////////
    /// Parse the graphics.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Graphic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Graphic"))
    {
        if (!(strTmp = xmlElem->Attribute("type"))) continue;
        if (!stricmp(strTmp, "COLOR_FILL"))
            mvGraphic.push_back(new GuiGraphic(xmlElem, this));
        else /// This is a GFX_FILL.
            mvGraphic.push_back(new GuiGraphic(xmlElem, this));
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Label.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Label"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Label"))
    {
        printParsedTextline(xmlElem);
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Textbox.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("TextBox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextBox"))
    {
        // Error: No name found. Fallback to label.
        if (!(strTmp = xmlElem->Attribute("name")))
        {
            Logger::log().warning() << "A Textbox without a name was found. Will be handled as static text.";
            printParsedTextline(xmlElem);
            continue;
        }
        int index = -1;
        for (int i = 0; i < GUI_ELEMENTS_SUM; ++i)
        {
            if (!stricmp(GuiImageset::getSingleton().getElementName(i), strTmp))
            {
                index = GuiImageset::getSingleton().getElementIndex(i);
                break;
            }
        }
        if (index <0)
        {
            Logger::log().warning() << "TextBox mame " << strTmp << " is unknown. Will be handled as static text.";
            printParsedTextline(xmlElem);
            continue;
        }
        TextLine *textline = new TextLine;
        textline->index = index;
        if ((strTmp = xmlElem->Attribute("font")))   textline->font = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("x")))      textline->x1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("y")))      textline->y1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("width")))  textline->width= atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("text")))   textline->text = strTmp;

        /// Calculate the needed gfx-buffer size for the text.
        if (GuiTextout::getSingleton().getClippingPos(*textline, mWidth, mHeight) == false)
        {
            delete textline;
            continue;
        }

        /// Fill the BG_Backup buffer with Window background, before printing.
        mvTextline.push_back(textline);
        textline->x2 = textline->x1 + textline->width;
        if (textline->x2 >= (unsigned int) mWidth) textline->x2 = mWidth-1;
        textline->BG_Backup = new uint32[(textline->x2- textline->x1) * (textline->y2- textline->y1)];
        mTexture.getPointer()->getBuffer()->blitToMemory(Box(
                    textline->x1, textline->y1,
                    textline->x2, textline->y2),
                PixelBox(
                    textline->x2- textline->x1,
                    textline->y2- textline->y1,
                    1, PF_A8R8G8B8, textline->BG_Backup));
        GuiTextout::getSingleton().Print(textline, mTexture.getPointer());
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Parse the listboxes.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Listbox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Listbox"))
    {
        if (!(strTmp = xmlElem->Attribute("name"))) continue;
        mvListbox.push_back(new GuiListbox(xmlElem, this));
    }
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the Statusbars.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Statusbar"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Statusbar"))
    {
        if (!(strTmp = xmlElem->Attribute("image_name"))) continue;
        mvStatusbar.push_back(new GuiStatusbar(xmlElem, this));
    }

    /// ////////////////////////////////////////////////////////////////////
    /// Parse the gadgets.
    /// ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Gadget"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Gadget"))
    {
        if ( !strcmp(xmlElem->Attribute("type"), "BUTTON"))
        {
            GuiGadgetButton *button = new GuiGadgetButton(xmlElem, this);
            button->setFunction(this->buttonPressed);
            mvGadgetButton.push_back(button);
        }
        else if ( !strcmp(xmlElem->Attribute("type"), "COMBOBOX"))
        {
            GuiGadgetCombobox *combobox = new GuiGadgetCombobox(xmlElem, this);
            mvGadgetCombobox.push_back(combobox);
        }
        else
            Logger::log().warning() << xmlElem->Attribute("type") << " is not a defined gadget type.";
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
        mNPC_HeadOverlay = OverlayManager::getSingleton().create("GUI_Overlay_Head");
        mNPC_HeadOverlay->setZOrder(500);
        mNPC_HeadOverlay->add3D(mSceneNode);
        mNPC_HeadOverlay->show();
    }
}

///================================================================================================
/// Parse and print a textline.
///================================================================================================
inline void GuiWindow::printParsedTextline(TiXmlElement *xmlElem)
{
    const char *strTmp;
    TextLine textline;
    textline.index = -1;
    textline.BG_Backup = 0;
    if ((strTmp = xmlElem->Attribute("x")))    textline.x1   = atoi(strTmp);
    if ((strTmp = xmlElem->Attribute("y")))    textline.y1   = atoi(strTmp);
    if ((strTmp = xmlElem->Attribute("font"))) textline.font = atoi(strTmp);
    if ((strTmp = xmlElem->Attribute("text"))) textline.text = strTmp;
    if (GuiTextout::getSingleton().getClippingPos(textline, mWidth, mHeight))
    {
        GuiTextout::getSingleton().Print(&textline, mTexture.getPointer());
    }
}

///================================================================================================
/// Create the window.
///================================================================================================
inline void GuiWindow::createWindow()
{
    mWindowNr = ++msInstanceNr;
    std::string strNum = StringConverter::toString(msInstanceNr);
    mTexture = TextureManager::getSingleton().createManual("GUI_Texture_" + strNum, "General",
               TEX_TYPE_2D, mWidth, mHeight, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY);
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
    // If the window is smaller then the texture - we have to set the delta-size to transparent.
    PixelBox pb = mTexture->getBuffer()->lock(Box(0,0, mTexture->getWidth(), mTexture->getHeight()), HardwareBuffer::HBL_READ_ONLY );
    uint32 *dest_data = (uint32*)pb.data;
    for (unsigned int y = 0; y < mTexture->getWidth() * mTexture->getHeight(); ++y)
        *dest_data++ = 0;
    mTexture->getBuffer()->unlock();
}

///================================================================================================
/// Mouse Event.
///================================================================================================
const char *GuiWindow::mouseEvent(int MouseAction, int rx, int ry)
{
    if (!mOverlay->isVisible()) return 0;
    // No gadget dragging && not within this window. No need for further checks.
    if (!mGadgetDrag && (rx < mPosX && rx > mPosX + mWidth && ry < mPosY && ry > mPosY + mHeight)) return 0;

    int x = rx - mPosX;
    int y = ry - mPosY;

    for (unsigned int i = 0; i < mvGadgetButton.size(); ++i)
    {
        if (mvGadgetButton[i]->mouseEvent(MouseAction, x, y))
            return "";
    }

    const char *actGadgetName = 0;

    switch (MouseAction)
    {
        case BUTTON_PRESSED:
        {
            GuiCursor::getSingleton().setState(mSrcPixelBox, GuiCursor::STATE_BUTTON_DOWN);
            // Mouse over this window?
            if (rx >= mPosX && rx <= mPosX + mWidth && ry >= mPosY && ry <= mPosY + mHeight)
            {
                actGadgetName = mStrName.c_str();
            }
            if (x > mDragPosX1 && x < mDragPosX2 && y > mDragPosY1 && y < mDragPosY2)
            {
                mDragOldMousePosX = rx;
                mDragOldMousePosY = ry;
                mMouseDragging = mWindowNr;
            }
        }
        case MOUSE_MOVEMENT:
            if (mMouseDragging == mWindowNr)
            {
                mPosX-= mDragOldMousePosX - rx;
                mPosY-= mDragOldMousePosY - ry;
                mDragOldMousePosX = rx;
                mDragOldMousePosY = ry;
                mElement->setPosition(mPosX, mPosY);
                // Animated head.
                if (mSceneNode)
                {
                    Real px = (Event->getCamCornerX()/ GuiManager::getSingleton().getScreenWidth() )*2
                              *(mPosX+ mHeadPosX) - Event->getCamCornerX();
                    Real py = (Event->getCamCornerY()/ GuiManager::getSingleton().getScreenHeight())*2
                              *(mPosY+ mHeadPosY) - Event->getCamCornerY();
                    mSceneNode->setPosition(px, py, -200);
                }
            }
            break;

        case BUTTON_RELEASED:
        {
            GuiCursor::getSingleton().setState(mSrcPixelBox, GuiCursor::STATE_STANDARD);
            mMousePressed = -1;
            mMouseDragging= -1;
            break;
        }

        default:
            break;
    }

    /*
        int gadget;
        const char *actGadgetName = 0;

        // Dont pass the action throw this window
        if (rx >= mPosX && rx <= mPosX + mWidth && ry >= mPosY && ry <= mPosY + mHeight)
            actGadgetName = "";
        switch (MouseAction)
        {
                //case M_RESIZE:
            case BUTTON_PRESSED:
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
                    mvGadgetButton[mMousePressed]->setState(GuiElement::STATE_PUSHED);
                    mvGadgetButton[mMousePressed]->draw();
                    return mvGadgetButton[mMousePressed]->getName();
                }
                else if (x > mDragPosX1 && x < mDragPosX2 && y > mDragPosY1 && y < mDragPosY2)
                {
                    mDragOldMousePosX = rx;
                    mDragOldMousePosY = ry;
                    mMouseDragging = mWindowNr;
                }
            }
            break;

            case BUTTON_RELEASED:
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
                        mvGadgetButton[mMousePressed]->setState(GuiElement::STATE_STANDARD);
                        mvGadgetButton[mMousePressed]->draw();
                        actGadgetName = mvGadgetButton[mMousePressed]->getName();

                        if (!stricmp(GuiImageset::getSingleton().getElementName(GUI_BUTTON_CLOSE), actGadgetName))
                        {
                            GuiManager::getSingleton().setTooltip(0);

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

            case MOUSE_MOVEMENT:
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
                    if (mMouseOver >= 0 && mvGadgetButton[mMouseOver]->mouseOver(x, y) == false)
                    {
                        if (mvGadgetButton[mMouseOver]->setState(GuiElement::STATE_STANDARD))
                        {
                            mvGadgetButton[mMouseOver]->draw();
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
                            if ( mvGadgetButton[gadget]->setState(GuiElement::STATE_M_OVER))
                            {  // (If not already done) change the gadget state to mouseover.
                                mvGadgetButton[gadget]->draw();
                                mMouseOver = gadget;
                                GuiManager::getSingleton().setTooltip(mvGadgetButton[gadget]->getTooltip());
                            }
                        }
                    }
                    else
                    {
                        gadget = getGadgetMouseIsOver(x, y);
                        if (gadget >=0 && mvGadgetButton[gadget]->setState(GuiElement::STATE_PUSHED))
                        { // (If not already done) change the gadget state to mouseover.
                            mvGadgetButton[gadget]->draw();
                            mMouseOver = gadget;
                        }
                    }
                    break;
                }
        }

        PreformActions();
        return actGadgetName;
        */

    return 0;  /// DELETE ME !!!!!!!
}

void GuiWindow::PreformActions()
{
    /*
        for( unsigned int i = 0 ; i < mvGadgetButton.size() ; i++ )
        {
            switch( mvGadgetButton[i]->getAction() )
            {
                case GUI_ACTION_START_TEXT_INPUT:
                    GuiManager::getSingleton().startTextInput(mWindowNr, mvGadget[i]->getIndex(), 20, true, true);
                    mvGadgetButton[i]->draw();
                    break;
            }
        }
    */
}

///================================================================================================
/// Parse a message.
///================================================================================================
const char *GuiWindow::Message(int message, int element, void *value)
{
    switch (message)
    {
        case GUI_MSG_ADD_TEXTLINE:
            for (unsigned int i = 0; i < mvListbox.size() ; ++i)
            {
                if (mvListbox[i]->getIndex() != element)
                    continue;
                mvListbox[i]->addTextline((const char *)value);
                break;
            }
            break;

        case GUI_MSG_BAR_CHANGED:
            for (unsigned int i = 0; i < mvStatusbar.size() ; ++i)
            {
                if (mvStatusbar[i]->getIndex() != element)
                    continue;
                mvStatusbar[i]->setValue(*((Real*)(value)));
                mvStatusbar[i]->draw();
                return 0;
            }
            break;

        case GUI_MSG_TXT_CHANGED:
            for (unsigned int i = 0; i < mvTextline.size() ; ++i)
            {
                if (mvTextline[i]->index != element)
                    continue;
                mvTextline[i]->text = (const char*)value;
                GuiTextout::getSingleton().Print(mvTextline[i], mTexture.getPointer());
                return 0;
            }
            for (unsigned int i = 0; i < mvGadgetCombobox.size() ; ++i)
            {
                if (mvGadgetCombobox[i]->getIndex() != element)
                    continue;
                mvGadgetCombobox[i]->setText((const char *)value);
                mvGadgetCombobox[i]->draw();
                return 0;
            }
            break;

        case GUI_MSG_TXT_GET:
            for (unsigned int i = 0; i < mvTextline.size() ; ++i)
            {
                if (mvTextline[i]->index != element)
                    continue;
                return mvTextline[i]->text.c_str();
            }
            for (unsigned int i = 0; i < mvGadgetCombobox.size() ; ++i)
            {
                if (mvGadgetCombobox[i]->getIndex() != element)
                    continue;
                return mvGadgetCombobox[i]->getText();
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
    if (mHeight == newHeight) return;
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
        (*i)->draw();
    }
}

///================================================================================================
/// .
///================================================================================================
void GuiWindow::buttonPressed(GuiWindow *me, int index)
{
    Sound::getSingleton().playStream(Sound::BUTTON_CLICK);
    GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"button event... ");
}


