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
#include "logger.h"
#include "gui_manager.h"
#include "gui_textout.h"
#include "gui_window.h"
#include "gui_element_listbox.h"

using namespace Ogre;

// Todo:
// - support gfx elements (they will be splitt over some lines if they are bigger then fontsize).

String GuiListbox::mKeywordPressed;

//================================================================================================
// Constructor.
//================================================================================================
GuiListbox::GuiListbox(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    mFontHeight = GuiTextout::getSingleton().getFontHeight(mFontNr);
    mMaxVisibleRows  = (int)((float)mHeight / (float)mFontHeight + 0.5);
    if (mMaxVisibleRows < 1) mMaxVisibleRows = 1;
    mScrollBarV = 0;
    for (TiXmlElement *xmlOpt = xmlElement->FirstChildElement("Element"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Element"))
    {
        if (!strcmp(xmlOpt->Attribute("type"), "SCROLLER") && !strcmp(xmlOpt->Attribute("name"), "VScrollbar"))
            mScrollBarV= new GuiElementScrollbar(xmlOpt, mParent, this);
    }
    clear();
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::clear()
{
    mPrintPos = 0;
    mActLines = 0;
    mBufferPos= 0;
    mPixelScroll = 0;
    mRowsToPrint = 0;
    mVScrollOffset=0;
    draw();
}

//================================================================================================
// Destructor.
//================================================================================================
GuiListbox::~GuiListbox()
{
    delete mScrollBarV;
}

//================================================================================================
// .
//================================================================================================
int GuiListbox::sendMsg(int message, const char *text, uint32 color)
{
    switch (message)
    {
        case GuiManager::MSG_ADD_ROW:
            addText(text, color);
            return 0;
        case GuiManager::MSG_CLEAR:
            clear();
            return 0;
        default:
            return 0;
    }
}

//================================================================================================
// .
//================================================================================================
const char *GuiListbox::getInfo(int info)
{
    switch (info)
    {
        case GuiManager::INFO_KEYWORD:
            return mKeywordPressed.c_str();
        default:
            return 0;
    }
}

//================================================================================================
// Add line(s) of text to the ring-buffer (perform auto-clipping).
//================================================================================================
int GuiListbox::addText(const char *txt, uint32 stdColor)
{
    if (!txt) return 0; // We MUST check for NULL, so we can't use String for the first parameter of addText().
    String strText = txt;
    // ////////////////////////////////////////////////////////////////////
    // Mask out all sound commands.
    // ////////////////////////////////////////////////////////////////////
    size_t start, stop, link;
    while ((start = strText.find(GuiTextout::TXT_CMD_SOUND))!= std::string::npos)
    {
        stop = strText.find('\n', start); // Sound cmd ends with a linebreak.
        Logger::log().warning() << "Listbox->Sound cmd: " << strText.substr(start+1, stop-start-1).c_str();
        strText.erase(start, stop-start+1);
    }
    GuiTextout::getSingleton().parseUserDefinedChars(strText);
    // ////////////////////////////////////////////////////////////////////
    // Keywords:
    // - Insert a "#x" (x == keyword number) after each keyword start.
    // - The keywords will be stored in keyword entry of the textrow.
    // ////////////////////////////////////////////////////////////////////
    String keyword;
    link = stop = 0;
    char keySign[] = { '#', '1', '\0' };
    while ((start = strText.find(GuiTextout::TXT_CMD_LINK, stop))!= std::string::npos)
    {
        if ((link = strText.find(GuiTextout::TXT_CMD_LOWLIGHT, ++start)) != std::string::npos)
        {
            stop = strText.find(GuiTextout::TXT_CMD_LINK, start);
            if (stop == std::string::npos) stop = strText.size();
            ++link;
            keyword+= strText.substr(link, stop-link) + GuiTextout::TXT_CMD_SEPARATOR;
            strText.erase(link, stop-link);
            stop = link;
        }
        else
        {
            stop = strText.find(GuiTextout::TXT_CMD_LINK, start);
            if (stop == std::string::npos) stop = strText.size();
            keyword+= strText.substr(start, stop-start) + GuiTextout::TXT_CMD_SEPARATOR;
        }
        strText.insert(start, keySign);
        stop+=3;
        ++keySign[1];
    }
    // ////////////////////////////////////////////////////////////////////
    // Add the rows to the ringbuffer.
    // ////////////////////////////////////////////////////////////////////
    String strLine="";
    int sumLines = 0;
    while (1)
    {
        // Get the text from start until linebreak.
        stop = strText.find('\n', 0);
        if (stop == std::string::npos) stop = strText.size();
        strLine = strText.substr(0, stop);
        // Does the full line fits into the listbox?
        stop = GuiTextout::getSingleton().getLastCharPosition(strLine.c_str(), mFontNr, mWidth);
        // Line needs clipping to fit into the window.
        if (stop < strLine.size())
        {
            size_t pos;
            for (pos = stop-1; pos > stop/2; --pos)
            {
                // Look for a good place to clip the text.
                if (strLine[pos] == ' '
                        || (strLine[pos] >= '*' && strLine[pos] <= '/') // *+,-./
                        ||  strLine[pos] == '('  // acii 40
                        ||  strLine[pos] == ':'  // acii 58
                        ||  strLine[pos] == ';'  // acii 59
                        ||  strLine[pos] == '='  // acii 61
                        ||  strLine[pos] == '?') // acii 63
                    break;
            }
            stop = pos;
        }
        strLine = strLine.substr(0, ++stop).c_str();
        row[mBufferPos & (SIZE_STRING_BUFFER-1)].text = strLine;
        row[mBufferPos & (SIZE_STRING_BUFFER-1)].color= stdColor;
        row[mBufferPos & (SIZE_STRING_BUFFER-1)].keyword= keyword;
        ++mBufferPos;
        ++mRowsToPrint;
        ++sumLines;
        if (mActLines >= mMaxVisibleRows)   ++mPrintPos;
        if (mActLines < SIZE_STRING_BUFFER) ++mActLines;
        strText.erase(0, stop);
        if (strText.empty()) return sumLines;
        // On a clipped line we start with the color of the last char from the line above.
        strText.insert(0, GuiTextout::getSingleton().getTextendColor(strLine));
    }
    return sumLines;
}

//================================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
int GuiListbox::mouseEvent(int MouseAction, int x, int y, int mouseWheel)
{
    // Scrollbar action?
    if (mScrollBarV && mScrollBarV->mouseEvent(MouseAction, x, y, mouseWheel) == GuiManager::EVENT_USER_ACTION)
    {
        scrollTextVertical(mScrollBarV->getScrollOffset());
        return GuiManager::EVENT_CHECK_DONE;
    }
    // MouseAction within the textarea?
    if (!mouseWithin(x, y))
        return GuiManager::EVENT_CHECK_NEXT;
    if (mouseWheel)
    {
        scrollTextVertical((mouseWheel>0)?-1:+1);
        return GuiManager::EVENT_CHECK_DONE;
    }
    if (MouseAction == GuiManager::BUTTON_RELEASED)
    {
        if (extractKeyword(x, y))
            return GuiManager::EVENT_USER_ACTION;
    }
    return GuiManager::EVENT_CHECK_DONE;
}

//================================================================================================
//
//================================================================================================
bool GuiListbox::extractKeyword(int mouseX, int mouseY)
{
    char key[] = { GuiTextout::TXT_CMD_LINK, GuiTextout::TXT_CMD_SEPARATOR, '\0'};
    int clickedLine = mMaxVisibleRows-(mouseY - mPosY)/ mFontHeight;
    clickedLine = (mActLines-clickedLine-mVScrollOffset) & (SIZE_STRING_BUFFER-1);
    mKeywordPressed  = row[clickedLine].keyword;
    if (mKeywordPressed.empty()) return false; // No keyword.
    String strLine = row[clickedLine].text;
    mouseX-=mPosX;
    size_t stringPosClicked = GuiTextout::getSingleton().getLastCharPosition(strLine.c_str(), mFontNr, mouseX);
    // ////////////////////////////////////////////////////////////////////
    // First look right of the click.
    // If there is a keyword-start-sign we are not within a keyword.
    // ////////////////////////////////////////////////////////////////////
    int testLine = clickedLine;
    size_t posKeySign = strLine.find(key[0], stringPosClicked);
    while (posKeySign ==  std::string::npos) // Because of clipping, a keyword can go over more then 1 row.
    {
        ++testLine &= (SIZE_STRING_BUFFER-1);
        if (row[testLine].keyword != mKeywordPressed) return false; // Already reached the next textline.
        strLine = row[testLine].text;
        posKeySign = strLine.find(key[0]);
    }
    // Found a keyword sign. If its a start sign, we are not over a keyword.
    if (strLine[posKeySign+1] == key[1]) return false;
    // ////////////////////////////////////////////////////////////////////
    // Now, that we know that right from the click is an end sign.
    // We must find the start sign left of the click.
    // ////////////////////////////////////////////////////////////////////
    strLine = row[clickedLine].text;
    posKeySign = strLine.rfind(key, stringPosClicked);
    while (posKeySign == std::string::npos) // Because of clipping, a keyword can go over more then 1 row.
    {
        --clickedLine &= (SIZE_STRING_BUFFER-1);
        if (row[clickedLine].keyword != mKeywordPressed) return false; // Already reached the next textline.
        strLine = row[clickedLine].text;
        posKeySign = strLine.rfind(key);
    }
    int nrKeyword = strLine[posKeySign+2] - '0';
    // ////////////////////////////////////////////////////////////////////
    // Now we know the number of the clicked keyword.
    // ////////////////////////////////////////////////////////////////////
    posKeySign = 0;
    while (--nrKeyword)
        posKeySign = mKeywordPressed.find(GuiTextout::TXT_CMD_SEPARATOR, posKeySign) +1;
    mKeywordPressed = mKeywordPressed.substr(posKeySign, mKeywordPressed.find(GuiTextout::TXT_CMD_SEPARATOR, posKeySign)-posKeySign);
    return true;
}

//================================================================================================
//
//================================================================================================
void GuiListbox::update(Ogre::Real dTime)
{
    if (!mRowsToPrint) return;
    --mRowsToPrint;
    draw();
}

//================================================================================================
// Display the textlines.
//================================================================================================
void GuiListbox::draw()
{
    uint32 *bak = mParent->getLayerBG() + mPosX + mPosY*mParent->getWidth();
    uint32 *dst = GuiManager::getSingleton().getBuildBuffer();
    GuiGraphic::getSingleton().restoreWindowBG(mWidth, mHeight, bak, dst, mParent->getWidth(), mWidth);
    int pos =0, offset = 0;
    for (int y = mActLines<mMaxVisibleRows?mMaxVisibleRows-mActLines:0; y < mMaxVisibleRows; ++y)
    {
        offset = y*mFontHeight;
        GuiTextout::getSingleton().printText(mWidth, mFontHeight, dst+offset*mWidth, mWidth, dst+offset*mWidth, mWidth,
                                             row[(mPrintPos-mVScrollOffset+pos)& (SIZE_STRING_BUFFER-1)].text.c_str(), mFontNr,
                                             row[(mPrintPos-mVScrollOffset+pos)& (SIZE_STRING_BUFFER-1)].color);
        ++pos;
    }
    mParent->getTexture()->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , dst /*+ mWidth * mPixelScroll*/),
        Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
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
    draw();
}
