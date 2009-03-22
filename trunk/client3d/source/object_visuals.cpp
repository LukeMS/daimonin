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

#include <tinyxml.h>
#include "define.h"
#include "option.h"
#include "logger.h"
#include "events.h"
#include "object_visuals.h"
#include "particle_manager.h"
#include "object_manager.h"
#include "gui_manager.h"

using namespace Ogre;

//===================================================
// Init all static Elemnts.
//===================================================

const int  TEXTURE_SIZE = 128;
const int  CHARNAME_FONT_NR = 3;
const char MATERIAL_NAME[] = "NPC_Visuals";
const char TEXTURE_NAME[]  = "TexVisuals";


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
void ObjectVisuals::Init()
{
    Logger::log().headline() << "Creating Object Visuals";
    for (int i=0; i < VISUAL_SUM; ++i) mNode[i] = 0;
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

    buildEntity(VISUAL_LIFEBAR, "MeshLifebar",  "EntityLifebar");
    if (!mPSystem) buildEntity(VISUAL_SELECTION, "MeshSelection", "EntitySelection");

    MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName(MATERIAL_NAME);
    /*
        tmpMaterial->setDepthBias(true);
        tmpMaterial->setDepthCheckEnabled(false);
        tmpMaterial->setDepthWriteEnabled(false);
    */
    //tmpMaterial->setDepthBias(false);
    tmpMaterial->setDepthBias(0.0f, 0.0f);
    tmpMaterial->setDepthCheckEnabled(true);
    tmpMaterial->setDepthWriteEnabled(true);

    mTexBuffer = new unsigned char[TEXTURE_SIZE * TEXTURE_SIZE * sizeof(uint32)];
    Image image;
    image.loadDynamicImage(mTexBuffer, TEXTURE_SIZE, TEXTURE_SIZE, PF_A8B8G8R8);
    TexturePtr pTexture = TextureManager::getSingleton().loadImage(TEXTURE_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, image, TEX_TYPE_2D);
    tmpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(TEXTURE_NAME);
    mHardwarePB = pTexture->getBuffer();
    mNode[VISUAL_SELECTION] = 0;
    mNode[VISUAL_LIFEBAR] = 0;
    /*
        mImage.load("VISUAL_Visuals.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
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
    ManualObject* mob = static_cast<ManualObject*>(Events::getSingleton().GetSceneManager()->createMovableObject(strMob, ManualObjectFactory::FACTORY_TYPE_NAME));

    mob->begin(MATERIAL_NAME);

    mob->position(-w, h, 0.0); mob->normal(0,0,1); mob->textureCoord(0.0, 0.0);
    mob->position(-w, -h, 0.0); mob->normal(0,0,1); mob->textureCoord(0.0, 1.0);
    mob->position( w,  h, 0.0); mob->normal(0,0,1); mob->textureCoord(1.0, 0.0);
    mob->position( w, -h, 0.0); mob->normal(0,0,1); mob->textureCoord(1.0, 1.0);
    mob->triangle(0, 1, 2);
    mob->triangle(3, 2, 1);
    mob->end();
    mob->convertToMesh(meshName);
    mEntity[index]=Events::getSingleton().GetSceneManager()->createEntity(entityName, meshName);
    mEntity[index]->setQueryFlags(ObjectManager::QUERY_NPC_SELECT_MASK);
}

//===================================================
// .
//===================================================
void ObjectVisuals::setPosLifebar(Vector3 pos)
{}

//===================================================
// Draw the Lifebar for a NPC.
// Will be replace by an window with all the stats.
//===================================================
void ObjectVisuals::setLifebar(Real percent, int barWidth)
{
    if (percent <=0.0) return;
    if (barWidth > TEXTURE_SIZE) barWidth = TEXTURE_SIZE;
    uint32 color, dColor;
    if (percent > 0.5)
    {   // green bar.
        color = 0xff005f00;
        dColor= 0x00001600;
    }
    else if (percent > 0.3)
    {   // yellow bar.
        color = 0xff5f5f00;
        dColor= 0x00161600;
    }
    else
    {   // red bar.
        color = 0xff5f0000;
        dColor= 0x00160000;
    }
    /*
        int len = GuiManager::getSingleton().calcTextWidth(mStrName.c_str(), CHARNAME_FONT_NR);
        if (len >TEXTURE_SIZE) len = TEXTURE_SIZE;
        len = (TEXTURE_SIZE - len) /2;
        // Print NPC name.
        dest_data = (uint32*)pb.data + (TEXTURE_SIZE-1-28) * TEXTURE_SIZE + len;
        if (!mStrName.empty())
            GuiManager::getSingleton().printText(TEXTURE_SIZE, TEXTURE_SIZE, dest_data, TEXTURE_SIZE, mStrName.c_str(), CHARNAME_FONT_NR, 0x00000000);
        int x1 = (TEXTURE_SIZE - barWidth)/2;
    */
    int xfill =  (int)(percent * barWidth);
    PixelBox pb = mHardwarePB->lock(Box(0, 0, TEXTURE_SIZE, TEXTURE_SIZE), HardwareBuffer::HBL_DISCARD);
    uint32 *dest_data = (uint32*)pb.data;

    // Clear the whole texture.
    for (int i=0; i < TEXTURE_SIZE*TEXTURE_SIZE; ++i) *dest_data++ = 0;

    dest_data = (uint32*)pb.data;
    for (int x = 0; x < barWidth; ++x) dest_data[x] = 0xff000000;
    dest_data+= TEXTURE_SIZE;
    for (int y = 0; y < 9; ++y)
    {
        for (int x = 0; x < barWidth; ++x)
        {
            dest_data[x] = (x > xfill)?0xff000000:color;
        }
        color+= (y < 4)?dColor:dColor*-1;
        dest_data+= TEXTURE_SIZE;
    }
    for (int x = 0; x < barWidth; ++x) dest_data[x] = 0xff000000;
    mHardwarePB->unlock();
}

//===================================================
// Select a NPC.
//===================================================
void ObjectVisuals::select(const AxisAlignedBox &AABB, SceneNode *node, int friendly, Real percent, const char *name)
{
    /*
        int index = PARTICLE_COLOR_NEUTRAL_STRT;
    */
    // ////////////////////////////////////////////////////////////////////
    // Selection ring.
    // ////////////////////////////////////////////////////////////////////
    if (mNode[VISUAL_SELECTION]) mNode[VISUAL_SELECTION]->getParentSceneNode()->removeAndDestroyChild(mNode[VISUAL_SELECTION]->getName());
    mNode[VISUAL_SELECTION] = node->createChildSceneNode();
    mNode[VISUAL_SELECTION]->attachObject(mPSystem);
    int index;
    if      (friendly >0) index = PARTICLE_COLOR_FRIEND_STRT;
    else if (friendly <0) index = PARTICLE_COLOR_ENEMY_STRT;
    else                  index = PARTICLE_COLOR_NEUTRAL_STRT;
    mPSystem->setVisible(true);
    mPSystem->clear();
    ParticleManager::getSingleton().setColorRange(mPSystem, particleColor[index], particleColor[index+1]);

    float sizeX = (AABB.getMaximum().x -AABB.getMinimum().x) * 1.8;
    float sizeZ = (AABB.getMaximum().z -AABB.getMinimum().z) * 1.8;
    if (sizeZ > sizeX) sizeX = sizeZ;
    ParticleManager::getSingleton().setEmitterSize(mPSystem, sizeX, sizeX, true);
    // ////////////////////////////////////////////////////////////////////
    // Lifebar.
    // ////////////////////////////////////////////////////////////////////
    if (percent >= 0)
    {
        if (mNode[VISUAL_LIFEBAR]) mNode[VISUAL_LIFEBAR]->getParentSceneNode()->removeAndDestroyChild(mNode[VISUAL_LIFEBAR]->getName());
        mNode[VISUAL_LIFEBAR] = mNode[VISUAL_SELECTION]->getParentSceneNode()->createChildSceneNode();
        mNode[VISUAL_LIFEBAR]->attachObject(mEntity[VISUAL_LIFEBAR]);
        Vector3 pos = mNode[VISUAL_LIFEBAR]->getPosition();
        mNode[VISUAL_LIFEBAR]->setPosition((AABB.getMinimum().x-AABB.getMinimum().x)/2, AABB.getMaximum().y +20, pos.z);
        mNode[VISUAL_LIFEBAR]->setInheritOrientation(false);
        mStrName = name;
        setLifebar(percent);
    }
    GuiManager::getSingleton().showWindow(GuiManager::WIN_PLAYERTARGET, true);
}

/*
void ObjectVisuals::selectPlayer()
{
    GuiManager::getSingleton().centerWindowOnMouse(GuiManager::WIN_PLAYERCONSOLE);
    GuiManager::getSingleton().showWindow(GuiManager::WIN_PLAYERCONSOLE, true);
}
*/

//===================================================
// Unselect.
//===================================================
void ObjectVisuals::unselect()
{
    if (mNode[VISUAL_SELECTION]) mNode[VISUAL_SELECTION]->getParentSceneNode()->removeAndDestroyChild(mNode[VISUAL_SELECTION]->getName());
    if (mNode[VISUAL_LIFEBAR])   mNode[VISUAL_LIFEBAR]->getParentSceneNode()->removeAndDestroyChild(mNode[VISUAL_LIFEBAR]->getName());
    if (mPSystem)             mPSystem->setVisible(false);
    mNode[VISUAL_SELECTION] = 0;
    mNode[VISUAL_LIFEBAR] = 0;
    GuiManager::getSingleton().showWindow(GuiManager::WIN_PLAYERTARGET, false);
}

//===================================================
// .
//===================================================
void ObjectVisuals::highlight(bool staticObject, int friendly, bool highlight)
{
    if (!highlight)
    {
        GuiManager::getSingleton().setMouseState(GuiManager::STATE_MOUSE_DEFAULT);
        return;
    }
    if (!staticObject)
    {
        int action;
        if      (friendly >0) action = GuiManager::STATE_MOUSE_TALK;
        else if (friendly <0) action = Events::getSingleton().isShiftDown()?GuiManager::STATE_MOUSE_LONG_RANGE_ATTACK:GuiManager::STATE_MOUSE_SHORT_RANGE_ATTACK;
        else                  action = GuiManager::STATE_MOUSE_DEFAULT;
        GuiManager::getSingleton().setMouseState(action);
    }
    else
    {
        GuiManager::getSingleton().setMouseState(GuiManager::STATE_MOUSE_OPEN);
    }
}
