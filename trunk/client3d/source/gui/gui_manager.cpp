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
#include <cmath>
#include <tinyxml.h>
#include <OISKeyboard.h>
#include <OgreFontManager.h>
#include "define.h"
#include "gui_manager.h"
#include "gui_window_dialog.h"
#include "gui_window.h"
#include "gui_cursor.h"
#include "gui_textinput.h"
#include "option.h"
#include "logger.h"
#include "resourceloader.h"

using namespace Ogre;

static const int TOOLTIP_SIZE = 1 << 8;
static const unsigned long TOOLTIP_DELAY = 2000; // Wait x ms before showing the tooltip.
const int   GuiManager::SUM_WIN_DIGITS = (int)log10((float)GuiManager::GUI_WIN_SUM) +1;
const char *GuiManager::GUI_MATERIAL_NAME     = "GUI/Window";
const char *GuiManager::OVERLAY_ELEMENT_TYPE  = "Panel"; // defined in Ogre::OverlayElementFactory.h
const char *GuiManager::OVERLAY_RESOURCE_NAME = "_Overlay";
const char *GuiManager::ELEMENT_RESOURCE_NAME = "_OverlayElement";
const char *GuiManager::TEXTURE_RESOURCE_NAME = "_Texture";
const char *GuiManager::MATERIAL_RESOURCE_NAME= "_Material";
enum
{
    MAX_OVERLAY_ZPOS = 550,
    CSR_OVERLAY_ZPOS = 540,
    DND_OVERLAY_ZPOS = 530,
    TTP_OVERLAY_ZPOS = 520,
    WIN_OVERLAY_ZPOS = 500
};

#define MANAGER_DESCRIPTION "GUI_"
const char *RESOURCE_MCURSOR = MANAGER_DESCRIPTION "MCursor";
const char *RESOURCE_TOOLTIP = MANAGER_DESCRIPTION "Tooltip";
const char *RESOURCE_WINDOW  = MANAGER_DESCRIPTION "Window";
const char *RESOURCE_DND     = MANAGER_DESCRIPTION "DnD";

GuiManager::GuiWinNam GuiManager::mGuiWindowNames[GUI_WIN_SUM]=
{
    { "Login",         GUI_WIN_LOGIN         },
    { "ServerSelect",  GUI_WIN_SERVERSELECT  },
    //{ "Creation",      GUI_WIN_CREATION      },

    { "Win_Equipment", GUI_WIN_EQUIPMENT     },
    { "Win_Inventory", GUI_WIN_INVENTORY     },
    { "Win_Trade",     GUI_WIN_TRADE         },
    { "Win_Shop",      GUI_WIN_SHOP          },
    { "Win_Container", GUI_WIN_CONTAINER     },
    { "Win_TileGround",GUI_WIN_TILEGROUND    },

    { "PlayerInfo",    GUI_WIN_PLAYERINFO    },
    { "PlayerConsole", GUI_WIN_PLAYERCONSOLE },

    { "DialogNPC",     GUI_WIN_NPCDIALOG     },
    { "TextWindow",    GUI_WIN_TEXTWINDOW    },
    { "ChatWindow",    GUI_WIN_TEXTWINDOW    },
    { "Statistics",    GUI_WIN_STATISTICS    },
};
class GuiWindow GuiManager::guiWindow[GUI_WIN_SUM];

//================================================================================================
// .
//================================================================================================
void GuiManager::Init(int w, int h)
{
    Logger::log().headline() << "Init GUI";
    mDragSrcWin     = -1;
    mScreenWidth    = w;
    mScreenHeight   = h;
    mMouseInside    = true;
    mTooltipRefresh = false;
    mActiveTextInput= false;
    String strTexture = RESOURCE_TOOLTIP; strTexture+= TEXTURE_RESOURCE_NAME;
    mTexture = TextureManager::getSingleton().createManual(strTexture, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
              TEX_TYPE_2D, TOOLTIP_SIZE, TOOLTIP_SIZE, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY,
              ManResourceLoader::getSingleton().getLoader());
    mTexture->load();
    mElement->setPosition((mScreenWidth-mTexture->getWidth())/3*2, (mScreenHeight-mTexture->getHeight())/2);
}

//================================================================================================
// (Re)loads the material and texture or creates them if they dont exist.
//================================================================================================
Overlay *GuiManager::loadResources(int w, int h, String name, int posZ)
{
    String strOverlay = name + OVERLAY_RESOURCE_NAME;
    String strElement = name + ELEMENT_RESOURCE_NAME;
    String strTexture = name + TEXTURE_RESOURCE_NAME;
    String strMaterial= name + MATERIAL_RESOURCE_NAME;
    Overlay *overlay = OverlayManager::getSingleton().getByName(strOverlay);
    if (!overlay)
    {
        OverlayElement *element = OverlayManager::getSingleton().createOverlayElement(OVERLAY_ELEMENT_TYPE, strElement);
        if (!element)
        {
            Logger::log().error() << "Could not create " << strElement;
            return 0;
        }
        element->setMetricsMode(GMM_PIXELS);
        overlay = OverlayManager::getSingleton().create(strOverlay);
        if (!overlay)
        {
            Logger::log().error() << "Could not create " << strElement;
            return 0;
        }
        overlay->add2D(static_cast<OverlayContainer*>(element));
        overlay->setZOrder(posZ);
    }
    OverlayElement *element = overlay->getChild(strElement);
    MaterialPtr material = MaterialManager::getSingleton().getByName(strMaterial);
    if (material.isNull())
    {
        material = MaterialManager::getSingleton().getByName(GUI_MATERIAL_NAME);
        if (material.isNull())
        {
            Logger::log().info() << "Material definition '" << GUI_MATERIAL_NAME
            << "' was not found in the default folders. Using a hardcoded material.";
            material = MaterialManager::getSingleton().create(GUI_MATERIAL_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            if (material.isNull())
            {
                Logger::log().error() << "Could not create default material " << GUI_MATERIAL_NAME;
                return 0;
            }
            material->setLightingEnabled(false);
            material->setDepthWriteEnabled(false);
            material->setDepthCheckEnabled(false);
            material->setSceneBlending(SBT_TRANSPARENT_ALPHA);
            material->getTechnique(0)->getPass(0)->createTextureUnitState();
            material->getTechnique(0)->getPass(0)->setAlphaRejectSettings(CMPF_GREATER, 128);
            material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(TFO_NONE);
        }
        material = material->clone(strMaterial);
        if (material.isNull())
        {
            Logger::log().error() << "Could not create " << strMaterial;
            return 0;
        }
    }
    TexturePtr texture = TextureManager::getSingleton().getByName(strTexture);
    if (texture.isNull())
    {
        texture = TextureManager::getSingleton().createManual(strTexture, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                  TEX_TYPE_2D, w, h, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY,
                  ManResourceLoader::getSingleton().getLoader());
        if (texture.isNull())
        {
            Logger::log().error() << "Could not create " << strTexture;
            return 0;
        }
    }
    // We must clear the whole texture (textures have always 2^n size while our gfx can be smaller).
    memset(texture->getBuffer()->lock(HardwareBuffer::HBL_DISCARD), 0x00, texture->getWidth()*texture->getHeight()*sizeof(uint32));
    texture->getBuffer()->unlock();

    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(strTexture);
    element->setDimensions(texture->getWidth(), texture->getHeight());
    element->setMaterialName(strMaterial);
    return overlay;
}

//================================================================================================
// (Re)loads the material and texture or creates them if they dont exist.
//================================================================================================
void GuiManager::loadResources(int posZ)
{
    mOverlay= loadResources(TOOLTIP_SIZE, TOOLTIP_SIZE, RESOURCE_TOOLTIP, posZ);
    String strElement = RESOURCE_TOOLTIP; strElement+= ELEMENT_RESOURCE_NAME;
    mElement= mOverlay->getChild(strElement);
    clearTooltip();
}

//================================================================================================
// Reload a manual resource.
//================================================================================================
void GuiManager::loadResources(Ogre::Resource *res)
{
    String name = res->getName();
    Logger::log().info() << "(Re)loading resource " << name;
    if (name.find(RESOURCE_MCURSOR) != std::string::npos)
    {
        GuiCursor::getSingleton().loadResources(CSR_OVERLAY_ZPOS);
        return;
    }
    if (name.find(RESOURCE_TOOLTIP) != std::string::npos)
    {
        loadResources(TTP_OVERLAY_ZPOS);
        return;
    }
    if (name.find(RESOURCE_WINDOW)  != std::string::npos)
    {
        int window = StringConverter::parseInt(name.substr(name.find_first_of("#")+1, SUM_WIN_DIGITS));
        if (name.find(RESOURCE_DND) != std::string::npos)
            guiWindow[window].loadDnDResources(WIN_OVERLAY_ZPOS);
        else
            guiWindow[window].loadResources(WIN_OVERLAY_ZPOS);
        return;
    }
    if (name.find(ManResourceLoader::TEMP_RESOURCE) != std::string::npos)
    {
        // No problem for a temporary resource to loose its content. Reloading will be ignored.
        return;
    }
    Logger::log().error() << "Resource " << name << " could not be found!";
}

//================================================================================================
// .
//================================================================================================
void GuiManager::centerWindowOnMouse(int window)
{
    guiWindow[window].centerWindowOnMouse((int)mMouse.x, (int)mMouse.y);
}

//================================================================================================
// Parse the windows datas.
//================================================================================================
void GuiManager::parseWindows(const char *fileWindows)
{
    TiXmlElement *xmlRoot, *xmlElem;
    TiXmlDocument doc(fileWindows);
    const char *valString;
    // ////////////////////////////////////////////////////////////////////
    // Check for a working window description.
    // ////////////////////////////////////////////////////////////////////
    if (!doc.LoadFile(fileWindows) || !(xmlRoot = doc.RootElement()))
    {
        Logger::log().error() << "XML-File '" << fileWindows << "' is missing or broken.";
        return;
    }
    if ((valString = xmlRoot->Attribute("name")))
        Logger::log().info() << "Parsing '" << valString << "' in file" << fileWindows << ".";
    else
        Logger::log().error() << "File '" << fileWindows << "' has no name entry.";
    // ////////////////////////////////////////////////////////////////////
    // Parse the fonts.
    // ////////////////////////////////////////////////////////////////////
    int sumEntries =0;
    if ((xmlElem = xmlRoot->FirstChildElement("Fonts")))
    {
        for (xmlElem = xmlElem->FirstChildElement("Font"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Font"))
        {
            if (!(valString = xmlElem->Attribute("type"))) continue;
            if (!stricmp(valString, "RAW"))
            {
                GuiTextout::getSingleton().loadRawFont(xmlElem->Attribute("name"));
                ++sumEntries;
            }
            else if (!stricmp(valString, "TTF"))
            {
                GuiTextout::getSingleton().loadTTFont(
                    xmlElem->Attribute("name"),
                    xmlElem->Attribute("size"),
                    xmlElem->Attribute("resolution"));
                ++sumEntries;
            }
        }
        Logger::log().info() << sumEntries << " Fonts were parsed.";
    }
    else
    {
        Logger::log().error() << "CRITICAL: No fonts found in " << fileWindows;
    }
    // ////////////////////////////////////////////////////////////////////
    // Parse the mouse-cursor.
    // ////////////////////////////////////////////////////////////////////
    GuiImageset::gfxSrcMouse *srcEntry;
    if ((xmlElem = xmlRoot->FirstChildElement("Cursor")) && ((valString = xmlElem->Attribute("name"))))
    {
        srcEntry = GuiImageset::getSingleton().getStateGfxPosMouse();
        if (srcEntry)
        {
            mHotSpotX = mHotSpotY =0;
            if ((xmlElem = xmlElem->FirstChildElement("HotSpotOffset")))
            {
                if ((valString = xmlElem->Attribute("x"))) mHotSpotX = atoi(valString);
                if ((valString = xmlElem->Attribute("y"))) mHotSpotY = atoi(valString);
            }
            GuiCursor::getSingleton().Init(srcEntry->w, srcEntry->h, RESOURCE_MCURSOR);
            GuiCursor::getSingleton().setStateImagePos(srcEntry->state);
        }
        else
        {
            Logger::log().warning() << "ImageSet has no mouse-cursor defined.";
        }
    }
    else
    {
        Logger::log().error() << "File '" << fileWindows << "' has no mouse-cursor defined.";
    }
    // ////////////////////////////////////////////////////////////////////
    // Init the windows.
    // ////////////////////////////////////////////////////////////////////
    for (xmlElem = xmlRoot->FirstChildElement("Window"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Window"))
    {
        if (!(valString = xmlElem->Attribute("name"))) continue;
        for (int i = 0; i < GUI_WIN_SUM; ++i)
        {
            if (!stricmp(mGuiWindowNames[i].name, valString))
            {
                guiWindow[i].Init(xmlElem, WIN_OVERLAY_ZPOS, RESOURCE_WINDOW, RESOURCE_DND, i);
                break;
            }
        }
    }
}

//================================================================================================
// .
//================================================================================================
int GuiManager::addTextline(int window, int element, const char *text, uint32 color)
{
    return guiWindow[window].addTextline(element, text, color);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::freeRecources()
{
    for (int i=0; i < GUI_WIN_SUM; ++i) guiWindow[i].freeRecources();
    GuiCursor::getSingleton().freeRecources();
    mTexture.setNull();
}

//================================================================================================
// KeyEvent was reported.
// The decision if a keypress belongs to gui is made in events.cpp.
//================================================================================================
bool GuiManager::keyEvent(const int key, const unsigned int keyChar)
{
    // Key event in npc-dialog window.
    if (GuiDialog::getSingleton().keyEvent(key, keyChar)) return true;
    // We have an active Textinput.
    if (mActiveTextInput)
    {
        if (key == OIS::KC_ESCAPE)
        {
            sendMessage(mActiveWindow, GUI_MSG_TXT_CHANGED, mActiveElement, (void*)mBackupTextInputString.c_str());
            GuiTextinput::getSingleton().canceled();
            mActiveTextInput = false;
            return true;
        }
        GuiTextinput::getSingleton().keyEvent(key, keyChar);
        if (GuiTextinput::getSingleton().wasFinished())
        {
            mStrTextInput = GuiTextinput::getSingleton().getText();
            sendMessage(mActiveWindow, GUI_MSG_TXT_CHANGED, mActiveElement, (void*)mStrTextInput.c_str());
            GuiTextinput::getSingleton().stop();
            mActiveTextInput = false;
        }
        return true;
    }
    // Activate the next window.
    if (key == OIS::KC_TAB)
    {
        // ToDo.
    }
    // Key event in active window.
    return guiWindow[mActiveWindow].keyEvent(keyChar, key);
}

//================================================================================================
// .
//================================================================================================
bool GuiManager::mouseEvent(int mouseAction, Vector3 &mouse)
{
    mMouse.x = mouse.x;
    mMouse.y = mouse.y;
    mMouse.z = mouse.z;
    GuiCursor::getSingleton().setPos((int)mMouse.x, (int)mMouse.y);
    mMouse.x+= mHotSpotX;
    mMouse.y+= mHotSpotY;
    // ////////////////////////////////////////////////////////////////////
    // Do we have an active drag from a slot?
    // ////////////////////////////////////////////////////////////////////
    if (mDragSrcWin >= 0)
    {
        if (mouseAction == GuiWindow::BUTTON_RELEASED) // End of dragging.
            //if (guiWindow[mDragSrcWin].mouseEvent(mouseAction, mMouse) == EVENT_DRAG_DONE)
        {
            guiWindow[0].hideDragOverlay();
            mDragDstWin = -1;
            for (unsigned int w = 0; w < GUI_WIN_SUM; ++w)
            {
                if (guiWindow[w].mouseWithin((int)mMouse.x, (int)mMouse.y))
                {
                    mDragDstWin = w;
                    mDragDstSlot = guiWindow[w].getMouseOverSlot((int)mMouse.x, (int)mMouse.y);
                    break;
                }
            }
            // Drop the item.
            Item::getSingleton().dropItem(mDragSrcWin, mDragSrcSlot, mDragDstWin, mDragDstSlot);
            mDragSrcWin = -1;
        }
        guiWindow[0].moveDragOverlay();
        return true;
    }
    // ////////////////////////////////////////////////////////////////////
    // Check for mouse action in all windows.
    // ////////////////////////////////////////////////////////////////////
    for (unsigned int i=0; i < GUI_WIN_SUM; ++i)
    {
        int ret = guiWindow[i].mouseEvent(mouseAction, mMouse);
        if (ret == EVENT_CHECK_DONE)
        {
            mActiveWindow = i;
            mMouseInside = true;
            return true;
        }
        if (ret == EVENT_DRAG_STRT)
        {
            mDragSrcWin = i;
            mDragSrcSlot= guiWindow[i].getDragSlot();
            return true;
        }
    }
    mMouseInside = false;
    return false;
}

//================================================================================================
// Send a message to a GuiWindow.
//================================================================================================
const char *GuiManager::sendMessage(int window, int message, int element, void *value1, void *value2)
{
    return guiWindow[window].Message(message, element, value1, value2);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::startTextInput(int window, int winElement, int maxChars, bool blockNumbers, bool blockWhitespaces)
{
    if (mActiveTextInput || !guiWindow[window].isVisible()) return;
    mActiveTextInput = true;
    mActiveWindow = window;
    mActiveElement= winElement;
    const char *tmp = sendMessage(mActiveWindow, GUI_MSG_TXT_GET, mActiveElement);
    if (tmp)
        mBackupTextInputString = tmp;
    else
        mBackupTextInputString = "";
    GuiTextinput::getSingleton().setString(mBackupTextInputString);
    GuiTextinput::getSingleton().startTextInput(maxChars, blockNumbers, blockWhitespaces);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::cancelTextInput()
{
    GuiTextinput::getSingleton().canceled();
    mActiveWindow = GUI_WIN_STATISTICS;
}

//================================================================================================
// .
//================================================================================================
void GuiManager::resetTextInput()
{
    GuiTextinput::getSingleton().reset();
}

//================================================================================================
// .
//================================================================================================
bool GuiManager::brokenTextInput()
{
    return GuiTextinput::getSingleton().wasCanceled();
}

//================================================================================================
// .
//================================================================================================
bool GuiManager::finishedTextInput()
{
    return GuiTextinput::getSingleton().wasFinished();
}

//================================================================================================
// .
//================================================================================================
const char *GuiManager::getTextInput()
{
    return mStrTextInput.c_str();
}

//================================================================================================
// .
//================================================================================================
void GuiManager::showWindow(int window, bool visible)
{
    guiWindow[window].setVisible(visible);
    if (visible)
        mActiveWindow = window;
    else
        mActiveWindow = GUI_WIN_STATISTICS;
}

//================================================================================================
// While loading screen is active, the tooltip texture is used for system messages.
//================================================================================================
void GuiManager::displaySystemMessage(const char *text)
{
    static int row =0;
    if (!text || !text[0])
    {
        row = 0;
        mOverlay->hide();
        return;
    }
    const int BORDER = 8;
    int fontH = GuiTextout::getSingleton().getFontHeight(GuiTextout::FONT_SYSTEM);
    if ((row+1)*fontH+2*BORDER >= (int)mTexture->getHeight())
    {
        row = 0;
        clearTooltip();
    }
    GuiTextout::TextLine label;
    label.index= -1;
    label.hideText= false;
    label.font = GuiTextout::FONT_SYSTEM;
    label.y1 = fontH * row + BORDER;
    label.y2 = label.y1 + fontH;
    label.x1 = BORDER;
    label.x2 = (int)mTexture->getWidth()-BORDER;
    label.text = text;
    label.color= 0x00ffffff;
    GuiTextout::getSingleton().Print(&label, mTexture.getPointer());
    mTooltipRefresh = false;
    mOverlay->show();
    ++row;
}

//================================================================================================
// Update all gui stuff.
//================================================================================================
void GuiManager::update(Real timeSinceLastFrame)
{
    // ////////////////////////////////////////////////////////////////////
    // Update textinput.
    // ////////////////////////////////////////////////////////////////////
    if (mActiveTextInput)
        sendMessage(mActiveWindow, GUI_MSG_TXT_CHANGED, mActiveElement, (void*)GuiTextinput::getSingleton().getText());
    // ////////////////////////////////////////////////////////////////////
    // Update windows.
    // ////////////////////////////////////////////////////////////////////
    for (unsigned int i=0; i < GUI_WIN_SUM; ++i)
        guiWindow[i].update(timeSinceLastFrame);
    // ////////////////////////////////////////////////////////////////////
    // Update tooltips.
    // ////////////////////////////////////////////////////////////////////
    if (mTooltipRefresh && Root::getSingleton().getTimer()->getMilliseconds() > mTooltipDelay)
    {
        // TODO: Make the background fit to the text.
        GuiTextout::TextLine label;
        label.hideText= false;
        label.index= -1;
        label.font = 2;
        label.color =0;
        label.x1 = label.y1 = 2;
        label.x2 = TOOLTIP_SIZE;
        label.y2 = GuiTextout::getSingleton().getFontHeight(label.font);
        label.text = mStrTooltip;
        clearTooltip();
        GuiTextout::getSingleton().Print(&label, mTexture.getPointer());
        mElement->setPosition((int)mMouse.x+33, (int)mMouse.y+38); // TODO:
        mOverlay->show();
        mTooltipRefresh = false;
    }
}

//================================================================================================
// Set a tooltip text. 0 hides the tooltip.
//================================================================================================
void GuiManager::setTooltip(const char *text)
{
    if (!text || !(*text))
    {
        mTooltipRefresh = false;
        mOverlay->hide();
    }
    else
    {
        mTooltipRefresh = true;
        mStrTooltip = text;
        mTooltipDelay = Root::getSingleton().getTimer()->getMilliseconds() + TOOLTIP_DELAY;
    }
}

//================================================================================================
// Fill the tooltip overlay with the default color (overwrite the old text).
//================================================================================================
void GuiManager::clearTooltip()
{
#define BORDER_COLOR 0xff888888
#define BACKGR_COLOR 0xff444488
    const int w = (int)mTexture->getWidth();
    const int h = (int)mTexture->getHeight();
    uint32 *dest = (uint32*)mTexture->getBuffer()->lock(0, w*h*sizeof(uint32), HardwareBuffer::HBL_DISCARD);
    uint32 color;
    for (int x = 0; x < w; ++x)
    {
        color = (x < 2 || x >= w-2)?BORDER_COLOR:BACKGR_COLOR;
        for (int y =   0; y <   2; ++y) *dest++ = BORDER_COLOR;
        for (int y =   2; y < h-2; ++y) *dest++ = color;
        for (int y = h-2; y < h  ; ++y) *dest++ = BORDER_COLOR;
    }
    mTexture->getBuffer()->unlock();
}
