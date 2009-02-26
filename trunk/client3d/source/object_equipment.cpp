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
#include "object_manager.h"
#include "object_equipment.h"
#include "particle_manager.h"
#include "sound.h"
#include "gui_manager.h"
#include "events.h"
#include "option.h"
#include "logger.h"

using namespace Ogre;

const char *FILE_SHADOW_IMAGE = "shadow.png";
String boneName[ObjectEquipment::BONE_SUM]=
{
    "Pelvis",
    "Center",
    "Head",
    "Neck",
    "Spine",     "Spine1",
    "LClavicle", "R_Clavicle",
    "LUpperArm", "RUpperArm",
    "LForeArm",  "RForeArm",
    "LHand",     "RHand",
    "RFingers",  "LFingers",
    "LThigh",    "RThigh",
    "LCalf",     "RCalf",
    "LFoot",     "RFoot",
    "LToes",     "RToes"
};

const char *particleName[ObjectEquipment::PARTICLE_FX_SUM]=
{
    "Particle/SwordGlow"
};

// Todo: use pointer to vector for this. and read meshnames from a xml-file.
const char *meshName[][ObjectEquipment::ITEM_SUM]=
{
    {   // ITEM_WEAPON
        "Sword_Short_01.mesh",
        "Mace_Small_01.mesh",
        "Short_Bow.mesh",
    },
    {   // ITEM_ARMOR_SHIELD
        "Shield_Round_01.mesh",
        "Shield_Round_02.mesh"
    },
    {   // ITEM_ARMOR_HEAD
        0,
        0
    },
    {   // ITEM_ARMOR_BODY
        0,
        0
    },
    {   // ITEM_ARMOR_LEGS
        0,
        0
    }
};

unsigned long ObjectEquipment::mIndex =0;
unsigned char *ObjectEquipment::mTexImageBuf = 0;
Image ObjectEquipment::shadowImage;

//const uint32 MASK_COLOR = 0xffc638db; // This is our mask. Pixel with this color will not be drawn.
const uint32 MASK_COLOR = 0xffdb38c6; // This is our mask. Pixel with this color will not be drawn.

const int MAX_MODEL_TEXTURE_SIZE = 512;

ObjectEquipment::sPicture ObjectEquipment::picFace =
{
    65, 75, // w, h
    324, 13,  // dst pos.
    188,  1,  // src pos.
    0, 79 // offset next src pic.
};
ObjectEquipment::sPicture ObjectEquipment::picHair =
{
    49, 85, // w, h
    130, 3,  // dst pos.
    254, 1,  // src pos.
    0, 86 // offset next src pic.
};
ObjectEquipment::sPicture ObjectEquipment::picBody[2] =
{
    { // Back
        186, 153, // w, h
        61,  70,  // dst pos.
        1,  155,  // src pos.
        0, 0 // offset next src pic.
    },
    { // Front
        186, 153, // w, h
        261,  70,  // dst pos.
        1,  1,  // src pos.
        0, 0 // offset next src pic.
    }
};
ObjectEquipment::sPicture ObjectEquipment::picArms[4] =
{
    { // Back Left
        38, 81, // w, h
        56,  155,  // dst pos.
        1,  529,  // src pos.
        0, 0 // offset next src pic.
    },
    { // Back Right
        38, 81, // w, h
        212,  155,  // dst pos.
        40,  529,  // src pos.
        0, 0 // offset next src pic.
    },
    { // Front Left
        38, 81, // w, h
        256,  155,  // dst pos.
        79,  529,  // src pos.
        0, 0 // offset next src pic.
    },
    { // Front Right
        38, 81, // w, h
        412,  155,  // dst pos.
        118, 529,  // src pos.
        0, 0 // offset next src pic.
    }
};
ObjectEquipment::sPicture ObjectEquipment::picHands[4] =
{
    { // Back
        29, 57, // w, h
        58,  236,  // dst pos.
        261, 297,  // src pos.
        0, 0 // offset next src pic.
    },
    { // Back
        29, 57, // w, h
        221, 236,  // dst pos.
        261, 355,  // src pos.
        0, 0 // offset next src pic.
    },
    { // Front
        29, 57, // w, h
        259, 236,  // dst pos.
        261, 413,  // src pos.
        0, 0 // offset next src pic.
    },
    { // Front
        29, 57, // w, h
        421, 236,  // dst pos.
        261, 471,  // src pos.
        0, 0 // offset next src pic.
    }
};
ObjectEquipment::sPicture ObjectEquipment::picBelt[2] =
{
    { // Back
        129, 22, // w, h
        89, 223,  // dst pos.
        157, 529,  // src pos.
        0, 87 // offset next src pic.
    },
    { // Front
        129, 22, // w, h
        291, 223,  // dst pos.
        157, 529,  // src pos.
        0, 87 // offset next src pic.
    }
};
ObjectEquipment::sPicture ObjectEquipment::picLegs[2] =
{
    { // Back
        129, 219, // w, h
        89, 245,  // dst pos.
        1,  309,  // src pos.
        0, 0 // offset next src pic.
    },
    { // Front
        129, 219, // w, h
        291, 245,  // dst pos.
        131,  309,  // src pos.
        0, 0 // offset (x,y) for next src pic.
    }
};
ObjectEquipment::sPicture ObjectEquipment::picShoes[2] =
{
    { // Left
        43, 63, // w, h
        304, 446,  // dst pos.
        157,  575,  // src pos.
        0, 0 // offset next src pic.
    },
    { // Right
        43, 63, // w, h
        363, 446,  // dst pos.
        201, 575,  // src pos.
        0, 0 // offset next src pic.
    }
};

//================================================================================================
// .
//================================================================================================
ObjectEquipment::~ObjectEquipment()
{}

//================================================================================================
// Free all recources.
//================================================================================================
void ObjectEquipment::freeRecources()
{
    mTexture.setNull();
}

//================================================================================================
// Init the model from the description file.
//================================================================================================
ObjectEquipment::ObjectEquipment(Entity *parentEntity)
{
    Logger::log().list() << "Adding Equipment.";
    if (!mIndex++)
    {
        try
        {
            shadowImage.load(FILE_SHADOW_IMAGE, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        }
        catch (Exception&)
        {
            Logger::log().error() << "ObjectEquipment: couldn't find shadow mask " << FILE_SHADOW_IMAGE;
        }
    }
    mParentEntity = parentEntity;
    for (int bone=0; bone < BONE_SUM; ++bone)
    {
        mItem[bone].entity  = 0;
        mItem[bone].particle= 0;
    }
    // Clone the ObjectNPC-Material.
    String tmpName = "Mat_NPC_" + StringConverter::toString(mIndex, 6, '0');
    MaterialPtr material = MaterialManager::getSingleton().getByName("NPC");
    material = material->clone(tmpName);
    mParentEntity->getSubEntity(0)->setMaterialName(tmpName);
    // Create a texture for the material.
    tmpName = "Tex_NPC_" + StringConverter::toString(mIndex, 6, '0');
    mTexture = TextureManager::getSingleton().createManual(tmpName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
               TEX_TYPE_2D, MAX_MODEL_TEXTURE_SIZE, MAX_MODEL_TEXTURE_SIZE, 0, PF_A8R8G8B8, TU_STATIC_WRITE_ONLY);
    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(tmpName);
    // Set the default Colors of the model.
    setTexture(0, 0, 0);
    setTexture(2, 1, 0); setTexture(3, 2, 0); setTexture(4, 3, 0);
    setTexture(5, 4, 0); setTexture(6, 5, 0); setTexture(7, 6, 0);
    // We need a buffer to draw the gfx, before we blit it to the texture.
    GuiManager::getSingleton().resizeBuildBuffer(MAX_MODEL_TEXTURE_SIZE*MAX_MODEL_TEXTURE_SIZE);
}

//================================================================================================
// Draw a part of the texture.
//================================================================================================
inline void ObjectEquipment::drawBopyPart(sPicture &picPart, uint32 texColor, uint32 texNumber)
{
    uint32 srcColor, dstColor;
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    uint32 *buf = dst;
    uint32 *texRace = (uint32*)shadowImage.getData();
    int srcWidth = (int)shadowImage.getWidth();
    texColor = texRace[texColor & 0xff];
    // Fill the buffer with the selected color (darkened by the shadow texture).
    for (int y=0; y < picPart.h; ++y)
    {
        for (int x=0; x < picPart.w; ++x)
        {
            srcColor = texRace[(y+picPart.srcY)*srcWidth + (x+picPart.srcX)];
            if (srcColor != MASK_COLOR)
            {
                dstColor = texColor;
                if (srcColor != 0xffffffff) // darkening.
                {
                    srcColor = 0xff - (srcColor & 0xff);
                    if ((dstColor & 0x0000ff) >= srcColor) dstColor-= srcColor; else dstColor-= dstColor & 0x0000ff;
                    srcColor <<= 8;
                    if ((dstColor & 0x00ff00) >= srcColor) dstColor-= srcColor; else dstColor-= dstColor & 0x00ff00;
                    srcColor <<= 8;
                    if ((dstColor & 0xff0000) >= srcColor) dstColor-= srcColor; else dstColor-= dstColor & 0xff0000;
                }
                *buf = dstColor;
            }
            ++buf;
        }
    }
    mTexture->getBuffer()->blitFromMemory(PixelBox(picPart.w, picPart.h, 1, PF_A8R8G8B8, dst), Box(picPart.dstX, picPart.dstY, picPart.dstX+picPart.w, picPart.dstY+picPart.h));

#ifdef WRITE_MODELTEXTURE_TO_FILE
    // Write the model-texture as png to disk.
    {
        static int nr = 0;
        String file = "NPC_Texture__" + StringConverter::toString(nr++, 3, '0') +".png";
        uint32 *tmpBuf = new uint32[mTexture->getWidth()*mTexture->getHeight()];
        mTexture->getBuffer()->blitToMemory(PixelBox(mTexture->getWidth(), mTexture->getHeight(), 1, PF_A8R8G8B8, tmpBuf));
        Image img;
        img = img.loadDynamicImage((unsigned char*)tmpBuf, mTexture->getWidth(), mTexture->getHeight(), PF_A8R8G8B8);
        img.save(file);
        delete[] tmpBuf;
    }
#endif
}

//================================================================================================
// Select a new texture.
//================================================================================================
void ObjectEquipment::setTexture(int pos, int textureColor, int textureNr)
{
    switch (pos)
    {
        case TEXTURE_POS_SKIN:
        {
            drawBopyPart(picFace, textureColor, textureNr);
            for (int side = 0; side < 4; ++side) drawBopyPart(picArms[side], textureColor, textureNr);
            break;
        }
        case TEXTURE_POS_FACE:
        {
            drawBopyPart(picFace, textureColor, textureNr);
            break;
        }
        case TEXTURE_POS_HAIR:
        {
            drawBopyPart(picHair, textureColor, textureNr);
            break;
        }
        case TEXTURE_POS_BODY:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picBody[side], textureColor, textureNr);
            break;
        }
        case TEXTURE_POS_LEGS:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picLegs[side], textureColor, textureNr);
            break;
        }
        case TEXTURE_POS_BELT:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picBelt[side], textureColor, textureNr);
            break;
        }
        case TEXTURE_POS_SHOES:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picShoes[side], textureColor, textureNr);
            break;
        }
        case TEXTURE_POS_HANDS:
        {
            for (int side = 0; side < 4; ++side) drawBopyPart(picHands[side], textureColor, textureNr);
            break;
        }
        default:
            Logger::log().warning() << "Unknown Texuture-pos (" << pos << ") for ObjectNPC.";
            break;
    }
}

//================================================================================================
// Create and attach an equipment item to bone.
//================================================================================================
void ObjectEquipment::equipItem(unsigned int bone, int type, int itemID, int particleID)
{
    if (bone >= BONE_SUM) return;
    dropItem(bone);
    // Add a particle system.
    if (particleID < 0 || particleID >= PARTICLE_FX_SUM)
    {
        mItem[bone].particle= 0;
    }
    else
    {
        mItem[bone].particle = ParticleManager::getSingleton().addBoneObject(mParentEntity, boneName[bone].c_str(), particleName[particleID], -1);
    }
    // Add a entity.
    if (itemID < 0 || itemID >= ITEM_SUM)
    {
        mItem[bone].entity = 0;
    }
    else
    {
        static unsigned long itemIndex =0;
        String tmpName = "Item_" + StringConverter::toString(++itemIndex, 8, '0');
        mItem[bone].entity= Events::getSingleton().GetSceneManager()->createEntity(tmpName, meshName[type][itemID]);
        mItem[bone].entity->setQueryFlags(ObjectManager::QUERY_EQUIPMENT_MASK);
        mParentEntity->attachObjectToBone(boneName[bone], mItem[bone].entity);
    }
}

//================================================================================================
// Detach and destroy an equipment item from bone.
//================================================================================================
void ObjectEquipment::dropItem(int bone)
{
    if (mItem[bone].entity)
    {
        mParentEntity->detachObjectFromBone(mItem[bone].entity);
        Events::getSingleton().GetSceneManager()->destroyEntity(mItem[bone].entity);
        mItem[bone].entity =0;
    }
    if (mItem[bone].particle)
    {
        mParentEntity->detachObjectFromBone(mItem[bone].particle);
        ParticleManager::getSingleton().delObject(mItem[bone].particle);
        mItem[bone].particle =0;
    }
}
