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

#include "define.h"
#include "gui_manager.h"
#include "gui_window_dialog.h"
#include "gui_window.h"
#include "gui_cursor.h"
#include "gui_textinput.h"
#include "option.h"
#include "logger.h"
#include <Ogre.h>
#include <OgreFontManager.h>
#include <tinyxml.h>

using namespace Ogre;

static const int TOOLTIP_SIZE_X = 256;
static const int TOOLTIP_SIZE_Y = 128;
static const unsigned long TOOLTIP_DELAY = 2000; // Wait x ms before showing the tooltip.

GuiManager::GuiWinNam GuiManager::mGuiWindowNames[GUI_WIN_SUM]=
    {
        { "Statistics",    GUI_WIN_STATISTICS    },
        { "PlayerInfo",    GUI_WIN_PLAYERINFO    },
        { "PlayerConsole", GUI_WIN_PLAYERCONSOLE },
        { "TextWindow",    GUI_WIN_TEXTWINDOW    },
        { "ServerSelect",  GUI_WIN_SERVERSELECT  },
        { "Login",         GUI_WIN_LOGIN         },
        //    { "Creation"  ,  GUI_WIN_CREATION   },
        { "DialogNPC",     GUI_WIN_NPCDIALOG     },
    };
class GuiWindow GuiManager::guiWindow[GUI_WIN_SUM];

//================================================================================================
// .
//================================================================================================
void GuiManager::Init(int w, int h)
{
    Logger::log().headline("Init GUI");
    mScreenWidth   = w;
    mScreenHeight  = h;
    // ////////////////////////////////////////////////////////////////////
    // Create the tooltip overlay.
    // ////////////////////////////////////////////////////////////////////
    Logger::log().info() << "Creating Overlay for System-Messages...";
    mTooltipRefresh = false;
    mTexture = TextureManager::getSingleton().createManual("GUI_ToolTip_Texture", "General",
               TEX_TYPE_2D, TOOLTIP_SIZE_X, TOOLTIP_SIZE_Y, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY);
    mOverlay = OverlayManager::getSingleton().create("GUI_Tooltip_Overlay");
    mOverlay->setZOrder(500);
    mElement = OverlayManager::getSingleton().createOverlayElement(OVERLAY_TYPE_NAME, "GUI_Tooltip_Frame");
    mElement->setMetricsMode(GMM_PIXELS);
    mElement->setDimensions (TOOLTIP_SIZE_X, TOOLTIP_SIZE_Y);
    mElement->setPosition((mScreenWidth-mTexture->getWidth())/3*2, (mScreenHeight-mTexture->getHeight())/2);
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("GUI/Window");
    mMaterial = tmpMaterial->clone("GUI_Tooltip_Material");
    if (mMaterial.isNull() || mMaterial->isLoaded())
    {
        Logger::log().success(false);
        return;
    }
    mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("GUI_ToolTip_Texture");
    //  mMaterial->reload();
    mElement->setMaterialName("GUI_Tooltip_Material");
    mOverlay->add2D(static_cast<OverlayContainer*>(mElement));
    mOverlay->show();
    mProcessingTextInput = false;
    Logger::log().success(true);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::parseWindows(const char *XML_windows_file)
{
    // ////////////////////////////////////////////////////////////////////
    // Parse the windows datas.
    // ////////////////////////////////////////////////////////////////////
    if (!parseWindowsData( XML_windows_file)) return;
}

//================================================================================================
// Parse the cursor and windows data.
//================================================================================================
bool GuiManager::parseWindowsData(const char *fileWindows)
{
    TiXmlElement *xmlRoot, *xmlElem;
    TiXmlDocument doc(fileWindows);
    const char *valString;
    // ////////////////////////////////////////////////////////////////////
    // Check for a working window description.
    // ////////////////////////////////////////////////////////////////////
    if ( !doc.LoadFile(fileWindows) || !(xmlRoot = doc.RootElement()) )
    {
        Logger::log().error() << "XML-File '" << fileWindows << "' is missing or broken.";
        return false;
    }
    if ((valString = xmlRoot->Attribute("name")))
    {
        Logger::log().info() << "Parsing '" << valString << "' in file" << fileWindows << ".";
    }
    else
    {
        Logger::log().error() << "File '" << fileWindows << "' has no name entry.";
    }
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
    GuiImageset::GuiSrcEntryMouse *srcEntry;
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
            int scale = 8;
            GuiCursor::getSingleton().Init(srcEntry->width, srcEntry->height, mScreenWidth, mScreenHeight, scale);
            GuiCursor::getSingleton().setStateImagePos(srcEntry->state);
            GuiCursor::getSingleton().draw();
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
            if (!stricmp(mGuiWindowNames[i].name,valString))
            {
                guiWindow[i].Init(xmlElem);
                break;
            }
        }
    }
    return true;
}

//================================================================================================
// .
//================================================================================================
void GuiManager::freeRecources()
{
    for (int i=0; i < GUI_WIN_SUM; ++i) guiWindow[i].freeRecources();
    GuiCursor::getSingleton().freeRecources();
    mMaterial.setNull();
    mTexture.setNull();
}

//================================================================================================
// KeyEvent was reported.
// The decision if a keypress belongs to gui is made in events.cpp.
//================================================================================================
bool GuiManager::keyEvent(const char keyChar, const unsigned char key)
{
    // Key event in npc-dialog window.
    if (GuiDialog::getSingleton().keyEvent(keyChar, key)) return true;
    // We have an active Textinput.
    if (mProcessingTextInput)
    {
        if (key == KC_ESCAPE)
        {
            sendMessage(mActiveWindow, GUI_MSG_TXT_CHANGED, mActiveElement, (void*)mBackupTextInputString.c_str());
            GuiTextinput::getSingleton().canceled();
            mProcessingTextInput = false;
            return true;
        }
        GuiTextinput::getSingleton().keyEvent(keyChar, key);
        if (GuiTextinput::getSingleton().wasFinished())
        {
            mStrTextInput = GuiTextinput::getSingleton().getText();
            sendMessage(mActiveWindow, GUI_MSG_TXT_CHANGED, mActiveElement, (void*)mStrTextInput.c_str());
            GuiTextinput::getSingleton().stop();
            mProcessingTextInput = false;
        }
        return true;
    }
    // Activate the next window.
    if (key == KC_TAB)
    {
        if (++mActiveWindow >= (int)mvActiveWindow.size()) mActiveWindow = mvActiveWindow[0];
        return true;
    }
    // Key event in active window.
    return guiWindow[mActiveWindow].keyEvent(keyChar, key);
}

//================================================================================================
// .
//================================================================================================
bool GuiManager::mouseEvent(int mouseAction, Real rx, Real ry)
{
    GuiCursor::getSingleton().setPos(rx, ry);
    mMouseX = (int) (rx * mScreenWidth);
    mMouseY = (int) (ry * mScreenHeight);
    for (unsigned int i=0; i < GUI_WIN_SUM; ++i)
    {
        if (guiWindow[i].mouseEvent(mouseAction, mMouseX + mHotSpotX, mMouseY + mHotSpotY))
        {
            mActiveWindow = i;
            return true;
        }
    }
    // Mouse event in npc-dialog window.
    if (GuiDialog::getSingleton().mouseEvent(mouseAction, mMouseX + mHotSpotX, mMouseY + mHotSpotY)) return true;
    return false;
}

//================================================================================================
// Send a message to a GuiWindow.
//================================================================================================
const char *GuiManager::sendMessage(int window, int message, int element, void *value1, void *value2)
{
    value2 = 0; // no need for value2 atm.
    return guiWindow[window].Message(message, element, value1);
}

//================================================================================================
// .
//================================================================================================
void GuiManager::startTextInput(int window, int winElement, int maxChars, bool blockNumbers, bool blockWhitespaces)
{
    if (mProcessingTextInput || !guiWindow[window].isVisible()) return;
    mProcessingTextInput = true;
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
    if (visible)
    {
        guiWindow[window].setVisible(true);
        mvActiveWindow.push_back(window);
        mActiveWindow = window;
    }
    else
    {
        guiWindow[window].setVisible(false);
        // Active window is now the that was opened before the current one.
        mActiveWindow = window-1;
        remove(mvActiveWindow.begin(), mvActiveWindow.end(), window);
    }
}

//================================================================================================
// Update all windows.
//================================================================================================
void GuiManager::update(Real timeSinceLastFrame)
{
    if (mProcessingTextInput)
    {
        sendMessage(mActiveWindow, GUI_MSG_TXT_CHANGED, mActiveElement, (void*)GuiTextinput::getSingleton().getText());
    }
    for (unsigned int i=0; i < GUI_WIN_SUM; ++i)
    {
        guiWindow[i].update(timeSinceLastFrame);
    }
    // ////////////////////////////////////////////////////////////////////
    // Check for Tooltips.
    // ////////////////////////////////////////////////////////////////////
    if (mTooltipRefresh)
    {
        if (Root::getSingleton().getTimer()->getMilliseconds() > mTooltipDelay)
        {
            // TODO: Make the background fit to the text. make a black border, ...
            TextLine label;
            label.hideText= false;
            label.index= -1;
            label.font = 2;
            label.x1 = label.y1 = 2;
            label.x2 = TOOLTIP_SIZE_X;
            label.y2 = GuiTextout::getSingleton().getFontHeight(label.font);
            label.text = mStrTooltip;
            clearTooltip();
            GuiTextout::getSingleton().Print(&label, mTexture.getPointer());
            mElement->setPosition(mMouseX+33, mMouseY+38); // TODO:
            mOverlay->show();
            mTooltipRefresh = false;
        }
    }
}

//================================================================================================
// CAUTION: no bounds check !!!.
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
    int fontH = GuiTextout::getSingleton().getFontHeight(FONT_SYSTEM);
    TextLine label;
    label.index= -1;
    label.hideText= false;
    label.font = FONT_SYSTEM;
    //label.clipped = false;
    label.x1 = 0;
    label.y1 = fontH * row;
    label.x2 = mTexture->getWidth()-1;
    label.y2 = fontH * row + GuiTextout::getSingleton().getFontHeight(FONT_SYSTEM);
    label.text = text;
    //  clearTooltip();
    GuiTextout::getSingleton().Print(&label, mTexture.getPointer());
    mTooltipRefresh = false;
    //  mElement->setPosition(300, 100);
    mOverlay->show();
    ++row;
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
    PixelBox pb = mTexture->getBuffer()->lock (Box(0,0, mTexture->getWidth(), mTexture->getHeight()), HardwareBuffer::HBL_DISCARD);
    uint32 *dest_data = (uint32*)pb.data;
    for (int y = mTexture->getWidth() * mTexture->getHeight(); y; --y)
    {
        *dest_data++ = 0x884444ff;
    }
    mTexture->getBuffer()->unlock();
}
