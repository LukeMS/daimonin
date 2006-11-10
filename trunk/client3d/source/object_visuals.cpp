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

#include <tinyxml.h>
#include "define.h"
#include "option.h"
#include "logger.h"
#include "events.h"
#include "object_visuals.h"
#include "particle_manager.h"
#include "object_manager.h"
#include "gui_textout.h"

//===================================================
// Init all static Elemnts.
//===================================================

const int TEXTURE_SIZE = 128;
const char MATERIAL_NAME[] = "NPC_Visuals";
const char TEXTURE_NAME[] = "TexVisuals";

//===================================================
// Free all recources.
//===================================================
void ObjectVisuals::freeRecources()
{
    mHardwarePB.setNull();
    delete[] mTexBuffer;
}

//===================================================
// .
//===================================================
ObjectVisuals::~ObjectVisuals()
{}

//===================================================
// .
//===================================================
ObjectVisuals::ObjectVisuals()
{}

//===================================================
// .
//===================================================
void ObjectVisuals::Init()
{
    Logger::log().headline("Creating Object Visuals");
    for (int i=0; i < NPC_SUM; ++i) mNode[i] = 0;
    // ////////////////////////////////////////////////////////////////////
    // Check for a working description file.
    // ////////////////////////////////////////////////////////////////////
    TiXmlElement *xmlRoot, *xmlElem, *xmlColor;
    TiXmlDocument doc(FILE_NPC_VISUALS);
    const char *strTemp;
    if (!doc.LoadFile() || !(xmlRoot = doc.RootElement()))
    {
        Logger::log().error() << "XML-File '" << FILE_NPC_VISUALS << "' is broken or missing.";
        return;
    }
    Logger::log().info() << "Parsing the ImageSet file '" << FILE_NPC_VISUALS << "'.";
    // ////////////////////////////////////////////////////////////////////
    // Parse the gfx coordinates.
    // ////////////////////////////////////////////////////////////////////
    mPSystem = 0;
    if ((xmlElem = xmlRoot->FirstChildElement("Particle")) && ((strTemp = xmlElem->Attribute("name"))))
    {
        int i=-1;
        float color[3] = {0.0, 0.0, 0.0};
        mPSystem = ParticleManager::getSingleton().addNodeObject(0, strTemp, -1);
        for (xmlColor = xmlElem->FirstChildElement("color"); xmlColor; xmlColor = xmlColor->NextSiblingElement("color"))
        {
            if (++i >= PARTICLE_COLOR_SUM)
            {
                Logger::log().error() << "XML-File '" << FILE_NPC_VISUALS << " Particle entries are broken.";
                break;
            }
            if ((strTemp = xmlColor->Attribute("red"  ))) color[0] = atof(strTemp);
            if ((strTemp = xmlColor->Attribute("green"))) color[1] = atof(strTemp);
            if ((strTemp = xmlColor->Attribute("blue" ))) color[2] = atof(strTemp);
            particleColor[i] = ColourValue(color[0], color[1], color[2], 1.0f);
        }
    }

    buildEntity(NPC_LIFEBAR, "MeshLifebar",  "EntityLifebar");
    if (!mPSystem) buildEntity(NPC_SELECTION, "MeshSelection", "EntitySelection");

    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_NAME);
    /*
        tmpMaterial->setDepthBias(true);
        tmpMaterial->setDepthCheckEnabled(false);
        tmpMaterial->setDepthWriteEnabled(false);
    */
    tmpMaterial->setDepthBias(false);
    tmpMaterial->setDepthCheckEnabled(true);
    tmpMaterial->setDepthWriteEnabled(true);

    mTexBuffer = new uchar[TEXTURE_SIZE * TEXTURE_SIZE * sizeof(uint32)];
    Image image;
    image.loadDynamicImage(mTexBuffer, TEXTURE_SIZE, TEXTURE_SIZE, PF_A8B8G8R8);
    TexturePtr pTexture = TextureManager::getSingleton().loadImage(TEXTURE_NAME, "General", image, TEX_TYPE_2D);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_NAME);
    mHardwarePB = pTexture->getBuffer();
    mNode[NPC_SELECTION] = 0;
    mNode[NPC_LIFEBAR] = 0;
    /*
        mImage.load("NPC_Visuals.png", "General");
        mSrcPixelBox = mImage.getPixelBox();
    */
}

//===================================================
// Create selection entity.
//===================================================
void ObjectVisuals::buildEntity(int index, const char *meshName, const char *entityName)
{
    Real h = 20.0, w = 10.0;
    String strMob = "Mob"+ StringConverter::toString(index, 3, '0');
    ManualObject* mob = static_cast<ManualObject*>(Event->GetSceneManager()->createMovableObject(strMob, ManualObjectFactory::FACTORY_TYPE_NAME));

    mob->begin(MATERIAL_NAME);

    mob->position(-w, h, 0.0);
    mob->normal(0,0,1);
    mob->textureCoord(0.0, 0.0);

    mob->position(-w, -h, 0.0);
    mob->normal(0,0,1);
    mob->textureCoord(0.0, 1.0);

    mob->position( w,  h, 0.0);
    mob->normal(0,0,1);
    mob->textureCoord(1.0, 0.0);

    mob->position( w, -h, 0.0);
    mob->normal(0,0,1);
    mob->textureCoord(1.0, 1.0);

    mob->triangle(0, 1, 2);
    mob->triangle(3, 2, 1);
    mob->end();
    mob->convertToMesh(meshName);
    mEntity[index]=Event->GetSceneManager()->createEntity(entityName, meshName);
    mEntity[index]->setQueryFlags(ObjectManager::QUERY_NPC_SELECT_MASK);
}

//===================================================
// .
//===================================================
void ObjectVisuals::setPosLifebar(Vector3 pos)
{}

//===================================================
// Draw the Lifebar for a NPC.
//===================================================
void ObjectVisuals::setLifebar(Real percent, int barWidth)
{
    if (percent <=0.0)
    {
        unselect();
        return;
    }
    if (barWidth > TEXTURE_SIZE) barWidth = TEXTURE_SIZE;
    uint32 color, dColor;
    if (percent > 0.5)
    {   // (green bar)
        color = 0xff005f00;
        dColor= 0x00001600;
    }
    else if (percent > 0.3)
    {   // (yellow bar)
        color = 0xff5f5f00;
        dColor= 0x00161600;
    }
    else
    {   // (red bar)
        color = 0xff5f0000;
        dColor= 0x00160000;
    }

    int x1 = (TEXTURE_SIZE - barWidth)/2;
    int xfill = barWidth - (int)(percent * barWidth);
    PixelBox pb = mHardwarePB->lock (Box(x1, TEXTURE_SIZE-11, x1 + barWidth, TEXTURE_SIZE-1), HardwareBuffer::HBL_DISCARD);
    uint32 * dest_data = (uint32*)pb.data;
    for (int x = 0; x < barWidth; ++x) dest_data[x] = 0xff000000;
    dest_data+= TEXTURE_SIZE;
    for (int y = 0; y < 9; ++y)
    {
        for (int x = 0; x < barWidth; ++x)
        {
            if (x <= xfill) dest_data[x] = 0xff000000;
            else            dest_data[x] = color;
        }
        if (y < 4) color+= dColor;
        else       color-= dColor;
        dest_data+= TEXTURE_SIZE;
    }
    for (int x = 0; x < barWidth; ++x) dest_data[x] = 0xff000000;
    mHardwarePB->unlock();
}

//===================================================
// Select a NPC.
//===================================================
void ObjectVisuals::selectNPC(ObjectNPC *npc, bool showLifebar)
{
    // ////////////////////////////////////////////////////////////////////
    // Selection ring.
    // ////////////////////////////////////////////////////////////////////
    if (mNode[NPC_SELECTION]) mNode[NPC_SELECTION]->getParentSceneNode()->removeAndDestroyChild(mNode[NPC_SELECTION]->getName());
	mNode[NPC_SELECTION] =npc->getSceneNode()->createChildSceneNode();
    mNode[NPC_SELECTION]->attachObject(mPSystem);
    int index;
    if      (npc->getFriendly() >0) index = PARTICLE_COLOR_FRIEND_STRT;
    else if (npc->getFriendly() <0) index = PARTICLE_COLOR_ENEMY_STRT;
    else                            index = PARTICLE_COLOR_NEUTRAL_STRT;
    mPSystem->setVisible(true);
    mPSystem->clear();
    ParticleManager::getSingleton().setColorRange(mPSystem, particleColor[index], particleColor[index+1]);
	const AxisAlignedBox &AABB = npc->getEntity()->getBoundingBox();
    float sizeX = (AABB.getMaximum().x -AABB.getMinimum().x) * 1.5;
    float sizeZ = (AABB.getMaximum().z -AABB.getMinimum().z) * 1.5;
	ParticleManager::getSingleton().setEmitterSize(mPSystem, sizeZ, sizeX, true);
    // ////////////////////////////////////////////////////////////////////
    // Lifebar.
    // ////////////////////////////////////////////////////////////////////
    if (!showLifebar) return;
    if (mNode[NPC_LIFEBAR]) mNode[NPC_LIFEBAR]->getParentSceneNode()->removeAndDestroyChild(mNode[NPC_LIFEBAR]->getName());
    mNode[NPC_LIFEBAR] = mNode[NPC_SELECTION]->getParentSceneNode()->createChildSceneNode();
    mNode[NPC_LIFEBAR]->attachObject(mEntity[NPC_LIFEBAR]);
    Vector3 pos = mNode[NPC_LIFEBAR]->getPosition();
    mNode[NPC_LIFEBAR]->setPosition((AABB.getMinimum().x-AABB.getMinimum().x)/2, AABB.getMaximum().y +20, pos.z);
    mNode[NPC_LIFEBAR]->setInheritOrientation(false);
    const int FONT_NR = 3;
    const char *name = npc->getNickName().c_str();
    int len = GuiTextout::getSingleton().CalcTextWidth(name, FONT_NR);
    if (len >TEXTURE_SIZE) len = TEXTURE_SIZE;
    len = (TEXTURE_SIZE - len) /2;
    PixelBox pb = mHardwarePB->lock (Box(0, 0, TEXTURE_SIZE, TEXTURE_SIZE), HardwareBuffer::HBL_DISCARD);
    // Clear the whole texture.
    uint32 *dest_data = (uint32*)pb.data;
    for (int i=0; i < TEXTURE_SIZE*TEXTURE_SIZE; ++i) *dest_data++ = 0;
    // Print NPC name.
    dest_data = (uint32*)pb.data + (TEXTURE_SIZE-1-28) * TEXTURE_SIZE + len;
    //GuiTextout::getSingleton().PrintToBuffer(TEXTURE_SIZE, 16, dest_data, name, FONT_NR,  0x00000000);
    mHardwarePB->unlock();
    setLifebar(npc->getHealthPercentage());
}

//===================================================
// Select an object.
//===================================================
void ObjectVisuals::selectStatic(ObjectStatic *obj, bool showLifebar)
{
    // ////////////////////////////////////////////////////////////////////
    // Selection ring.
    // ////////////////////////////////////////////////////////////////////
    if (mNode[NPC_SELECTION]) mNode[NPC_SELECTION]->getParentSceneNode()->removeAndDestroyChild(mNode[NPC_SELECTION]->getName());
	mNode[NPC_SELECTION] =obj->getSceneNode()->createChildSceneNode();
    mNode[NPC_SELECTION]->attachObject(mPSystem);
    int index = PARTICLE_COLOR_NEUTRAL_STRT;
    mPSystem->setVisible(true);
    mPSystem->clear();
    ParticleManager::getSingleton().setColorRange(mPSystem, particleColor[index], particleColor[index+1]);
	const AxisAlignedBox &AABB = obj->getEntity()->getBoundingBox();
    float sizeX = (AABB.getMaximum().x -AABB.getMinimum().x) * 1.5;
    float sizeZ = (AABB.getMaximum().z -AABB.getMinimum().z) * 1.5;
	ParticleManager::getSingleton().setEmitterSize(mPSystem, sizeZ, sizeX, true);
}

//===================================================
// Unselect.
//===================================================
void ObjectVisuals::unselect()
{
    if (mNode[NPC_SELECTION]) mNode[NPC_SELECTION]->getParentSceneNode()->removeAndDestroyChild(mNode[NPC_SELECTION]->getName());
    if (mNode[NPC_LIFEBAR])   mNode[NPC_LIFEBAR]->getParentSceneNode()->removeAndDestroyChild(mNode[NPC_LIFEBAR]->getName());
    if (mPSystem)             mPSystem->setVisible(false);
    mNode[NPC_SELECTION] = 0;
    mNode[NPC_LIFEBAR] = 0;
}
