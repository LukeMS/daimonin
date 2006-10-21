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

#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_listbox.h"
#include "gui_manager.h"
#include "gui_textout.h"
#include "logger.h"
#include "gui_window.h"

//static const unsigned long SCROLL_SPEED = 12;
static const unsigned long SCROLL_SPEED = 4;
static const Real CLOSING_SPEED  = 10.0f;  // default: 10.0f

//================================================================================================
// Constructor.
//================================================================================================
GuiListbox::GuiListbox(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    // ////////////////////////////////////////////////////////////////////
    // Create buffer to hold the pixel information of the listbox.
    // ////////////////////////////////////////////////////////////////////
    mFontHeight = GuiTextout::getSingleton().getFontHeight(mFontNr);
    int size = mWidth * mHeight + mWidth * (mFontHeight+1);
    mGfxBuffer = new uint32[size];

    if (mFillType == FILL_GFX)
    {
        // ////////////////////////////////////////////////////////////////////
        // Save the original background.
        // ////////////////////////////////////////////////////////////////////
        Texture *texture = ((GuiWindow*) mParent)->getTexture();
        texture->getBuffer()->blitToMemory(
            Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight),
            PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, mGfxBuffer));
    }
    else
    {
        for (register int i =0; i < size; ++i) mGfxBuffer[i] = mFillColor;
    }
    // ////////////////////////////////////////////////////////////////////
    // Set defaults.
    // ////////////////////////////////////////////////////////////////////
    mIsClosing    = false;
    mIsOpening    = false;
    mDragging     = false;
    mBufferPos    = 0;
    mPrintPos     = 0;
    mRowsToScroll = 0;
    mMaxVisibleRows  = mHeight / mFontHeight;
    if (mMaxVisibleRows < 1) mMaxVisibleRows = 1;
    mScroll       = 0;
    mScrollBarV   = 0;
    mScrollBarH   = 0;
    mActLines     = 0;
    TiXmlElement *xmlOpt;

    for (xmlOpt = xmlElement->FirstChildElement("Gadget"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Gadget"))
    {
        if (!strcmp(xmlOpt->Attribute("type"), "SCROLLER"))
        {
            if (!strcmp(xmlOpt->Attribute("name"), "List_Msg_VScroll"))
            {
                mScrollBarV= new GuiGadgetScrollbar(xmlOpt, mParent, this);
                mScrollBarV->setFunction(this->scrollbarAction);
            }
            else if (!strcmp(xmlOpt->Attribute("name"), "List_Msg_HScroll"))
            {
                mScrollBarH= new GuiGadgetScrollbar(xmlOpt, mParent, this);
                mScrollBarH->setFunction(this->scrollbarAction);
            }
        }
    }
}

//================================================================================================
// Destructor.
//================================================================================================
GuiListbox::~GuiListbox()
{
    delete[] mGfxBuffer;
    delete mScrollBarV;
    delete mScrollBarH;
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::scrollbarAction(GuiListbox *me, int index, int scroll)
{
    if (index >= GuiGadgetScrollbar::BUTTON_V_ADD)
        me->scrollTextVertical(scroll);
    else
        me->scrollTextHorizontal(scroll);
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::scrollTextVertical(int offset)
{
    // Pay attention to the ring buffer.
    if (mPrintPos> SIZE_STRING_BUFFER)
        offset+= mPrintPos & (SIZE_STRING_BUFFER-1);
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    uint32 *gfxBuf = mGfxBuffer;

    if (mFillType == FILL_GFX)
    {
        // Restore old background.
        texture->getBuffer()->blitFromMemory(
            PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
            Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
        // Print the lines.
         // Print the lines.
        TextLine label;
        label.hideText= false;
        label.index= -1;
        label.font = mFontNr;
        label.x1 = mPosX;
        label.x2 = label.x1 + mWidth;
        label.y1 = mPosY -mFontHeight;;
        for (int i =0; i <= mMaxVisibleRows; ++i)
        {
            label.y1+= mFontHeight;
            if ((int) label.y1 > mHeight+mPosY) break;
            label.y2 = label.y1 + mFontHeight;
            if ((int) label.y2 > mHeight+mPosY) label.y2 = mHeight+mPosY;
            label.text = row[(mPrintPos + offset+ (i)-4-mMaxVisibleRows)& (SIZE_STRING_BUFFER-1)].str.c_str();
            GuiTextout::getSingleton().Print(&label, texture);
        }
    }
    else
    {
        for (int i=0; i < mMaxVisibleRows+1; ++i)
        {
            GuiTextout::getSingleton().PrintToBuffer(mWidth, mFontHeight, gfxBuf, row[(offset+i)& (SIZE_STRING_BUFFER-1)].str.c_str(), mFontNr, mFillColor);
            gfxBuf+= mWidth * mFontHeight;
        }
        texture->getBuffer()->blitFromMemory(
            PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
            Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
    }
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::scrollTextHorizontal(int offset)
{
    if (mFillType == FILL_GFX)
    {
        ;
    }
    else
    {
        ;
    }
}

//================================================================================================
// Add a line of text to the ring-buffer (perform auto-clipping).
//================================================================================================
void GuiListbox::addTextline(const char *srcText)
{
    static int  key_start = 0;
    static int  key_count = 0;

    // ////////////////////////////////////////////////////////////////////
    // Copy the text to a temp buffer, skipping all whitespaces.
    // ////////////////////////////////////////////////////////////////////
    char *buf2 = new char [strlen(srcText)+1];
    char *text = buf2;
    for (int i= 0; srcText[i]; ++i)
    {
        if (srcText[i] >= 32 || srcText[i] == 0x0a ||
                srcText[i] == TXT_CMD_SOUND)
            *text++ = srcText[i];
    }
    *text =0;

    // ////////////////////////////////////////////////////////////////////
    // Mask out the sound command.
    // ////////////////////////////////////////////////////////////////////
    char *tag, *tagend, savetagend;
    while ((tag = strchr(buf2, TXT_CMD_SOUND)))
    {
        tagend = strchr(tag, 0x0a);
        if (!tagend)
            tagend = tag + strlen(tag);
        if (tagend > tag+1)
        {
            savetagend = *tagend;
            *tagend = '\0';
            // init_media_tag(tag);
            *tagend = savetagend;
            *tag = '\0';
        }
        // Remove sound command from the string.
        memmove(tag, tagend, strlen(tag+1));
    }

    // ////////////////////////////////////////////////////////////////////
    // Cut the string to make it fit into the window.
    // ////////////////////////////////////////////////////////////////////
    int w = 0, dstPos = 0, srcPos =0;
    int ii, ix, it, tx;
    char buf[4096];
    while (1)
    {
        if (buf2[srcPos] == TXT_CMD_HIGHLIGHT || buf2[srcPos] == TXT_CMD_LINK)
        {
            buf[dstPos++] = buf2[srcPos++];
            if (buf2[srcPos] == TXT_SUB_CMD_COLOR)
                for (int skip = 9; skip; --skip) buf[dstPos++] = buf2[srcPos++];
        }
        w += GuiTextout::getSingleton().getCharWidth(mFontNr, buf2[srcPos]);

        // Here comes a linebreak.
        if (w >= mWidth || buf2[srcPos] <= 0x0a)
        {
            // now the special part - lets look for a good point to cut
            if (w >= mWidth && dstPos > 10)
            {
                ii = ix = dstPos;
                it = tx = srcPos;
                while (ii >= dstPos / 2)
                {
                    if (buf2[it] == ' '
                            || buf2[it] == ':'
                            || buf2[it] == '.'
                            || buf2[it] == ','
                            || buf2[it] == '('
                            || buf2[it] == ';'
                            || buf2[it] == '-'
                            || buf2[it] == '+'
                            || buf2[it] == '*'
                            || buf2[it] == '?'
                            || buf2[it] == '/'
                            || buf2[it] == '='
                            || buf2[it] == '.'
                            || buf2[it] == 0x0a
                            || buf2[it] == 0)
                    {
                        tx = it;
                        ix = ii;
                        break;
                    }
                    --it;
                    --ii;
                }
                srcPos = tx;
                dstPos = ix;
            }
            buf[dstPos] =0;
            row[mBufferPos & (SIZE_STRING_BUFFER-1)].str = buf;
            row[mBufferPos & (SIZE_STRING_BUFFER-1)].key_clipped = key_start;
            ++mBufferPos;
            ++mRowsToScroll;
            dstPos = w = 0;
            if (buf2[srcPos] == ' ') ++srcPos;

            // hack: because of autoclip we must scan every line again.
            for (text = buf; *text; ++text)
                if (*text == TXT_CMD_LINK)
                    key_count = (key_count + 1) & 1;
            if (key_count)
                key_start = 0x1000;
            else
                key_start = 0;

            if (!buf2[srcPos])
                break;
        }
        if (buf2[srcPos] != 0x0a)
            buf[dstPos++] = buf2[srcPos];
        ++srcPos;
    }
    delete[] buf2;
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
bool GuiListbox::mouseEvent(int MouseAction, int x, int y)
{
    if (mScrollBarV && mScrollBarV->mouseEvent(MouseAction, x, y)) return true;
    if (mScrollBarH && mScrollBarH->mouseEvent(MouseAction, x, y)) return true;
    return false;
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::drawScrollbar()
{
    if (mScrollBarV) mScrollBarV->draw();
    if (mScrollBarH) mScrollBarH->draw();
}

//================================================================================================
// Display the textlines.
//================================================================================================
void GuiListbox::draw()
{
    if (!mRowsToScroll || mDragging) return;
    static unsigned long time = Root::getSingleton().getTimer()->getMilliseconds();
    if (Root::getSingleton().getTimer()->getMilliseconds() - time < SCROLL_SPEED) return;
    time = Root::getSingleton().getTimer()->getMilliseconds();
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    // ////////////////////////////////////////////////////////////////////
    // Graphical background.
    // ////////////////////////////////////////////////////////////////////
    if (mFillType == FILL_GFX)
    {
        // Restore old background.
        texture->getBuffer()->blitFromMemory(
            PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
            Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
        // Print the lines.
        TextLine label;
        label.hideText= false;
        label.index= -1;
        label.font = mFontNr;
        label.x1 = mPosX;
        label.x2 = label.x1 + mWidth;
        label.y1 = mPosY - mScroll;
        for (int i =0; i <= mMaxVisibleRows; ++i)
        {
            label.y1+= mFontHeight;
            if ((int) label.y1 > mHeight+mPosY) break;
            label.y2 = label.y1 + mFontHeight;
            if ((int) label.y2 > mHeight+mPosY) label.y2 = mHeight+mPosY;
            label.text = row[(mPrintPos+ (i)-mMaxVisibleRows)& (SIZE_STRING_BUFFER-1)].str.c_str();
            GuiTextout::getSingleton().Print(&label, texture);
        }
        // The complete row was scrolled.
        if (++mScroll >= mFontHeight)
        {
            --mRowsToScroll;
            ++mPrintPos;
            mScroll =0;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Single color background.
    // ////////////////////////////////////////////////////////////////////
    else
    {
        // New Line to scroll in.
        if (!mScroll)
        {
            // Print it to the (invisible) last line of the listbox.
            GuiTextout::getSingleton().PrintToBuffer(mWidth, mFontHeight, mGfxBuffer + mWidth * mHeight, row[(mPrintPos)& (SIZE_STRING_BUFFER-1)].str.c_str(), mFontNr, mFillColor);
        }
        texture->getBuffer()->blitFromMemory(
            PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer + mWidth * mScroll),
            Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));

        // The complete row was scrolled.
        if (++mScroll >= mFontHeight)
        {
            --mRowsToScroll;
            ++mPrintPos;
            mScroll =0;
            memcpy(mGfxBuffer, mGfxBuffer + mWidth *mFontHeight, mWidth * mHeight * sizeof(uint32));
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Handle Vertical Scrollbar.
    // ////////////////////////////////////////////////////////////////////
    if (mScrollBarV)
    {
        if (mActLines < SIZE_STRING_BUFFER) ++mActLines;
        mScrollBarV->updateSliderSize(mMaxVisibleRows, mPrintPos, SIZE_STRING_BUFFER);
    }
}

