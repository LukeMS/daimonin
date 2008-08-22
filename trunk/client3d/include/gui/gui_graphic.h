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

#ifndef GUI_GRAPHIC_H
#define GUI_GRAPHIC_H

#include <Ogre.h>
#include <tinyxml.h>
#include "gui_element.h"

/**
 ** This class provides a graphical element, drawn into a gui element.
 *****************************************************************************/
class GuiGraphic : public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiGraphic(TiXmlElement *xmlElement, void *parent, bool drawOnInit = true);
    ~GuiGraphic() {};
    void draw();
    void drawSlot(Ogre::uint32 *gfxDataItem, int busyTime, int sumItems);
private:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    Ogre::uint32 alphaBlend(const Ogre::uint32 bg, const Ogre::uint32 gfx);
    inline void drawBusyGfx(int itemSize, int busyTime);
};

#endif
