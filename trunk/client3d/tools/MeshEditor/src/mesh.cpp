/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include <FL/Fl.H>
#include <fstream>
#include <vector>
#include "define.h"
#include "logfile.h"
#include "mesh.h"
#include "skeleton.h"
#include "material.h"
#include "mainwindow.h"

struct sGender
{
    char id[4];
    char text[8];
};

struct sGender Gender[] =
{
    { "_M_" , "Male"},
    { "_F_" , "Female"},
    { "_N_" , "Neutrum"},
};

static vector<string>vecMeshXML;
static vector<string>vecSpecNames;
static vector<string>vecProfNames;    
static string skeltonFile;

// ========================================================================
// 
// ========================================================================
const char *Mesh::getXML_Line(int lineNr)
{
    return vecMeshXML[lineNr].c_str();
}

// ========================================================================
// 
// ========================================================================
void Mesh::setXML_Line(int lineNr, const string &line)
{
    vecMeshXML[lineNr] = line;
}

// ========================================================================
// 
// ========================================================================
const char *Mesh::getSkeletonName()
{
    return skeltonFile.c_str();
}

// ========================================================================
// 
// ========================================================================
void Mesh::renameMesh()
{
    string MeshName, SkelName, line;
    MeshName  = gpMainWin->selMeshSpecies->text();
    MeshName += Gender[gpMainWin->selMeshGender->value()].id;
    MeshName += gpMainWin->selMeshProfession->text();
    SkelName  = MeshName;
    MeshName += ".mesh.xml";
    SkelName += ".skeleton";
    gpMainWin->MeshName    ->value(MeshName.c_str());
    gpMainWin->SkeletonName->value(SkelName.c_str());
    line  = "    <skeletonlink name=\"";
    line += SkelName;
    line += "\" />";
    vecMeshXML[mSkelFilenameLine] = line;
    save(MeshName.c_str());
    line = SkelName + ".xml";
    Skeleton::getSingleton().save(line.c_str());
    gpMainWin->txtMeshNameStatus->value("Meshname is daimonin conform.");
    gpMainWin->txtMeshNameStatus->textcolor(FL_BLACK);
}

// ========================================================================
// 
// ========================================================================
void Mesh::selectSpecies(int pos)
{
    gpMainWin->selMeshProfession->clear();
    ifstream file(FILE_MESH_NAMES);
    if (!file) { return; }
    string line;
    while(getline(file, line))
    { 
        if (line.find("-") == string::npos) { --pos; }
        if (pos < 0) { break; }
    }
    while(getline(file, line))
    { 
        if (line.find("-") == string::npos) { break; }
        line = line.substr(1, line.size()-1);
        vecProfNames.push_back(line);
        gpMainWin->selMeshProfession->add(line.c_str());
    }
    gpMainWin->selMeshProfession->value(0);    
}

// ========================================================================
// 
// ========================================================================
bool Mesh::Init()
{
    string line;
   	///////////////////////////////////////////////////////////////////////// 
	// Import Gender Names:
	/////////////////////////////////////////////////////////////////////////
    gpMainWin->selMeshGender->add(Gender[0].text);
    gpMainWin->selMeshGender->add(Gender[1].text);
    gpMainWin->selMeshGender->add(Gender[2].text);
    gpMainWin->selMeshGender->value(0);

   	///////////////////////////////////////////////////////////////////////// 
	// Import Species Names:
	/////////////////////////////////////////////////////////////////////////
    LogFile::getSingleton().Info("Open Mesh Names: '%s'..." , FILE_MESH_NAMES);
    ifstream inSpeciesNames(FILE_MESH_NAMES);
    if (!inSpeciesNames)
    {
        LogFile::getSingleton().Success(false);
        return false;
    }
    LogFile::getSingleton().Success(true);    
    while(getline(inSpeciesNames, line))
    { 
        if (line.find("-") == string::npos)
        {
            vecSpecNames.push_back(line);
            gpMainWin->selMeshSpecies->add(line.c_str());
        }
    }
    gpMainWin->selMeshSpecies->value(0);
    selectSpecies(0);
    gpMainWin->selMeshSpecies->deactivate();
    gpMainWin->selMeshGender->deactivate();
    gpMainWin->selMeshProfession->deactivate();
    gpMainWin->butRenameMesh->deactivate();
    return true;
}

// ========================================================================
// 
// ========================================================================
void Mesh::save(const char *filename)
{
    ofstream outSkel(filename);
    for(unsigned int i = 0; i < vecMeshXML.size(); ++i) { outSkel << vecMeshXML[i] << endl; }
    string file = FILE_OGRE_XML_CONV;
    file += filename;
    LogFile::getSingleton().Info("Executing: '%s'\n" , file.c_str());    
    system(file.c_str());
}

// ========================================================================
// 
// ========================================================================
void Mesh::load(const char *filename)
{
    string MeshName, MeshFilename, MeshSkelFilename, line;
    MeshFilename = filename;
    MeshFilename = MeshFilename.substr(MeshFilename.rfind("/")+1, MeshFilename.size());
    MeshName = MeshFilename.substr(0, MeshFilename.find(".mesh.xml"));
    vecMeshXML.clear();

    // Read in MeshFile.
    ifstream inMesh(MeshFilename.c_str());
    if (!inMesh)
    {
        LogFile::getSingleton().Error("Critical: Could not open mesh-file '%s'\n" , MeshFilename.c_str());
        return;
    }
    LogFile::getSingleton().Info("Opend Meshfile: '%s'\n" , MeshFilename.c_str());

    int start, lineNr=-1;
    while(getline(inMesh, line))
    {
        ++lineNr;
        if (line.find("skeletonlink name") != string::npos)
        { 
            mSkelFilenameLine = lineNr;
            start = line.find("\"");
            skeltonFile = line.substr(++start, line.find("\"",start)-start) + ".xml";
        }
        else if (line.find("submesh material") != string::npos)
        { 
            start = line.find("\"");
            Material::getSingleton().addMaterial(lineNr, line.substr(++start, line.find("\"",start)-start));
        } 
        vecMeshXML.push_back(line);
    }
    gpMainWin->MeshName->value(MeshFilename.c_str());
    checkMeshName(MeshFilename);
    Material::getSingleton().update();
}

// ========================================================================
// 
// ========================================================================
void Mesh::checkMeshName(string &MeshFilename)
{
    int stop, start =0;
    stop = MeshFilename.find("_");
    string MeshNameSpec = MeshFilename.substr(start , stop);
    start = ++stop;
    stop = MeshFilename.find("_", start);
    string MeshNameGend = MeshFilename.substr(start , stop-start);
    start = ++stop;
    stop = MeshFilename.find(".", start);    
    string MeshNameProf = MeshFilename.substr(start, stop-start);

    mMeshNameOK = false;
    for (unsigned int i = 0; i < vecSpecNames.size(); ++i)
    {
        if (vecSpecNames[i] == MeshNameSpec)
        { 
            // Species is daimonin conform. Now check Profession.
            for (unsigned int j = 0; j < vecProfNames.size(); ++j)
            {
                if (vecProfNames[j] == MeshNameProf)
                { 
                    if (MeshNameGend == "M" || MeshNameGend == "F" || MeshNameGend == "N" )
                    { 
                        mMeshNameOK = true;
                        gpMainWin->txtMeshNameStatus->value("Meshname is daimonin conform.");
                        gpMainWin->txtMeshNameStatus->textcolor(FL_BLACK);
                        gpMainWin->selMeshSpecies   ->deactivate();
                        gpMainWin->selMeshGender    ->deactivate();
                        gpMainWin->selMeshProfession->deactivate();
                        gpMainWin->butRenameMesh    ->deactivate();
                        return;
                    }
                    break;
                }
            }
            break;
        }
    }
    // Non Conform Meshname.
    gpMainWin->txtMeshNameStatus->value("Error: Meshname is not daimonin conform. MUST be renamed"); 
    gpMainWin->txtMeshNameStatus->textcolor(FL_RED);        
    gpMainWin->selMeshSpecies   ->activate();
    gpMainWin->selMeshGender    ->activate();
    gpMainWin->selMeshProfession->activate();
    gpMainWin->butRenameMesh    ->activate();    
}
