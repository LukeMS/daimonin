#include "dialog.h"


//=================================================================================================
// Init all elements.
// BEWARE: Ogre has not init at this point.
//=================================================================================================
bool Dialog::Init()
{
    return true;
}


//=================================================================================================
// Return the instance.
//=================================================================================================
Dialog &Dialog::getSingelton()
{
   static Dialog singelton;
   return singelton;
}