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

#ifndef NETWORK_ACCOUNT_H
#define NETWORK_ACCOUNT_H

#include <string>

/**
 ** .
 *****************************************************************************/
class NetworkAccount
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { ACCOUNT_MAX_PLAYER = 6 };
    typedef struct
    {
        std::string name;
        int  level;
        int  race;
        bool gender;
    } account;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static NetworkAccount &getSingleton()
    {
        static NetworkAccount Singleton; return Singleton;
    }

    void fillAccount(int count, const unsigned char *data);
    void clearAccount()
    {
        mSumEntries = 0;
    }
    void setSelected(int pos)
    {
        if (pos < ACCOUNT_MAX_PLAYER)  mSelected = pos;
    }
    int getSumChars() const
    {
        return mSumEntries;
    }
    const account *getAccountEntry(int pos)
    {
        if (pos < ACCOUNT_MAX_PLAYER)
            return &mAccountEntry[pos];
        return 0;
    }
    std::string getSelectedChar()
    {
        return mAccountEntry[mSelected].name;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    account mAccountEntry[ACCOUNT_MAX_PLAYER];
    int mSumEntries; /**< Number of chars already created. **/
    int mSelected;   /**< Actually selected character. **/


    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    NetworkAccount() { clearAccount(); }
    ~NetworkAccount() {}
    NetworkAccount(const NetworkAccount&);            /**< disable copy-constructor. **/
    NetworkAccount &operator=(const NetworkAccount&); /**< disable assignment operator. **/
};

#endif
