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
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <fstream>

#include "define.h"
#include "logfile.h"
#include "mesh.h"
#include "material.h"
#include "mainwindow.h"

struct sMaterial
{
  string name;
  bool nameOK;
  int lineNr;
};
vector<sMaterial*>vecMatNames;
vector<string>vecValidMatNames;
string mMesh;

// ========================================================================
// Rename a Material.
// ========================================================================
void Material::renameMaterial()
{
  string line, newName;
  unsigned int start;
  if (gpMainWin->selRenameMaterial->value() > gpMainWin->selRenameMaterial->size())
  {
    return;
  }
  if (gpMainWin->selRenameMaterial->size() == 0)
  {
    return;
  }
  //    if (!fl_ask("Do you want to write the new Material name?")) { return; }
  int nr = vecMatNames[gpMainWin->selMaterialName->value()]->lineNr;
  vecMatNames[nr]->name = mMesh + vecValidMatNames[gpMainWin->selRenameMaterial->value()];
  vecMatNames[nr]->nameOK = true;
  line = Mesh::getSingleton().getXML_Line(nr);
  start = line.find("\"");
  start = line.find( "\"", ++start);
  line = line.substr(++start, line.size());
  newName = gpMainWin->selRenameMaterial->text();
  start = newName.find(".");
  if (start != string::npos)
  {
    newName.replace(start, 1, "/");
  }
  line =  "\t<submesh material=\"" + newName + "\"" + line;
  //  LogFile::getSingleton().Info("es: '%s'" , line.c_str());
  Mesh::getSingleton().setXML_Line(nr, line);
  checkNames();
  fillMatSelectCombo();
  selMaterial(nr);
  //    Mesh::getSingleton().save(gpMainWin->MeshName->value());
}

// ========================================================================
// .
// ========================================================================
void Material::update()
{
  //gpMainWin->selRenameMaterial->add(line.c_str());
  checkNames();
  fillMatSelectCombo();
  fillValidCombo();
  selMaterial(0);
}

// ========================================================================
// .
// ========================================================================
void Material::selMaterial(int nr)
{
  if (gpMainWin->selMaterialName->size() == 0)
  {
    return;
  }
  gpMainWin->selMaterialName->value(nr);
  if (vecMatNames[nr]->nameOK)
  {
    gpMainWin->butRenameMaterial->deactivate();
    gpMainWin->selRenameMaterial->deactivate();
  }
  else
  {
    gpMainWin->butRenameMaterial->activate();
    gpMainWin->selRenameMaterial->activate();
  }
}

// ========================================================================
// Check if material-name is daimonin conform.
// ========================================================================
bool Material::parseMaterialName(const string &name)
{
  for (unsigned int i = 0; i < vecMatNames.size(); ++i)
  {
    if (name == vecValidMatNames[i]) return true;
  }
  return false;
}

// ========================================================================
// .
// ========================================================================
void Material::checkNames()
{
  mNamesOK = true;
  for (unsigned int i = 0; i < vecMatNames.size(); ++i)
  {
    if (vecMatNames[i]->nameOK == false)
    {
      vecMatNames[i]->name+= "  (ERROR: Not daimonin conform - MUST be renamed!)";
      mNamesOK = false;
    }
  }
  if (!mNamesOK)
  {
    gpMainWin->txtMaterialNameStatus->value("Error: Found not daimonin conform Material-Name(s).");
    gpMainWin->txtMaterialNameStatus->textcolor(FL_RED);
  }
  else
  {
    gpMainWin->txtMaterialNameStatus->value("All Material-Names are daimonin conform.");
    gpMainWin->txtMaterialNameStatus->textcolor(FL_BLACK);
  }
}

// ========================================================================
// Fill the Material-Selection-ComboBox.
// ========================================================================
void Material::fillMatSelectCombo()
{
  string name;
  unsigned int start;
  gpMainWin->selMaterialName->clear();
  for (unsigned int i = 0; i < vecMatNames.size(); ++i)
  {
    name = vecMatNames[i]->name;
    start = name.find("/");
    if (start != string::npos)
    {
      name.replace(start, 1, ".");
    }
    gpMainWin->selMaterialName->add(name.c_str());
  }
  gpMainWin->selMaterialName->activate();
}

// ========================================================================
// Clear all materials
// ========================================================================
void Material::clearMaterials()
{
  gpMainWin->selMaterialName->clear();
  for (unsigned int i = 0; i < vecMatNames.size(); i++)
  {
    delete vecMatNames[i];
  }
  vecMatNames.clear();
  gpMainWin->selMaterialName->redraw();
}



// ========================================================================
// Fill the Valid_Names-ComboBox.
// ========================================================================
void Material::fillValidCombo()
{
  string name;
  mMesh = Mesh::getSingleton().getSkeletonName();
  mMesh = mMesh.substr(0, mMesh.find(".")) + ".";
  gpMainWin->selRenameMaterial->clear();
  for (unsigned int i = 0; i < vecValidMatNames.size(); ++i)
  {
    name = mMesh + vecValidMatNames[i];
    gpMainWin->selRenameMaterial->add(name.c_str());
  }
  gpMainWin->selRenameMaterial->activate();
}

// ========================================================================
//
// ========================================================================
void Material::addMaterial(unsigned int lineNr, const string &MaterialName)
{
  sMaterial *mat = new sMaterial;
  mat->lineNr = lineNr;
  mat->name   = MaterialName;
  vecMatNames.push_back(mat);
}

// ========================================================================
// Init.
// ========================================================================
bool Material::Init()
{
  LogFile::getSingleton().Info("Open Material Names: '%s'..." , FILE_MATERIAL_NAMES);
  ifstream inMaterialNames(FILE_MATERIAL_NAMES);
  if (!inMaterialNames)
  {
    LogFile::getSingleton().Success(false);
  }
  LogFile::getSingleton().Success(true);
  string line;
  while(getline(inMaterialNames, line))
  {
    vecValidMatNames.push_back(line);
  }
  gpMainWin->butRenameMaterial->deactivate();
  gpMainWin->selRenameMaterial->deactivate();
  return true;
}
