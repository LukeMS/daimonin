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

#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include "network_serverfile.h"
#include "logger.h"
#include "define.h"
#include "zlib.h"

using namespace std;

//================================================================================================
// Get length and checksum from (server sended) files.
//================================================================================================
void ServerFile::getFileAttibutes(int file_enum)
{
    setStatus(file_enum, SERVER_FILE_STATUS_OK);
    setLength(file_enum, SERVER_FILE_STATUS_OK);
    setCRC   (file_enum, SERVER_FILE_STATUS_OK);
    ifstream in(srv_file[file_enum].filename, ios::in | ios::binary);
    Logger::log().info()  << "- Reading Attributes from " << srv_file[file_enum].filename << "...";
    if (!in.is_open())
    {
        Logger::log().success(false);
        Logger::log().error()  << "Can't open file '" << srv_file[file_enum].filename << "'.";
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
    for (int i=0; i< SERVER_FILE_SUM; i++) getFileAttibutes(i);
}
