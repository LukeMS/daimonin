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

class TextInput
{
  public:
	 TextInput() { stop(); };
	~TextInput() {};
    static TextInput &getSingleton()
	{
		static TextInput singleton;
		return singleton;
	}

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
	}

	const char *getString()
	{ 
		return mStrTextInput.c_str(); 
	}


	void start(int maxChars, bool useNumbers = true, bool useWhitespaces = true)
	{
		// we start only over, if the last operation was ended.
		if (mInProgress == true) { return; }
		mInProgress      = true;
		mFinished        = false;
		mCanceled        = false;
		mUseNumbers      = useNumbers;
		mUseWhitespaces  = useWhitespaces;
		mMaxChars    = maxChars;
	}

	void addChar(const char addChar )
	{ 
		if (mStrTextInput.size() >= mMaxChars) return;
		if (!mUseNumbers    && (addChar >= '0' && addChar <= '9')) return;
		if (!mUseWhitespaces && (addChar <'A' || addChar > 'z' 
			|| (addChar >'Z' && addChar < 'a'))	) return;
			
		mStrTextInput+= addChar;
		++mSize;
	}
	
	void delLastChar()
	{ 
		if (mStrTextInput.size())
		{ 
			mStrTextInput.resize(mStrTextInput.size()-1); 
			--mSize;
		}
	}

  private:
	bool mFinished, mCanceled, mInProgress;
	unsigned int  mMaxChars;
	int mSize;
	bool mUseNumbers;
	bool mUseWhitespaces;
    TextInput(const TextInput&); // disable copy-constructor.
	string mStrTextInput;
};

#endif
