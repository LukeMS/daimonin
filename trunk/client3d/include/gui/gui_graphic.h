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


/**
 ** This class provides draw functions.
 *****************************************************************************/
#ifndef GUI_GRAPHIC_H
#define GUI_GRAPHIC_H

class GuiGraphic
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiGraphic &getSingleton()
    {
        static GuiGraphic Singleton;
        return Singleton;
    }
    void restoreWindowBG(int w, int h, unsigned int *bak, unsigned int *dst, int bakRowSkip, int dstRowSkip);
    void drawGfxToBuffer(int w, int h, int srcW, int srcH, unsigned int *src, unsigned int *bak, unsigned int *dst, int srcRowSkip, int bakRowSkip, int dstRowSkip);
    void drawColorToBuffer(int w, int h, unsigned int color, unsigned int *dst, int dstRowSkip);
    void drawColorToBuffer(int w, int h, unsigned int color, unsigned int *bak, unsigned int *dst, int bakRowSkip, int dstRowSkip);
    unsigned int alphaBlend(const unsigned int bg, const unsigned int gfx);
    unsigned int fontBlend(const unsigned int bg, const unsigned int gfx);

private:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiGraphic()  {}
    ~GuiGraphic() {}
    GuiGraphic(const GuiGraphic&);            /**< disable copy-constructor. **/
    GuiGraphic &operator=(const GuiGraphic&); /**< disable assignment operator. **/
};

#endif
