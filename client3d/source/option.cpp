#include "option.h"


enum
{
    SEL_BUTTON,
    SEL_CHECKBOX,
    SEL_RANGE,
    SEL_TEXT
}; /* selection types */ 

static _options  options; 

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
