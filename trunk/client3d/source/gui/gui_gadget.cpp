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

#include <OgreHardwarePixelBuffer.h>
#include "gui_gadget.h"
#include "gui_textout.h"
#include "logger.h"
#include "gui_imageset.h"
#include "gui_window.h"

GuiGadget::GuiGadget(TiXmlElement *xmlElem, int w, int h, int maxX, int maxY) :GuiElement(xmlElem, w, h, maxX, maxY)
{
    const char* strTmp;
    GuiSrcEntry *srcEntry;

    /// Find the gfx data in the tileset.
    if (!(strTmp = xmlElem->Attribute("image_name")))
        return;
    srcEntry = GuiImageset::getSingleton().getStateGfxPositions(strTmp);
    if (srcEntry)
    {
        for (unsigned int i = 0; i < srcEntry->state.size(); ++i)
            setStateImagePos(srcEntry->state[i]->name, srcEntry->state[i]->x, srcEntry->state[i]->y);
    }
    else
    {
        Logger::log().warning() << strTmp << " was defined in '" << FILE_GUI_WINDOWS
        << "' but the gfx-data in '" << FILE_GUI_IMAGESET << "' is missing.";
    }

    if ((strTmp = xmlElem->Attribute("name")))
    {
        for (int i = 0; i < GUI_ELEMENTS_SUM; ++i)
        {
            if (!stricmp(GuiImageset::getSingleton().getElementName(i), strTmp))
            {
                index = GuiImageset::getSingleton().getElementIndex(i);
                break;
            }
        }
    }
}

GuiGadget::~GuiGadget()
{
}

///================================================================================================
/// .
///================================================================================================
void GuiGadget::draw(PixelBox &mSrcPixelBox, Texture *texture)
{
}

///================================================================================================
/// Just to forward method or to silently handle it
///================================================================================================
bool GuiGadget::setState(int state)
{
    return GuiElement::setState(state);
}

void GuiGadget::addTextline(const char *value)
{
}

void GuiGadget::setText(const char *value)
{
}

const char *GuiGadget::getText()
{
    return NULL;
}

int GuiGadget::getAction()
{
    int ret = mAction;
    mAction = GuiWindow::GUI_ACTION_NONE;
    return ret;
}

bool GuiGadget::mouseOver(int x, int y)
{
    return GuiElement::mouseOver(x,y);
}
