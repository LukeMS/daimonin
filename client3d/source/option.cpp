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

#include "define.h"
#include "option.h"
#include "logfile.h"

enum
{
    SEL_BUTTON,
    SEL_CHECKBOX,
    SEL_RANGE,
    SEL_TEXT
}; /* selection types */ 

_options options;

struct _option opt[] =
{
    /* Sound */
    {"Sound volume:", "set sound volume for effects.","", "",SEL_RANGE, 0,100,5,100, &options.sound_volume, VAL_INT},
    {"Music volume:", "set music volume for background.","Sub info","", SEL_RANGE, 0,100,5,80, &options.music_volume, VAL_INT},
    {"#"},
    /* End of Page */

    {0} /* End of Options */
}; 

//=================================================================================================
// Open a description file.
//=================================================================================================
bool Option::openDescFile(const char *filename)
{
	closeDescFile();
	mDescFile = new ifstream(filename, ios::in);
    if (!mDescFile) { return false; }
    mFilename = filename;
	mDescBuffer ="";
	string buf;
	while (getline(*mDescFile, buf)) 
	{   // delete comments.
		if (buf.find("#")> 5) { mDescBuffer+= buf; }
	}
	return true;
}

//=================================================================================================
// Close a description file.
//=================================================================================================
void Option::closeDescFile()
{
	if (!mDescFile) { return; }
	mDescFile->close();
	delete mDescFile;
	mDescFile = 0;
}

//=================================================================================================
// Get the value of the nth (=posNr) incidence of a keyword.
// If keyword is not found on posNr, return the first incidence.
//=================================================================================================
bool Option::getDescStr(const char *strKeyword, string &strBuffer, int posNr)
{
	int pos=0, startPos=0, stopPos, entryTest;
  checkForKeyword:
	startPos = mDescBuffer.find(strKeyword, startPos);
	if (startPos <0) { return false; }
	entryTest= mDescBuffer.find(":",  startPos)+1;
	startPos = mDescBuffer.find("\"", startPos)+1;
	// keyword and value can have the same name. If ':' comes before '"' in the description-text
	// we have a keyword, else we have the value and search again.
	if (entryTest>startPos) { goto checkForKeyword; }
	stopPos  = mDescBuffer.find("\"", startPos)-startPos;
	strBuffer = mDescBuffer.substr(startPos, stopPos);
	if (++pos < posNr)  { goto checkForKeyword; }
	if (strBuffer.size() == 0) { return false; }
	return true;
}

//=================================================================================================
// Create/Overwrite the Optionfile.
//=================================================================================================
bool Option::Init()
{
	// filename: FILE_OPTIONS
	LogFile::getSingleton().Headline("Init Options");

	mDescFile = 0;
	mMetaServer ="damn.informatik.uni-bremen.de";
	mMetaServerPort = 13326;
	mSelectedMetaServer =0;
	mStartNetwork = false;

/*
    _options  *ptions = &options;

	for (int i=0; opt[i].name; ++i)
    {
		if (opt[i].name[0] == '#') { continue; }
        opt[i].clean_memory = false;


        // *((int*) opt[i].int_value) = i;

		int val = opt[i].int_value;
		opt[i].int_value = i;

		// *((bool *) opt[i].value)
        if (opt[i].name[0])
		{   // int value. 
            opt[i].val_actual = opt[i].val_default;
		}
		else
		{  // string value.
			opt[i].val_actual = 0; // offset to defualt string.
            //opt[i].str_value  = 
		}
	}
*/	

  
    return true;
 }

//=================================================================================================
// Constructor.
//=================================================================================================
Option::Option()
{
	GameStatus =  GAME_STATUS_INIT;
	mLogin = false;
	mDescFile =0;
}

//=================================================================================================
// Destructor.
//=================================================================================================
Option::~Option()
{
	closeDescFile();
/*
    for (int i=0; opt[i].name; ++i)
    { 
		if (opt[i].name[0] == '#') { continue; }
        if (opt[i].clean_memory == true)  { delete[] opt[i].val_text; }
	}	
*/
}
