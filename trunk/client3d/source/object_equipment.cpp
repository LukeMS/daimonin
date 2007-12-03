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
#include "events.h"
#include "option.h"
#include "logger.h"

using namespace Ogre;

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
unsigned char *ObjectEquipment::texImageBuf = 0;

const uint32 MASK_COLOR = 0xffc638db; // This is our mask. Pixel with this color will not be drawn.

const int MAX_MODEL_TEXTURE_SIZE = 512;

ObjectEquipment::sPicture ObjectEquipment::picFace =
    {
        65, 75, // w, h
        324, 13,  // dst pos.
        188,  1,  // src pos.
        0, 79 // offest next src pic.
    };
ObjectEquipment::sPicture ObjectEquipment::picHair =
    {
        49, 85, // w, h
        130, 3,  // dst pos.
        254, 1,  // src pos.
        0, 86 // offest next src pic.
    };
ObjectEquipment::sPicture ObjectEquipment::picBody[2] =
    {
        { // Back
            186, 153, // w, h
            61,  70,  // dst pos.
            1,  155,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front
            186, 153, // w, h
            261,  70,  // dst pos.
            1,  1,  // src pos.
            0, 0 // offest next src pic.
        }
    };
ObjectEquipment::sPicture ObjectEquipment::picArms[4] =
    {
        { // Back Left
            38, 81, // w, h
            56,  155,  // dst pos.
            1,  529,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Back Right
            38, 81, // w, h
            212,  155,  // dst pos.
            40,  529,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front Left
            38, 81, // w, h
            256,  155,  // dst pos.
            79,  529,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front Right
            38, 81, // w, h
            412,  155,  // dst pos.
            118, 529,  // src pos.
            0, 0 // offest next src pic.
        }
    };
ObjectEquipment::sPicture ObjectEquipment::picHands[4] =
    {
        { // Back
            29, 57, // w, h
            58,  236,  // dst pos.
            261, 297,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Back
            29, 57, // w, h
            221, 236,  // dst pos.
            261, 355,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front
            29, 57, // w, h
            259, 236,  // dst pos.
            261, 413,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front
            29, 57, // w, h
            421, 236,  // dst pos.
            261, 471,  // src pos.
            0, 0 // offest next src pic.
        }
    };
ObjectEquipment::sPicture ObjectEquipment::picBelt[2] =
    {
        { // Back
            129, 22, // w, h
            89, 223,  // dst pos.
            157, 529,  // src pos.
            0, 87 // offest next src pic.
        },
        { // Front
            129, 22, // w, h
            291, 223,  // dst pos.
            157, 529,  // src pos.
            0, 87 // offest next src pic.
        }
    };
ObjectEquipment::sPicture ObjectEquipment::picLegs[2] =
    {
        { // Back
            129, 219, // w, h
            89, 245,  // dst pos.
            1,  309,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Front
            129, 219, // w, h
            291, 245,  // dst pos.
            131,  309,  // src pos.
            0, 0 // offest (x,y) for next src pic.
        }
    };
ObjectEquipment::sPicture ObjectEquipment::picShoes[2] =
    {
        { // Left
            43, 63, // w, h
            304, 446,  // dst pos.
            157,  575,  // src pos.
            0, 0 // offest next src pic.
        },
        { // Right
            43, 63, // w, h
            363, 446,  // dst pos.
            201, 575,  // src pos.
            0, 0 // offest next src pic.
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
    delete[] texImageBuf;
}

//================================================================================================
// Init the model from the description file.
//================================================================================================
ObjectEquipment::ObjectEquipment(Entity *parentEntity)
{
    Logger::log().list()  << "Adding Equipment.";
    if (!mIndex++)
    {
        texImageBuf = new unsigned char[MAX_MODEL_TEXTURE_SIZE * MAX_MODEL_TEXTURE_SIZE * sizeof(uint32)];
    }
    mParentEntity = parentEntity;
    for (int bone=0; bone < BONE_SUM; ++bone)
    {
        mItem[bone].entity  = 0;
        mItem[bone].particle= 0;
    }

    // ////////////////////////////////////////////////////////////////////
    // We ignore the material of the mesh and create an own material.
    // ////////////////////////////////////////////////////////////////////
    // Clone the ObjectNPC-Material.
    String tmpName = "EQ_" + StringConverter::toString(mIndex, 6, '0');
    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("NPC");
    MaterialPtr mMaterial = tmpMaterial->clone(tmpName);
    mParentEntity->getSubEntity(0)->setMaterialName(tmpName);
    // Create a texture for the material.
    Image image;
    image.loadDynamicImage(texImageBuf, MAX_MODEL_TEXTURE_SIZE, MAX_MODEL_TEXTURE_SIZE, PF_A8R8G8B8);
    tmpName +="_Texture";
    mTexture = TextureManager::getSingleton().loadImage(tmpName, "General", image, TEX_TYPE_2D, 3, 1.0f);
    mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(tmpName);

    // Set the default Colors of the model.
    setTexture(0, 0, 0);
    setTexture(2, 1, 0);
    setTexture(3, 2, 0);
    setTexture(4, 3, 0);
    setTexture(5, 4, 0);
    setTexture(6, 5, 0);
    setTexture(7, 6, 0);
}

//================================================================================================
// Draw a part of the texture.
//================================================================================================
inline void ObjectEquipment::drawBopyPart(sPicture &picPart, Image &image, uint32 texColor, uint32 texNumber)
{
    uint32 srcColor, dstColor;
    uint32 *texRace = (uint32*)image.getData();
    uint32 *buffer  = new uint32[picPart.w * picPart.h];
    uint32 *buf = buffer;
    int width = (int)image.getWidth();
    // Get the color information from the top line of the texture-shadow picture.
    // We have to swap R and B Color information (default texture format is A8R8G8B8).
    srcColor = texRace[texColor & 0xff];
    texColor = (srcColor & 0xff00ff00);
    texColor+= (srcColor & 0x000000ff) << 16;
    texColor+= (srcColor & 0x00ff0000) >> 16;
    // Get the current model-texture fragment.
    PixelBox pb(picPart.w, picPart.h, 1, PF_A8B8G8R8 , buffer);
    mTexture->getBuffer()->blitToMemory(
        Box(picPart.dstX,
            picPart.dstY,
            picPart.dstX + picPart.w,
            picPart.dstY + picPart.h),
        pb);
    // Fill the buffer with the selected color (darkened by the shadow texture).
    for (int y=0; y < picPart.h; ++y)
    {
        for (int x=0; x < picPart.w; ++x)
        {
            srcColor = texRace[(y+picPart.srcY)*width + (x+picPart.srcX)];
            if (srcColor != MASK_COLOR)
            {
                dstColor = texColor;
                if (srcColor != 0xffffffff) // darkening.
                {
                    srcColor = 0xff - (srcColor & 0xff);
                    if ((dstColor & 0x0000ff) >= srcColor ) dstColor-= srcColor; else dstColor-= dstColor & 0x0000ff;
                    srcColor <<= 8;
                    if ((dstColor & 0x00ff00) >= srcColor ) dstColor-= srcColor; else dstColor-= dstColor & 0x00ff00;
                    srcColor <<= 8;
                    if ((dstColor & 0xff0000) >= srcColor ) dstColor-= srcColor; else dstColor-= dstColor & 0xff0000;
                }
                *buffer = dstColor;
            }
            ++buffer;
        }
    }
    // Copy the buffer back into the model-texture.
    mTexture->getBuffer()->blitFromMemory(
        pb, Box(picPart.dstX,
                picPart.dstY,
                picPart.dstX + picPart.w,
                picPart.dstY + picPart.h));

    delete[] buf;
#ifdef WRITE_MODELTEXTURE_TO_FILE
    // Writes the just blitted model-texture as png to disk.
    {
        Image img;
        uint32 *sysFontBuf = new uint32[mTexture->getWidth()*mTexture->getHeight()];
        mTexture->getBuffer()->blitToMemory(PixelBox(mTexture->getWidth(), mTexture->getHeight(), 1, PF_A8R8G8B8, sysFontBuf));
        img = img.loadDynamicImage((unsigned char*)sysFontBuf, mTexture->getWidth(), mTexture->getHeight(), PF_A8R8G8B8);
        img.save("Texture_Changed.png");
    }
#endif

}

//================================================================================================
// Select a new texture.
//================================================================================================
void ObjectEquipment::setTexture(int pos, int textureColor, int textureNr)
{
    // Load the shadow texture.
    Image image;
    image.load("shadow.png", "General");
    switch (pos)
    {
        case TEXTURE_POS_SKIN:
        {
            drawBopyPart(picFace, image, textureColor, textureNr);
            for (int side = 0; side < 4; ++side) drawBopyPart(picArms[side], image, textureColor, textureNr);
            break;
        }

        case TEXTURE_POS_FACE:
        {
            drawBopyPart(picFace, image, textureColor, textureNr);
            break;
        }

        case TEXTURE_POS_HAIR:
        {
            drawBopyPart(picHair, image, textureColor, textureNr);
            break;
        }

        case TEXTURE_POS_BODY:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picBody[side], image, textureColor, textureNr);
            break;
        }

        case TEXTURE_POS_LEGS:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picLegs[side], image, textureColor, textureNr);
            break;
        }

        case TEXTURE_POS_BELT:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picBelt[side], image, textureColor, textureNr);
            break;
        }

        case TEXTURE_POS_SHOES:
        {
            for (int side = 0; side < 2; ++side) drawBopyPart(picShoes[side], image, textureColor, textureNr);
            break;
        }

        case TEXTURE_POS_HANDS:
        {
            for (int side = 0; side < 4; ++side) drawBopyPart(picHands[side], image, textureColor, textureNr);
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
        //Logger::log().error() << meshName[type][itemID];
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
