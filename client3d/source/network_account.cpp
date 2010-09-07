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

#include "network_account.h"
#include "logger.h"
#include "profiler.h"

//================================================================================================
//
//================================================================================================
void NetworkAccount::fillAccount(int len, const unsigned char *data)
{
    PROFILE()
    int i=0;
    for (mSumEntries = 0; mSumEntries < NetworkAccount::ACCOUNT_MAX_PLAYER; ++mSumEntries)
    {
        if (i >= len || !data[++i]) break;
        mAccountEntry[mSumEntries].name = (const char*)data+i;
        i+= (int)mAccountEntry[mSumEntries].name.size();
        mAccountEntry[mSumEntries].level  = data[++i];
        mAccountEntry[mSumEntries].race   = data[++i];
        mAccountEntry[mSumEntries].gender = data[++i]?true:false;
    }
}
