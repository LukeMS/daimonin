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

#include "define.h"
#include "option.h"
#include "logger.h"

//================================================================================================
// Open a description file.
//================================================================================================
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

//================================================================================================
// Close a description file.
//================================================================================================
void Option::closeDescFile()
{
    if (!mDescFile) return;
    mDescFile->close();
    delete mDescFile;
    mDescFile = 0;
}

//================================================================================================
// Get the value of the nth (=posNr) incidence of a keyword.
// If keyword is not found on posNr, return the first incidence.
//================================================================================================
bool Option::getDescStr(const char *strKeyword, string &strBuffer, unsigned int posNr)
{
    size_t pos=0, startPos=0, stopPos, entryTest;
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

//================================================================================================
// Constructor.
//================================================================================================
Option::Option()
{
    Logger::log().headline("Init Options");
    mDescFile =0;
}

//================================================================================================
// Destructor.
//================================================================================================
Option::~Option()
{
    closeDescFile();
}
