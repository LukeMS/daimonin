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

#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <vector>
#include <Ogre.h>
#include "gui_textout.h"
#include "gui_gadget.h"
#include "gui_gadget_button.h"
#include "gui_gadget_combobox.h"
#include "gui_graphic.h"
#include "gui_window.h"
#include "gui_listbox.h"
#include "gui_statusbar.h"

using namespace Ogre;

enum
{
    GUI_ACTION_NONE,
    GUI_ACTION_START_TEXT_INPUT,
    GUI_ACTION_SUM
};

class GuiWindow
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ~GuiWindow()
    {}
    GuiWindow();
    void freeRecources();
    bool isVisible()
    {
        return mOverlay->isVisible();
    }
    void Init(TiXmlElement *xmlElem);
    void keyEvent(int obj_type, int action, int val1=0, int val2=0);
    void updateDragAnimation();
    void updateAnimaton(Real timeSinceLastFrame);
    void updateListbox();
    void PreformActions();
    const char *getName()
    {
        return mStrName.c_str();
    }
    const char *Message(int message, int element, const char *value);
    const char *mouseEvent(int MouseAction, int x, int y);
    const char *getTooltip()
    {
        return mStrTooltip.c_str();
    }

private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    bool isInit;
    static int msInstanceNr, mMouseDragging;
    static std::string mStrTooltip;
    int mWindowNr;
    int mMousePressed, mMouseOver;
    Image mTileImage;
    int mPosX, mPosY, mPosZ, mWidth, mHeight;
    int mHeadPosX, mHeadPosY;

    int mDragPosX1, mDragPosX2, mDragPosY1, mDragPosY2, mDragOldMousePosX, mDragOldMousePosY;
    int mMinimized, mDefaultHeight;
    bool mSizeRelative;
    PixelBox mSrcPixelBox;
    SceneManager *mSceneMgr;
    SceneNode *mSceneNode;
    std::string mStrName;
    std::string mStrImageSetGfxFile,  mStrFont, mStrXMLFile;
    std::vector<GuiGadget *>mvGadget;
    std::vector<GuiGadgetCombobox*>mvCombobox;
    std::vector<GuiGadgetButton *>mvButton;
    std::vector<GuiGraphic*>mvGraphic;
    std::vector<GuiListbox*>mvListbox;
    std::vector<TextLine*>mvTextline;
    std::vector<GuiStatusbar*>mvStatusbar;
    Overlay *mOverlay, *mNPC_HeadOverlay;
    OverlayElement *mElement;
    AnimationState *mSpeakAnimState, *mManualAnimState;
    MaterialPtr mMaterial;
    TexturePtr mTexture;
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    int getGadgetMouseIsOver(int x, int y);
    void createWindow();
    void setHeight(int h);
    void delGadget(int number);
    void drawAll();
    void parseWindowData(TiXmlElement *xmlElem);
};

#endif
