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
const int   GuiManager::SUM_WIN_DIGITS = (int)log10((float)GuiManager::WIN_SUM) +1;
const char *GuiManager::GUI_MATERIAL_NAME     = "GUI/Window";
const char *GuiManager::OVERLAY_ELEMENT_TYPE  = "Panel"; // defined in Ogre::OverlayElementFactory.h
const char *GuiManager::OVERLAY_RESOURCE_NAME = "_Overlay";
const char *GuiManager::ELEMENT_RESOURCE_NAME = "_OverlayElement";
const char *GuiManager::TEXTURE_RESOURCE_NAME = "_Texture";
const char *GuiManager::MATERIAL_RESOURCE_NAME= "_Material";

#define MANAGER_DESCRIPTION "GUI_"
const char *RESOURCE_MCURSOR = MANAGER_DESCRIPTION "MCursor";
const char *RESOURCE_TOOLTIP = MANAGER_DESCRIPTION "Tooltip";
const char *RESOURCE_WINDOW  = MANAGER_DESCRIPTION "Window";
const char *RESOURCE_DND     = MANAGER_DESCRIPTION "DnD";

unsigned char GuiManager::guiWindowZPos[WIN_SUM];
GuiManager::WindowID GuiManager::mWindowID[WIN_SUM]=
{
    { "Login",         WIN_LOGIN         },
    { "ServerSelect",  WIN_SERVERSELECT  },
    //{ "Creation",      WIN_CREATION      },

    { "Win_Equipment", WIN_EQUIPMENT     },
    { "Win_Inventory", WIN_INVENTORY     },
    { "Win_Trade",     WIN_TRADE         },
    { "Win_Shop",      WIN_SHOP          },
    { "Win_Container", WIN_CONTAINER     },
    { "Win_TileGround",WIN_TILEGROUND    },

    { "PlayerInfo",    WIN_PLAYERINFO    },
    { "PlayerConsole", WIN_PLAYERCONSOLE },

    { "DialogNPC",     WIN_NPCDIALOG     },
    { "TextWindow",    WIN_TEXTWINDOW    },
    { "ChatWindow",    WIN_TEXTWINDOW    },
    { "Statistics",    WIN_STATISTICS    },
};
class GuiWindow GuiManager::guiWindow[WIN_SUM];

//================================================================================================
// .
//================================================================================================
void GuiManager::Init(int w, int h)
{
    Logger::log().headline() << "Init GUI";
    mDragSrcWin     = NO_ACTIVE_WINDOW;
    mActiveWindow   = NO_ACTIVE_WINDOW;
    mTextInputWindow = NO_ACTIVE_WINDOW;
    mScreenWidth    = w;
    mScreenHeight   = h;
    mMouseInside    = true;
    mTooltipRefresh = false;
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
Overlay *GuiManager::loadResources(int w, int h, String name)
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
void GuiManager::loadResources()
{
    mOverlay= loadResources(TOOLTIP_SIZE, TOOLTIP_SIZE, RESOURCE_TOOLTIP);
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
        GuiCursor::getSingleton().loadResources();
        return;
    }
    if (name.find(RESOURCE_TOOLTIP) != std::string::npos)
    {
        loadResources();
        return;
    }
    if (name.find(RESOURCE_WINDOW)  != std::string::npos)
    {
        int window = StringConverter::parseInt(name.substr(name.find_first_of("#")+1, SUM_WIN_DIGITS));
        if (name.find(RESOURCE_DND) != std::string::npos)
            guiWindow[window].loadDnDResources();
        else
            guiWindow[window].loadResources();
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
// Parse the windows data.
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
    if ((xmlElem = xmlRoot->FirstChildElement("Cursor")) && ((valString = xmlElem->Attribute("name"))))
    {
        if (GuiImageset::getSingleton().getStateGfxPosMouse())
        {
            mHotSpotX = mHotSpotY =0;
            if ((xmlElem = xmlElem->FirstChildElement("HotSpotOffset")))
            {
                if ((valString = xmlElem->Attribute("x"))) mHotSpotX = atoi(valString);
                if ((valString = xmlElem->Attribute("y"))) mHotSpotY = atoi(valString);
            }
            GuiCursor::getSingleton().Init(RESOURCE_MCURSOR);
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
    int z=0;
    for (int i = 0; i < WIN_SUM; ++i)
        guiWindowZPos[i] = i; // default zPos.
    for (xmlElem = xmlRoot->FirstChildElement("Window"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Window"))
    {
        if (!(valString = xmlElem->Attribute("name"))) continue;
        for (int winNr = 0; winNr < WIN_SUM; ++winNr)
        {
            if (!stricmp(mWindowID[winNr].name, valString))
            {
                guiWindow[winNr].Init(xmlElem, RESOURCE_WINDOW, RESOURCE_DND, winNr, z++);
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
    for (int i=0; i < WIN_SUM; ++i) guiWindow[i].freeRecources();
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
    if (mTextInputWindow != NO_ACTIVE_WINDOW && mTextInputWindow == mActiveWindow)
    {
        if (key == OIS::KC_ESCAPE)
        {
            setElementText(mTextInputWindow, mTextInputElement, mBackupStrTextInput.c_str());
            cancelTextInput();
            return true;
        }
        mTextInputUserAction = GuiTextinput::getSingleton().keyEvent(key, keyChar);
        if (GuiTextinput::getSingleton().wasFinished())
        {
            mStrTextInput = GuiTextinput::getSingleton().getText();
            setElementText(mTextInputWindow, mTextInputElement, mStrTextInput.c_str());
            GuiTextinput::getSingleton().stop();
            mTextInputWindow = NO_ACTIVE_WINDOW;
        }
        return true;
    }
    // Activate the next window.
    if (key == OIS::KC_TAB)
    {
        // ToDo.
    }
    // Key event in active window.
    if (mActiveWindow == NO_ACTIVE_WINDOW) return false;
    return guiWindow[mActiveWindow].keyEvent(keyChar, key);
}

//================================================================================================
// .
//================================================================================================
bool GuiManager::mouseEvent(int mouseAction, Vector3 &mouse)
{
    mMouse = mouse;
    GuiCursor::getSingleton().setPos((int)mMouse.x, (int)mMouse.y);
    mMouse.x+= mHotSpotX;
    mMouse.y+= mHotSpotY;
    // ////////////////////////////////////////////////////////////////////
    // Do we have an active drag from a slot?
    // ////////////////////////////////////////////////////////////////////
    if (mDragSrcWin != NO_ACTIVE_WINDOW)
    {
        if (mouseAction == GuiWindow::BUTTON_RELEASED) // End of dragging.
            //if (guiWindow[mDragSrcWin].mouseEvent(mouseAction, mMouse) == EVENT_DRAG_DONE)
        {
            guiWindow[0].hideDragOverlay();
            mDragDstWin = -1;
            for (unsigned int w = 0; w < WIN_SUM; ++w)
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
            mDragSrcWin = NO_ACTIVE_WINDOW;
        }
        guiWindow[0].moveDragOverlay();
        return true;
    }
    // ////////////////////////////////////////////////////////////////////
    // Check for mouse action in all windows.
    // ////////////////////////////////////////////////////////////////////
    for (unsigned int i=0; i < WIN_SUM; ++i)
    {
        int ret = guiWindow[i].mouseEvent(mouseAction, mMouse);
        if (ret == EVENT_CHECK_DONE)
        {
            if (mouseAction == GuiWindow::BUTTON_PRESSED)
                windowToFront(i);
            mActiveWindow = i;
            return (mMouseInside = true);
        }
        if (ret == EVENT_DRAG_STRT)
        {
            mDragSrcWin = i;
            mDragSrcSlot= guiWindow[i].getDragSlot();
            return true;
        }
    }
    return (mMouseInside = false);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::startTextInput(int window, int element, int maxChars, bool blockNumbers, bool blockWhitespaces)
{
    if (mTextInputWindow != NO_ACTIVE_WINDOW || !guiWindow[window].isVisible()) return;
    mTextInputWindow = window;
    mTextInputElement= element;
    mBackupStrTextInput = getElementText(window, element);
    GuiTextinput::getSingleton().setString(mBackupStrTextInput);
    GuiTextinput::getSingleton().startTextInput(maxChars, blockNumbers, blockWhitespaces);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::cancelTextInput()
{
    GuiTextinput::getSingleton().canceled();
    mTextInputWindow = NO_ACTIVE_WINDOW;
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
void GuiManager::resetTextInput()
{
    GuiTextinput::getSingleton().reset();
}

//================================================================================================
// .
//================================================================================================
void GuiManager::windowToFront(int window)
{
    unsigned char actPos = guiWindow[window].getZPos();
    while (actPos != WIN_SUM-1)
    {
        guiWindowZPos[actPos] = guiWindowZPos[actPos+1];
        guiWindow[guiWindowZPos[actPos]].setZPos(actPos);
        ++actPos;
    }
    guiWindowZPos[actPos] = window;
    guiWindow[window].setZPos(WIN_SUM-1);
    mActiveWindow = window;
}

//================================================================================================
// .
//================================================================================================
void GuiManager::showWindow(int window, bool visible)
{
    guiWindow[window].setVisible(visible);
    if (visible)
    {
        windowToFront(window);
        return;
    }
    if (window == mTextInputWindow)
        mTextInputWindow = NO_ACTIVE_WINDOW;
    if (window == mActiveWindow)
        mActiveWindow = NO_ACTIVE_WINDOW;
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
    if (mTextInputWindow != NO_ACTIVE_WINDOW)
        setElementText(mTextInputWindow, mTextInputElement, GuiTextinput::getSingleton().getText());
    // ////////////////////////////////////////////////////////////////////
    // Update windows.
    // ////////////////////////////////////////////////////////////////////
    for (unsigned int i=0; i < WIN_SUM; ++i)
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
