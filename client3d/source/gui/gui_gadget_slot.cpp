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

const unsigned int ITEM_SIZE = 64; // Only 64 or 32 are allowed!

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
    mActiveSlot = -1;
    mlIconContainer =0;
    mMouseOver = false;
    mActiveDrag = false;
    mMouseButDown = false;
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
                updateSlot(mActiveSlot, GuiImageset::STATE_ELEMENT_DEFAULT);
            mActiveSlot = activeSlot;
            updateSlot(mActiveSlot, GuiImageset::STATE_ELEMENT_M_OVER);
        }
        if (MouseAction == GuiWindow::BUTTON_PRESSED && !mActiveDrag)
        {
            mDragSlot = mActiveSlot;
            mActiveDrag = drawDragItem();
        }
        if (MouseAction == GuiWindow::BUTTON_RELEASED && mActiveDrag)
        {
            if ((unsigned int)x > mSlotWidth || (unsigned int)y > mSlotHeight)
            {
                Item::getSingleton().dropInventoryItemToFloor(mDragSlot);
            }
            else
                GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_CHATWINDOW, GuiImageset::GUI_LIST_MSGWIN, StringConverter::toString(activeSlot).c_str());
            mDnDOverlay->hide();
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
            updateSlot(mActiveSlot, GuiImageset::STATE_ELEMENT_DEFAULT);
            mActiveSlot = -1;
            return 0; // No need to check other gadgets.
        }
    }
    if (mActiveDrag) return 1;
    return -1; // No action here, check the other gadgets.
}

//================================================================================================
// Get the item pos inthe item-texture-atlas.
//================================================================================================
int GuiGadgetSlot::getTextureAtlasPos(int itemFace)
{
    String gfxName = ObjectWrapper::getSingleton().getMeshName(itemFace);
    for (unsigned int i =0; i < mvGfxPositions.size(); ++i)
    {
        if (mvGfxPositions[i] == gfxName) return i;
    }
    return -1; // Not found.
}

//================================================================================================
// Draws the slots.
//================================================================================================
void GuiGadgetSlot::updateSlot(int slot, int state)
{
    if (!mlIconContainer) return;
    static GuiTextout::TextLine label;
    std::list<Item::sItem*>::iterator iter = mlIconContainer->begin();
    int slotNr = 0;
    if (slot != UPDATE_ALL_SLOTS)
    {
        slotNr = slot;
        for (int i =0; i < slot; ++i)
            if (iter != mlIconContainer->end()) ++iter;
    }
    for (; slotNr < mSumCol * mSumRow; ++slotNr)
    {
        int row = slotNr / mSumCol;
        int col = slotNr - (row * mSumCol);
        int strtX = mPosX + col * (mColSpace + mWidth);
        int strtY = mPosY + row * (mRowSpace + mHeight);
        Texture *texture = ((GuiWindow*) mParent)->getTexture();
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
        int gfxNr = (slotNr >= (int)mlIconContainer->size())?-1:getTextureAtlasPos((*iter)->face & ~0x8000);
        if (gfxNr >=0)
        {
            srcItem = mAtlasTexture.getPixelBox().getSubVolume(Box(
                          0,
                          ITEM_SIZE * gfxNr,
                          ITEM_SIZE,
                          ITEM_SIZE *(gfxNr+1)));
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
                if (gfxNr >=0 && x > mItemOffsetX && x < (int)ITEM_SIZE + mItemOffsetX &&y > mItemOffsetY && y < (int)ITEM_SIZE+ mItemOffsetY)
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
        if (slotNr < (int)mlIconContainer->size() && (*iter)->nrof)
        {
            //label.text = (*iter)->d_name;
            label.text = StringConverter::toString((*iter)->nrof);
            label.hideText= false;
            label.index= -1;
            label.font = 2;
            label.x1 = strtX + 9;
            label.y1 = strtY + 9;
            label.x2 = label.x1 + mWidth;
            label.y2 = label.y1 + GuiTextout::getSingleton().getFontHeight(label.font);
            label.color= 0x00888888;
            GuiTextout::getSingleton().Print(&label, texture);
        }
        if (slot != UPDATE_ALL_SLOTS) break;
        if (iter != mlIconContainer->end()) ++iter;
    }

    /*
        long time = Root::getSingleton().getTimer()->getMilliseconds();
        for (int z = 0; z < 500; ++z)
        {
        }
        Logger::log().error() <<"time: "<<  Root::getSingleton().getTimer()->getMilliseconds() - time;
    */
}

//================================================================================================
// Only called once. Draws the empty slots to the background.
//================================================================================================
void GuiGadgetSlot::draw()
{
    /*
    int state = GuiImageset::STATE_ELEMENT_DEFAULT;
    int strtX, strtY = mPosY;
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    PixelBox srcSlot = ((GuiWindow*) mParent)->getPixelBox()->getSubVolume(Box(
                           gfxSrcPos[state].x, gfxSrcPos[state].y,
                           gfxSrcPos[state].x + mWidth, gfxSrcPos[state].y + mHeight));
    for (int y = 0; y < mSumRow; ++y)
    {
        strtX = mPosX;
        for (int x = 0; x < mSumCol; ++x)
        {
            texture->getBuffer()->blitFromMemory(srcSlot, Box(strtX, strtY, strtX + mWidth, strtY + mHeight));
            strtX += mWidth;
        }
        strtY += mHeight;
    }
    */
}

//================================================================================================
//
//================================================================================================
bool GuiGadgetSlot::drawDragItem()
{
    if (mDragSlot >= (int)mlIconContainer->size()) return false;
    std::list<Item::sItem*>::iterator iter = mlIconContainer->begin();
    for (int i = 0; i < mDragSlot; ++i) ++iter;
    int gfxNr = getTextureAtlasPos((*iter)->face & ~0x8000);
    mDnDTexture->getBuffer()->blitFromMemory(mAtlasTexture.getPixelBox().getSubVolume(Box(0, ITEM_SIZE * gfxNr, ITEM_SIZE, ITEM_SIZE *(gfxNr+1))));
    Real x, y;
    GuiCursor::getSingleton().getPos(x, y);
    mDnDElement->setPosition(x, y);
    mDnDOverlay->show();
    return true;
}
