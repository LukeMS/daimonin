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
#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif
#include "logger.h"
#include "gui_gadget_slot.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_manager.h"
#include "item.h"
#include "option.h"
#include "tile_map_wrapper.h"

using namespace Ogre;

const unsigned int ITEM_SIZE = 64;

Overlay *GuiGadgetSlot::mDnDOverlay =0;
OverlayElement *GuiGadgetSlot::mDnDElement =0;
Image GuiGadgetSlot::mAtlasTexture;
MaterialPtr GuiGadgetSlot::mDnDMaterial;
TexturePtr GuiGadgetSlot::mDnDTexture;

//================================================================================================
// Constructor.
//================================================================================================
GuiGadgetSlot::GuiGadgetSlot(TiXmlElement *xmlElement, void *parent, bool drawOnInit):GuiElement(xmlElement, parent)
{
    mMouseOver = false;
    mMouseButDown = false;
    mActiveDrag = false;
    mActiveSlot = -1;
    std::string filename;

    const char *tmp;
    TiXmlElement *xmlOpt;
    if ((xmlOpt = xmlElement->FirstChildElement("Sum")))
    {
        if ((tmp = xmlOpt->Attribute("col" ))) mSumCol = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("row" ))) mSumRow = atoi(tmp);
    }
    if ((xmlOpt = xmlElement->FirstChildElement("Offset")))
    {
        if ((tmp = xmlOpt->Attribute("col" ))) mColSpace = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("row" ))) mRowSpace = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("itemX" ))) mItemOffsetX = atoi(tmp);
        if ((tmp = xmlOpt->Attribute("itemY" ))) mItemOffsetY = atoi(tmp);
    }
    mSlotWidth = (mWidth + mColSpace) * mSumCol;
    mSlotHeight= (mHeight+ mRowSpace) * mSumRow;
    BG_Backup = new uint32[mWidth * mHeight];
    mGfxNr = new int[mSumCol*mSumRow];
    // This stuff is static, so we have to do it only once.
    if (!mDnDOverlay)
    {
        // ////////////////////////////////////////////////////////////////////
        // Create the item texture atlas.
        // ////////////////////////////////////////////////////////////////////
        //if (Option::getSingleton().getIntValue(Option::CMDLINE_CREATE_ITEMS))
        {
            std::vector<std::string> itemFilename;
#ifdef WIN32
            String filename = PATH_ITEM_TEXTURES;
            filename+="\\*.png";
            BOOL found = true;
            WIN32_FIND_DATA FindFileData;
            HANDLE handle=FindFirstFile(filename.c_str(), &FindFileData);
            while (handle && found)
            {
                if (!strstr(FindFileData.cFileName, FILE_ITEM_TEXTURE_ATLAS))
                    itemFilename.push_back(FindFileData.cFileName);
                found = FindNextFile(handle, &FindFileData);
            }
#else
            struct dirent *dir_entry;
            DIR *dir = opendir(PATH_ITEM_TEXTURES); // Open the current directory
            while ((dir_entry = readdir(dir)))
            {
                if (strstr(dir_entry->d_name, ".png") && !strstr(dir_entry->d_name, FILE_ITEM_TEXTURE_ATLAS))
                    itemFilename.push_back(dir_entry->d_name);
            }
            closedir(dir);
#endif
            if (itemFilename.empty())
            {
                Logger::log().error() << "Could not find any item graphics in " << PATH_ITEM_TEXTURES;
                return;
            }
            Image itemImage, itemAtlas;
            uint32 *itemBuffer = new uint32[ITEM_SIZE * ITEM_SIZE * itemFilename.size()];
            uint32 *nextPos = itemBuffer;
            itemAtlas = itemAtlas.loadDynamicImage((unsigned char*)itemBuffer, ITEM_SIZE, ITEM_SIZE * itemFilename.size(), 1, PF_A8B8G8R8);
            for (unsigned int i = 0; i < itemFilename.size(); ++i)
            {
                itemImage.load(itemFilename[i], "General");
                if (itemImage.getHeight() != ITEM_SIZE || itemImage.getWidth() != ITEM_SIZE)
                {
                    Logger::log().warning() << "You tried to use and item with unsupported image size. Only Items of "
                    << ITEM_SIZE << " * " << ITEM_SIZE << " pixel are allowed "<< "[" << itemFilename[i] << "].";
                    break;
                }
                if (itemImage.getFormat() != PF_A8B8G8R8)
                {
                    Logger::log().warning() << "You tried to use and item with unsupported format ("<< itemImage.getFormat() <<")."
                    << " Only 32bit png format is allowed "<< "[" << itemFilename[i] << "].";
                    break;
                }
                memcpy(nextPos, itemImage.getData(), ITEM_SIZE * ITEM_SIZE * sizeof(uint32));
                nextPos+=ITEM_SIZE * ITEM_SIZE;
            }
            // ////////////////////////////////////////////////////////////////////
            // Write the hires version.
            // ////////////////////////////////////////////////////////////////////
            filename = PATH_ITEM_TEXTURES;
            filename+= "/";
            filename+= FILE_ITEM_TEXTURE_ATLAS;
            filename+= ".png";
            itemAtlas.save(filename);
            // Write the textfile.
            filename.replace(filename.find(".png"), 4, ".txt");
            std::ofstream txtFile(filename.c_str(), std::ios::out | std::ios::binary);
            txtFile << "# This file holds the content of the image-texture-atlas." << std::endl;
            if (txtFile)
            {
                for (unsigned int i = 0; i < itemFilename.size(); ++i)
                    txtFile << itemFilename[i] << std::endl;
            }
            txtFile.close();
            itemFilename.clear();
            // ////////////////////////////////////////////////////////////////////
            // Write the lores version (for 800x600 resolution)..
            // ////////////////////////////////////////////////////////////////////
            /*
            itemAtlas.resize((ushort)itemAtlas.getWidth()/2, (ushort)itemAtlas.getHeight()/2, Image::FILTER_BICUBIC);
            filename = PATH_ITEM_TEXTURES;
            filename+= "/";
            filename+= FILE_ITEM_TEXTURE_ATLAS;
            filename+= "_32.png";
            itemAtlas.save(filename);
            */
            delete[] itemBuffer;
        }
        // ////////////////////////////////////////////////////////////////////
        // Read in the gfxpos of the items.
        // ////////////////////////////////////////////////////////////////////
        filename = PATH_ITEM_TEXTURES;
        filename+= "/";
        filename+= FILE_ITEM_TEXTURE_ATLAS;
        filename+= ".txt";
        std::ifstream txtFile;
        txtFile.open(filename.c_str(), std::ios::in | std::ios::binary);
        if (!txtFile)
        {
            Logger::log().error() << "Error on file " << filename;
        }
        getline(txtFile, filename); // skip the comment.
        while (getline(txtFile, filename))
        {
            mvGfxPositions.push_back(filename);
        }
        txtFile.close();
        // ////////////////////////////////////////////////////////////////////
        // Read in the texture atlas.
        // ////////////////////////////////////////////////////////////////////
        filename = FILE_ITEM_TEXTURE_ATLAS;
        filename+= ".png";
        mAtlasTexture.load(filename, "General");
        // ////////////////////////////////////////////////////////////////////
        // Create drag'n'drop overlay.
        // ////////////////////////////////////////////////////////////////////
        mDnDTexture = TextureManager::getSingleton().createManual("GUI_SlotDnD_Texture", "General",
                      TEX_TYPE_2D, ITEM_SIZE, ITEM_SIZE, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY);
        mDnDOverlay = OverlayManager::getSingleton().create("GUI_SlotDnD_Overlay");
        mDnDOverlay->setZOrder(500);
        mDnDElement = OverlayManager::getSingleton().createOverlayElement(GuiWindow::OVERLAY_ELEMENT_TYPE, "GUI_SlotDnD_Frame");
        mDnDElement->setMetricsMode(GMM_PIXELS);
        mDnDElement->setDimensions (ITEM_SIZE, ITEM_SIZE);
        MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("GUI/Window");
        mDnDMaterial = tmpMaterial->clone("GUI_SlotDnD_Material");
        mDnDMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("GUI_SlotDnD_Texture");
        mDnDElement->setMaterialName("GUI_SlotDnD_Material");
        mDnDOverlay->add2D(static_cast<OverlayContainer*>(mDnDElement));
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw the container.
    // ////////////////////////////////////////////////////////////////////
    if (drawOnInit) draw();
}

//================================================================================================
// .
//================================================================================================
GuiGadgetSlot::~GuiGadgetSlot()
{
    mDnDMaterial.setNull();
    mDnDTexture.setNull();
    mvGfxPositions.clear();
    delete[] mGfxNr;
    //delete[] BG_Backup; // done in GuiElement.cpp
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
int GuiGadgetSlot::mouseEvent(int MouseAction, int x, int y)
{
    x-= mPosX;
    y-= mPosY;
    if ((unsigned int) x < mSlotWidth && (unsigned int) y < mSlotHeight || mActiveDrag)
    {
        int activeSlot = y/(mHeight+ mRowSpace)*mSumCol +   x/(mWidth + mColSpace);
        if (mActiveSlot != activeSlot && !mActiveDrag)
        {
            // We are no longer over this slot, so draw the defalut gfx.
            if (mActiveSlot >=0)
                drawSlot(mActiveSlot, SLOT_UPDATE,GuiImageset::STATE_ELEMENT_DEFAULT);
            mActiveSlot = activeSlot;
            drawSlot(mActiveSlot, SLOT_UPDATE, GuiImageset::STATE_ELEMENT_M_OVER);
        }
        if (MouseAction == GuiWindow::BUTTON_PRESSED && !mActiveDrag && mGfxNr[mActiveSlot] >=0)
        {
            mActiveDrag = true;
            mDragSlot = mActiveSlot;
            drawDragItem(0);
        }
        if (MouseAction == GuiWindow::BUTTON_RELEASED && mActiveDrag)
        {
            if ((unsigned int)x > mSlotWidth || (unsigned int)y > mSlotHeight)
            {
                Item::getSingleton().dropInventoryItemToFloor(mDragSlot);
            }
            else
                GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, StringConverter::toString(activeSlot).c_str());
            drawDragItem(-1);
            mActiveDrag = false;
        }
        if (MouseAction == GuiWindow::MOUSE_MOVEMENT && mActiveDrag)
        {
            Real x, y;
            GuiCursor::getSingleton().getPos(x, y);
            mDnDElement->setPosition(x, y);
        }
        return 0; // No need to check other gadgets.
    }
    else  // Mouse is no longer over the the gadget.
    {
        if (mActiveSlot >=0)
        {
            drawSlot(mActiveSlot, SLOT_UPDATE, GuiImageset::STATE_ELEMENT_DEFAULT);
            mActiveSlot = -1;
            return 0; // No need to check other gadgets.
        }
    }
    if (mActiveDrag) return 1;
    return -1; // No action here, check the other gadgets.
}

//================================================================================================
// Draw a single slot.
//================================================================================================
void GuiGadgetSlot::drawSlot(int slotNr, int gfxNr, int state)
{
    int row = slotNr / mSumCol;
    int col = slotNr - (row * mSumCol);
    int strtX = mPosX + col * (mColSpace + mWidth);
    int strtY = mPosY + row * (mRowSpace + mHeight);
    Texture *texture = ((GuiWindow*) mParent)->getTexture();

    //if (slotNr >=
    if (gfxNr != SLOT_UPDATE)
    {
        mGfxNr[slotNr] = -1;
        String gfxName = ObjectWrapper::getSingleton().getMeshName(gfxNr);
        for (unsigned int i =0; i < mvGfxPositions.size(); ++i)
        {
            if (mvGfxPositions[i] == gfxName)
            {
                mGfxNr[slotNr] = i;
                break;
            }
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Slot gfx.
    // ////////////////////////////////////////////////////////////////////
    PixelBox srcSlot = ((GuiWindow*) mParent)->getPixelBox()->getSubVolume(Box(
                           gfxSrcPos[state].x,
                           gfxSrcPos[state].y,
                           gfxSrcPos[state].x + mWidth,
                           gfxSrcPos[state].y + mHeight));
    uint32 *srcSlotData = static_cast<uint32*>(srcSlot.data);
    int rowSkipSlot = (int)((GuiWindow*) mParent)->getPixelBox()->getWidth();
    // ////////////////////////////////////////////////////////////////////
    // Item gfx.
    // ////////////////////////////////////////////////////////////////////
    PixelBox srcItem;
    uint32 *srcItemData;
    if (mGfxNr[slotNr] >=0)
    {
        srcItem = mAtlasTexture.getPixelBox().getSubVolume(Box(
                      0,
                      ITEM_SIZE * mGfxNr[slotNr],
                      ITEM_SIZE,
                      ITEM_SIZE *(mGfxNr[slotNr]+1)));
        srcItemData = static_cast<uint32*>(srcItem.data);
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw into the buffer.
    // ////////////////////////////////////////////////////////////////////
    int dSlotY = 0, dItemY = 0, destY =0;
    for (int y =0; y < mHeight; ++y)
    {
        for (int x =0; x < mWidth; ++x)
        {
            // First check if item has a non transparent pixel to draw.
            if (mGfxNr[slotNr] >=0 && x > mItemOffsetX && x < (int)ITEM_SIZE + mItemOffsetX &&y > mItemOffsetY && y < (int)ITEM_SIZE+ mItemOffsetY)
            {
                if (srcItemData[dItemY + x- mItemOffsetX] > 0x00ffffff)
                {
                    BG_Backup[destY + x] = srcItemData[dItemY + x- mItemOffsetX];
                    continue;
                }
            }
            // Now check for the background.
            if (srcSlotData[dSlotY + x] > 0x00ffffff)
                BG_Backup[destY + x] = srcSlotData[dSlotY + x];
        }
        if (y > mItemOffsetY)
            dItemY+= ITEM_SIZE;
        dSlotY+= (int)rowSkipSlot;
        destY+= mWidth;
    }
    srcSlot = PixelBox(mWidth, mHeight, 1, PF_A8B8G8R8, BG_Backup);
    // ////////////////////////////////////////////////////////////////////
    // Blit the buffer.
    // ////////////////////////////////////////////////////////////////////
    texture->getBuffer()->blitFromMemory(srcSlot, Box(strtX, strtY, strtX + mWidth, strtY + mHeight));
    /*
        // only for testing.
        GuiTextout::TextLine label;
        Item::sItem *item = Item::getSingleton().getBackpackItem(pos);
        if (!item) return;
        label.text = item->d_name;
        label.hideText= false;
        label.index= -1;
        label.font = 0;
        label.x1 = strtX + 5;
        label.y1 = strtY + 5;
        label.x2 = label.x1 + mWidth;
        label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
        label.color= 0x00ffffff;
        GuiTextout::getSingleton().Print(&label, texture);
    */
}

//================================================================================================
// Only used to CLEAR the slots. For drawing use drawSlot(...)
//================================================================================================
void GuiGadgetSlot::draw()
{
    for (int slotNr = 0; slotNr < mSumRow * mSumCol; ++slotNr)
        drawSlot(slotNr, SLOT_CLEAR, GuiImageset::STATE_ELEMENT_DEFAULT);
}

//================================================================================================
//
//================================================================================================
void GuiGadgetSlot::drawDragItem(int gfxNr)
{
    // Hide DnD overlay.
    if (gfxNr < 0)
    {
        mDnDOverlay->hide();
        return;
    }
    // Draw dragged item.
    PixelBox srcItem;
    uint32 *srcItemData;

    if (mGfxNr[mDragSlot] >=0)
    {
        srcItem = mAtlasTexture.getPixelBox().getSubVolume(Box(
                      0,
                      ITEM_SIZE * mGfxNr[mDragSlot],
                      ITEM_SIZE,
                      ITEM_SIZE *(mGfxNr[mDragSlot]+1)));
        srcItemData = static_cast<uint32*>(srcItem.data);
    }
    mDnDTexture->getBuffer()->blitFromMemory(srcItem);
    Real x, y;
    GuiCursor::getSingleton().getPos(x, y);
    mDnDElement->setPosition(x, y);
    mDnDOverlay->show();
}
