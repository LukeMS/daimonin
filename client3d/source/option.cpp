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
// Create/Overwrite the Optionfile.
//=================================================================================================
bool Option::Init(char *filename)
{
	LogFile::getSingelton().Headline("Init Options");

    _options  *ptions = &options; 


/*
	for (int i=0; opt[i].name; ++i)
    {
		if (opt[i].name[0] == '#') { continue; }
        opt[i].clean_memory = false;


        //*((int*) opt[i].int_value) = i;

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
// Destructor.
//=================================================================================================
Option::~Option()
{
/*
    for (int i=0; opt[i].name; ++i)
    { 
		if (opt[i].name[0] == '#') { continue; }
        if (opt[i].clean_memory == true)  { delete[] opt[i].val_text; }
	}	
*/
}

//=================================================================================================
// Returns the option as string.
//=================================================================================================
