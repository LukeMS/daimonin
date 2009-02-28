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
#include "gui_element_slot.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_graphic.h"
#include "gui_manager.h"
#include "item.h"
#include "option.h"
#include "resourceloader.h"
#include "tile_map_wrapper.h"

using namespace Ogre;

// TODO: Use the tooltip overlay/texture for the dnd overlay/texturet.
const uint32 SLOT_BUSY_COLOR     = 0xdd777777;
const uint32 SLOT_QUANTITY_COLOR = 0x00888888;
const char UNKONWN_ITEM_GFX_FILENAME[] = "item_noGfx.png";
const int  UNKNOWN_ITEM_GFX = 0;
String GuiElementSlot::mResourceName = "";
Overlay *GuiElementSlot::mDnDOverlay =0;
OverlayElement *GuiElementSlot::mDnDElement =0;
Image GuiElementSlot::mAtlasTexture;
TexturePtr GuiElementSlot::mDnDTexture;
std::vector<Ogre::String> GuiElementSlot::mvAtlasGfxName;
int GuiElementSlot::mDragSlot =  -1;
int GuiElementSlot::mActiveSlot= -1;
int uid = -1;

//================================================================================================
// Constructor.
//================================================================================================
GuiElementSlot::GuiElementSlot(TiXmlElement *xmlElement, void *parent, const char *resourceName):GuiElement(xmlElement, parent)
{
    std::string filename;
    mSlotNr = ++uid;
    mItemGfxID = -1;
    mBusyTime = 1.0;  // Default time for a slot to be busy (MUST be > 0).
    mBusyOldVal = -1;
    mBusyTimeExpired = 0;
    // This stuff is static, so we have to do it only once.
    if (!uid)
    {
        mResourceName = resourceName;
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
            // Write the data to disc.
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
    }
    // ////////////////////////////////////////////////////////////////////
    // Look for a background graphic (its a png from the item folder).
    // ////////////////////////////////////////////////////////////////////
    if ((xmlElement = xmlElement->FirstChildElement("Image")))
    {
        const char *tmp;
        if ((tmp = xmlElement->Attribute("bg_item_image_filename")))
            mSlotGfxBG = getTextureAtlasPos(tmp);
        else
            mSlotGfxBG = -1;
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw the container.
    // ////////////////////////////////////////////////////////////////////
    if (mWidth > MAX_SIZE) mWidth = MAX_SIZE-1;
    if (mHeight> MAX_SIZE) mHeight= MAX_SIZE-1;
    draw();
}

//================================================================================================
// (Re)loads the material and texture or creates them if they dont exist.
//================================================================================================
void GuiElementSlot::loadResources()
{
    mDnDOverlay = GuiManager::getSingleton().loadResources(ITEM_SIZE, ITEM_SIZE, mResourceName);
    mDnDElement = mDnDOverlay->getChild(mResourceName + GuiManager::ELEMENT_RESOURCE_NAME);
    draw();
}

//================================================================================================
// Destructor.
//================================================================================================
GuiElementSlot::~GuiElementSlot()
{
    mDnDTexture.setNull();
    mvAtlasGfxName.clear();
}

//================================================================================================
//
//================================================================================================
int GuiElementSlot::sendMsg(int message, const char *text, uint32 param)
{
    switch (message)
    {
        case GuiManager::MSG_ADD_ITEM:
        {
            setItem(mvAtlasGfxName[1].c_str(), param); // Just testing
        }
    }
    return -1;
}

//================================================================================================
//
//================================================================================================
void GuiElementSlot::setItem(const char *gfxName, int quantity)
{
    for (unsigned int i = 0; i < mvAtlasGfxName.size(); ++i)
    {
        if (mvAtlasGfxName[i] == gfxName)
        {
            mItemGfxID = i;
            mQuantity = quantity;
            draw();
        }
    }
}

//================================================================================================
// Get the item pos in the item-texture-atlas.
//================================================================================================
int GuiElementSlot::getTextureAtlasPos(const char *gfxName)
{
    for (unsigned int i =0; i < mvAtlasGfxName.size(); ++i)
        if (mvAtlasGfxName[i] == gfxName) return i;
    return UNKNOWN_ITEM_GFX; // No gfx for this Item was found.
}

//================================================================================================
// Draw a busy gfx over the slot.
//================================================================================================
void GuiElementSlot::update(Real dTime)
{
    if (!mBusyTimeExpired) return;
    mBusyTimeExpired += dTime;
    int newVal = (int)((mBusyTimeExpired / mBusyTime)*360) ;
    if (newVal > 360) mBusyTimeExpired = 0;
    if (mBusyOldVal != newVal)
    {
        mBusyOldVal = newVal;
        draw();
    }
}

//================================================================================================
// Test if Mouse is over this slot.
//================================================================================================
bool GuiElementSlot::mouseWithin(int x, int y)
{
    if (x < mPosX || x > mPosX + mWidth || y < mPosY || y > mPosY + mHeight)
        return false;
    return true;
}

//================================================================================================
// .
//================================================================================================
int GuiElementSlot::mouseEvent(int MouseAction, int x, int y)
{
    if (mouseWithin(x, y))
    {
        if (mActiveSlot != mSlotNr)
        {
            mActiveSlot = mSlotNr;
            if (setState(GuiImageset::STATE_ELEMENT_M_OVER)) draw();
            //GuiManager::getSingleton().setTooltip(mStrTooltip.c_str());
            return GuiManager::EVENT_CHECK_NEXT;
        }
        if (MouseAction == GuiManager::BUTTON_PRESSED && mItemGfxID >= 0)
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
            if (setState(GuiImageset::STATE_ELEMENT_DEFAULT)) draw();
            mActiveSlot = -1;
            GuiManager::getSingleton().setTooltip("");
            return GuiManager::EVENT_CHECK_DONE;
        }
    }
    return GuiManager::EVENT_CHECK_NEXT;
}

//================================================================================================
//
//================================================================================================
void GuiElementSlot::drawDragItem()
{
    if (mItemGfxID < 0) return;
    if (mDnDTexture.isNull())
    {
        mDnDTexture = TextureManager::getSingleton().createManual(mResourceName+GuiManager::TEXTURE_RESOURCE_NAME,
                      ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                      TEX_TYPE_2D, ITEM_SIZE, ITEM_SIZE, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY,
                      ManResourceLoader::getSingleton().getLoader());
        mDnDTexture->load();
    }
    mDnDTexture->getBuffer()->blitFromMemory(mAtlasTexture.getPixelBox().getSubVolume(Box(0, ITEM_SIZE * mItemGfxID, ITEM_SIZE, ITEM_SIZE *(mItemGfxID+1))));
    moveDragOverlay();
    mDnDOverlay->show();
}

//================================================================================================

// .
//================================================================================================
void GuiElementSlot::draw()
{
    if (!mIsVisible || mItemGfxID < 0)
    {
        GuiElement::draw();
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // Draw the slot-gfx to the build-buffer.
    // ////////////////////////////////////////////////////////////////////
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    uint32 *bak = mParent->getLayerBG() + mPosX + mPosY*mParent->getWidth();
    if (mGfxSrc)
    {
        PixelBox src = mParent->getPixelBox()->getSubVolume(Box(mGfxSrc->state[mState].x, mGfxSrc->state[mState].y,
                       mGfxSrc->state[mState].x + mGfxSrc->w, mGfxSrc->state[mState].y + mGfxSrc->h));
        int srcRowSkip = (int)mParent->getPixelBox()->getWidth();
        GuiGraphic::getSingleton().drawGfxToBuffer(mWidth, mHeight, mGfxSrc->w, mGfxSrc->h, (uint32*)src.data, bak, dst, srcRowSkip, mParent->getWidth(), MAX_SIZE);
    }
    else
        GuiGraphic::getSingleton().drawColorToBuffer(mWidth, mHeight, mFillColor, bak, dst, mParent->getWidth(), MAX_SIZE);
    // ////////////////////////////////////////////////////////////////////
    // Draw the item-gfx to the build-buffer.
    // ////////////////////////////////////////////////////////////////////
    int dX  = (mWidth  - ITEM_SIZE) /2;
    int dY  = (mHeight - ITEM_SIZE) /2;
    PixelBox src = mAtlasTexture.getPixelBox().getSubVolume(Box(0, ITEM_SIZE * mItemGfxID, ITEM_SIZE, ITEM_SIZE *(mItemGfxID+1)));
    uint32 *buf = dst + dX + dY * MAX_SIZE;
    GuiGraphic::getSingleton().drawGfxToBuffer(ITEM_SIZE, ITEM_SIZE, ITEM_SIZE, ITEM_SIZE, (uint32*)src.data, buf, buf, ITEM_SIZE, MAX_SIZE, MAX_SIZE);
    // ////////////////////////////////////////////////////////////////////
    // Draw the busy-gfx to the build-buffer.
    // ////////////////////////////////////////////////////////////////////
    if (mBusyTimeExpired)
        drawBusy((int)mBusyOldVal);
    // ////////////////////////////////////////////////////////////////////
    // Copy the build-buffer to the window texture.
    // ////////////////////////////////////////////////////////////////////
    mParent->getTexture()->getBuffer()->blitFromMemory(PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, dst), Box(mPosX, mPosY, mPosX+mWidth, mPosY+mHeight));
}

//================================================================================================
// Draws the busy gfx into the build buffer.
//================================================================================================
void GuiElementSlot::drawBusy(int angle)
{
    int x2,x3,y2,dY,dX,xStep,yStep,delta,posY;
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();;
    int x = mWidth/2;
    int y = mHeight/2;
    if (angle == 180)
        GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight, SLOT_BUSY_COLOR, dst, MAX_SIZE);
    else if (angle > 180)
    {
        if (angle < 225)
        {
            Real step = mWidth/45.0f;
            x2 = (int)(mWidth - (angle-180)*step)/2;
            y2 = mHeight;
        }
        else if (angle <= 315)
        {
            x2 = 0;
            Real step = mHeight/90.0f;
            y2 = (int)(mHeight - (angle-225)*step);
        }
        else
        {
            Real step = mWidth/45.0f;
            x2= (int)((angle-315)*step)/2;
            y2 = -1;
        }
        dX = Math::IAbs(x2-x);
        dY = Math::IAbs(y2-y);
        delta= dX - dY;
        xStep= (x2>x)?1:-1;
        yStep= (y2>y)?1:-1;
        x3= (y2<mHeight/2)?mWidth/2:0;
        while (x!=x2)
        {
            if (delta >= 0)
            {
                x+= xStep;
                delta-= dY;
            }
            else
            {
                y+= yStep;
                delta+= dX;
                posY = y*MAX_SIZE;
                if (x < x3)
                    GuiGraphic::getSingleton().drawColorToBuffer(x3-x, 1, SLOT_BUSY_COLOR, dst + posY+x, MAX_SIZE);
                else
                    GuiGraphic::getSingleton().drawColorToBuffer(x-x3, 1, SLOT_BUSY_COLOR, dst + posY+x3, MAX_SIZE);
            }
        }
        if (angle < 270)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight/2+1, SLOT_BUSY_COLOR, dst, MAX_SIZE);
        else if (angle < 315)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, y2+1, SLOT_BUSY_COLOR, dst, MAX_SIZE);
    }
    else // 0...180°
    {
        if (angle <= 45)
        {
            Real step = mWidth/45.0f;
            x2= (int)(angle*step + mWidth)/2;
            y2= -1;
        }
        else if (angle <= 135)
        {
            Real step = mHeight/90.0f;
            x2 = mWidth;
            y2 = (int)((angle-45)*step);
            if (angle > 90)
                GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight-y2, SLOT_BUSY_COLOR, dst + mWidth/2 + y2*MAX_SIZE, MAX_SIZE);
        }
        else
        {
            Real step = mWidth/45.0f;
            x2 = mWidth-(int)((angle-135)*step)/2;
            y2 = mHeight;
        }
        dX = Math::IAbs(x2-x);
        dY = Math::IAbs(y2-y);
        delta = dX - dY;
        yStep = (y2 >y)?1:-1;
        x3=(y2 >y)?mWidth/2:mWidth;
        while (x!=x2)
        {
            if (delta >= 0)
            {
                ++x;
                delta-= dY;
            }
            else
            {
                y+= yStep;
                delta+= dX;
                posY = y*MAX_SIZE;
                if (x < x3)
                    GuiGraphic::getSingleton().drawColorToBuffer(x3-x, 1, SLOT_BUSY_COLOR, dst + posY+x, MAX_SIZE);
                else
                    GuiGraphic::getSingleton().drawColorToBuffer(x-x3, 1, SLOT_BUSY_COLOR, dst + posY+x3, MAX_SIZE);
            }
        }
        // Fill the lower right side.
        if (angle <= 90)
            GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight/2, SLOT_BUSY_COLOR, dst + mWidth/2 + mHeight/2*MAX_SIZE, MAX_SIZE);
        // Fill the complete left side.
        GuiGraphic::getSingleton().drawColorToBuffer(mWidth/2, mHeight, SLOT_BUSY_COLOR, dst, MAX_SIZE);
    }
}
