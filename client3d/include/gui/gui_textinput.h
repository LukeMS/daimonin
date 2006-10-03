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

#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include <string>
#include "sound.h"

using namespace std;

enum
{
    INPUT_MODE_TEXT,             /**< Input modus: text. **/
    INPUT_MODE_CURSOR_SELECTION, /**< Input modus: move cursor in a selection field. **/
    INPUT_MODE_SUM               /**< Sum of input modes **/
};

const int CURSOR_FREQUENCY = 500;
const char CURSOR[] =
    {
        CHARS_IN_FONT+31, 0
    };

/**
 ** TextInput class which manages the keyboard input for the Dialog class.
 *****************************************************************************/
class GuiTextinput
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiTextinput &getSingleton()
    {
        static GuiTextinput singleton; return singleton;
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
        return (int) mStrTextInput.size();
    };
    void reset()
    {
        stop();
        mCanceled = false;
    }
    /**
     ** Important: stop() must be called when text input is done.
     *****************************************************************************/
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
    const char *getText(bool hideText = false, bool showTextCursor = true)
    {
        static clock_t time = Root::getSingleton().getTimer()->getMilliseconds();
        static bool cursorOn = true;
        if (!showTextCursor || mFinished || mCanceled)
        {
            return mStrTextInput.c_str();
        }
        mStrTextInputWithCursor = mStrTextInput;
        if (clock()-time > 500)
        {
            time = clock();
            cursorOn = !cursorOn;
        }
        if (cursorOn)
            mStrTextInputWithCursor.insert(mCursorPos, CURSOR);
        else
            mStrTextInputWithCursor.insert(mCursorPos, " ");
        return mStrTextInputWithCursor.c_str();
    }
    /**
     ** Inits a text input session.
     ** Important: stop() must be called when text input is done.
     ** @param maxChars Maximum text input length.
     ** @param blockNumbers Input of numbers will be blocked.
     ** @param blockWhitespaces Input of whitespaces will be blocked.
     *****************************************************************************/
    void startTextInput(int maxChars, bool blockNumbers, bool blockWhitespaces)
    {
        // we start only over, if the last operation was ended.
        if (mInProgress == true) return;
        mInProgress  = true;
        mInputMode   = INPUT_MODE_TEXT;
        mFinished    = false;
        mCanceled    = false;
        mBlockNumbers  = blockNumbers;
        mBlockWhiteSpace = blockWhitespaces;
        mMaxChars    = maxChars;
        mCursorPos   = (int)mStrTextInput.size();
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
        if (mMaxValue > MAX_SELECTION_ENTRIES) mMaxValue = MAX_SELECTION_ENTRIES;
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
            finished();
            return;
        }
        if (key == KEY_ESCAPE)
        {
            canceled();
            return;
        }
        // ////////////////////////////////////////////////////////////////////
        // Input modus TEXT.
        // ////////////////////////////////////////////////////////////////////
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
                    || (mBlockNumbers  && (keyChar >= '0' && keyChar <= '9'))
                    || (mBlockWhiteSpace && (keyChar <'A' || keyChar > 'z' || (keyChar >'Z' && keyChar < 'a'))))
            {
                Sound::getSingleton().playStream(Sound::BUTTON_CLICK);
                return;
            }
            mStrTextInput.insert(mCursorPos,1,keyChar);
            ++mCursorPos;
        }
        // ////////////////////////////////////////////////////////////////////
        // Input modus CURSOR-SELECTION.
        // ////////////////////////////////////////////////////////////////////
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
    // ////////////////////////////////////////////////////////////////////
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    /** The maximum number of selectable entries for a selection field. **/
    static const unsigned int MAX_SELECTION_ENTRIES = 20;
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
    bool mBlockNumbers, mBlockWhiteSpace;
    string mStrTextInput, mStrTextInputWithCursor;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    GuiTextinput()
{}
    ~GuiTextinput()
    {}
    GuiTextinput(const GuiTextinput&);
};

#endif
