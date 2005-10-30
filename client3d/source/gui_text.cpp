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

#include <OgreHardwareBuffer.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_text.h"
#include "logger.h"
#include <ctime>

const int MAX_TEXTLINE_LEN = 1024;

const uint32 TXT_COLOR_DEFAULT = 0x00ffffff;
const uint32 TXT_COLOR_RED     = 0x00ff0000;
const uint32 TXT_COLOR_GREEN   = 0x0000ff00;
const uint32 TXT_COLOR_BLUE    = 0x000000ff;

const char TXT_CMD_HIGHLIGHT   = '~';
const char TXT_CMD_LOWLIGHT    = '°';
const char TXT_CMD_LINK        = '^';
const char TXT_SUB_CMD_VALUE32 = '#';
const char TXT_SUB_CMD_VALUE08 = '@';

enum
{
  TXT_STATE_HIGHLIGHT =1,
  TXT_STATE_LOWLIGHT,
  TXT_STATE_LINK,
  TXT_STATE_SUM
};

///=================================================================================================
/// Constructor.
///=================================================================================================
GuiTextout::GuiTextout()
{
  maxFontHeight = 0;
  maxFontWidth  = 0;
  mSumFonts = -1;
  loadRawFont("font_12.png");
  loadRawFont("font_16.png");
  loadRawFont("font_16.png");
  TextGfxBuffer = new uint32[maxFontHeight * MAX_TEXTLINE_LEN];
}

///=================================================================================================
/// Destructor.
///=================================================================================================
GuiTextout::~GuiTextout()
{
  delete[] TextGfxBuffer;
}

///=================================================================================================
/// Load a RAW font into main memory.
///=================================================================================================
void GuiTextout::loadRawFont(const char *filename)
{
  static int mSumFonts=-1;
  if (++mSumFonts >= FONT_SUM) return;
  mFont[mSumFonts].image.load(filename, "General");
  mFont[mSumFonts].data = (uint32*)mFont[mSumFonts].image.getData();
  mFont[mSumFonts].height = mFont[mSumFonts].image.getHeight() -1;
  if (mFont[mSumFonts].height > maxFontHeight)  maxFontHeight = mFont[mSumFonts].height;
  mFont[mSumFonts].textureWidth = mFont[mSumFonts].image.getWidth();
  mFont[mSumFonts].width  = mFont[mSumFonts].image.getWidth() / CHARS_IN_FONT;
  if (mFont[mSumFonts].width > maxFontWidth)  maxFontWidth = mFont[mSumFonts].width;
  unsigned int x;
  for (int i=0; i < CHARS_IN_FONT; ++i)
  {
    for (x=0; x < mFont[mSumFonts].width-1; ++x)
    {
      if (mFont[mSumFonts].data[x+i*mFont[mSumFonts].width] == 0xff00ff00) break;
    }
    mFont[mSumFonts].charWidth[i] = x;
  }
}

///=================================================================================================
/// Load a TTF into main memory.
///=================================================================================================
void GuiTextout::loadTTFont(const char *filename)
{
// TODO
/*
  // TTF Fonts to bitmap.
  FontPtr testFont = FontManager::getSingleton().getByName(filename);
  Material *material = testFont.getPointer()->getMaterial().getPointer();
  std::string strTexture = material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
  TexturePtr ptexture = TextureManager::getSingleton().getByName(strTexture);
  Texture *texture = ptexture.getPointer();

  int h = texture->getHeight();
  for (int i = 33 i < 128; ++i)
  {
    // Blit every char into the new structure.
  }
*/
}

///=================================================================================================
/// Print a dynamic text.
///=================================================================================================
void GuiTextout::Print(TextLine *line, Texture *texture, const char *text)
{
  int fontNr = 0;
  // Restore background.
  int y =0;
  for (unsigned int j =0; j < mFont[fontNr].height; ++j)
  {
    for (int x=0; x < line->width; ++x)
    {
      TextGfxBuffer[x + y] = line->BG_Backup[x + y];
    }
    y += line->width;
  }
  if (!text || text[0] == 0) return;
  // draw the text into buffer.
  drawText(line->width, TextGfxBuffer, text);
  // Blit it into the window.
  texture->getBuffer()->blitFromMemory(
    PixelBox(line->width, mFont[fontNr].height, 1, PF_A8R8G8B8 , TextGfxBuffer),
    Box(line->x, line->y, line->x+line->width, line->y+mFont[fontNr].height));
}

///=================================================================================================
/// Print a static text.
///=================================================================================================
void GuiTextout::Print(int x, int y, int gfxLen, Texture *texture, const char *text)
{
  if (!text || text[0] == 0) return;
  int fontNr = 0;
  int x2 = x + strlen(text) * (mFont[fontNr].width +1);
  if (x2 > x+gfxLen) x2 = x+gfxLen;
  PixelBox pb = texture->getBuffer()->lock(Box(x, y, x2, y+mFont[fontNr].height), HardwareBuffer::HBL_READ_ONLY );
  drawText(texture->getWidth(), (uint32*)pb.data, text);
  texture->getBuffer()->unlock();
}

///=================================================================================================
/// Print to a buffer.
///=================================================================================================
void GuiTextout::PrintToBuffer(int width, uint32 *dest_data, const char*text, uint32 bgColor)
{
  int fontNr = 0;
  /// Clear the textline.
  for (unsigned int i =0; i < width * mFont[fontNr].height; ++i) dest_data[i] = bgColor;
  if (!text || text[0] == 0) return;
  drawText(width, dest_data, text);
}

///=================================================================================================
/// .
///=================================================================================================
void GuiTextout::drawText(int width, uint32 *dest_data, const char*text)
{
  int fontPosX;
  int fontNr = 0;
  uint32 pixFont, pixColor;
  uint32 color = TXT_COLOR_DEFAULT;
  int state = 0;

  while (*text)
  {
    // Parse format commands.
    switch (*text)
    {
      case TXT_CMD_HIGHLIGHT:
        if (!*(++text)) return;
        state^= TXT_STATE_HIGHLIGHT;
        if (state & TXT_STATE_HIGHLIGHT)
        {
          /// Parse the highlight color (8 byte hex string to uint32).
          if (*text == TXT_SUB_CMD_VALUE32)
          {
            color =0;
            for (int i = 28; i>=0; i-=4)
            {
              color += (*(++text) >='a') ? (*text - 87) <<i : (*text >='A') ? (*text - 55) <<i :(*text -'0') <<i;
            }
            ++text;
          }
          /// Use standard highlight color.
          else color = 0x0000ff00;
        }
        else color = TXT_COLOR_DEFAULT;
        break;
      default:
        fontPosX = (*text - 32) * mFont[fontNr].width;
        for (unsigned int y =0; y < mFont[fontNr].height; ++y)
        {
          for (int x=0; x < mFont[fontNr].charWidth[*text -32]; ++x)
          { /// PixelFormat: A8 R8 G8 B8.
            if ((pixFont = mFont[fontNr].data[y*mFont[fontNr].textureWidth + fontPosX + x]) == 0xff000000 ) continue;
            pixColor = pixFont & 0xff000000;
            pixColor+= ((color&0x0000ff) < (pixFont& 0x0000ff))? color & 0x0000ff : pixFont & 0x0000ff;
            pixColor+= ((color&0x00ff00) < (pixFont& 0x00ff00))? color & 0x00ff00 : pixFont & 0x00ff00;
            pixColor+= ((color&0xff0000) < (pixFont& 0xff0000))? color & 0xff0000 : pixFont & 0xff0000;
            dest_data[y*width + x] = pixColor;
          }
        }
        dest_data += mFont[fontNr].charWidth[*text++ - 32] +1;
        break;
    }
  }
}








#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include <ctime>
#include <string>
#include "sound.h"

using namespace std;

////////////////////////////////////////////////////////////
/// Defines.
////////////////////////////////////////////////////////////
enum
{
  INPUT_MODE_TEXT,             /**< Input modus: text. **/
  INPUT_MODE_CURSOR_SELECTION, /**< Input modus: move cursor in a selection field. **/
  INPUT_MODE_SUM               /**< Sum of input modes **/
};

/**
 ** TextInput class which manages the keyboard input for the Dialog class.
 *****************************************************************************/
class TextInput
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  static TextInput &getSingleton()
  {
    static TextInput singleton; return singleton;
  }
  void addString(string &addString)
  {
    mStrTextInput+= addString;
  }
  void setString(string &newString)
  {
    mStrTextInput = newString;
  }
  /** Clears the the input text. **/
  void clearText()
  {
    mStrTextInput = "";
  }
  /** Finish the text input **/
  void finished()
  {
    mFinished = true;
  };
  /** Cancel the text input **/
  void canceled()
  {
    mCanceled = true;
  };
  bool wasFinished()
  {
    return mFinished;
  };
  bool wasCanceled()
  {
    return mCanceled;
  };
  int size()
  {
    return mStrTextInput.size();
  };
  void stop()
  {
    mStrTextInput= "";
    mCursorPos   = 0;
    mInProgress  = false;
    mChange      = false;
  }
  /**
   ** Returns the input text.
   ** @param showTextCursor Shows the cursor within the text.
   *****************************************************************************/

  const char *getText(bool showTextCursor = false)
  {
    static clock_t time = clock();
    static bool cursorOn = true;
    if (!showTextCursor || mFinished || mCanceled) return mStrTextInput.c_str();
    mStrTextInputWithCursor = mStrTextInput;
    /////////////////////////////////////////////////////////////////////////
    /// For Cursor blinking "|" and " " must have the same size.
    /// Can be finetuned by the space_width setting in TextAreaOverlay.
    /////////////////////////////////////////////////////////////////////////
    if (clock()-time > 500)
    {
      time = clock();
      cursorOn = !cursorOn;
    }
    if (cursorOn)
      mStrTextInputWithCursor.insert(mCursorPos, "|");
    else
      mStrTextInputWithCursor.insert(mCursorPos, " ");
    return mStrTextInputWithCursor.c_str();
  }

  /**
   ** Inits a text input session.
   ** @param maxChars Maximuum text input length.
   ** @param useNumbers Input of numbers is allowed.
   ** @param useWhitespaces Input of whhitespaces is allowed.
   *****************************************************************************/
  void startTextInput(int maxChars, bool useNumbers = true, bool useWhitespaces = true)
  {
    // we start only over, if the last operation was ended.
    if (mInProgress == true) return;
    mInProgress  = true;
    mInputMode   = INPUT_MODE_TEXT;
    mFinished    = false;
    mCanceled    = false;
    mUseNumbers  = useNumbers;
    mUseWhiteSpc = useWhitespaces;
    mMaxChars    = maxChars;
  }

  /**
   ** Inits a cursor selection session, cursor can be moved up and down
   ** to select an entry in the selection field.
   ** @param sizeSeletionField The number of selectable entries in the selection field.
   ** @param actualSelectedPos The initial position of the selection cursor.
   ** @return false if there is already a cursor selection running.
   *****************************************************************************/
  bool startCursorSelection(unsigned int sizeSeletionField, unsigned int actualSelectedPos=0)
  {
    if (mInProgress == true) return false;
    mInProgress = true;
    mInputMode  = INPUT_MODE_CURSOR_SELECTION;
    mFinished   = false;
    mCanceled   = false;
    mChange     = true;
    mMaxValue   = sizeSeletionField-1;
    if (mMaxValue > MAX_SELECTION_ENTRYS) mMaxValue = MAX_SELECTION_ENTRYS;
    mActValue   = actualSelectedPos;
    if (mActValue > mMaxValue) mActValue = mMaxValue;
    return true;
  }

  /**
   ** Process a key event.
   ** @param keyChar The ascii value of the pressed key.
   ** @param key The The keycode of the key.
   *****************************************************************************/
  void keyEvent(const char keyChar, const unsigned char key)
  {
    if (key == KEY_RETURN || key == KEY_TAB)
    {
      TextInput::getSingleton().finished();
      return;
    }
    if (key == KEY_ESCAPE)
    {
      TextInput::getSingleton().canceled();
      return;
    }
    /////////////////////////////////////////////////////////////////////////
    /// Input modus TEXT.
    /////////////////////////////////////////////////////////////////////////
    if (mInputMode == INPUT_MODE_TEXT)
    {
      if (key == KEY_BACKSPACE && mCursorPos > 0)
      {
        mStrTextInput.erase(--mCursorPos, 1);
        return;
      }
      if (key == KEY_DELETE)
      {
        mStrTextInput.erase(mCursorPos, 1);
        return;
      }
      if (key == KEY_LEFT && mCursorPos > 0)
      {
        --mCursorPos;
        return;
      }
      if (key == KEY_RIGHT && mCursorPos < mStrTextInput.size())
      {
        ++mCursorPos;
        return;
      }
      if ((!keyChar || mStrTextInput.size() >= mMaxChars)
              || (keyChar == '_' ) // used for TextCursor.
              || (!mUseNumbers  && (keyChar >= '0' && keyChar <= '9'))
              || (!mUseWhiteSpc && (keyChar <'A' || keyChar > 'z' || (keyChar >'Z' && keyChar < 'a'))))
      {
        Sound::getSingleton().playSample(SAMPLE_BUTTON_CLICK);
        return;
      }
      mStrTextInput.insert(mCursorPos,1,keyChar);
      ++mCursorPos;
    }
    /////////////////////////////////////////////////////////////////////////
    /// Input modus CURSOR-SELECTION.
    /////////////////////////////////////////////////////////////////////////
    else
    {
      unsigned int oldValue = mActValue;
      if (key == KEY_UP && mActValue > 0)
        --mActValue;
      else if (key == KEY_DOWN && mActValue < mMaxValue)
        ++mActValue;
      if (oldValue != mActValue) mChange =true;
    }
  }

  int getInputMode()
  {
    return mInputMode;
  }
  unsigned int getSelCursorPos()
  {
    return mActValue;
  }

  /**
   ** Determine if the cursor has moved in the selection field.
   ** @return false if there was no cursor movement.
   *****************************************************************************/
  bool hasChanged()
  {
    if (!mChange) return false;
    mChange = false;
    return true;
  }

private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  /** The maximum number of selectable entries for a selection field. **/
  static const unsigned int MAX_SELECTION_ENTRYS = 20;
  enum
  {
    KEY_RETURN = 0x1C, KEY_TAB = 0x0F, KEY_DELETE= 0xD3, KEY_BACKSPACE = 0x0E,
    KEY_ESCAPE = 0x01, KEY_LEFT= 0xCB, KEY_RIGHT = 0xCD, KEY_UP =0xC8,  KEY_DOWN =0xD0
  };
  unsigned int mActValue, mMaxValue;
  unsigned int mMaxChars;
  unsigned int mCursorPos;
  int  mInputMode;
  bool mChange;
  bool mFinished, mCanceled, mInProgress;
  bool mUseNumbers;
  bool mUseWhiteSpc;
  string mStrTextInput, mStrTextInputWithCursor;

  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  TextInput()
{}
  ~TextInput()
  {}
  TextInput(const TextInput&);
};

#endif
