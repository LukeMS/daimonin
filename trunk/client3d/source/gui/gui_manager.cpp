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

const int BORDER = 8;
const uint32 BORDER_COLOR = 0xff888888;
const uint32 BACKGR_COLOR = 0xff444488;
const char *TOOLTIP_LINEBREAK_SIGN = "#";

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

short GuiManager::mWindowZPos[WIN_SUM];
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
    { "ChatWindow",    WIN_CHATWINDOW    },
    { "Statistics",    WIN_STATISTICS    },
};

class GuiWindow guiWindow[GuiManager::WIN_SUM];

GuiManager::ElementID GuiManager::mStateStruct[GUI_ELEMENTS_SUM]=
{
    // Standard Buttons (Handled inside of gui_windows).
    { -1, -1, "But_Close",          GUI_BUTTON_CLOSE    },
    { -1, -1, "But_OK",             GUI_BUTTON_OK       },
    { -1, -1, "But_Cancel",         GUI_BUTTON_CANCEL   },
    { -1, -1, "But_Min",            GUI_BUTTON_MINIMIZE },
    { -1, -1, "But_Max",            GUI_BUTTON_MAXIMIZE },
    { -1, -1, "But_Resize",         GUI_BUTTON_RESIZE   },
    // Unique Buttons (Handled outside of gui_windows).
    { -1, -1, "But_NPC_Accept",     GUI_BUTTON_NPC_ACCEPT },
    { -1, -1, "But_NPC_Decline",    GUI_BUTTON_NPC_DECLINE},
    { -1, -1, "But_Test"       ,    GUI_BUTTON_TEST},
    // Listboxes.
    { -1, -1, "List_Msg",           GUI_LIST_MSGWIN    },
    { -1, -1, "List_Chat",          GUI_LIST_CHATWIN   },
    { -1, -1, "List_NPC",           GUI_LIST_NPC       },
    // Statusbar.
    { -1, -1, "Bar_Health",         GUI_STATUSBAR_NPC_HEALTH    },
    { -1, -1, "Bar_Mana",           GUI_STATUSBAR_PLAYER_MANA   },
    { -1, -1, "Bar_Grace",          GUI_STATUSBAR_PLAYER_GRACE  },
    { -1, -1, "Bar_PlayerHealth",   GUI_STATUSBAR_PLAYER_HEALTH },
    { -1, -1, "Bar_PlayerMana",     GUI_STATUSBAR_NPC_MANA      },
    { -1, -1, "Bar_PlayerGrace",    GUI_STATUSBAR_NPC_GRACE     },
    // TextValues.
    { -1, -1, "Engine_CurrentFPS",  GUI_TEXTBOX_STAT_CUR_FPS   },
    { -1, -1, "Engine_BestFPS",     GUI_TEXTBOX_STAT_BEST_FPS  },
    { -1, -1, "Engine_WorstFPS",    GUI_TEXTBOX_STAT_WORST_FPS },
    { -1, -1, "Engine_SumTris",     GUI_TEXTBOX_STAT_SUM_TRIS  },
    { -1, -1, "Login_ServerInfo1",  GUI_TEXTBOX_SERVER_INFO1     },
    { -1, -1, "Login_ServerInfo2",  GUI_TEXTBOX_SERVER_INFO2     },
    { -1, -1, "Login_ServerInfo3",  GUI_TEXTBOX_SERVER_INFO3     },
    { -1, -1, "Login_LoginWarn",    GUI_TEXTBOX_LOGIN_WARN       },
    { -1, -1, "Login_PswdVerify",   GUI_TEXTBOX_LOGIN_PSWDVERIFY },
    { -1, -1, "Login_LoginInfo1",   GUI_TEXTBOX_LOGIN_INFO1      },
    { -1, -1, "Login_LoginInfo2",   GUI_TEXTBOX_LOGIN_INFO2      },
    { -1, -1, "Login_LoginInfo3",   GUI_TEXTBOX_LOGIN_INFO3      },
    { -1, -1, "NPC_Headline",       GUI_TEXTBOX_NPC_HEADLINE     },
    { -1, -1, "Inv_Equipment",      GUI_TEXTBOX_INV_EQUIP        },
    { -1, -1, "Inv_Equip_Weight",   GUI_TEXTBOX_INV_EQUIP_WEIGHT },
    // TextInput.
    { -1, -1, "Input_Login_Name",   GUI_TEXTINPUT_LOGIN_NAME   },
    { -1, -1, "Input_Login_Passwd", GUI_TEXTINPUT_LOGIN_PASSWD },
    { -1, -1, "Input_Login_Verify", GUI_TEXTINPUT_LOGIN_VERIFY },
    { -1, -1, "Input_NPC_Dialog",   GUI_TEXTINPUT_NPC_DIALOG   },
    // Table
    { -1, -1, "Table_Server",       GUI_TABLE },
    // Combobox.
    { -1, -1, "ComboBoxTest",       GUI_COMBOBOX_TEST  },
    // Element_Slot
    { -1, -1, "Slot_Quickslot",     GUI_SLOT_QUICKSLOT    },
    { -1, -1, "Slot_Equipment",     GUI_SLOT_EQUIPMENT    },
    { -1, -1, "Slot_Inventory",     GUI_SLOT_INVENTORY    },
    { -1, -1, "Slot_Container",     GUI_SLOT_CONTAINER    },
    { -1, -1, "Slot_TradeOffer",    GUI_SLOT_TRADE_OFFER  },
    { -1, -1, "Slot_TradeReturn",   GUI_SLOT_TRADE_RETURN },
    { -1, -1, "Slot_Shop",          GUI_SLOT_SHOP         },
};

const uint32 GuiManager::COLOR_BLACK = 0xff000000;
const uint32 GuiManager::COLOR_BLUE  = 0xff0000ff;
const uint32 GuiManager::COLOR_GREEN = 0xff00ff00;
const uint32 GuiManager::COLOR_LBLUE = 0xff00ffff;
const uint32 GuiManager::COLOR_RED   = 0xffff0000;
const uint32 GuiManager::COLOR_PINK  = 0xffff00ff;
const uint32 GuiManager::COLOR_YELLOW= 0xffffff00;
const uint32 GuiManager::COLOR_WHITE = 0xffffffff;


//================================================================================================
// .
//================================================================================================
int GuiManager::calcTextWidth(unsigned char *text, int fontNr)
{
    return GuiTextout::getSingleton().calcTextWidth(text, fontNr);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::setMouseState(int action)
{
    GuiCursor::getSingleton().setState(action);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::loadRawFont(const char *filename)
{
    GuiTextout::getSingleton().loadRawFont(FILE_SYSTEM_FONT);
}

//================================================================================================
// .
//================================================================================================
int GuiManager::sendMsg(int element, int message, void *parm1, void *parm2, void *parm3)
{
    //if (message == 8 && element > 22) return -1;
    for (unsigned int i = 0; i < GUI_ELEMENTS_SUM; ++i)
    {
        if (element == mStateStruct[i].index && guiWindow[mStateStruct[i].windowNr].isInit())
            return guiWindow[mStateStruct[i].windowNr].sendMsg(mStateStruct[i].winElementNr, message, parm1, parm2, parm3);
    }
    return -1;
}

//================================================================================================
// .
//================================================================================================
int GuiManager::getElementIndex(const char *name, int windowID, int winElementNr)
{
    if (name)
    {
        for (int i = 0; i < GUI_ELEMENTS_SUM; ++i)
        {
            if (mStateStruct[i].name && !stricmp(mStateStruct[i].name, name))
            {
                if (windowID >=0)
                {
                    mStateStruct[i].windowNr = windowID;
                    mStateStruct[i].winElementNr = winElementNr;
                }
                return mStateStruct[i].index;
            }
        }
    }
    return -1;
}

//================================================================================================
// .
//================================================================================================
void GuiManager::setStatusbarValue(int window, int element, Ogre::Real value)
{
    guiWindow[window].setStatusbarValue(element, value);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::Init(int w, int h)
{
    Logger::log().headline() << "Init GUI";
    mDragSrcWin     = NO_ACTIVE_WINDOW;
    mActiveWindow   = NO_ACTIVE_WINDOW;
    mTextInputWindow= NO_ACTIVE_WINDOW;
    mScreenWidth    = w;
    mScreenHeight   = h;
    mBuildBuffer    = 0;
    mMouseInside    = true;
    mTooltipDelay   = 0;
    String strTexture = RESOURCE_TOOLTIP; strTexture+= TEXTURE_RESOURCE_NAME;
    mTexture = TextureManager::getSingleton().createManual(strTexture, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
               TEX_TYPE_2D, TOOLTIP_SIZE, TOOLTIP_SIZE, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY,
               ManResourceLoader::getSingleton().getLoader());
    mTexture->load();
    resizeBuildBuffer(TOOLTIP_SIZE*TOOLTIP_SIZE);
    mElement->setPosition((mScreenWidth-mTexture->getWidth())/3*2, (mScreenHeight-mTexture->getHeight())/2);
    // ////////////////////////////////////////////////////////////////////
    // If requested (by cmd-line) print all element names.
    // ////////////////////////////////////////////////////////////////////
    if (Option::getSingleton().getIntValue(Option::CMDLINE_LOG_GUI_ELEMENTS))
    {
        Logger::log().info() << "These elements are currently known and can be used in " << FILE_GUI_WINDOWS<< ":";
        for (int i =0; i < GUI_ELEMENTS_SUM; ++i) Logger::log().warning() << mStateStruct[i].name;
    }
}

//================================================================================================
// Buildbuffer is used to draw the gui elements before blitting them to the texture.
// The lock()/unlock() alternative gave me some problems (GNU/Linux only) when locking only
// parts of the texture-buffer.
//================================================================================================
void GuiManager::resizeBuildBuffer(size_t newSize)
{
    static size_t size = 0;
    if (newSize != size)
    {
        delete[] mBuildBuffer;
        mBuildBuffer = new uint32[newSize];
        size = newSize;
    }
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
                  TEX_TYPE_2D, w, h, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY, ManResourceLoader::getSingleton().getLoader());
        if (texture.isNull())
        {
            Logger::log().error() << "Could not create " << strTexture;
            return 0;
        }
    }
    // We must clear the whole texture (textures have always 2^n size while our gfx can be smaller).
    int size = texture->getWidth()*texture->getHeight()*sizeof(uint32);
    memset(texture->getBuffer()->lock(0, size, HardwareBuffer::HBL_DISCARD), 0xFF, size);
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
    //drawTooltip();
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

        if (name.find(RESOURCE_DND) != std::string::npos)
        {
            guiWindow[0].loadResources(true);
            return;
        }
        int window = StringConverter::parseInt(name.substr(name.find_first_of("#")+1, SUM_WIN_DIGITS));
        guiWindow[window].loadResources(false);
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
// Parse the Image data.
//================================================================================================
void GuiManager::parseImageset(const char *XML_imageset_file)
{
    GuiImageset::getSingleton().parseXML(XML_imageset_file);
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
        mWindowZPos[i] = i; // default zPos.
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
            sendMsg(mTextInputElement, GuiManager::MSG_SET_TEXT, (void*) mBackupStrTextInput.c_str());
            cancelTextInput();
            return true;
        }
        mTextInputUserAction = GuiTextinput::getSingleton().keyEvent(key, keyChar);
        if (GuiTextinput::getSingleton().wasFinished())
        {
            mStrTextInput = GuiTextinput::getSingleton().getText();
            sendMsg(mTextInputElement, GuiManager::MSG_SET_TEXT, (void*)mStrTextInput.c_str());
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
    return (guiWindow[mActiveWindow].keyEvent(keyChar, key) == EVENT_CHECK_DONE);
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
        if (mouseAction == BUTTON_RELEASED) // End of dragging.
            //if (guiWindow[mDragSrcWin].mouseEvent(mouseAction, mMouse) == EVENT_DRAG_DONE)
        {
            guiWindow[0].hideDragOverlay();
            mDragDstWin = -1;
            for (unsigned int w = 0; w < WIN_SUM; ++w)
            {
                if (guiWindow[w].mouseWithin((int)mMouse.x, (int)mMouse.y), (int) mMouse.z)
                {
                    mDragDstWin = w;
                    //mDragDstSlot = guiWindow[w].getMouseOverSlot((int)mMouse.x, (int)mMouse.y);
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
    //if (guiWindow[mActiveWindow].mouseEvent(mouseAction, mMouse) == EVENT_CHECK_DONE) return (mMouseInside = true);
    for (unsigned int i = 0; i < WIN_SUM; ++i)
    {
        int ret = guiWindow[mWindowZPos[WIN_SUM-i-1]].mouseEvent(mouseAction, mMouse);
        if (ret == EVENT_CHECK_DONE)
        {
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
    mBackupStrTextInput = "";//getElementText(window, element);
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
    while (actPos < WIN_SUM-1)
    {
        mWindowZPos[actPos] = mWindowZPos[actPos+1];
        guiWindow[mWindowZPos[actPos]].setZPos(actPos);
        ++actPos;
    }
    mWindowZPos[WIN_SUM-1] = window;
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
// Update all gui stuff.
//================================================================================================
void GuiManager::update(Real timeSinceLastFrame)
{
    // ////////////////////////////////////////////////////////////////////
    // Update textinput.
    // ////////////////////////////////////////////////////////////////////
    if (mTextInputWindow != NO_ACTIVE_WINDOW)
        sendMsg(mTextInputElement, GuiManager::MSG_SET_TEXT, (void*)GuiTextinput::getSingleton().getText());
    // ////////////////////////////////////////////////////////////////////
    // Update windows.
    // ////////////////////////////////////////////////////////////////////
    for (unsigned int i=0; i < WIN_SUM; ++i)
        guiWindow[i].update(timeSinceLastFrame);
    // ////////////////////////////////////////////////////////////////////
    // Update the tooltip.
    // ////////////////////////////////////////////////////////////////////
    if (mTooltipDelay && Root::getSingleton().getTimer()->getMilliseconds() > mTooltipDelay)
        drawTooltip();
}

//================================================================================================
// Set the tooltip text.
// 0 hides the tooltip.
// # indicates a linebreaks.
// While loading screen is active, the tooltip texture is used for system messages.
//================================================================================================
void GuiManager::setTooltip(const char *text, bool systemMessage)
{
    if (!text || !(*text))
    {
        mStrTooltip ="";
        mOverlay->hide();
        mTooltipDelay = 0;
        return;
    }
    if (systemMessage)
    {
        mStrTooltip+= text;
        mStrTooltip+= TOOLTIP_LINEBREAK_SIGN; // Linebreak.
        mTooltipDelay = 0;
        drawTooltip();
        return;
    }
    mStrTooltip = text;
    mStrTooltip+= TOOLTIP_LINEBREAK_SIGN;
    mTooltipDelay = Root::getSingleton().getTimer()->getMilliseconds() + TOOLTIP_DELAY;
}

//================================================================================================
// Draw the tooltip. Clipping is performed.
//================================================================================================
void GuiManager::drawTooltip()
{
    if (!mStrTooltip.size()) return;
    const int MAX_TOOLTIP_LINES = 16;
    std::string line[MAX_TOOLTIP_LINES];
    int txtWidth[MAX_TOOLTIP_LINES];
    int sumLines = 0;
    int stop, start = 0;
    int fontHeight = GuiTextout::getSingleton().getFontHeight(GuiTextout::FONT_SYSTEM);
    for (sumLines = 0; sumLines < MAX_TOOLTIP_LINES; ++sumLines)
    {
        stop = mStrTooltip.find(TOOLTIP_LINEBREAK_SIGN, start);
        if (stop == (int)std::string::npos)
        {
            if (start < (int)mStrTooltip.size())
                line[sumLines] = mStrTooltip.substr(start, mStrTooltip.size());
            break;
        }
        line[sumLines] = mStrTooltip.substr(start, stop-start);
        start = stop+1;
    }
    if (MAX_TOOLTIP_LINES*fontHeight + 2*BORDER > TOOLTIP_SIZE)
    {
        sumLines =3;
        line[0] = "*ERROR* ";
        line[1] = "Tooltip-text doesnt fit into the texture!";
        line[2] = "change MAX_TOOLTIP_LINES in gui_manager.cpp.";
    }
    if (sumLines >= MAX_TOOLTIP_LINES-1)
        line[MAX_TOOLTIP_LINES-1] = "(...)"; // Show the user that there is no more space left for another textline.
    // ////////////////////////////////////////////////////////////////////
    // Calculate the needed dimension of the tooltip.
    // ////////////////////////////////////////////////////////////////////
    int maxWidth = 0;
    for (int i = 0; i < sumLines; ++i)
    {
        txtWidth[i] = calcTextWidth((unsigned char*)line[i].c_str(), GuiTextout::FONT_SYSTEM);
        if (txtWidth[i] > TOOLTIP_SIZE) txtWidth[i] = TOOLTIP_SIZE;
        if (txtWidth[i] > maxWidth) maxWidth = txtWidth[i];
    }
    maxWidth+=2*BORDER;
    if (maxWidth > TOOLTIP_SIZE) maxWidth = TOOLTIP_SIZE;
    // ////////////////////////////////////////////////////////////////////
    // Draw the background.
    // ////////////////////////////////////////////////////////////////////
    const int w = (int)mTexture->getWidth();
    const int h = (int)mTexture->getHeight();
    uint32 *dest = (uint32*)mTexture->getBuffer()->lock(0, w*h*sizeof(uint32), HardwareBuffer::HBL_DISCARD);
    uint32 *back = dest;
    uint32 color;
    int x,y, endY = (sumLines+1)*fontHeight;
    for (y = 0; y < endY; ++y)
    {
        color =(y < 2 || y >= endY-2)?BORDER_COLOR:BACKGR_COLOR;
        *dest++ = BORDER_COLOR;
        *dest++ = BORDER_COLOR;
        for (x = 2; x < maxWidth-2; ++x) *dest++ = color;
        *dest++ = BORDER_COLOR;
        *dest++ = BORDER_COLOR;
        for (; x < w-2; ++x) *dest++ = 0;
    }
    y = (h-endY) * w;
    for (;--y;) *dest++ = 0;
    maxWidth-=BORDER*2;
    // ////////////////////////////////////////////////////////////////////
    // Draw the text.
    // ////////////////////////////////////////////////////////////////////
    color = BACKGR_COLOR;
    for (int i = 0; i < sumLines; ++i)
    {
        dest = back + BORDER + (BORDER + i*fontHeight) * w;
        GuiTextout::getSingleton().printText(maxWidth, fontHeight, dest, w, &color, 0, line[i].c_str(), GuiTextout::FONT_SYSTEM, 0x00ffffff);
    }
    mTexture->getBuffer()->unlock();
    if (mTooltipDelay)
    {
        x = (int)mMouse.x+40;
        y = (int)mMouse.y+40;
        if (x+ maxWidth > (int)mScreenWidth)  x = mScreenWidth - maxWidth-40;
        if (y+ endY > (int)mScreenHeight) y = mScreenHeight- endY-40;
        mElement->setPosition(x,y);
        mTooltipDelay =0;
    }
    mOverlay->show();
}
