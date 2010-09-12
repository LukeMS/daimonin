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

#include <iterator>
#include "network_serverfile.h"
#include "network.h"
#include "logger.h"
#include "profiler.h"
#include "zlib.h"

using namespace std;

//================================================================================================
// Constructor.
//================================================================================================
ServerFile::ServerFile()
{
    PROFILE()
    srv_file[FILE_SKILLS  ].filename = "";
    srv_file[FILE_SPELLS  ].filename = "";
    srv_file[FILE_SETTINGS].filename = "";
    srv_file[FILE_BMAPS   ].filename = "";
    srv_file[FILE_SKILLS  ].status = STATUS_OUTDATED;
    srv_file[FILE_SPELLS  ].status = STATUS_OUTDATED;
    srv_file[FILE_SETTINGS].status = STATUS_OUTDATED;
    srv_file[FILE_BMAPS   ].status = STATUS_OUTDATED;
}

//================================================================================================
// .
//================================================================================================
void ServerFile::checkFileStatus(char *param, int fileNr)
{
    PROFILE()
    Logger::log().attempt() << "Server file status of ["<< fileNr<< "] " << srv_file[fileNr].filename << "...";
    if (!strcmp((const char*)param, "OK"))
    {
        srv_file[fileNr].status = STATUS_OK;
    }
    else
    {
        int pos = 0;
        for (; param[pos]!='|'; ++pos) ;
        param[pos++] = '\0';
        srv_file[fileNr].status = STATUS_OUTDATED;
        setLength(fileNr, atoi(param));
        param+= pos;
        setCRC(fileNr, strtoul(param, 0, 16));
        Logger::log().error() << "";
    }
}

//================================================================================================
// Get length and checksum from (server sended) files.
//================================================================================================
void ServerFile::checkFile(eSeverfileNr nr, const char *filename)
{
    PROFILE()
    srv_file[nr].filename = filename;
    ifstream in(filename, ios::in | ios::binary);
    if (!in.is_open())
    {
        setCRC   (nr, 0);
        setLength(nr, 0);
    }
    else
    {
        ostringstream out(ios::binary);
        in.unsetf(ios::skipws); // don't skip whitespace  (!ios::skipws and ios::binary must be set).
        copy(istream_iterator<char>(in), istream_iterator<char>(), ostream_iterator<char>(out));
        setCRC   (nr, crc32(1L, (const unsigned char *)out.str().c_str(), (int)out.str().size()));
        setLength(nr, (int)out.str().size());
        in.close();
    }
}

//================================================================================================
// Request all files that have changed since last login.
//================================================================================================
bool ServerFile::requestFiles()
{
    PROFILE()
    for (unsigned char i = 0; i < FILE_SUM; ++i)
    {
        if (srv_file[i].status == STATUS_UPDATING) return false;
        if (srv_file[i].status != STATUS_OK)
        {
            std::stringstream strCmd;
            strCmd << i;
            Network::getSingleton().send_command_binary(Network::CLIENT_CMD_REQUESTFILE, strCmd);
            srv_file[i].status = STATUS_UPDATING;
            return false;
        }
    }
    return true;
}
