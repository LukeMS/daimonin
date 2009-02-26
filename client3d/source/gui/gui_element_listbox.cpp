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
#include "logger.h"
#include "gui_manager.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_element_listbox.h"

using namespace Ogre;

static const Real SCROLL_SPEED = 0.001f;

// todo: speed up the scrolling with the amount of mRowsToScroll.

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
    if (mGfxSrc)
    {
        // ////////////////////////////////////////////////////////////////////
        // Save the original background.
        // ////////////////////////////////////////////////////////////////////
        Texture *texture = mParent->getTexture();
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
    mMaxVisibleRows  = (int)((float)mHeight / (float)mFontHeight + 0.5);
    if (mMaxVisibleRows < 1) mMaxVisibleRows = 1;
    mScrollBarV = 0;
    mScrollBarH = 0;
    TiXmlElement *xmlOpt;
    for (xmlOpt = xmlElement->FirstChildElement("Element"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Element"))
    {
        if (!strcmp(xmlOpt->Attribute("type"), "SCROLLER"))
        {
            if (!strcmp(xmlOpt->Attribute("name"), "VScrollbar"))
                mScrollBarV= new GuiElementScrollbar(xmlOpt, mParent, this);
            if (!strcmp(xmlOpt->Attribute("name"), "HScrollbar"))
                mScrollBarH= new GuiElementScrollbar(xmlOpt, mParent, this);
        }
    }
    clear();
    draw();
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::clear()
{
    mTime = 0;
    mKeyStart = 0;
    mKeyCount = 0;
    mActLines = 0;
    mPrintPos = 0;
    mActLines = 0;
    mBufferPos= 0;
    mPixelScroll = 0;
    mRowsToScroll= 0;
    mSelectedLine = -1;
    mVScrollOffset=  0;
    memcpy(mGfxBuffer, mGfxBuffer + mGfxBufferSize, mGfxBufferSize*sizeof(uint32));
    Texture *texture = mParent->getTexture();
    texture->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
        Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
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
int GuiListbox::sendMsg(int message, void *parm1, void *parm2, void *parm3)
{
    switch (message)
    {
        case GuiManager::MSG_ADD_ROW:
        {
            uint32 color = (uint32) parm2;
            if (!color) color = 0x00ffffff;
            addRow((const char*)parm1, color);
            return 0;
        }
        case GuiManager::MSG_CLEAR:
            clear();
            return 0;
        default:
            return 0;
    }
}

//================================================================================================
// Add line(s) of text to the ring-buffer (perform auto-clipping).
//================================================================================================
int GuiListbox::addRow(String srcText, uint32 default_color)
{
    GuiTextout::getSingleton().parseUserDefinedChars(srcText);
    unsigned char *buf2 = (unsigned char*)srcText.c_str();
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
            if (mActLines < SIZE_STRING_BUFFER) ++mActLines;
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
int GuiListbox::mouseEvent(int MouseAction, int x, int y, int mouseWheel)
{
    if (mouseWheel)
    {
        scrollTextVertical((mouseWheel>0)?-1:+1);
        return GuiManager::EVENT_CHECK_DONE;
    }
    // Scrollbar action?
    if (mScrollBarV && mScrollBarV->mouseEvent(MouseAction, x, y, mouseWheel) == GuiManager::EVENT_USER_ACTION)
    {
        scrollTextVertical(mScrollBarV->getScrollOffset());
        return GuiManager::EVENT_CHECK_DONE;
    }
    if (mScrollBarH && mScrollBarH->mouseEvent(MouseAction, x, y, mouseWheel) == GuiManager::EVENT_USER_ACTION)
    {
        scrollTextVertical(mScrollBarV->getScrollOffset());
        return GuiManager::EVENT_CHECK_DONE;
    }
    // Mouseclick within the textarea?
    if (MouseAction != GuiManager::BUTTON_PRESSED || x< mPosX || x> mPosX+mWidth || y< mPosY || y> mPosY+mHeight) return false;
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
            //return "MSG_LINE_PRESSED"; activated(mSelectedLine);
        }
        else
            mSelectedLine = -1;
    }
    return GuiManager::EVENT_CHECK_DONE;
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
// todo: speed up the scrolling speed with the amount of mRowsToScroll.
//================================================================================================
void GuiListbox::update(Ogre::Real dTime)
{
    mTime += dTime;
    if (mTime < SCROLL_SPEED || !mRowsToScroll) return;
    mTime = 0;
    draw();
}

//================================================================================================
// Display the textlines.
//================================================================================================
void GuiListbox::draw()
{
    Texture *texture = mParent->getTexture();
    // ////////////////////////////////////////////////////////////////////
    // Graphical background.
    // ////////////////////////////////////////////////////////////////////
    if (mGfxSrc)
    {
        // Restore old background into the work-buffer.
        memcpy(mGfxBuffer, mGfxBuffer + mGfxBufferSize, mGfxBufferSize*sizeof(uint32));
        uint32 *gfxBuf = mGfxBuffer + mWidth * (mFontHeight-mPixelScroll-1);
        for (int i=0; i < mMaxVisibleRows; ++i)
        {
            uint32 color = row[(mPrintPos+i-mMaxVisibleRows)& (SIZE_STRING_BUFFER-1)].color;
            GuiTextout::getSingleton().printText(mWidth, mFontHeight, mGfxBuffer, mWidth,
                                                 mGfxBuffer, mWidth,
                                                 row[(mPrintPos+i-mMaxVisibleRows)& (SIZE_STRING_BUFFER-1)].str.c_str(), mFontNr, color);
            gfxBuf+= mWidth * mFontHeight;
        }
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
            uint32 color = row[(mPrintPos)& (SIZE_STRING_BUFFER-1)].color;
            GuiTextout::getSingleton().printText(mWidth, mFontHeight, mGfxBuffer + mWidth * mHeight, mWidth,
                                                 mGfxBuffer + mWidth * mHeight, mWidth,
                                                 row[(mPrintPos)& (SIZE_STRING_BUFFER-1)].str.c_str(), mFontNr, color);
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
        mScrollBarV->updateSliderSize(SIZE_STRING_BUFFER, mVScrollOffset, mMaxVisibleRows, mActLines);
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::scrollTextVertical(int offset)
{
    if (mActLines < mMaxVisibleRows || !offset) return; // Nothing to scroll.
    if (offset < 0)
    {
        if (mVScrollOffset >= mActLines)  return;
        mVScrollOffset-= offset;
        if (mVScrollOffset > mActLines-mMaxVisibleRows) mVScrollOffset = mActLines-mMaxVisibleRows;
    }
    else
    {
        if (!mVScrollOffset) return;
        mVScrollOffset-= offset;
        if (mVScrollOffset <0) mVScrollOffset = 0;
    }
    uint32 *gfxBuf = mGfxBuffer;
    // Restore old background into the work-buffer.
    memcpy(mGfxBuffer, mGfxBuffer + mGfxBufferSize, mGfxBufferSize*sizeof(uint32));
    for (int i= -mMaxVisibleRows; i; ++i)
    {
        GuiTextout::getSingleton().printText(mWidth, mFontHeight, gfxBuf, mWidth, gfxBuf, mWidth,
                                             row[(mPrintPos+i-mVScrollOffset) & (SIZE_STRING_BUFFER-1)].str.c_str(), mFontNr,
                                             row[(mPrintPos+i-mVScrollOffset) & (SIZE_STRING_BUFFER-1)].color);
        gfxBuf+= mWidth * mFontHeight;
    }
    Texture *texture = mParent->getTexture();
    texture->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
        Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
    if (mScrollBarV)
        mScrollBarV->updateSliderSize(SIZE_STRING_BUFFER, mVScrollOffset, mMaxVisibleRows, mActLines);
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::scrollTextHorizontal(int offset)
{
    if (mGfxSrc)
    {
        ;
    }
    else
    {
        ;
    }
}
