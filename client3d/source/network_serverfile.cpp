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

#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include "network_serverfile.h"
#include "network.h"
#include "logger.h"
#include "define.h"
#include "zlib.h"

using namespace std;

//================================================================================================
// Constructor.
//================================================================================================
ServerFile::ServerFile()
{
    mRequestFileChain = FILE_SKILLS;
    srv_file[FILE_SKILLS  ].filename = FILE_CLIENT_SKILLS;
    srv_file[FILE_SPELLS  ].filename = FILE_CLIENT_SPELLS;
    srv_file[FILE_SETTINGS].filename = FILE_CLIENT_SETTINGS;
    srv_file[FILE_ANIMS   ].filename = FILE_CLIENT_ANIMS;
    srv_file[FILE_BMAPS   ].filename = FILE_CLIENT_BMAPS;
}

//================================================================================================
// Get length and checksum from (server sended) files.
//================================================================================================
void ServerFile::getFileAttibutes(int file_enum)
{
    setStatus(file_enum, STATUS_OK);
    setLength(file_enum, STATUS_OK);
    setCRC   (file_enum, STATUS_OK);
    ifstream in(srv_file[file_enum].filename, ios::in | ios::binary);
    Logger::log().info()  << "- Reading Attributes from " << srv_file[file_enum].filename << "...";
    if (!in.is_open())
    {
        Logger::log().success(false);
        //Logger::log().error()  << "Can't open file '" << srv_file[file_enum].filename << "'.";
        return;
    }
    Logger::log().success(true);

    ostringstream out(ios::binary);
    in.unsetf(ios::skipws); // don't skip whitespace  (!ios::skipws and ios::binary must be set).
    copy(istream_iterator<char>(in), istream_iterator<char>(), ostream_iterator<char>(out));
    setCRC   (file_enum, crc32(1L, (const unsigned char *)out.str().c_str(), (int) out.str().size()));
    setLength(file_enum, (int)out.str().size());
}

//================================================================================================
// Get length and checksum from (server sended) files.
//================================================================================================
void ServerFile::checkFiles()
{
    Logger::log().info() << "Checking all files coming from server:";
    for (int i=0; i< FILE_SUM; ++i) getFileAttibutes(i);
}

//================================================================================================
// Request all files that have changed since last login.
//================================================================================================
bool ServerFile::requestFiles()
{
    // File is upToDate.
    while (srv_file[mRequestFileChain].status == STATUS_OK)
    {
        if (++mRequestFileChain >= FILE_SUM)
        {
            mRequestFileChain = FILE_SKILLS;
            return true;
        }
    }
    // File is outdated, ask server for the latest version.
    if (srv_file[mRequestFileChain].status == STATUS_OUTDATED)
    {
        std::stringstream strCmd;
        strCmd << "rf " << mRequestFileChain;
        //Network::getSingleton().cs_write_string(strCmd.str().c_str());
        srv_file[mRequestFileChain].status = STATUS_UPDATING;
    }
    return false;
}


