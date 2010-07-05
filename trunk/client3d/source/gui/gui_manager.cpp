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

#include <OgreRoot.h>
#include <OgreTimer.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include <OgreOverlayManager.h>
#include <OgreMaterialManager.h>
#include <OgreOverlayContainer.h>
#include <OgreHardwarePixelBuffer.h>
#include <OISKeyboard.h>
#include "logger.h"
#include "profiler.h"
#include "gui/gui_manager.h"
#include "gui/gui_window.h"
#include "gui/gui_cursor.h"
#include "gui/gui_textinput.h"

using namespace Ogre;

Overlay        *GuiManager::mOverlay = 0;
OverlayElement *GuiManager::mElement = 0;

const int   GuiManager::SUM_WIN_DIGITS = (int)log10((float)GuiManager::WIN_SUM) +1;
const char *GuiManager::TEXTURE_RESOURCE_NAME   = "_Texture";
const char *GuiManager::FILE_TXT_WINDOWS  = "GUI_Windows.xml";
const char *GuiManager::FILE_TXT_IMAGESET = "GUI_ImageSet.xml";
const char *GuiManager::FILE_ITEM_ATLAS   = "Atlas_Gui_Items.png";
const char *GuiManager::FILE_ITEM_UNKNOWN = "item_noGfx.png";
const char *GuiManager::FILE_SYSTEM_FONT  = "SystemFont.png";

const uint32 GuiManager::COLOR_BLACK = 0xff000000;
const uint32 GuiManager::COLOR_BLUE  = 0xff0000ff;
const uint32 GuiManager::COLOR_GREEN = 0xff00ff00;
const uint32 GuiManager::COLOR_LBLUE = 0xff00ffff;
const uint32 GuiManager::COLOR_RED   = 0xffff0000;
const uint32 GuiManager::COLOR_PINK  = 0xffff00ff;
const uint32 GuiManager::COLOR_YELLOW= 0xffffff00;
const uint32 GuiManager::COLOR_WHITE = 0xffffffff;

static const int BORDER = 8;
static const int TOOLTIP_SIZE = 1 << 8;
static const unsigned long TOOLTIP_DELAY = 2000; // Wait x ms before showing the tooltip.
static const uint32 BORDER_COLOR = 0xff888888;
static const uint32 BACKGR_COLOR = 0xff444488;
static const char *RESOURCE_MCURSOR = "GUI-MCursor";
static const char *RESOURCE_TOOLTIP = "GUI-Tooltip";
static const char *RESOURCE_WINDOW  = "GUI-Window";
static const char *TOOLTIP_LINEBREAK_SIGN = "#";

GuiManager::WindowID GuiManager::mWindowID[WIN_SUM]=
{
    { "Win_Login",         WIN_LOGIN         },
    { "Win_ServerSelect",  WIN_SERVERSELECT  },
    { "Win_Equipment",     WIN_EQUIPMENT     },
    { "Win_Inventory",     WIN_INVENTORY     },
    { "Win_Trade",         WIN_TRADE         },
    { "Win_Shop",          WIN_SHOP          },
    { "Win_Container",     WIN_CONTAINER     },
    { "Win_TileGround",    WIN_TILEGROUND    },
    { "Win_Dialog",        WIN_NPCDIALOG     },
    { "Win_PlayerInfo",    WIN_PLAYERINFO    },
    { "Win_TextWindow",    WIN_TEXTWINDOW    },
    { "Win_ChatWindow",    WIN_CHATWINDOW    },
    { "Win_Statistics",    WIN_STATISTICS    },
    { "Win_PlayerTarget",  WIN_PLAYERTARGET  },
//  { "Win_Creation",      WIN_CREATION      },
};

class GuiWindow guiWindow[GuiManager::WIN_SUM];
short GuiManager::mWindowZPos[WIN_SUM];

GuiManager::ElementID GuiManager::mStateStruct[ELEMENTS_SUM]=
{
    /** User action on these elements will be handled inside the gui only.**/
    { -1, -1, "But_Close",          BUTTON_CLOSE    },
    { -1, -1, "But_Min",            BUTTON_MINIMIZE },
    { -1, -1, "But_Max",            BUTTON_MAXIMIZE },
    { -1, -1, "But_Resize",         BUTTON_RESIZE   },
    /** User action on these elements will be send to the world outside. **/
    // Buttons.
    { -1, -1, "But_NPC_Accept",     BUTTON_NPC_ACCEPT },
    { -1, -1, "But_NPC_Decline",    BUTTON_NPC_DECLINE},
    { -1, -1, "But_Test"       ,    BUTTON_TEST},
    // Listboxes.
    { -1, -1, "List_Msg",           LIST_MSGWIN    },
    { -1, -1, "List_Chat",          LIST_CHATWIN   },
    { -1, -1, "List_NPC",           LIST_NPC       },
    // Statusbar.
    { -1, -1, "Bar_Mana",           STATUSBAR_NPC_MANA      },
    { -1, -1, "Bar_Grace",          STATUSBAR_NPC_GRACE     },
    { -1, -1, "Bar_Health",         STATUSBAR_NPC_HEALTH    },
    { -1, -1, "Bar_PlayerMana",     STATUSBAR_PLAYER_MANA   },
    { -1, -1, "Bar_PlayerGrace",    STATUSBAR_PLAYER_GRACE  },
    { -1, -1, "Bar_PlayerHealth",   STATUSBAR_PLAYER_HEALTH },
    // TextValues.
    { -1, -1, "Engine_CurrentFPS",  TEXTBOX_STAT_CUR_FPS   },
    { -1, -1, "Engine_BestFPS",     TEXTBOX_STAT_BEST_FPS  },
    { -1, -1, "Engine_WorstFPS",    TEXTBOX_STAT_WORST_FPS },
    { -1, -1, "Engine_SumTris",     TEXTBOX_STAT_SUM_TRIS  },
    { -1, -1, "Engine_BatchCount",  TEXTBOX_STAT_SUM_BATCH },
    { -1, -1, "Login_ServerInfo1",  TEXTBOX_SERVER_INFO1     },
    { -1, -1, "Login_ServerInfo2",  TEXTBOX_SERVER_INFO2     },
    { -1, -1, "Login_ServerInfo3",  TEXTBOX_SERVER_INFO3     },
    { -1, -1, "Login_LoginWarn",    TEXTBOX_LOGIN_WARN       },
    { -1, -1, "Login_PswdVerify",   TEXTBOX_LOGIN_PSWDVERIFY },
    { -1, -1, "Login_LoginInfo1",   TEXTBOX_LOGIN_INFO1      },
    { -1, -1, "Login_LoginInfo2",   TEXTBOX_LOGIN_INFO2      },
    { -1, -1, "Login_LoginInfo3",   TEXTBOX_LOGIN_INFO3      },
    { -1, -1, "NPC_Headline",       TEXTBOX_NPC_HEADLINE     },
    { -1, -1, "Inv_Equipment",      TEXTBOX_INV_EQUIP        },
    { -1, -1, "Inv_Equip_Weight",   TEXTBOX_INV_EQUIP_WEIGHT },
    // TextInput.
    { -1, -1, "Input_Login_Name",   TEXTINPUT_LOGIN_NAME   },
    { -1, -1, "Input_Login_Passwd", TEXTINPUT_LOGIN_PASSWD },
    { -1, -1, "Input_Login_Verify", TEXTINPUT_LOGIN_VERIFY },
    { -1, -1, "Input_NPC_Dialog",   TEXTINPUT_NPC_DIALOG   },
    // Table
    { -1, -1, "Table_Server",       TABLE },
    // Combobox.
//    { -1, -1, "ComboBoxTest",       COMBOBOX_TEST  },
    // Element_Slot
    { -1, -1, "Slot_Quickslot",     SLOT_QUICKSLOT      },
    { -1, -1, "Slot_Equipment",     SLOT_EQUIPMENT      },
    { -1, -1, "Slot_Inventory",     SLOTGROUP_INVENTORY },
    { -1, -1, "Slot_Container",     SLOT_CONTAINER      },
    { -1, -1, "Slot_TradeOffer",    SLOT_TRADE_OFFER    },
    { -1, -1, "Slot_TradeReturn",   SLOT_TRADE_RETURN   },
    { -1, -1, "Slot_Shop",          SLOT_SHOP           },
};

//================================================================================================
// .
//================================================================================================
const char *GuiManager::getElementName(int index)
{
    PROFILE()
    return (index >= ELEMENTS_SUM)?0:mStateStruct[index].name;
}

//================================================================================================
// .
//================================================================================================
void GuiManager::printText(int width, int height, uint32 *dst, int dstLineSkip,
                           uint32 *bak, int bakLineSkip, const char *txt, unsigned int fontNr, uint32 color, bool hideText)
{
    PROFILE()
    GuiTextout::getSingleton().printText(width, height, dst, dstLineSkip, bak, bakLineSkip, txt, fontNr, color, hideText);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::setMouseState(int action)
{
    PROFILE()
    GuiCursor::getSingleton().setState(action);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::sendMsg(int element, Message message, const char *text, uint32 param, const char *text2)
{
    PROFILE()
    for (unsigned int i = 0; i < ELEMENTS_SUM; ++i)
    {
        if (element == mStateStruct[i].index)
        {
            mMsgRetStr = !text?"":text;
            mMsgRetInt = param;
            guiWindow[mStateStruct[i].windowNr].sendMsg(mStateStruct[i].winElementNr, message, mMsgRetStr, mMsgRetInt, text2);
            return;
        }
    }
}

//================================================================================================
// .
//================================================================================================
void GuiManager::playSound(const char *filename)
{
    PROFILE()
    if (!filename) return;
    if (!filename[0])
        mvSound.push_back(mSoundWrongInput);
    else
        mvSound.push_back(filename);
}

//================================================================================================
// .
//================================================================================================
const char *GuiManager::getNextSound()
{
    PROFILE()
    if (mvSound.empty()) return 0;
    String strSound = mvSound[0];
    //Logger::log().warning() << "Gui Manager sound cmd: " << strSound;
    mvSound.erase(mvSound.begin());
    return strSound.c_str();
}

//================================================================================================
// .
//================================================================================================
int GuiManager::getElementIndex(const char *name, int windowID, int winElementNr)
{
    PROFILE()
    if (name)
    {
        for (int i = 0; i < ELEMENTS_SUM; ++i)
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
int GuiManager::getElementPressed()
{
    PROFILE()
    return guiWindow[0].getElementPressed();
}

//================================================================================================
// .
//================================================================================================
void GuiManager::Init(int w, int h, bool createMedia, bool printInfo, const char *soundActionFailed, const char *pathTxt, const char *pathGfx, const char *pathFonts, const char *pathItems)
{
    PROFILE()
    Logger::log().headline() << "Init GUI";
    mDragSrcWin     = NO_ACTIVE_WINDOW;
    mActiveWindow   = NO_ACTIVE_WINDOW;
    mTextInputWindow= NO_ACTIVE_WINDOW;
    mScreenWidth    = w;
    mScreenHeight   = h;
    mCreateMedia    = createMedia;
    mPrintInfo      = printInfo;
    mBuildBuffer    = 0;
    mTooltipDelay   = 0;
    mSoundWrongInput=soundActionFailed;
    mPathTextures      = pathGfx;
    mPathDescription   = pathTxt;
    mPathTexturesFonts = pathFonts;
    mPathTexturesItems = pathItems;
    // ////////////////////////////////////////////////////////////////////
    // Create the tooltip (texure/overlay/material).
    // ////////////////////////////////////////////////////////////////////
    String strTexture = StringConverter::toString(TOOLTIP_SIZE)+"_";
    strTexture+= RESOURCE_TOOLTIP;
    strTexture+= "_Texture";
    mTexture = createTexture(strTexture);
    mElement = createOverlay(RESOURCE_TOOLTIP, strTexture, mOverlay);
    mElement->setPosition((mScreenWidth-mTexture->getWidth())/3.0f*2.0f, (mScreenHeight-mTexture->getHeight())/2.0f);
    resizeBuildBuffer(TOOLTIP_SIZE*TOOLTIP_SIZE);
    // ////////////////////////////////////////////////////////////////////
    // If requested (by cmd-line) print all element names.
    // ////////////////////////////////////////////////////////////////////
    if (mPrintInfo)
    {
        Logger::log().info() << "These elements are currently known and can be used in " << FILE_TXT_WINDOWS<< ":";
        for (int i =0; i < ELEMENTS_SUM; ++i) Logger::log().info() << "- "<< mStateStruct[i].name;
    }
    // Load the default font.
    GuiTextout::getSingleton().loadRawFont(FILE_SYSTEM_FONT);
}

//================================================================================================
// Create an overlay and an overlay-elemrnt.
//================================================================================================
OverlayElement *GuiManager::createOverlay(String name, String strTexture, Overlay *&overlay)
{
    PROFILE()
    // ////////////////////////////////////////////////////////////////////
    // Create the material.
    // ////////////////////////////////////////////////////////////////////
    const char *MATERIAL_RESOURCE_NAME = "_Material";
    MaterialPtr material = MaterialManager::getSingleton().create(name + MATERIAL_RESOURCE_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    if (material.isNull())
    {
        Logger::log().error() << "Could not create material." << name + MATERIAL_RESOURCE_NAME;
        return 0;
    }
    material->setLightingEnabled(false);
    material->setDepthWriteEnabled(false);
    material->setDepthCheckEnabled(false);
    material->setSceneBlending(SBT_TRANSPARENT_ALPHA);
    material->getTechnique(0)->getPass(0)->setAlphaRejectSettings(CMPF_GREATER, 128);
    material->getTechnique(0)->getPass(0)->createTextureUnitState();
    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(TFO_NONE);
    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(strTexture);
    // ////////////////////////////////////////////////////////////////////
    // Create the overlay and its child-element.
    // ////////////////////////////////////////////////////////////////////
    overlay = OverlayManager::getSingleton().create(name + "_Overlay");
    OverlayElement *element = OverlayManager::getSingleton().createOverlayElement("Panel", name + "_OverlayElement");
    if (!overlay || !element)
    {
        Logger::log().error() << "Could not create Overlay " << name;
        return 0;
    }
    overlay->add2D(static_cast<OverlayContainer*>(element));
    element->setMetricsMode(GMM_PIXELS);
    Real size = (Real)StringConverter::parseInt(strTexture.substr(0, strTexture.find("_")));
    element->setDimensions(size, size);
    element->setMaterialName(name + MATERIAL_RESOURCE_NAME);
    return element;
}

//================================================================================================
// Creates a texture manually.
// texture name format: a_b_c_d
// a: size of the texture (power of 2).
// b: name of the resource.
// c: number of the resource (if needed).
// d: resource type (in this case texture).
//================================================================================================
TexturePtr GuiManager::createTexture(String strTexture)
{
    PROFILE()
    StringVector vString = StringUtil::split(strTexture, "_", 3);
    int s = atoi(vString[0].c_str());
    // ////////////////////////////////////////////////////////////////////
    // Create the texture.
    // ////////////////////////////////////////////////////////////////////
    TexturePtr texture = TextureManager::getSingleton().getByName(strTexture);
    if (texture.isNull())
    {
        Logger::log().info() << "(Re)creating texture: " << vString[1] << " of size: " << s << "x" << s;
        texture = TextureManager::getSingleton().createManual(strTexture, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                  TEX_TYPE_2D, s, s, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY, &mManualLoader);
        if (texture.isNull())
            Logger::log().error() << "(Re)creating texture: " << vString[1]  << " of size: " << s << "x" << s << " failed!";
        return texture;
    }
    // ////////////////////////////////////////////////////////////////////
    // Clear the whole texture (textures have always 2^n size while our gfx can be smaller).
    // ////////////////////////////////////////////////////////////////////
    size_t size = texture->getWidth()*texture->getHeight()*sizeof(uint32);
    memset(texture->getBuffer()->lock(0, size, HardwareBuffer::HBL_DISCARD), 0x00, size);
    texture->getBuffer()->unlock();
    // ////////////////////////////////////////////////////////////////////
    // (Re)draw the pixeldata.
    // ////////////////////////////////////////////////////////////////////
    Logger::log().info() << "(Re)drawing texture: " << vString[1] << " of size: " << s << "x" << s;
    if      (vString[1] == RESOURCE_TOOLTIP) drawTooltip();
    else if (vString[1] == RESOURCE_MCURSOR) GuiCursor::getSingleton().draw();
    else if (vString[1] == RESOURCE_WINDOW)
    {
        int window = StringConverter::parseInt(vString[2]);
        guiWindow[window].update(1.0);
    }
    return texture;
}

//================================================================================================
// Buildbuffer is used to draw the gui elements before blitting them to the texture.
//================================================================================================
void GuiManager::resizeBuildBuffer(size_t newSize)
{
    PROFILE()
    static size_t size = 0;
    if (size < newSize)
    {
        delete[] mBuildBuffer;
        mBuildBuffer = new uint32[newSize];
        size = newSize;
    }
}

//================================================================================================
// .
//================================================================================================
void GuiManager::centerWindowOnMouse(int window)
{
    PROFILE()
    guiWindow[window].centerWindowOnMouse((int)mMouse.x, (int)mMouse.y);
}

//================================================================================================
// Parse the windows data.
//================================================================================================
void GuiManager::parseWindows()
{
    PROFILE()
    // ////////////////////////////////////////////////////////////////////
    // Parse the imageset.
    // ////////////////////////////////////////////////////////////////////
    String file = mPathDescription + FILE_TXT_IMAGESET;
    GuiImageset::getSingleton().parseXML(file.c_str(), mCreateMedia);
    // ////////////////////////////////////////////////////////////////////
    // Parse the windows.
    // ////////////////////////////////////////////////////////////////////
    TiXmlElement *xmlRoot, *xmlElem;
    file = mPathDescription + FILE_TXT_WINDOWS;
    TiXmlDocument doc(file.c_str());
    const char *valString;
    // Check for a working window description.
    if (!doc.LoadFile() || !(xmlRoot = doc.RootElement()))
    {
        Logger::log().error() << "XML-File '" << file << "' is missing or broken.";
        return;
    }
    if ((valString = xmlRoot->Attribute("name")))
        Logger::log().info() << "Parsing '" << valString << "' in file" << file << ".";
    else
        Logger::log().error() << "File '" << file << "' has no name entry.";
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
                    xmlElem->Attribute("resolution"),
                    mCreateMedia);
                ++sumEntries;
            }
        }
        Logger::log().list() << sumEntries << " Fonts were parsed.";
    }
    else
    {
        Logger::log().error() << "CRITICAL: No fonts found in " << file;
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
        Logger::log().error() << "File '" << file << "' has no mouse-cursor defined.";
    }
    // ////////////////////////////////////////////////////////////////////
    // Init the windows.
    // ////////////////////////////////////////////////////////////////////
    mSumDefinedWindows = 0;
    for (xmlElem = xmlRoot->FirstChildElement("Window"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Window"))
    {
        if (!(valString = xmlElem->Attribute("name"))) continue;
        for (int winNr = 0; winNr < WIN_SUM; ++winNr)
        {
            if (!stricmp(mWindowID[winNr].name, valString))
            {
                // todo: check if a window was defined more than once.
                mWindowZPos[mSumDefinedWindows] = winNr;
                guiWindow[winNr].Init(xmlElem, RESOURCE_WINDOW, winNr, mSumDefinedWindows++);
                break;
            }
        }
    }
    Logger::log().list() << mSumDefinedWindows << " Windows were parsed.";
}

//================================================================================================
// .
//================================================================================================
void GuiManager::freeRecources()
{
    PROFILE()
    for (int i=0; i < WIN_SUM; ++i) guiWindow[i].freeRecources();
    GuiCursor::getSingleton().freeRecources();
    mTexture.setNull();
    mvSound.clear();
}

//================================================================================================
// KeyEvent was reported.
// The decision if a keypress belongs to gui is made in events.cpp.
//================================================================================================
bool GuiManager::keyEvent(const int key, const unsigned int keyChar)
{
    PROFILE()
    // We have an active Textinput.
    if (mTextInputWindow != NO_ACTIVE_WINDOW && mTextInputWindow == mActiveWindow)
    {
        if (key == OIS::KC_ESCAPE)
        {
            setText(mTextInputElement, mBackupStrTextInput.c_str());
            cancelTextInput();
            return true;
        }
        mTextInputUserAction = GuiTextinput::getSingleton().keyEvent(key, keyChar);
        mStrTextInput = GuiTextinput::getSingleton().getText();
        setText(mTextInputElement, mStrTextInput.c_str());
        if (GuiTextinput::getSingleton().wasFinished())
        {
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
int GuiManager::mouseEvent(int mouseAction, Vector3 &mouse)
{
    PROFILE()
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
            mOverlay->hide();
            mDragDstWin = -1;
            for (unsigned int w = 0; w < WIN_SUM; ++w)
            {
                if (guiWindow[w].mouseWithin((int)mMouse.x, (int)mMouse.y))
                {
                    mDragDstWin = w;
                    //mDragDstSlot = guiWindow[w].getMouseOverSlot((int)mMouse.x, (int)mMouse.y);
                    break;
                }
            }
            // Drop the item.
            mDragSrcWin = NO_ACTIVE_WINDOW;
            return GuiManager::EVENT_DRAG_DONE;
        }
        mElement->setPosition(mMouse.x-24.0f, mMouse.y-24.0f);
        return GuiManager::EVENT_CHECK_DONE;
    }
    // ////////////////////////////////////////////////////////////////////
    // Check for mouse action in all windows.
    // ////////////////////////////////////////////////////////////////////
    mMouseWithin = false;
    for (unsigned int i = 0; i < WIN_SUM; ++i)
    {
        int ret = guiWindow[mWindowZPos[WIN_SUM-i-1]].mouseEvent(mouseAction, mMouse);
        if (ret != EVENT_OUTSIDE_WIN) mMouseWithin = true;
        if (ret == EVENT_CHECK_DONE || ret == EVENT_USER_ACTION)
        {
            static int lastWindow = mWindowZPos[WIN_SUM-i-1];
            if (lastWindow != mWindowZPos[WIN_SUM-i-1])
            {
                guiWindow[lastWindow].mouseLeftWindow();
                lastWindow = mWindowZPos[WIN_SUM-i-1];
            }
            return ret;
        }
        if (ret == EVENT_DRAG_STRT)
        {
            mDragSrcWin = i;
            mDragSrcSlot= guiWindow[i].getDragSlot();
            mElement->setPosition(mMouse.x-24.0f, mMouse.y-24.0f);
            return GuiManager::EVENT_CHECK_DONE;
        }
    }
    return GuiManager::EVENT_CHECK_NEXT;
}

//================================================================================================
// .
//================================================================================================
void GuiManager::startTextInput(int window, int element, int maxChars, bool blockNumbers, bool blockWhitespaces)
{
    PROFILE()
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
    PROFILE()
    GuiTextinput::getSingleton().canceled();
    mTextInputWindow = NO_ACTIVE_WINDOW;
}

//================================================================================================
// .
//================================================================================================
bool GuiManager::brokenTextInput()
{
    PROFILE()
    return GuiTextinput::getSingleton().wasCanceled();
}

//================================================================================================
// .
//================================================================================================
bool GuiManager::finishedTextInput()
{
    PROFILE()
    return GuiTextinput::getSingleton().wasFinished();
}

//================================================================================================
// .
//================================================================================================
void GuiManager::resetTextInput()
{
    PROFILE()
    GuiTextinput::getSingleton().reset();
}

//================================================================================================
// .
//================================================================================================
void GuiManager::windowToFront(int window)
{
    PROFILE()
    uchar actPos = guiWindow[window].getZPos();
    while (actPos < mSumDefinedWindows-1)
    {
        mWindowZPos[actPos] = mWindowZPos[actPos+1];
        guiWindow[mWindowZPos[actPos]].setZPos(actPos);
        ++actPos;
    }
    mWindowZPos[mSumDefinedWindows-1] = window;
    guiWindow[window].setZPos(mSumDefinedWindows-1);
    mActiveWindow = window;
}

//================================================================================================
// .
//================================================================================================
void GuiManager::showWindow(int window, bool visible)
{
    PROFILE()
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
    PROFILE()
    // ////////////////////////////////////////////////////////////////////
    // Update textinput.
    // ////////////////////////////////////////////////////////////////////
    static clock_t time = Root::getSingleton().getTimer()->getMilliseconds();
    if (mTextInputWindow != NO_ACTIVE_WINDOW)
    {
        if (Root::getSingleton().getTimer()->getMilliseconds() - time > CURSOR_FREQUENCY)
        {
            time = Root::getSingleton().getTimer()->getMilliseconds();
            setText(mTextInputElement, GuiTextinput::getSingleton().getText());
        }
    }
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
    PROFILE()
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
    PROFILE()
    if (!mStrTooltip.size()) return;
    const int MAX_TOOLTIP_LINES = 16;
    std::string line[MAX_TOOLTIP_LINES];
    int txtWidth[MAX_TOOLTIP_LINES];
    int sumLines = 0;
    int stop, start = 0;
    int fontHeight = GuiTextout::getSingleton().getFontHeight(GuiTextout::FONT_SYSTEM);
    for (sumLines = 0; sumLines < MAX_TOOLTIP_LINES; ++sumLines)
    {
        stop = (int)mStrTooltip.find(TOOLTIP_LINEBREAK_SIGN, start);
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
        txtWidth[i] = GuiTextout::getSingleton().calcTextWidth(line[i].c_str(), GuiTextout::FONT_SYSTEM);
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
    for (; --y;) *dest++ = 0;
    maxWidth-=BORDER*2;
    // ////////////////////////////////////////////////////////////////////
    // Draw the text.
    // ////////////////////////////////////////////////////////////////////
    color = BACKGR_COLOR;
    for (int i = 0; i < sumLines; ++i)
    {
        dest = back + BORDER + (BORDER + i*fontHeight) * w;
        GuiTextout::getSingleton().printText(maxWidth, fontHeight, dest, w, color, line[i].c_str(), GuiTextout::FONT_SYSTEM);
    }
    mTexture->getBuffer()->unlock();
    if (mTooltipDelay)
    {
        Real x = mMouse.x+40.0f;
        Real y = mMouse.y+40.0f;
        if (x+ maxWidth > mScreenWidth)  x = mScreenWidth - maxWidth-40.0f;
        if (y+ endY     > mScreenHeight) y = mScreenHeight- endY-40.0f;
        mElement->setPosition(x,y);
        mTooltipDelay = 0;
    }
    mOverlay->show();
}

//================================================================================================
// Draw the tooltip. Clipping is performed.
//================================================================================================
void GuiManager::drawDragElement(const PixelBox &src)
{
    PROFILE()
    size_t size = mTexture->getWidth()*mTexture->getHeight()*sizeof(uint32);
    memset(mTexture->getBuffer()->lock(0, size, HardwareBuffer::HBL_DISCARD), 0x00, size);
    mTexture->getBuffer()->unlock();
    size = src.getWidth();
    mTexture->getBuffer()->blitFromMemory(src, Box(0, 0, size, size));
    mOverlay->show();
}

//================================================================================================
//
//================================================================================================
void GuiManager::debugText(const char *text, Ogre::uint32 timeBeforeNextMsg)
{
    PROFILE()
    static String strText;
    strText ="[Debug] ";
    strText+= text;
    sendMsg(LIST_MSGWIN, MSG_SET_DEBUG_TEXT, strText.c_str(), timeBeforeNextMsg);
}

