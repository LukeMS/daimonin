/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#ifndef GUI_STATUSBAR_H
#define GUI_STATUSBAR_H

#include "gui_element.h"

/**
 ** This class provides a statusbar.
 *****************************************************************************/
class GuiStatusbar : public GuiElement
{

public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiStatusbar(TiXmlElement *xmlElement, void *parent);
    ~GuiStatusbar() {}
    int sendMsg(int message, const char *text, Ogre::uint32 param);
    void draw();
    void setValue(int value);
    void update(Ogre::Real dTime);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    bool mVertical;
    bool mAutoColor;            /**< Change the color from green to red depending on the filling percentage. **/
    bool mSmoothChange;         /**< Change the filling percentage in small steps to the new value. **/
    int mLength, mDiameter;
    int mValue;
    Ogre::Real mDrawn;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void drawGfxBar(Ogre::uint32 *dst);
    void drawColorBar(Ogre::uint32 *dst);
};

#endif
