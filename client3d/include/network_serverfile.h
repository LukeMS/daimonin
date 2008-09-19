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

#ifndef SERVERFILES_H
#define SERVERFILES_H

#include "define.h"

/**
 ** This singleton class handles files send by the server.
 *****************************************************************************/
class ServerFile
{

public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        FILE_SKILLS,
        FILE_SPELLS,
        FILE_SETTINGS,
        FILE_ANIMS,
        FILE_BMAPS,
        FILE_SUM
    };

    enum
    {
        STATUS_OK,
        STATUS_OUTDATED,
        STATUS_UPDATING
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static ServerFile &getSingleton()
    {
        static ServerFile Singleton; return Singleton;
    }
    void checkFiles();

    int getStatus(int file_enum)
    {
        return srv_file[file_enum].status;
    }
    int getLength(int file_enum)
    {
        return srv_file[file_enum].length;
    }
    unsigned int getCRC(int file_enum)
    {
        return srv_file[file_enum].crc;
    }
    const char *getFilename(int file_enum)
    {
        return srv_file[file_enum].filename;
    }

    void setStatus(int file_enum, int value)
    {
        srv_file[file_enum].status = value;
    }
    void setLength(int file_enum, int value)
    {
        srv_file[file_enum].length = value;
    }
    void setCRC(int file_enum, unsigned int value)
    {
        srv_file[file_enum].crc = value;
    }
    bool requestFiles();

    void updateDone()
    {
        srv_file[mRequestFileChain].status = STATUS_OK;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    struct _srv_file
    {
        int          status;
        int          length;
        unsigned int crc;
        const char  *filename;
    }
    srv_file[FILE_SUM];
    int mRequestFileChain;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ServerFile();
    ~ServerFile()
    {}
    ServerFile(const ServerFile&); // disable copy-constructor.
    void getFileAttibutes(int file_enum);
};

#endif
