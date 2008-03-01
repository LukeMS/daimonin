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

const char UNKONWN_ITEM_GFX_FILENAME[] = "item_noGfx.png";
const int UNKNOWN_ITEM_GFX = 0;
const int BITS_FACEFILTER = ~0x8000; // Filter to extract the face number (gfx-id).
Overlay *GuiGadgetSlot::mDnDOverlay =0;
OverlayElement *GuiGadgetSlot::mDnDElement =0;
Image GuiGadgetSlot::mAtlasTexture;
MaterialPtr GuiGadgetSlot::mDnDMaterial;
TexturePtr GuiGadgetSlot::mDnDTexture;
std::vector<Ogre::String> GuiGadgetSlot::mvAtlasGfxName;
int GuiGadgetSlot::mDragSlot =  -1;
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
    mBusyTime = 1.0;  // Default time for a slot to be busy (MUST be > 0).
    mBusyTimeExpired = 0;
    // This stuff is static, so we have to do it only once.
    if (!mDnDOverlay)
    {
        // ////////////////////////////////////////////////////////////////////
        // Create the item texture atlas.
        // ////////////////////////////////////////////////////////////////////
        //if (Option::getSingleton().getIntValue(Option::CMDLINE_CREATE_ITEMS))
        {
            bool unknownGfxFound = false;
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
                {
                    // Force the unknown item gfx to be the first gfx.
                    if (!strcmp(FindFileData.cFileName, UNKONWN_ITEM_GFX_FILENAME))
                    {
                        itemFilename.insert(itemFilename.begin(), FindFileData.cFileName);
                        unknownGfxFound = true;
                    }
                    else
                        itemFilename.push_back(FindFileData.cFileName);
                }
                found = FindNextFile(handle, &FindFileData);
            }
#else
            struct dirent *dir_entry;
            DIR *dir = opendir(PATH_ITEM_TEXTURES); // Open the current directory
            while ((dir_entry = readdir(dir)))
            {
                if (strstr(dir_entry->d_name, ".png") && !strstr(dir_entry->d_name, FILE_ITEM_TEXTURE_ATLAS))
                {
                    // Force the unknown item gfx to be the first gfx.
                    if (!strcmp(dir_entry->d_name, UNKONWN_ITEM_GFX_FILENAME))
                    {
                        itemFilename.insert(itemFilename.begin(), dir_entry->d_name);
                        unknownGfxFound = true;
                    }
                    else
                        itemFilename.push_back(dir_entry->d_name);
                }
            }
            closedir(dir);
#endif
            if (itemFilename.empty())
            {
                Logger::log().error() << "Could not find any item graphics in " << PATH_ITEM_TEXTURES;
                return;
            }
            if (!unknownGfxFound)
            {
                Logger::log().error() << "Could not find the gfx for an unknown item (filename: " << UNKONWN_ITEM_GFX_FILENAME
                << " in folder " << PATH_ITEM_TEXTURES << ").";
            }
            Image itemImage, itemAtlas;
            uint32 *itemBuffer = new uint32[ITEM_SIZE * ITEM_SIZE * itemFilename.size()];
            uint32 *nextPos = itemBuffer;
            itemAtlas = itemAtlas.loadDynamicImage((unsigned char*)itemBuffer, ITEM_SIZE, ITEM_SIZE * itemFilename.size(), 1, PF_A8R8G8B8);
            for (unsigned int i = 0; i < itemFilename.size(); ++i)
            {
                itemImage.load(itemFilename[i], ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                if (itemImage.getHeight() != ITEM_SIZE || itemImage.getWidth() != ITEM_SIZE)
                {
                    Logger::log().warning() << "CreateItemAtlas: Unsupported image size. Only Items of "
                    << (int)ITEM_SIZE << " * " << (int)ITEM_SIZE << " pixel are allowed "<< "[" << itemFilename[i] << "].";
                    break;
                }
                if (itemImage.getFormat() != PF_A8R8G8B8)
                {
                    Logger::log().warning() << "CreateItemAtlas: Unsupported image format ("<< itemImage.getFormat() <<")."
                    << " Only 32bit [ARGB] png format is allowed "<< "[" << itemFilename[i] << "].";
                    break;
                }
                memcpy(nextPos, itemImage.getData(), ITEM_SIZE * ITEM_SIZE * sizeof(uint32));
                nextPos+=ITEM_SIZE * ITEM_SIZE;
            }
            // ////////////////////////////////////////////////////////////////////
            // Write the datas to disc.
            // ////////////////////////////////////////////////////////////////////
            filename = PATH_ITEM_TEXTURES;
            filename+= "/";
            filename+= FILE_ITEM_TEXTURE_ATLAS;
            filename+= ".png";
            itemAtlas.save(filename);
            filename.replace(filename.find(".png"), 4, ".txt");
            std::ofstream txtFile(filename.c_str(), std::ios::out | std::ios::binary);
            txtFile << "# This file holds the content of the image-texture-atlas." << std::endl;
            txtFile << "# The filename for an undefined/missing gfx must be: " << UNKONWN_ITEM_GFX_FILENAME << std::endl;
            if (txtFile)
            {
                for (unsigned int i = 0; i < itemFilename.size(); ++i)
                    txtFile << itemFilename[i] << std::endl;
            }
            txtFile.close();
            itemFilename.clear();
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
        mAtlasTexture.load(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        // ////////////////////////////////////////////////////////////////////
        // Create drag'n'drop overlay.
        // We must clear the whole texture, because textures do always have
        // 2^n size - while slots can have any size.
        // ////////////////////////////////////////////////////////////////////
        mDnDTexture = TextureManager::getSingleton().createManual("GUI_SlotDnD_Texture", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                      TEX_TYPE_2D, ITEM_SIZE, ITEM_SIZE, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY);
        mDnDOverlay = OverlayManager::getSingleton().create("GUI_SlotDnD_Overlay");
        memset(mDnDTexture->getBuffer()->lock(HardwareBuffer::HBL_DISCARD), 0x00,
               mDnDTexture->getWidth()*mDnDTexture->getHeight()*sizeof(uint32));
        mDnDTexture->getBuffer()->unlock();
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
    // Look for a background graphic (its a png from the item folder).
    // ////////////////////////////////////////////////////////////////////
    TiXmlElement *xmlGadget;
    if ((xmlGadget = xmlElement->FirstChildElement("Image")))
    {
        const char *tmp;
        if ((tmp = xmlGadget->Attribute("bg_item_image_filename")))
            mSlotGfxBG = getTextureAtlasPos(tmp);
        else
            mSlotGfxBG = -1;
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw the container.
    // ////////////////////////////////////////////////////////////////////
    if (drawOnInit) draw();
}

//================================================================================================
// Destructor..
//================================================================================================
GuiGadgetSlot::~GuiGadgetSlot()
{
    mDnDMaterial.setNull();
    mDnDTexture.setNull();
    mvAtlasGfxName.clear();
}

//================================================================================================
// Draw a busy gfx over the slot..
//================================================================================================
void GuiGadgetSlot::update(Real dTime)
{
    if (!mBusyTimeExpired) return;
    mBusyTimeExpired += dTime;
    if (mBusyTimeExpired > mBusyTime) // Busy animation completed.
        mBusyTimeExpired = 0;
    draw();
}

//================================================================================================
// Test if Mouse is over this slot..
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
    if (mouseWithin(x, y))
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
    return getTextureAtlasPos(ObjectWrapper::getSingleton().getMeshName(itemFace & BITS_FACEFILTER));
}

//================================================================================================
// Get the item pos in the item-texture-atlas.
//================================================================================================
int GuiGadgetSlot::getTextureAtlasPos(const char *gfxName)
{
    for (unsigned int i =0; i < mvAtlasGfxName.size(); ++i)
        if (mvAtlasGfxName[i] == gfxName) return i;
    return UNKNOWN_ITEM_GFX; // No gfx for this Item was found.
}

//================================================================================================
// .
//================================================================================================
void GuiGadgetSlot::draw()
{
    // no item and no background gfx in this slot.
    if (!mItem && mSlotGfxBG < 0)
    {
        GuiGraphic::draw();
        return;
    }
    // We only need a redraw after the value has changed.
    int newVal = static_cast<int>((ITEM_SIZE*4 * mBusyTimeExpired) / mBusyTime);
    if (mBusyTimeExpired && mBusyOldVal == newVal) return;
    mBusyOldVal = newVal;
    // Draw a background gfx into the slot.
    if (!mItem)
    {
        PixelBox srcItem = mAtlasTexture.getPixelBox().getSubVolume(Box(0, ITEM_SIZE * mSlotGfxBG, ITEM_SIZE, ITEM_SIZE *(mSlotGfxBG+1)));
        GuiGraphic::drawSlot(static_cast<uint32*>(srcItem.data), static_cast<int>((ITEM_SIZE*4 * mBusyTimeExpired) / mBusyTime), 0);
    }
    // Draw an item into the slot.
    else
    {
        int gfxNr = getTextureAtlasPos(mItem->face);
        PixelBox srcItem = mAtlasTexture.getPixelBox().getSubVolume(Box(0, ITEM_SIZE * gfxNr, ITEM_SIZE, ITEM_SIZE *(gfxNr+1)));
        GuiGraphic::drawSlot(static_cast<uint32*>(srcItem.data), newVal, mItem->nrof);
    }
}

//================================================================================================
//
//================================================================================================
void GuiGadgetSlot::drawDragItem()
{
    if (!mItem) return;
    int gfxNr = getTextureAtlasPos(mItem->face);
    mDnDTexture->getBuffer()->blitFromMemory(mAtlasTexture.getPixelBox().getSubVolume(Box(0, ITEM_SIZE * gfxNr, ITEM_SIZE, ITEM_SIZE *(gfxNr+1))));
    moveDragOverlay();
    mDnDOverlay->show();
}

