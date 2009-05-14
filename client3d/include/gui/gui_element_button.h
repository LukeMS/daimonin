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

#ifndef GUI_ELEMENT_BUTTON_H
#define GUI_ELEMENT_BUTTON_H

#include "gui_element.h"

/**
 ** This class provides an interactive button.
 *****************************************************************************/
class GuiElementButton: public GuiElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiElementButton(TiXmlElement *xmlElement, const void *parent, const bool drawOnInit);
    ~GuiElementButton() {}
    /// @copydoc GuiElement::sendMsg
    virtual void sendMsg(const int message, Ogre::String &text, Ogre::uint32 &param, const char *text2);
    virtual int mouseEvent(const int mouseAction, int mouseX, int mouseY, int mouseWheel);
    void draw();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::String mStrTooltip;
    bool mMouseOver, mMouseButDown;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /** Set a new label text for a button.
     ** @param newText The button label.
     *****************************************************************************/
    void setLabel(const Ogre::String newText)
    {
        mLabelString = newText;
        draw();
    }
};

#endif
