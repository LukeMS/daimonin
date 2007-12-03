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

#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_listbox.h"
#include "gui_manager.h"
#include "gui_textout.h"
#include "logger.h"
#include "gui_window.h"

using namespace Ogre;

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
    mGfxBufferSize = mWidth * mHeight + mWidth * (mFontHeight+1);
    mGfxBuffer = new uint32[mGfxBufferSize*2];

    if (mFillType == FILL_GFX)
    {
        // ////////////////////////////////////////////////////////////////////
        // Save the original background.
        // ////////////////////////////////////////////////////////////////////
        Texture *texture = ((GuiWindow*) mParent)->getTexture();
        texture->getBuffer()->blitToMemory(
            Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight),
            PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8, mGfxBuffer+mGfxBufferSize));
    }
    else
    {
        for (register int i =0; i < mGfxBufferSize+mGfxBufferSize; ++i) mGfxBuffer[i] = mFillColor;
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
    mPixelScroll  = 0;
    mVScrollOffset = 0;
    mScrollBarV   = 0;
    mScrollBarH   = 0;
    mActLines     = 0;
    mSelectedLine = -1;
    mKeyStart = 0;
    mKeyCount = 0;

    mTime = Root::getSingleton().getTimer()->getMilliseconds();
    TiXmlElement *xmlOpt;

    for (xmlOpt = xmlElement->FirstChildElement("Gadget"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Gadget"))
    {
        if (!strcmp(xmlOpt->Attribute("type"), "SCROLLER"))
        {
            if (!strcmp(xmlOpt->Attribute("name"), "VScrollbar"))
            {
                mScrollBarV= new GuiGadgetScrollbar(xmlOpt, mParent, this);
                mScrollBarV->setFunction(this->scrollbarAction);
            }
            else if (!strcmp(xmlOpt->Attribute("name"), "HScrollbar"))
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
void GuiListbox::clear()
{
    mRowsToScroll =0;
    mPixelScroll =0;
    mPrintPos =0;
    mActLines =0;
    mBufferPos=0;
    memcpy(mGfxBuffer, mGfxBuffer + mGfxBufferSize, mGfxBufferSize*sizeof(uint32));
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    texture->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
        Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::scrollbarAction(GuiListbox *me, int index, int scroll)
{
    if (index >= GuiGadgetScrollbar::BUTTON_V_ADD)
    {
        if (me->mActLines > me->mMaxVisibleRows)
            me->scrollTextVertical(scroll);
    }
    else
        me->scrollTextHorizontal(scroll);
}


//================================================================================================
// Add line(s) of text to the ring-buffer (perform auto-clipping).
//================================================================================================
int GuiListbox::addTextline(const char *srcText, uint32 default_color)
{
    unsigned char *buf2 = (unsigned char*)GuiTextout::getSingleton().showUserDefinedChars(srcText);
    // ////////////////////////////////////////////////////////////////////
    // Mask out the sound command.
    // ////////////////////////////////////////////////////////////////////
    char *tag, *tagend, savetagend;
    while ((tag = strchr((char*)buf2, GuiTextout::TXT_CMD_SOUND)))
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
    int linecount =0;
    int w = 0, dstPos = 0, srcPos =0;
    int ii, ix, it, tx;
    unsigned char buf[4096];
    int startLine =0;
    while (1)
    {
        if (buf2[srcPos] == GuiTextout::TXT_CMD_HIGHLIGHT
                || buf2[srcPos] == GuiTextout::TXT_CMD_LINK
                || buf2[srcPos] == GuiTextout::TXT_CMD_LOWLIGHT)
        {
            buf[dstPos++] = buf2[srcPos++];
            if (buf2[srcPos] == GuiTextout::TXT_SUB_CMD_COLOR)
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
                            || buf2[it] == ';'
                            || buf2[it] == '.'
                            || buf2[it] == ','
                            || buf2[it] == '('
                            || buf2[it] == '-'
                            || buf2[it] == '+'
                            || buf2[it] == '*'
                            || buf2[it] == '/'
                            || buf2[it] == '?'
                            || buf2[it] == '='
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
            row[mBufferPos & (SIZE_STRING_BUFFER-1)].color= default_color;
            row[mBufferPos & (SIZE_STRING_BUFFER-1)].str = (char*)buf;
            row[mBufferPos & (SIZE_STRING_BUFFER-1)].keyword_clipped = mKeyStart;
            row[mBufferPos & (SIZE_STRING_BUFFER-1)].startLine = startLine++;
            ++mBufferPos;
            ++mRowsToScroll;
            ++linecount;
            ++mActLines;
            dstPos = w = 0;
            if (buf2[srcPos] == ' ') ++srcPos;

            // hack: because of autoclip we must scan every line again.
            for (unsigned char *text = buf; *text; ++text)
                if (*text == GuiTextout::TXT_CMD_LINK)
                    mKeyCount = (mKeyCount + 1) & 1;
            if (mKeyCount)
                mKeyStart = 0x1000;
            else
                mKeyStart = 0;

            if (!buf2[srcPos])
                break;
        }
        if (buf2[srcPos] != 0x0a)
            buf[dstPos++] = buf2[srcPos];
        ++srcPos;
    }
    return linecount;
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
bool GuiListbox::mouseEvent(int MouseAction, int x, int y, int z)
{
    // z is the mousewheel +/0/-
    // Scrollbar action?
    if (mScrollBarV && mScrollBarV->mouseEvent(MouseAction, x, y)) return true;
    if (mScrollBarH && mScrollBarH->mouseEvent(MouseAction, x, y)) return true;
    // Mouseclick within the textarea?
    if (MouseAction != GuiWindow::BUTTON_PRESSED || x< mPosX || x> mPosX+mWidth || y< mPosY || y> mPosY+mHeight) return false;
    if (mVScrollOffset)
    {
        if (mVScrollOffset <0) mVScrollOffset =0;
        mSelectedLine = (mVScrollOffset + (y - mPosY)/ mFontHeight) & (SIZE_STRING_BUFFER-1);
    }
    else
    {
        // Find out which line was pressed.
        mSelectedLine = (mPrintPos-(mMaxVisibleRows-(y - mPosY)/ mFontHeight)-1) & (SIZE_STRING_BUFFER-1);
        if (mSelectedLine <= mActLines)
        {
            // On a mulitline text, give back the first line.
            mSelectedLine -=row[mSelectedLine & (SIZE_STRING_BUFFER-1)].startLine;
            activated(mSelectedLine);
        }
        else
            mSelectedLine = -1;
    }
    return true;
}

//================================================================================================
// TODO...
//================================================================================================
const char *GuiListbox::getSelectedKeyword()
{
    if (mSelectedLine <0) return 0;
    const char *textline = row[mSelectedLine].str.c_str();
    // Extract the keyword.

    mSelectedLine = -1;
    return textline;
}

//================================================================================================
// Display the textlines.
//================================================================================================
void GuiListbox::draw()
{
    if (!mRowsToScroll || mDragging) return;
    if (Root::getSingleton().getTimer()->getMilliseconds() - mTime < SCROLL_SPEED) return;
    mTime = Root::getSingleton().getTimer()->getMilliseconds();
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    // ////////////////////////////////////////////////////////////////////
    // Graphical background.
    // ////////////////////////////////////////////////////////////////////
    if (mFillType == FILL_GFX)
    {
        // Restore old background into the work-buffer.
        memcpy(mGfxBuffer, mGfxBuffer + mGfxBufferSize, mGfxBufferSize*sizeof(uint32));
        uint32 *gfxBuf = mGfxBuffer + mWidth * (mFontHeight-mPixelScroll);
        for (int i=0; i < mMaxVisibleRows+1; ++i)
        {
            GuiTextout::getSingleton().PrintToBuffer(mWidth, mFontHeight, gfxBuf,
                    row[(mPrintPos+ (i)-mMaxVisibleRows)& (SIZE_STRING_BUFFER-1)].str.c_str(), mFontNr,
                    row[(mPrintPos+ (i)-mMaxVisibleRows)& (SIZE_STRING_BUFFER-1)].color);
            gfxBuf+= mWidth * mFontHeight;
        }
        Texture *texture = ((GuiWindow*) mParent)->getTexture();
        texture->getBuffer()->blitFromMemory(
            PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
            Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));

        // The complete row was scrolled.
        if (++mPixelScroll >= mFontHeight)
        {
            --mRowsToScroll;
            ++mPrintPos;
            mPixelScroll =0;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Single color background.
    // ////////////////////////////////////////////////////////////////////
    else
    {
        // New Line to scroll in.
        if (!mPixelScroll)
        {
            // Print it to the (invisible) last line of the listbox.
            memcpy(mGfxBuffer+ mWidth * mHeight, mGfxBuffer + mGfxBufferSize, mWidth * mFontHeight*sizeof(uint32));
            GuiTextout::getSingleton().PrintToBuffer(mWidth, mFontHeight, mGfxBuffer + mWidth * mHeight,
                    row[(mPrintPos)& (SIZE_STRING_BUFFER-1)].str.c_str(), mFontNr,
                    row[(mPrintPos)& (SIZE_STRING_BUFFER-1)].color);
        }
        texture->getBuffer()->blitFromMemory(
            PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer + mWidth * mPixelScroll),
            Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));

        // The complete row was scrolled.
        if (++mPixelScroll >= mFontHeight)
        {
            --mRowsToScroll;
            ++mPrintPos;
            mPixelScroll =0;
            memcpy(mGfxBuffer, mGfxBuffer + mWidth *mFontHeight, mWidth * mHeight * sizeof(uint32));
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Handle Vertical Scrollbar.
    // ////////////////////////////////////////////////////////////////////
    if (mScrollBarV)
    {
        mScrollBarV->updateSliderSize(mMaxVisibleRows, mPrintPos, SIZE_STRING_BUFFER);
    }
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::scrollTextVertical(int offset)
{
    // Pay attention to the ring buffer.
    if (mPrintPos> SIZE_STRING_BUFFER)
        offset+= mPrintPos & (SIZE_STRING_BUFFER-1);
    mVScrollOffset = offset==0?-1:offset;
    uint32 *gfxBuf = mGfxBuffer;
    // Restore old background into the work-buffer.
    memcpy(mGfxBuffer, mGfxBuffer + mGfxBufferSize, mGfxBufferSize*sizeof(uint32));
    for (int i=0; i < mMaxVisibleRows+1; ++i)
    {
        GuiTextout::getSingleton().PrintToBuffer(mWidth, mFontHeight, gfxBuf,
                row[(offset+i)& (SIZE_STRING_BUFFER-1)].str.c_str(), mFontNr,
                row[(offset+i)& (SIZE_STRING_BUFFER-1)].color);
        gfxBuf+= mWidth * mFontHeight;
    }
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    texture->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
        Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
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
