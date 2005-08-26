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
 
You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "define.h"
#include "option.h"
#include "logger.h"

///=================================================================================================
/// Open a description file.
///=================================================================================================
bool Option::openDescFile(const char *filename)
{
  closeDescFile();
  mDescFile = new ifstream(filename, ios::in);
  if (!mDescFile)  return false;
  mFilename = filename;
  mDescBuffer ="";
  string buf;
  while (getline(*mDescFile, buf))
  { // skip comments.
    if (buf.find("#") > 5)  mDescBuffer+= buf;
  }
  return true;
}

///=================================================================================================
/// Close a description file.
///=================================================================================================
void Option::closeDescFile()
{
  if (!mDescFile) return;
  mDescFile->close();
  delete mDescFile;
  mDescFile = 0;
}

///=================================================================================================
/// Get the value of the nth (=posNr) incidence of a keyword.
/// If keyword is not found on posNr, return the first incidence.
///=================================================================================================
bool Option::getDescStr(const char *strKeyword, string &strBuffer, unsigned int posNr)
{
  unsigned int pos=0, startPos=0, stopPos, entryTest;
checkForKeyword:
  startPos = mDescBuffer.find(strKeyword, startPos);
  if (startPos == string::npos)
  {
    return false;
  }
  entryTest= mDescBuffer.find(":",  startPos)+1;
  startPos = mDescBuffer.find("\"", startPos)+1;
  // keyword and value can have the same name. If ':' comes before '"' in the description-text
  // we have a keyword, else we have the value and search again.
  if (entryTest > startPos)
  {
    goto checkForKeyword;
  }
  stopPos   = mDescBuffer.find("\"", startPos)-startPos;
  strBuffer = mDescBuffer.substr(startPos, stopPos);
  if (++pos < posNr)
  {
    goto checkForKeyword;
  }
  //if (posNr) LogFile::getSingleton().Error("string: %s\n", strBuffer.c_str());
  if (strBuffer.size() == 0)
  {
    return false;
  }
  return true;
}

///=================================================================================================
/// Create/Overwrite the Optionfile.
///=================================================================================================
bool Option::Init()
{
  Logger::log().headline("Init Options");
  mDescFile = 0;
  mMetaServer ="damn.informatik.uni-bremen.de";
  mMetaServerPort = 13326;
  mSelectedMetaServer =0;
  mStartNetwork = false;
  int i, sheets = 0;
  for (i=0; optStruct[i].name; ++i)
  {
    if (optStruct[i].name[0] == '#')
    {
      ++sheets;
      continue;
    }
    if (optStruct[i].value_type == VAL_TEXT)
    {
      *((std::string*)optStruct[i].value) = optStruct[i].val_text;
    }
    else
    {
      *((int*)optStruct[i].value) = optStruct[i].default_val;
    }
  }
  Logger::log().info() << "Parsing of " << i-sheets << " options on "<< sheets << " sheets was done.";
  return true;
}

///=================================================================================================
/// Constructor.
///=================================================================================================
Option::Option()
{
  GameStatus =  GAME_STATUS_INIT;
  mLogin = false;
  mDescFile =0;
}

///=================================================================================================
/// Destructor.
///=================================================================================================
Option::~Option()
{
  closeDescFile();
}
