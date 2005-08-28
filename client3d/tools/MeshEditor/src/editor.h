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

#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include "mainwindow.h"
using namespace std;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Singleton class.
////////////////////////////////////////////////////////////
class Editor
{
public:
  ////////////////////////////////////////////////////////////
  // Functions.
  ////////////////////////////////////////////////////////////
  static Editor &getSingleton()
  {
    static Editor Singleton; return Singleton;
  }
  void loadModel();
  void scaleModel();
  void updateModelScale() { mScale = gpMainWin->countScaleModel->value(); }

private:
  ////////////////////////////////////////////////////////////
  // Variables.
  ////////////////////////////////////////////////////////////
  double mScale;
  ////////////////////////////////////////////////////////////
  // Functions.
  ////////////////////////////////////////////////////////////
  Editor()
  {
    mScale =1.0;
  }
  ~Editor()
  {}
  Editor(const Editor&); // disable copy-constructor.
};

#endif
