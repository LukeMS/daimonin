/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#include <Ogre.h>
#include <tinyxml.h>
#include "define.h"
#include "logger.h"
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
#include "gui_window_dialog.h"

using namespace Ogre;

const int MIN_GFX_SIZE = 4;

//const char XML_BACKGROUND[] = "Background";

//================================================================================================
// Init all static Elemnts.
//================================================================================================
const char *GuiWindow::OVERLAY_ELEMENT_TYPE = "Panel"; // defined in Ogre::OverlayElementFactory.h
int GuiWindow::msInstanceNr = -1;
int GuiWindow::mMouseDragging = -1;
std::string GuiWindow::mStrTooltip ="";

//================================================================================================
// Constructor.
//================================================================================================
GuiWindow::GuiWindow()
{
    isInit = false;
    mWinLayerBG = 0;
    mSumUsedSlots = 0;
    mGadgetDrag = -1;
    mElement = 0;
}

//================================================================================================
// Destructor.
//================================================================================================
void GuiWindow::freeRecources()
{
    // Delete the slots.
    for (std::vector<GuiGadgetSlot*>::iterator i = mvSlot.begin(); i < mvSlot.end(); ++i)
        delete (*i);
    mvSlot.clear();

    // Delete the buttons.
    for (std::vector<GuiGadgetButton*>::iterator i = mvGadgetButton.begin(); i < mvGadgetButton.end(); ++i)
        delete (*i);
    mvGadgetButton.clear();

    // Delete the comboboxes.
    for (std::vector<GuiGadgetCombobox*>::iterator i = mvGadgetCombobox.begin(); i < mvGadgetCombobox.end(); ++i)
        delete (*i);
    mvGadgetCombobox.clear();

    // Delete the listboxes.
    for (std::vector<GuiListbox*>::iterator i = mvListbox.begin(); i < mvListbox.end(); ++i)
        delete (*i);
    mvListbox.clear();

    // Delete the graphics.
    for (std::vector<GuiGraphic*>::iterator i = mvGraphic.begin(); i < mvGraphic.end(); ++i)
        delete (*i);
    mvGraphic.clear();

    // Delete the textlines.
    for (std::vector<GuiTextout::TextLine*>::iterator i = mvTextline.begin(); i < mvTextline.end(); ++i)
    {
        if ((*i)->index >= 0) delete[] (*i)->LayerWindowBG;
        delete (*i);
    }
    mvTextline.clear();

    // Delete the statusbars.
    for (std::vector<GuiStatusbar*>::iterator i = mvStatusbar.begin(); i < mvStatusbar.end(); ++i)
        delete (*i);
    mvStatusbar.clear();

    // Delete the tables.
    for (std::vector<GuiTable*>::iterator i = mvTable.begin(); i < mvTable.end(); ++i)
        delete (*i);
    mvTable.clear();

    // Set all shared pointer to null.
    delete[] mWinLayerBG;
    mMaterial.setNull();
    mTexture.setNull();
}

//================================================================================================
// Build a window out of a xml description file.
//================================================================================================
void GuiWindow::Init(TiXmlElement *xmlElem, int zOrder)
{
    mMousePressed  =-1;
    mMouseOver     =-1;
    mSpeakAnimState= 0;
    mHeight = MIN_GFX_SIZE;
    mWidth  = MIN_GFX_SIZE;
    mSrcPixelBox = GuiImageset::getSingleton().getPixelBox();
    parseWindowData(xmlElem, zOrder);
    isInit = true;
}

//================================================================================================
// .
//================================================================================================
void GuiWindow::setVisible(bool visible)
{
    if (!isInit) return;
    if (!visible) mOverlay->hide(); else mOverlay->show();
}

//================================================================================================
// Parse the xml window data..
//================================================================================================
void GuiWindow::parseWindowData(TiXmlElement *xmlRoot, int zOrder)
{
    TiXmlElement *xmlElem;
    const char *strTmp;
    int screenH = GuiManager::getSingleton().getScreenHeight();
    int screenW = GuiManager::getSingleton().getScreenWidth();
    if ((strTmp = xmlRoot->Attribute("name")))
        Logger::log().info () << "Parsing window: " << strTmp;
    // ////////////////////////////////////////////////////////////////////
    // Parse the Coordinates type.
    // ////////////////////////////////////////////////////////////////////
    mSizeRelative = false;
    if ((strTmp = xmlRoot->Attribute("relativeCoords")))
    {
        if (!stricmp(strTmp, "true")) mSizeRelative = true;
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Size entries.
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElem = xmlRoot->FirstChildElement("Size")))
    {
        if ((strTmp = xmlElem->Attribute("width")))  mWidth = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("height"))) mHeight= atoi(strTmp);
        if (mWidth < MIN_GFX_SIZE) mWidth  = MIN_GFX_SIZE;
        if (mWidth > screenW) mWidth = screenW;
        if (mHeight< MIN_GFX_SIZE) mHeight = MIN_GFX_SIZE;
        if (mHeight > screenH) mHeight = screenH;
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Alignment entries.
    // ////////////////////////////////////////////////////////////////////
    int aX =1, aY =1;
    if ((xmlElem = xmlRoot->FirstChildElement("Alignment")))
    {
        if ((strTmp = xmlElem->Attribute("x")))
        {
            if (!stricmp(strTmp, "center")) aX = 0;
            else if (!stricmp(strTmp, "right"))  aX =-1;
        }
        if ((strTmp = xmlElem->Attribute("y")))
        {
            if (!stricmp(strTmp, "center")) aY = 0;
            else if (!stricmp(strTmp, "bottom")) aY =-1;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Position entries.
    // ////////////////////////////////////////////////////////////////////
    mPosX = mPosY = mPosZ = 0;
    if ((xmlElem = xmlRoot->FirstChildElement("Pos")))
    {
        if ((strTmp = xmlElem->Attribute("x")))
        {
            if (aX <0) mPosX = screenW+1 - atoi(strTmp);
            else if (aX==0) mPosX =(screenW- mWidth) /2 + atoi(strTmp);
            else mPosX = atoi(strTmp);
        }
        if ((strTmp = xmlElem->Attribute("y")))
        {
            if (aY <0) mPosY = screenH+1 - atoi(strTmp);
            else if (aY==0) mPosY =(screenH- mHeight) /2 + atoi(strTmp);
            else mPosY = atoi(strTmp);
        }
        if ((strTmp = xmlElem->Attribute("zOrder")))
            mPosZ = atoi(strTmp);
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Dragging entries.
    // ////////////////////////////////////////////////////////////////////
    mDragPosX1 = mDragPosX2 = mDragPosY1 = mDragPosY2 = -100;
    if ((xmlElem = xmlRoot->FirstChildElement("DragArea")))
    {
        if ((strTmp = xmlElem->Attribute("x")))      mDragPosX1 = atoi(strTmp);
        if (mDragPosX1 > mWidth) mDragPosX1 = mWidth-1;
        if ((strTmp = xmlElem->Attribute("y")))      mDragPosY1 = atoi(strTmp);
        if (mDragPosY1 > mHeight) mDragPosY1 = mHeight-1;
        if ((strTmp = xmlElem->Attribute("width")))  mDragPosX2 = mDragPosX1 + atoi(strTmp);
        if (mDragPosX2 > mWidth) mDragPosX2 = mWidth;
        if ((strTmp = xmlElem->Attribute("height"))) mDragPosY2 = mDragPosY1 + atoi(strTmp);
        if (mDragPosY2 > mHeight) mDragPosY2 = mHeight;
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Tooltip entries.
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElem = xmlRoot->FirstChildElement("Tooltip")))
    { // We will show tooltip only if mouse is over the moving area.
        if ((strTmp = xmlElem->Attribute("text"))) mStrTooltip = strTmp;
    }
    // ////////////////////////////////////////////////////////////////////
    // Now we have all datas to create the window..
    // ////////////////////////////////////////////////////////////////////
    createWindow(zOrder);
    // ////////////////////////////////////////////////////////////////////
    // Parse the graphics.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Graphic"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Graphic"))
    {
        mvGraphic.push_back(new GuiGraphic(xmlElem, this));
    }

    // ////////////////////////////////////////////////////////////////////
    // Parse the Label.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Label"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Label"))
    {
        printParsedTextline(xmlElem);
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Textbox.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("TextBox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("TextBox"))
    {
        // Error: No name found. Fallback to label.
        if (!(strTmp = xmlElem->Attribute("name")))
        {
            Logger::log().warning() << "A Textbox without a name was found. Will be handled as static text.";
            printParsedTextline(xmlElem);
            continue;
        }
        String strName = strTmp;
        int index = -1;
        for (int i = 0; i < GuiImageset::GUI_ELEMENTS_SUM; ++i)
        {
            if (!stricmp(GuiImageset::getSingleton().getElementName(i), strTmp))
            {
                index = GuiImageset::getSingleton().getElementIndex(i);
                break;
            }
        }
        if (index <0)
        {
            Logger::log().warning() << "TextBox name " << strTmp << " is unknown. Will be handled as static text.";
            printParsedTextline(xmlElem);
            continue;
        }
        GuiTextout::TextLine *textline = new GuiTextout::TextLine;
        textline->index = index;
        textline->hideText= false;
        textline->color = 0x00ffffff;
        if ((strTmp = xmlElem->Attribute("font")))  textline->font = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("x")))     textline->x1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("y")))     textline->y1   = atoi(strTmp);
        if ((strTmp = xmlElem->Attribute("width"))) textline->x2   = atoi(strTmp) + textline->x1;
        textline->y2 = textline->y1 + GuiTextout::getSingleton().getFontHeight(textline->font);
        if (textline->x1 > (unsigned int) mWidth)   textline->x1 = 0; // If pos is out of window set it to leftmost pos.
        if (textline->x2 > (unsigned int) mWidth)   textline->x2 = mWidth-1;
        if (textline->y1 > (unsigned int) mHeight)  textline->y1 = 0; // If pos is out of window set it to topmost pos.
        if (textline->y2 > (unsigned int) mHeight)  textline->y1 = mHeight-1;
        if ((strTmp = xmlElem->Attribute("text")))   textline->text = strTmp;
        if ((strTmp = xmlElem->Attribute("hide")))   textline->hideText= (atoi(strTmp)==1);

        // Fill the LayerWindowBG buffer with Window background, before printing.
        mvTextline.push_back(textline);
        textline->LayerWindowBG = new uint32[(textline->x2- textline->x1) * (textline->y2- textline->y1)];
        mTexture.getPointer()->getBuffer()->blitToMemory(Box(
                    textline->x1, textline->y1,
                    textline->x2, textline->y2),
                PixelBox(
                    textline->x2- textline->x1,
                    textline->y2- textline->y1,
                    1, PF_A8R8G8B8, textline->LayerWindowBG));
        GuiTextout::getSingleton().Print(textline, mTexture.getPointer());
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the listboxes.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Listbox"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Listbox"))
    {
        if (!(strTmp = xmlElem->Attribute("name"))) continue;
        GuiListbox *listbox = new GuiListbox(xmlElem, this);
        listbox->setFunction(this->listboxPressed);
        mvListbox.push_back(listbox);
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the tables.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Table"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Table"))
    {
        if (!(strTmp = xmlElem->Attribute("name"))) continue;
        mvTable.push_back(new GuiTable(xmlElem, this));
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the Statusbars.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Statusbar"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Statusbar"))
    {
        if (!(strTmp = xmlElem->Attribute("image_name"))) continue;
        mvStatusbar.push_back(new GuiStatusbar(xmlElem, this));
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the gadgets.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Gadget"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Gadget"))
    {
        if ( !strcmp(xmlElem->Attribute("type"), "BUTTON"))
        {
            GuiGadgetButton *button = new GuiGadgetButton(xmlElem, this);
            button->setFunction(this->buttonPressed);
            mvGadgetButton.push_back(button);
        }
        else if ( !strcmp(xmlElem->Attribute("type"), "SLOT"))
        {
            mvSlot.push_back(new GuiGadgetSlot(xmlElem, this));
        }
        else if ( !strcmp(xmlElem->Attribute("type"), "COMBOBOX"))
        {
            GuiGadgetCombobox *combobox = new GuiGadgetCombobox(xmlElem, this);
            mvGadgetCombobox.push_back(combobox);
        }
        else
            Logger::log().warning() << xmlElem->Attribute("type") << " is not a defined gadget type.";
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the "Talking Head".
    // ////////////////////////////////////////////////////////////////////
    mSceneNode = 0;
    // Currently we are using only 1 head !
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
        Entity* head = Events::getSingleton().GetSceneManager()->createEntity("Head", strTmp);
        //        mSceneNode = Event->GetSceneManager()->createSceneNode();
        mSceneNode = new SceneNode(0);
        mSceneNode->attachObject(head);

        Real px, py;
        px = (Events::getSingleton().getCamCornerX()/ screenW )*2
             *(mPosX+ mHeadPosX) - Events::getSingleton().getCamCornerX();
        py = (Events::getSingleton().getCamCornerY()/ screenH)*2
             *(mPosY+ mHeadPosY) - Events::getSingleton().getCamCornerY();
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

//================================================================================================
// Parse and print a text (label element).
//================================================================================================
inline void GuiWindow::printParsedTextline(TiXmlElement *xmlElem)
{
    const char *strTmp;
    GuiTextout::TextLine textline;
    textline.index = -1;
    textline.hideText= false;
    textline.LayerWindowBG = 0;
    textline.color = 0x00ffffff;
    if ((strTmp = xmlElem->Attribute("x")))    textline.x1   = atoi(strTmp);
    if ((strTmp = xmlElem->Attribute("y")))    textline.y1   = atoi(strTmp);
    if ((strTmp = xmlElem->Attribute("font"))) textline.font = atoi(strTmp);
    if ((strTmp = xmlElem->Attribute("text"))) textline.text = strTmp;
    if (textline.x1 > (unsigned int) mWidth)  textline.x1 = 0; // If pos is out of window set it to leftmost pos.
    if (textline.y1 > (unsigned int) mHeight) textline.y1 = 0; // If pos is out of window set it to topmost pos.
    textline.x2 = mWidth - textline.x1;
    textline.y2 = textline.y1 + GuiTextout::getSingleton().getFontHeight(textline.font);
    GuiTextout::getSingleton().Print(&textline, mTexture.getPointer());
}

//================================================================================================
// Create the window.
//================================================================================================
inline void GuiWindow::createWindow(int zOrder)
{
    mWindowNr = ++msInstanceNr;
    std::string strNum = StringConverter::toString(msInstanceNr);
    mTexture = TextureManager::getSingleton().createManual("GUI_Texture_" + strNum, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
               TEX_TYPE_2D, mWidth, mHeight, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY);
    mOverlay = OverlayManager::getSingleton().create("GUI_Overlay_"+strNum);
    mOverlay->setZOrder(400-zOrder);
    mElement = OverlayManager::getSingleton().createOverlayElement (OVERLAY_ELEMENT_TYPE, "GUI_Frame_" + strNum);
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
    // We must clear the whole texture (textures have always 2^n size while windows can be smaller).
    memset(mTexture->getBuffer()->lock(HardwareBuffer::HBL_DISCARD), 0x00, mTexture->getWidth() * mTexture->getHeight() * sizeof(uint32));
    mTexture->getBuffer()->unlock();
    mWinLayerBG = new uint32[mWidth * mHeight];
    memset(mWinLayerBG, 0x00, mWidth * mHeight * sizeof(uint32));
}

//================================================================================================
// .
//================================================================================================
void GuiWindow::centerWindowOnMouse(int x, int y)
{
    mPosX = (int)(x-mTexture->getWidth())/2;
    mPosY = (int)(y-mTexture->getHeight())/2 - 50;
    mElement->setPosition(mPosX, mPosY);
}

//================================================================================================
// Key event.
//================================================================================================
bool GuiWindow::keyEvent(const char keyChar, const unsigned char key)
{
    for (unsigned int i = 0; i < mvTable.size(); ++i)
    {
        if (mvTable[i]->keyEvent(keyChar, key))
            return true;
    }
    return false;
}

//================================================================================================
// Mouse Event.
//================================================================================================
int GuiWindow::mouseEvent(int MouseAction, Vector3 &mouse)
{
    if (!isInit || !mOverlay->isVisible()) return GuiManager::EVENT_CHECK_NEXT;
    int rx = (int) mouse.x;
    int ry = (int) mouse.y;

    if (rx < mPosX && rx > mPosX + mWidth && ry < mPosY && ry > mPosY + mHeight)
        return GuiManager::EVENT_CHECK_NEXT;
    int x = rx - mPosX;
    int y = ry - mPosY;

    int sumPressed = 0;
    for (unsigned int i = 0; i < mvGadgetButton.size(); ++i)
    {   // We dont return on a true return value - to avoid 2 buttons selected at same time..
        // This will still happen when their gfx is overlapping.
        if (mvGadgetButton[i]->mouseEvent(MouseAction, x, y)) ++sumPressed;
    }
    if (sumPressed) return GuiManager::EVENT_CHECK_DONE;

    for (unsigned int i = 0; i < mvSlot.size(); ++i)
    {
        if (mvSlot[i]->mouseEvent(MouseAction, x, y) == GuiManager::EVENT_DRAG_STRT)
        {
            mGadgetDrag = i;
            return GuiManager::EVENT_DRAG_STRT;
        }
    }
    for (unsigned int i = 0; i < mvGadgetCombobox.size(); ++i)
    {
//        if (mvGadgetCombobox[i]->mouseEvent(MouseAction, x, y))
//            return true;
    }
    for (unsigned int i = 0; i < mvListbox.size(); ++i)
    {
        if (mvListbox[i]->mouseEvent(MouseAction, x, y, (int) mouse.z))
            return GuiManager::EVENT_CHECK_DONE;
    }
    for (unsigned int i = 0; i < mvTable.size(); ++i)
    {
        if (mvTable[i]->mouseEvent(MouseAction, x, y))
            return GuiManager::EVENT_CHECK_DONE;
    }

    switch (MouseAction)
    {
        case BUTTON_PRESSED:
        {
            //GuiCursor::getSingleton().setState(GuiImageset::STATE_MOUSE_PUSHED);
            if (x > mDragPosX1 && x < mDragPosX2 && y > mDragPosY1 && y < mDragPosY2)
            {
                mDragOldMousePosX = rx;
                mDragOldMousePosY = ry;
                mMouseDragging = mWindowNr;
                return GuiManager::EVENT_CHECK_DONE;
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
                    Real px = (Events::getSingleton().getCamCornerX()/ GuiManager::getSingleton().getScreenWidth() )*2
                              *(mPosX+ mHeadPosX) - Events::getSingleton().getCamCornerX();
                    Real py = (Events::getSingleton().getCamCornerY()/ GuiManager::getSingleton().getScreenHeight())*2
                              *(mPosY+ mHeadPosY) - Events::getSingleton().getCamCornerY();
                    mSceneNode->setPosition(px, py, -200);
                }
            }
            break;

        case BUTTON_RELEASED:
        {
            //GuiCursor::getSingleton().setState(GuiImageset::STATE_MOUSE_DEFAULT);
            mMousePressed = -1;
            mMouseDragging= -1;
            break;
        }

        default:
            break;
    }
    // Mouse over this window?
    if (rx >= mPosX && rx <= mPosX + mWidth && ry >= mPosY && ry <= mPosY + mHeight)
        return GuiManager::EVENT_CHECK_DONE;
    return GuiManager::EVENT_CHECK_NEXT;
}


//================================================================================================
// .
//================================================================================================
int GuiWindow::getTableActivated(int element)
{
    for (unsigned int i = 0; i < mvTable.size() ; ++i)
    {
        if (mvTable[i]->getIndex() == element)
            return mvTable[i]->getActivatedRow();
    }
    return -1;
}

//================================================================================================
// .
//================================================================================================
class GuiGadgetButton *GuiWindow::getButtonHandle(int element)
{
    for (unsigned int i = 0; i < mvGadgetButton.size() ; ++i)
    {
        if (mvGadgetButton[i]->getIndex() == element)
            return mvGadgetButton[i];
    }
    return 0;
}

//================================================================================================
// .
//================================================================================================
int GuiWindow::getTableSelection(int element)
{
    for (unsigned int i = 0; i < mvTable.size() ; ++i)
    {
        if (mvTable[i]->getIndex() == element)
            return mvTable[i]->getSelectedRow();
    }
    return -1;
}

//================================================================================================
// .
//================================================================================================
bool GuiWindow::getTableUserBreak(int element)
{
    for (unsigned int i = 0; i < mvTable.size() ; ++i)
    {
        if (mvTable[i]->getIndex() == element)
            return mvTable[i]->getUserBreak();
    }
    return -1;
}

//================================================================================================
// .
//================================================================================================
void GuiWindow::clearTable(int element)
{
    for (unsigned int i = 0; i < mvTable.size() ; ++i)
    {
        if (mvTable[i]->getIndex() == element)
            mvTable[i]->clearRows();
    }
}

//================================================================================================
// .
//================================================================================================
void GuiWindow::setSlotBusyTime(int element, Real busyTime)
{
    if (element < (int)mvSlot.size())
    {
        mvSlot[element]->setBusyTime(busyTime);
    }
}

//================================================================================================
// .
//================================================================================================
void GuiWindow::setSlotBusy(int element)
{
    if (element < (int)mvSlot.size())
    {
        mvSlot[element]->setBusy();
    }
}

//================================================================================================
// .
//================================================================================================
int GuiWindow::addTextline(int element, const char *text, uint32 color)
{
    for (unsigned int i = 0; i < mvListbox.size() ; ++i)
    {
        if (mvListbox[i]->getIndex() == element)
            return mvListbox[i]->addTextline(text, color);
    }
    return 0;
}

//================================================================================================
// .
//================================================================================================
void GuiWindow::clearListbox(int element)
{
    for (unsigned int i = 0; i < mvListbox.size() ; ++i)
    {
        if (mvListbox[i]->getIndex() == element)
        {
            mvListbox[i]->clear();
            return ;
        }
    }
}

//================================================================================================
// .
//================================================================================================
int GuiWindow::getMouseOverSlot(int x, int y)
{
    for (unsigned int i = 0; i < mvSlot.size(); ++i)
    {
        if (mvSlot[i]->mouseWithin(x - mPosX, y - mPosY))
            return i;
    }
    return -1;
}

//================================================================================================
// Adds an item to the slots.
//================================================================================================
void GuiWindow::addItem(Item::sItem *item)
{
    if (mSumUsedSlots < mvSlot.size())
    {
        mvSlot[mSumUsedSlots++]->setItem(item);
    }
}

//================================================================================================
// .
//================================================================================================
void GuiWindow::delItem(Item::sItem *item)
{
    for (unsigned int i = 0; i < mSumUsedSlots; ++i)
    {
        if (item == mvSlot[i]->getItem())
        {
            // Found the item that will be deleted.
            for (; i < mSumUsedSlots; ++i)
                mvSlot[i]->setItem(mvSlot[i+1]->getItem());
        }
    }
    mvSlot[--mSumUsedSlots]->setItem(0);
}

//================================================================================================
// Set all slots to empty.
//================================================================================================
void GuiWindow::clrItem()
{
    for (unsigned int i = 0; i < mvSlot.size(); ++i)
        mvSlot[i]->setItem(0);
    mSumUsedSlots = 0;
}

//================================================================================================
// Parse a message.
//================================================================================================
const char *GuiWindow::Message(int message, int element, void *value, void *value2)
{
    switch (message)
    {
        case GuiManager::GUI_MSG_ADD_TABLEROW:
            for (unsigned int i = 0; i < mvTable.size() ; ++i)
            {
                if (mvTable[i]->getIndex() != element)
                    continue;
                mvTable[i]->addRow((const char *)value);
                break;
            }
            break;

        case GuiManager::GUI_MSG_BAR_CHANGED:
            for (unsigned int i = 0; i < mvStatusbar.size() ; ++i)
            {
                if (mvStatusbar[i]->getIndex() != element)
                    continue;
                mvStatusbar[i]->setValue(*((Real*)(value)));
                mvStatusbar[i]->draw();
                return 0;
            }
            break;

        case GuiManager::GUI_MSG_TXT_CHANGED:
            for (unsigned int i = 0; i < mvTextline.size() ; ++i)
            {
                if (mvTextline[i]->index == element)
                {
                    mvTextline[i]->text = (const char*)value;
                    GuiTextout::getSingleton().Print(mvTextline[i], mTexture.getPointer());
                    return 0;
                }
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

        case GuiManager::GUI_MSG_TXT_GET:
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

//================================================================================================
// .
//================================================================================================
void GuiWindow::setHeight(int newHeight)
{
    if (mHeight == newHeight) return;
    if (newHeight < mDragPosY2)
        newHeight = mDragPosY2;
    if (newHeight > (int)mTexture->getHeight())
        newHeight = (int)mTexture->getHeight();
    mElement->setHeight(newHeight);
}

//================================================================================================
// .
//================================================================================================
void GuiWindow::update(Real timeSinceLastFrame)
{
    if (!isInit || !mOverlay->isVisible()) return;
    // Update drag animation (move back on wrong drag).
    ;
    // Speak Animation.
    if (mSpeakAnimState)
        mSpeakAnimState->addTime(timeSinceLastFrame);
    // Update slots.
    for (unsigned int i = 0; i < mvSlot.size(); ++i)
    {
        mvSlot[i]->update(timeSinceLastFrame);
    }
    // Update listboxes.
    for (std::vector<GuiListbox*>::iterator i = mvListbox.begin(); i < mvListbox.end(); ++i)
    {
        (*i)->draw();
    }
}

//================================================================================================
// Button was pressed.
//================================================================================================
void GuiWindow::listboxPressed(GuiWindow *me, int index, int line)
{
    Sound::getSingleton().playStream(Sound::BUTTON_CLICK);
    switch (index)
    {
        case GuiImageset::GUI_LIST_NPC:
            GuiDialog::getSingleton().mouseEvent(line);
            return;
    }
}

//================================================================================================
// Button was pressed.
//================================================================================================
void GuiWindow::buttonPressed(GuiWindow *me, int index)
{
    Sound::getSingleton().playStream(Sound::BUTTON_CLICK);
    switch (index)
    {
            // Standard buttons.
        case GuiImageset::GUI_BUTTON_CLOSE:
            me->setVisible(false);
            return;
            // Unique buttons.
        case GuiImageset::GUI_BUTTON_NPC_ACCEPT:
            GuiDialog::getSingleton().buttonEvent(0);
            return;
        case GuiImageset::GUI_BUTTON_NPC_DECLINE:
            GuiDialog::getSingleton().buttonEvent(1);
            return;
    }
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "button event... ");
}

