/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that t will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include <string>

using namespace std;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////
enum { INPUT_MODE_TEXT, INPUT_MODE_CURSOR_SELECTION, INPUT_MODE_SUM };

////////////////////////////////////////////////////////////
// Singleton class.
////////////////////////////////////////////////////////////
class TextInput
{
  public:
    ////////////////////////////////////////////////////////////
    // Functions.
    ////////////////////////////////////////////////////////////
     TextInput() { stop(); };
    ~TextInput() {};
    static TextInput &getSingleton() {    static TextInput singleton; return singleton; }

    void addString(string &addString) { mStrTextInput+= addString; }
    void setString(string &newString) { mStrTextInput = newString; }
    void clearText() { mStrTextInput = ""; }
    void finished()   { mFinished = true; };
    void canceled()   { mCanceled = true; };
    bool isFinished() { return mFinished; };
    bool isCanceled() { return mCanceled; };
    int  size()       { return mSize;     };

    void stop()
    {
        mStrTextInput= "";
        mSize =0;
        mInProgress  = false;
        mChange = false;
    }

    const char *getString()
    {
        return mStrTextInput.c_str();
    }


    void startTextInput(int maxChars, bool useNumbers = true, bool useWhitespaces = true)
    {
        // we start only over, if the last operation was ended.
        if (mInProgress == true) { return; }
        mInProgress      = true;
        mInputMode = INPUT_MODE_TEXT;
        mFinished        = false;
        mCanceled        = false;
        mUseNumbers      = useNumbers;
        mUseWhitespaces  = useWhitespaces;
        mMaxChars    = maxChars;
    }


    //=================================================================================================
    // Init a cursor selection input.
    // Returns true if it is the first call (will be used for init overlay elements only once).
    //=================================================================================================
    bool startCursorSelection(unsigned int minValue, unsigned int maxValue, unsigned int startValue=0)
    {
        if (mInProgress == true) { return false; }
        mInProgress      = true;
        mInputMode = INPUT_MODE_CURSOR_SELECTION;
        mFinished        = false;
        mCanceled        = false;
        mChange =true;
        mActValue = startValue;
        mMaxValue = maxValue-1;
        mMinValue = minValue;
        return true;
    }

    void keyEvent(const unsigned char keyChar, const unsigned char key)
    {
        if (mInputMode == INPUT_MODE_TEXT)
        {
            if (!keyChar || mStrTextInput.size() >= mMaxChars) return;
            if (!mUseNumbers    && (keyChar >= '0' && keyChar <= '9')) return;
            if (!mUseWhitespaces && (keyChar <'A' || keyChar > 'z'
                || (keyChar >'Z' && keyChar < 'a'))    ) return;
            mStrTextInput+= keyChar;
            ++mSize;
        }
        else
        {
            unsigned int change = mActValue;
            if        (key == 0xC8) { if (mActValue > mMinValue) --mActValue; } // cursor up.
            else if    (key == 0xD0) { if (mActValue < mMaxValue) ++mActValue; } // cursor down.
            if (change != mActValue) mChange =true;
        }
    }

    void backspace()
    {
        if (mInputMode == INPUT_MODE_TEXT && mStrTextInput.size())
        {
            mStrTextInput.resize(mStrTextInput.size()-1);
            --mSize;
        }
    }

    int getInputMode() { return mInputMode; }
    unsigned int getSelCursorPos() { return mActValue;  }
    bool getChange()
    {
        if (!mChange) { return false; }
        mChange = false;
        return true;
    }



  private:
    ////////////////////////////////////////////////////////////
    // Variables.
    ////////////////////////////////////////////////////////////
    bool mChange;
    unsigned int mMinValue, mActValue, mMaxValue;
    bool mFinished, mCanceled, mInProgress;
    unsigned int  mMaxChars;
    int mSize;
    bool mUseNumbers;
    int mInputMode;
    bool mUseWhitespaces;
    string mStrTextInput;

    ////////////////////////////////////////////////////////////
    // Functions.
    ////////////////////////////////////////////////////////////
    TextInput(const TextInput&); // disable copy-constructor.
};

#endif
