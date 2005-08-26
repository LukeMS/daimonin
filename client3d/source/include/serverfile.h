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
 
You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef SERVERFILES_H
#define SERVERFILES_H

#include <string>
#include "define.h"

////////////////////////////////////////////////////////////
/// Defines.
////////////////////////////////////////////////////////////
enum
{
  SERVER_FILE_SKILLS,
  SERVER_FILE_SPELLS,
  SERVER_FILE_SETTINGS,
  SERVER_FILE_ANIMS,
  SERVER_FILE_BMAPS,
  SERVER_FILE_SUM
};

enum
{
  SERVER_FILE_STATUS_OK,
  SERVER_FILE_STATUS_UPDATE,
};

////////////////////////////////////////////////////////////
// Singleton class.
////////////////////////////////////////////////////////////
class ServerFile
{
public:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  int          getStatus   (int file_enum)
  {
    return srv_file[file_enum].status;
  }
  int          getLength   (int file_enum)
  {
    return srv_file[file_enum].length;
  }
  int          getSrvLength(int file_enum)
  {
    return srv_file[file_enum].server_len;
  }
  unsigned int getSrvCRC   (int file_enum)
  {
    return srv_file[file_enum].server_crc;
  }
  unsigned int getCRC      (int file_enum)
  {
    return srv_file[file_enum].crc;
  }
  const char*  getFilename (int file_enum)
  {
    return srv_file[file_enum].filename;
  }

  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  static ServerFile &getSingleton()
  {
    static ServerFile Singleton; return Singleton;
  }
  bool Init();
  void checkFiles();
  void  setStatus   (int file_enum, int value)
  {
    srv_file[file_enum].status     = value;
  }
  void  setLength   (int file_enum, int value)
  {
    srv_file[file_enum].length     = value;
  }
  void  setSrvLength(int file_enum, int value)
  {
    srv_file[file_enum].server_len = value;
  }
  void  setSrvCRC   (int file_enum, unsigned int value)
  {
    srv_file[file_enum].server_crc = value;
  }
  void  setCRC      (int file_enum, unsigned int value)
  {
    srv_file[file_enum].crc        = value;
  }
  bool checkID(int file_enum, const char* id)
  {
    return (srv_file[file_enum].strID == id);
  }


private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  struct _srv_file
  {
    int          status;
    int          length;
    unsigned int crc;
    int          server_len;
    unsigned int server_crc;
    const char  *filename;
    std::string strID;
  }
  srv_file[SERVER_FILE_SUM];

  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  ServerFile()
  {
    srv_file[SERVER_FILE_SKILLS  ].filename = FILE_CLIENT_SKILLS;
    srv_file[SERVER_FILE_SKILLS  ].strID    = "skf";
    srv_file[SERVER_FILE_SPELLS  ].filename = FILE_CLIENT_SPELLS;
    srv_file[SERVER_FILE_SPELLS  ].strID    = "spf";
    srv_file[SERVER_FILE_SETTINGS].filename = FILE_CLIENT_SETTINGS;
    srv_file[SERVER_FILE_SETTINGS].strID    = "stf";
    srv_file[SERVER_FILE_ANIMS   ].filename = FILE_CLIENT_ANIMS;
    srv_file[SERVER_FILE_ANIMS   ].strID    = "amf";
    srv_file[SERVER_FILE_BMAPS   ].filename = FILE_CLIENT_BMAPS;
    srv_file[SERVER_FILE_BMAPS   ].strID    = "bpf";
  }
  ~ServerFile()
  {}
  ServerFile(const ServerFile&); // disable copy-constructor.

  void getFileAttibutes(int file_enum);
};

#endif
