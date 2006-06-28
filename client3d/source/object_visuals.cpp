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

///===================================================
/// Init all static Elemnts.
///===================================================

struct
{
    std::string name;
    Real x1, x2, z1, z2;
    Real sizeX, sizeY, sizeZ;
}
GfxEntry[ObjectVisuals::NPC_SUM];

const int TEXTURE_SIZE = 128;
const char MATERIAL_NAME[] = "NPC_Visuals";

///===================================================
/// Free all recources.
///===================================================
void ObjectVisuals::freeRecources()
{}

///===================================================
/// .
///===================================================
ObjectVisuals::~ObjectVisuals()
{}

///===================================================
/// .
///===================================================
ObjectVisuals::ObjectVisuals()
{
    Logger::log().headline("Creating Object Visuals.");
    for (int i=0; i < NPC_SUM; ++i) mNode[i] = 0;
    /// ////////////////////////////////////////////////////////////////////
    /// Check for a working description file.
    /// ////////////////////////////////////////////////////////////////////
    TiXmlElement *xmlRoot, *xmlElem, *xmlColor;
    TiXmlDocument doc(FILE_NPC_VISUALS);
    const char *strTemp;
    if (!doc.LoadFile() || !(xmlRoot = doc.RootElement()))
    {
        Logger::log().error() << "XML-File '" << FILE_NPC_VISUALS << "' is broken or missing.";
        return;
    }
    Logger::log().info() << "Parsing the ImageSet file '" << FILE_NPC_VISUALS << "'.";
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the gfx coordinates.
    /// ////////////////////////////////////////////////////////////////////
    int index =-1;
    float color[3];
    mPSystem = 0;
    if ((xmlElem = xmlRoot->FirstChildElement("Particle")) && ((strTemp = xmlElem->Attribute("name"))))
    {
        int i=-1;
        mPSystem = ParticleManager::getSingleton().addNodeObject(Vector3(0,0,0), 0, strTemp, -1);
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
    for (xmlElem = xmlRoot->FirstChildElement("Image"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Image"))
    {
        if (!(strTemp = xmlElem->Attribute("name"))) continue;
        if (++index >= NPC_SUM) break;
        GfxEntry[index].name = strTemp;
        if ((strTemp = xmlElem->Attribute("posX"     ))) GfxEntry[index].x1    = atof(strTemp)/ TEXTURE_SIZE;
        if ((strTemp = xmlElem->Attribute("posY"     ))) GfxEntry[index].z1    = atof(strTemp)/ TEXTURE_SIZE;
        if ((strTemp = xmlElem->Attribute("width"    ))) GfxEntry[index].x2    = atof(strTemp)/ TEXTURE_SIZE + GfxEntry[index].x1;
        if ((strTemp = xmlElem->Attribute("height"   ))) GfxEntry[index].z2    = atof(strTemp)/ TEXTURE_SIZE + GfxEntry[index].z1;
        if ((strTemp = xmlElem->Attribute("meshSizeX"))) GfxEntry[index].sizeX  = atof(strTemp);
        if ((strTemp = xmlElem->Attribute("meshSizeY"))) GfxEntry[index].sizeY  = atof(strTemp);
        if ((strTemp = xmlElem->Attribute("meshSizeZ"))) GfxEntry[index].sizeZ  = atof(strTemp);
    }
    //    buildEntity(NPC_LIFEBAR_L, "MeshLifebarL",  "EntityLifebarL");
    //    buildEntity(NPC_LIFEBAR_R, "MeshLifebarR",  "EntityLifebarR");
    if (!mPSystem) buildEntity(NPC_SELECTION, "MeshSelection", "EntitySelection");
}

///===================================================
/// Create selection entity.
///===================================================
void ObjectVisuals::buildEntity(int index, const char *meshName, const char *entityName)
{
    String strMob = "Mob"+ StringConverter::toString(index, 3, '0');
    ManualObject* mob = static_cast<ManualObject*>(Event->GetSceneManager()->createMovableObject(strMob, ManualObjectFactory::FACTORY_TYPE_NAME));
    mob->begin(MATERIAL_NAME);

    mob->position(-GfxEntry[index].sizeX, 0.0, -GfxEntry[index].sizeZ);
    mob->normal(0,0,1);
    mob->textureCoord(GfxEntry[index].x1, GfxEntry[index].z1);

    mob->position(-GfxEntry[index].sizeX, -GfxEntry[index].sizeY, GfxEntry[index].sizeZ);
    mob->normal(0,0,1);
    mob->textureCoord(GfxEntry[index].x1, GfxEntry[index].z2);

    mob->position(GfxEntry[index].sizeX, 0.0, -GfxEntry[index].sizeZ);
    mob->normal(0,0,1);
    mob->textureCoord(GfxEntry[index].x2, GfxEntry[index].z1);

    mob->position(GfxEntry[index].sizeX, -GfxEntry[index].sizeY, GfxEntry[index].sizeZ);
    mob->normal(0,0,1);
    mob->textureCoord(GfxEntry[index].x2, GfxEntry[index].z2);

    mob->triangle(0, 1, 2);
    mob->triangle(3, 2, 1);
    mob->end();
    mob->convertToMesh(meshName);
    mEntity[index]=Event->GetSceneManager()->createEntity(entityName, meshName);
    mEntity[index]->setQueryFlags(QUERY_NPC_SELECT_MASK);
}


///===================================================
/// .
///===================================================
void ObjectVisuals::setPosLifebar(Vector3 pos)
{}

///===================================================
/// .
///===================================================
void ObjectVisuals::setLengthLifebar(int maxLength, int currentLength)
{
    if (!maxLength) return; // prevent division by zero.
    //    Real filling = (mWidthLifebarGFX * currentLength) / maxLength;
    // subMesh3->setScale(filling, 1.0f, 1.0f);
}

///===================================================
/// Select a NPC.
///===================================================
void ObjectVisuals::selectNPC(MovableObject *mob, int friendly)
{
    const AxisAlignedBox &AABB = mob->getBoundingBox();
    Vector3 pos;
    String strNode = "NodeObjVisuals"+ StringConverter::toString(NPC_SELECTION, 3, '0');
    if (mPSystem)
    {
        if (mNode[NPC_SELECTION]) mNode[NPC_SELECTION]->getParentSceneNode()->removeAndDestroyChild(strNode);
        mNode[NPC_SELECTION] = mob->getParentSceneNode()->createChildSceneNode(strNode);
        mNode[NPC_SELECTION]->attachObject(mPSystem);
        int index;
             if (friendly >0) index = PARTICLE_COLOR_FRIEND_STRT;
        else if (friendly <0) index = PARTICLE_COLOR_ENEMY_STRT;
        else                  index = PARTICLE_COLOR_NEUTRAL_STRT;
        for (int i=0; i < mPSystem->getNumEmitters(); ++i)
        {
            mPSystem->clear();
            mPSystem->getEmitter(i)->setColourRangeStart(particleColor[index]);
            mPSystem->getEmitter(i)->setColourRangeEnd  (particleColor[index+1]);
        }
    }
    else
    {
        if (mNode[NPC_SELECTION]) mNode[NPC_SELECTION]->getParentSceneNode()->removeAndDestroyChild(strNode);
        mNode[NPC_SELECTION] = mob->getParentSceneNode()->createChildSceneNode(strNode);
        mNode[NPC_SELECTION]->attachObject(mEntity[NPC_SELECTION]);
        pos = mNode[NPC_SELECTION]->getPosition();
        mNode[NPC_SELECTION]->setPosition(pos.x, AABB.getMinimum().y +3, pos.z);

        MaterialPtr mMaterial = MaterialManager::getSingleton().getByName(mEntity[NPC_SELECTION]->getMesh()->getSubMesh(0)->getMaterialName());
        TextureUnitState *tu = mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0);
        if (friendly >0)
        {
            tu->setTextureUScroll(0);
            tu->setTextureVScroll(0);
        }
        else if (friendly <0)
        {
            tu->setTextureUScroll(0);
            tu->setTextureVScroll(.46875);
        }
        else
        {
            tu->setTextureUScroll(.46875);
            tu->setTextureVScroll(.46875);
        }
    }
    /*
        // Lifebar left bracket.
        strNode = "NodeObjVisuals"+ StringConverter::toString(NPC_LIFEBAR_L, 3, '0');
        if (mNode[NPC_LIFEBAR_L]) mNode[NPC_LIFEBAR_L]->getParentSceneNode()->removeAndDestroyChild(strNode);
        mNode[NPC_LIFEBAR_L] = mob->getParentSceneNode()->createChildSceneNode(strNode);
        mNode[NPC_LIFEBAR_L]->attachObject(mEntity[NPC_LIFEBAR_L]);
        pos = mNode[NPC_LIFEBAR_L]->getPosition();
        mNode[NPC_LIFEBAR_L]->setPosition(AABB.getMinimum().x, AABB.getMaximum().y +5, pos.z);

        // Lifebar right bracket.
        strNode = "NodeObjVisuals"+ StringConverter::toString(NPC_LIFEBAR_R, 3, '0');
        if (mNode[NPC_LIFEBAR_R]) mNode[NPC_LIFEBAR_R]->getParentSceneNode()->removeAndDestroyChild(strNode);
        mNode[NPC_LIFEBAR_R] = mob->getParentSceneNode()->createChildSceneNode(strNode);
        mNode[NPC_LIFEBAR_R]->attachObject(mEntity[NPC_LIFEBAR_R]);
        pos = mNode[NPC_LIFEBAR_R]->getPosition();
        mNode[NPC_LIFEBAR_R]->setPosition(AABB.getMaximum().x, AABB.getMaximum().y +5, pos.z);
    */
}

///===================================================
/// Always face the camera.
///===================================================
void ObjectVisuals::updateSelection(Real facing)
{}
