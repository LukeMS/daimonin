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

#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include <Ogre.h>
#include <OISKeyboard.h>
#include "sound.h"

/**
 ** TextInput class which manages the keyboard input for the Dialog class.
 *****************************************************************************/
class GuiTextinput
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        INPUT_MODE_TEXT,             /**< Input modus: text. **/
        INPUT_MODE_CURSOR_SELECTION, /**< Input modus: move cursor in a selection field. **/
        INPUT_MODE_SUM               /**< Sum of input modes **/
    };

    enum { CURSOR_FREQUENCY = 500 };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static GuiTextinput &getSingleton()
    {
        static GuiTextinput singleton; return singleton;
    }
    void addString(Ogre::String &addString)
    {
        mStrTextInput+= addString;
    }
    void setString(Ogre::String &newString)
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
        static clock_t time = Ogre::Root::getSingleton().getTimer()->getMilliseconds();
        static bool cursorOn = true;
        if (!showTextCursor || mFinished || mCanceled)
        {
            return mStrTextInput.c_str();
        }
        mStrTextInputWithCursor = mStrTextInput;
        if (Ogre::Root::getSingleton().getTimer()->getMilliseconds() - time > CURSOR_FREQUENCY)
        {
            time =  Ogre::Root::getSingleton().getTimer()->getMilliseconds();
            cursorOn = !cursorOn;
        }
        if (cursorOn)
            mStrTextInputWithCursor.insert(mCursorPos, GuiTextout::CURSOR);
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
    void keyEvent(const int key, const unsigned int keyChar)
    {
        if (key == OIS::KC_RETURN || key == OIS::KC_TAB)
        {
            finished();
            return;
        }
        if (key == OIS::KC_ESCAPE)
        {
            canceled();
            return;
        }
        // ////////////////////////////////////////////////////////////////////
        // Input modus TEXT.
        // ////////////////////////////////////////////////////////////////////
        if (mInputMode == INPUT_MODE_TEXT)
        {
            if (key == OIS::KC_BACK && mCursorPos > 0)
            {
                mStrTextInput.erase(--mCursorPos, 1);
                return;
            }
            if (key == OIS::KC_DELETE)
            {
                mStrTextInput.erase(mCursorPos, 1);
                return;
            }
            if (key == OIS::KC_LEFT && mCursorPos > 0)
            {
                --mCursorPos;
                return;
            }
            if (key == OIS::KC_RIGHT && mCursorPos < mStrTextInput.size())
            {
                ++mCursorPos;
                return;
            }
            if ((!keyChar || mStrTextInput.size() >= mMaxChars)
                    || (mBlockNumbers  && (keyChar >= '0' && keyChar <= '9'))
                    || (mBlockWhiteSpace && (keyChar <'A' || keyChar > 'z')))
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
            if (key == OIS::KC_UP && mActValue > 0)
                --mActValue;
            else if (key == OIS::KC_DOWN && mActValue < mMaxValue)
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
    unsigned int mActValue, mMaxValue;
    unsigned int mMaxChars;
    unsigned int mCursorPos;
    int  mInputMode;
    bool mChange;
    bool mFinished, mCanceled, mInProgress;
    bool mBlockNumbers, mBlockWhiteSpace;
    Ogre::String mStrTextInput, mStrTextInputWithCursor;

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
