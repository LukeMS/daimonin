/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
 
Copyright (c) 2004 The Daimonin Team
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

#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>

using namespace std;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Singleton class.
////////////////////////////////////////////////////////////
class Mesh
{
public:
  ////////////////////////////////////////////////////////////
  // Functions.
  ////////////////////////////////////////////////////////////
  static Mesh &getSingleton()
  {
    static Mesh Singleton; return Singleton;
  }
  bool Init();
  void load(const char *filename);
  void scale(double value);
  void renameMesh();
  void selectSpecies(int pos);
  const char *getSkeletonName();
  void save(const char *filename);
  const char *getXML_Line(int lineNr);
  void setXML_Line(int lineNr, const string &line);
private:
  ////////////////////////////////////////////////////////////
  // Variables.
  ////////////////////////////////////////////////////////////
  bool mMeshNameOK;
  int mSkelFilenameLine;

  ////////////////////////////////////////////////////////////
  // Functions.
  ////////////////////////////////////////////////////////////
  Mesh()
  {}
  ~Mesh()
  {}
  Mesh(const Mesh&); // disable copy-constructor.
  void checkMeshName(string &MeshFilename);
};

#endif
