/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include "serverfile.h"
#include "logfile.h"
#include "define.h"
#include "zlib.h"

using namespace std;

//=================================================================================================
// Get length and checksum from (server sended) files.
//=================================================================================================
void ServerFile::getFileAttibutes(int file_enum)
{
    setStatus(file_enum, SERVER_FILE_STATUS_OK);
	setLength(file_enum, SERVER_FILE_STATUS_OK);
	setCRC   (file_enum, SERVER_FILE_STATUS_OK);

    LogFile::getSingleton().Info("- Reading Attributes from %s...", srv_file[file_enum].filename);
    ifstream in(srv_file[file_enum].filename, ios::in|ios::binary);
    if (!in)
	{ 
        LogFile::getSingleton().Info("File not found.\n");
		return;
	} 
	ostringstream out(ios::binary);
	in.unsetf(ios::skipws); // don't skip whitespace  (!ios::skipws and ios::binary must be set).
    copy(istream_iterator<char>(in), istream_iterator<char>(), ostream_iterator<char>(out));
    setCRC   (file_enum, crc32(1L, (const unsigned char *)out.str().c_str(),  out.str().size()));
    setLength(file_enum, out.str().size());
    LogFile::getSingleton().Info("(Size: %d)l\n", out.str().size(), srv_file[file_enum].length);    
}

//=================================================================================================
// Get length and checksum from (server sended) files.
//=================================================================================================
void ServerFile::checkFiles()
{
    LogFile::getSingleton().Info("Checking all files coming from server:\n");
    for (int i=0; i< SERVER_FILE_SUM; i++)
	{ 
		getFileAttibutes(i);
	}
    LogFile::getSingleton().Info("\n");
}

//=================================================================================================
// Return the instance.
//=================================================================================================
ServerFile &ServerFile::getSingleton()
{
   static ServerFile Singleton;
   return Singleton;
}
