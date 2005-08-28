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

#ifndef SKELETON_H
#define SKELETON_H

#include <vector>

using namespace std;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Singleton class.
////////////////////////////////////////////////////////////
class Skeleton
{
public:
  ////////////////////////////////////////////////////////////
  // Functions.
  ////////////////////////////////////////////////////////////
  static Skeleton &getSingleton()
  {
    static Skeleton Singleton; return Singleton;
  }
  bool Init();
  void save(const char *filename);
  void load(const char *filename);
  void scale(double scale);
  void selAnimation(int animNr);
  void changeAnimLength();
  void updateAnimLength();
  void renameAnim();
  void checkAnimNames();
  void clearAnimations();
  void fillAnimSelectCombo();
  bool parseAnimName(const string &name);

private:
  ////////////////////////////////////////////////////////////
  // Variables.
  ////////////////////////////////////////////////////////////
  char mBuffer[80];
  bool mActive;
  int mAnimNr;
  bool mAnimNamesOK;

  ////////////////////////////////////////////////////////////
  // Functions.
  ////////////////////////////////////////////////////////////
  Skeleton()
  {
    mActive = false;
  }
  ~Skeleton()
  {
    clearAnimations();
  }
  Skeleton(const Skeleton&); // disable copy-constructor.
};

#endif
