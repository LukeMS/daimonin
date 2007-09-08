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
const int BITS_FACEFILTER = ~0x8000; // Filter to extract the face number (gfx).
Overlay *GuiGadgetSlot::mDnDOverlay =0;
OverlayElement *GuiGadgetSlot::mDnDElement =0;
Image GuiGadgetSlot::mAtlasTexture;
MaterialPtr GuiGadgetSlot::mDnDMaterial;
TexturePtr GuiGadgetSlot::mDnDTexture;
std::vector<Ogre::String> GuiGadgetSlot::mvAtlasGfxName;
int GuiGadgetSlot::mDragSlot = -1;
int GuiGadgetSlot::mActiveSlot= -1;
int uid = 0;

//================================================================================================
// Constructor.
//================================================================================================
GuiGadgetSlot::GuiGadgetSlot(TiXmlElement *xmlElement, void *parent, bool drawOnInit):GuiGraphic(xmlElement, parent, drawOnInit)
{
    std::string filename;
    mSlotNr = uid++;
    mItem = 0;
    mBusyTime = 0;

    const char *tmp;
    if ((tmp = xmlElement->Attribute("bg_image_name" )))
        Logger::log().error() << "bg_image_name: " << tmp;
    else
        Logger::log().error() << "none bg_image_name";

    LayerWindowBG = new uint32[mWidth * mHeight];
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
            itemAtlas = itemAtlas.loadDynamicImage((unsigned char*)itemBuffer, ITEM_SIZE, ITEM_SIZE * itemFilename.size(), 1, PF_A8R8G8B8);
            for (unsigned int i = 0; i < itemFilename.size(); ++i)
            {
                itemImage.load(itemFilename[i], "General");
                if (itemImage.getHeight() != ITEM_SIZE || itemImage.getWidth() != ITEM_SIZE)
                {
                    Logger::log().warning() << "You tried to use an item with unsupported image size. Only Items of "
                    << ITEM_SIZE << " * " << ITEM_SIZE << " pixel are allowed "<< "[" << itemFilename[i] << "].";
                    break;
                }
                if (itemImage.getFormat() != PF_A8R8G8B8)
                {
                    Logger::log().warning() << "You tried to use an item with unsupported format ("<< itemImage.getFormat() <<")."
                    << " Only 32bit [ARGB] png format is allowed "<< "[" << itemFilename[i] << "].";
                    break;
                }
                memcpy(nextPos, itemImage.getData(), ITEM_SIZE * ITEM_SIZE * sizeof(uint32));
                nextPos+=ITEM_SIZE * ITEM_SIZE;
            }
            // ////////////////////////////////////////////////////////////////////
            // Write the hires version (32x32 pix).
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
            // Write the lores version (32x32 pix).
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
            mvAtlasGfxName.push_back(filename);
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
    mvAtlasGfxName.clear();
    //delete[] LayerWindowBG; // done in GuiElement.cpp
}

//================================================================================================
// TODO: draw something like a watchlike gfx over the slot to show the busy status.
//================================================================================================
void GuiGadgetSlot::drawBusy(Real dTime)
{
    mBusyTimeExpired+= dTime;
    int degree = (int) ((360 * mBusyTimeExpired) / mBusyTime);
    if (degree > 360)
    {
        mBusyTime = 0;
        draw();  // Clear the busy gfx.
        return;
    }
    // TODO: draw the busy gfx...
}

//================================================================================================
// .
//================================================================================================
bool GuiGadgetSlot::mouseWithin(int x, int y)
{
    if (x < mPosX || x > mPosX + mWidth || y < mPosY || y > mPosY + mHeight)
        return false;
    return true;
}

//================================================================================================
// .
//================================================================================================
int GuiGadgetSlot::mouseEvent(int MouseAction, int x, int y)
{
    if (x >= mPosX && x <= mPosX + mWidth && y >= mPosY && y <= mPosY + mHeight)
    {
        if (mActiveSlot != mSlotNr)
        {
            mActiveSlot = mSlotNr;
            setState(GuiImageset::STATE_ELEMENT_M_OVER);
            GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            return GuiManager::EVENT_CHECK_NEXT;
        }
        if (MouseAction == GuiWindow::BUTTON_PRESSED)
        {
            mDragSlot = mActiveSlot;
            drawDragItem();
            return GuiManager::EVENT_DRAG_STRT;
        }
        return GuiManager::EVENT_CHECK_DONE; // No need to check other gadgets.
    }
    else // Mouse is no longer over this slot.
    {
        if (getState() != GuiImageset::STATE_ELEMENT_DEFAULT)
        {
            setState(GuiImageset::STATE_ELEMENT_DEFAULT);
            mActiveSlot = -1;
            GuiManager::getSingleton().setTooltip("");
            return GuiManager::EVENT_CHECK_DONE;
        }
    }
    return GuiManager::EVENT_CHECK_NEXT;
}

//================================================================================================
// Get the item pos in the item-texture-atlas.
//================================================================================================
int GuiGadgetSlot::getTextureAtlasPos(int itemFace)
{
    String gfxName = ObjectWrapper::getSingleton().getMeshName(itemFace & BITS_FACEFILTER);
    for (unsigned int i =0; i < mvAtlasGfxName.size(); ++i)
    {
        if (mvAtlasGfxName[i] == gfxName) return i;
    }
    return -1; // Not found.
}

//================================================================================================
// .
//================================================================================================
void GuiGadgetSlot::draw()
{
    // ////////////////////////////////////////////////////////////////////
    // Empty Slot.
    // ////////////////////////////////////////////////////////////////////
    if (!mItem)
    {
        GuiGraphic::draw();
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Slot holds an item.
    // ////////////////////////////////////////////////////////////////////
    int strtX = mPosX;
    int strtY = mPosY;
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    // ////////////////////////////////////////////////////////////////////
    // Slot gfx.
    // ////////////////////////////////////////////////////////////////////
    PixelBox srcSlot = ((GuiWindow*) mParent)->getPixelBox()->getSubVolume(Box(
                           gfxSrcPos[mState].x,
                           gfxSrcPos[mState].y,
                           gfxSrcPos[mState].x + mWidth,
                           gfxSrcPos[mState].y + mHeight));
    uint32 *srcSlotData = static_cast<uint32*>(srcSlot.data);
    int rowSkipSlot = (int)((GuiWindow*) mParent)->getPixelBox()->getWidth();
    // ////////////////////////////////////////////////////////////////////
    // Item gfx.
    // ////////////////////////////////////////////////////////////////////
    PixelBox srcItem;
    uint32 *srcItemData;
    int gfxNr = getTextureAtlasPos(mItem->face);
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
    // Draw slot + item into the buffer.
    // ////////////////////////////////////////////////////////////////////
    int dSlotY = 0, dItemY = 0, destY =0;
    for (int y =0; y < mHeight; ++y)
    {
        for (int x =0; x < mWidth; ++x)
        {
            // First check if item has a non transparent pixel to draw.
            if (gfxNr >=0 && x < (int)ITEM_SIZE && y < (int)ITEM_SIZE)
            {
                if (srcItemData[dItemY + x] > 0x00ffffff)
                {
                    LayerWindowBG[destY + x] = srcItemData[dItemY + x];
                    continue;
                }
            }
            // Now check for the background.
            if (srcSlotData[dSlotY + x] > 0x00ffffff)
                LayerWindowBG[destY + x] = srcSlotData[dSlotY + x];
        }
        dItemY+= ITEM_SIZE;
        dSlotY+= (int)rowSkipSlot;
        destY+= mWidth;
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw busy-gfx into the buffer.
    // ////////////////////////////////////////////////////////////////////
/*
    int busy = 15;

    int x1 = mWidth/2;
    int x2 = x1 + busy;
    int y1 = mHeight;
    int y2 = mHeight/2;


    int x = x1;
    int dx = x2-x1; if (dx < 0) dx*=-1;
    int y = y1;
    int dy = x2-x1; if (dx < 0) dx*=-1;
    int delta = dx - dy;
    int xStep, yStep;

    if (x2 > x1) xStep = 1; else xStep = -1;
    if (y2 > y1) yStep = 1; else yStep = -1;
    while (x != x2 || y != y2)
    {
        if (delta >=0) x+= xStep; delta-= dy;
        if (delta < 0) y+= yStep; delta+= dx;

        //for (int posX = x1; posX < x; ++posX) LayerWindowBG[posX + mWidth * y] = 0x00ffffff;
    }
*/



    // ////////////////////////////////////////////////////////////////////
    // Blit the buffer.
    // ////////////////////////////////////////////////////////////////////
    texture->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, LayerWindowBG),
        Box(strtX, strtY, strtX + mWidth, strtY + mHeight));
    static GuiTextout::TextLine label;
    if (mItem->nrof)
    {
        //label.text = mItem->d_name;
        label.text = StringConverter::toString(mItem->nrof);
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

    /*
        long time = Root::getSingleton().getTimer()->getMilliseconds();
        for (int z = 0; z < 500; ++z)
        {}
        Logger::log().error() <<"time: "<<  Root::getSingleton().getTimer()->getMilliseconds() - time;
    */
}

//================================================================================================
//
//================================================================================================
void GuiGadgetSlot::drawDragItem()
{
    int gfxNr = getTextureAtlasPos(mItem->face);
    mDnDTexture->getBuffer()->blitFromMemory(mAtlasTexture.getPixelBox().getSubVolume(Box(0, ITEM_SIZE * gfxNr, ITEM_SIZE, ITEM_SIZE *(gfxNr+1))));
    moveDragOverlay();
    mDnDOverlay->show();
}

