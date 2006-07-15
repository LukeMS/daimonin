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
const char TEXTURE_NAME[] = "TexVisuals";

///===================================================
/// Free all recources.
///===================================================
void ObjectVisuals::freeRecources()
{
    mHardwarePB.setNull();
    delete[] mTexBuffer;
}

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
    buildEntity(NPC_LIFEBAR, "MeshLifebar",  "EntityLifebar");
    if (!mPSystem) buildEntity(NPC_SELECTION, "MeshSelection", "EntitySelection");

    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_NAME);
    mTexBuffer = new uchar[TEXTURE_SIZE * TEXTURE_SIZE * sizeof(uint32)];
    Image image;
    image.loadDynamicImage(mTexBuffer, TEXTURE_SIZE, TEXTURE_SIZE, PF_A8B8G8R8);
    TexturePtr pTexture = TextureManager::getSingleton().loadImage(TEXTURE_NAME, "General", image, TEX_TYPE_2D);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_NAME);
    mHardwarePB = pTexture->getBuffer();
    /*
        mImage.load("NPC_Visuals.png", "General");
        mSrcPixelBox = mImage.getPixelBox();
    */
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
/// Draw the Lifebar for a NPC.
///===================================================
void ObjectVisuals::setLifebar(Real percent, int barWidth)
{
    if (percent <0.0) percent =0.0;
    uint32 color, dColor;
    if (percent > 0.5)
    {   /// (green bar)
        color = 0xff005f00;
        dColor= 0x00001600;
    }
    else if (percent > 0.3)
    {   /// (yellow bar)
        color = 0xff5f5f00;
        dColor= 0x00161600;
    }
    else
    {   /// (red bar)
        color = 0xff5f0000;
        dColor= 0x00160000;
    }
    int x1 = (TEXTURE_SIZE - barWidth)/2;
    int x2 = x1 + barWidth;
    int xfill = x2 - (int)(percent * barWidth);

    PixelBox pb = mHardwarePB->lock(Box(0, 127-10, TEXTURE_SIZE, 127), HardwareBuffer::HBL_NORMAL);
    uint32 * dest_data = (uint32*)pb.data;
    for (int x = x1; x < x2; ++x) dest_data[x] = 0xff000000;
    dest_data+= TEXTURE_SIZE;
    for (int y = 0; y < 9; ++y)
    {
        for (int x = x1; x < x2; ++x)
        {
            if (x <= xfill) dest_data[x] = 0xff000000;
            else            dest_data[x] = color;
        }
        if (y < 4) color+= dColor;
        else       color-= dColor;
        dest_data+= TEXTURE_SIZE;
    }
    for (int x = x1; x < x2; ++x) dest_data[x] = 0xff000000;
    mHardwarePB->unlock();
}

///===================================================
/// Select a NPC.
///===================================================
void ObjectVisuals::selectNPC(MovableObject *mob, int friendly)
{
    const AxisAlignedBox &AABB = mob->getBoundingBox();
    Vector3 pos;
    String strNode = "NodeObjVisuals"+ StringConverter::toString(NPC_SELECTION, 3, '0');
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

    /// Lifebar.
    strNode = "NodeObjVisuals"+ StringConverter::toString(NPC_LIFEBAR, 3, '0');
    if (mNode[NPC_LIFEBAR]) mNode[NPC_LIFEBAR]->getParentSceneNode()->removeAndDestroyChild(strNode);
    mNode[NPC_LIFEBAR] = mob->getParentSceneNode()->createChildSceneNode(strNode);
    mNode[NPC_LIFEBAR]->attachObject(mEntity[NPC_LIFEBAR]);
    pos = mNode[NPC_LIFEBAR]->getPosition();
    mNode[NPC_LIFEBAR]->setPosition((AABB.getMinimum().x-AABB.getMinimum().x)/2, AABB.getMaximum().y +40, pos.z);
    mNode[NPC_LIFEBAR]->setInheritOrientation(false);

    const int fontNr = 4;
    const char *name = ObjectManager::getSingleton().getSelectedNPC()->getNickName().c_str();
    int len = GuiTextout::getSingleton().CalcTextWidth(name, fontNr);
    if (len >128) len = 128;
    len = (TEXTURE_SIZE - len) /2;

    PixelBox pb = mHardwarePB->lock(Box(0, 0, 127, 127), HardwareBuffer::HBL_NORMAL);
    /// Clear the whole texture.
    uint32 *dest_data = (uint32*)pb.data;
    for (int i=0; i < 128*128; ++i) *dest_data++ = 0x0;
    /// Print NPC name.
    dest_data = (uint32*)pb.data + (127-28) * TEXTURE_SIZE + len;
    GuiTextout::getSingleton().PrintToBuffer(TEXTURE_SIZE, 16, dest_data, name, fontNr,  0x00000000);
    mHardwarePB->unlock();
    setLifebar(1.0);
}
