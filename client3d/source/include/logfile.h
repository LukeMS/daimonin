/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2004 The Daimonin Team
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

#ifndef LOGFILE_H
#define LOGFILE_H

#include <ctime>
#include <cstdio>
#include <cstdarg>

const int LOG_BUFFER_SIZE  = 255;

class LogFile
{
  public:
	LogFile()   {};
    ~LogFile();
    static LogFile &getSingleton();

    void Info    (char *text, ...);
    void Error   (char *text, ...);
    void Success (bool status);
    void Headline(const char *text);
    bool Init    ();
  private:
    time_t m_time;
    tm *m_localtime;
    char  m_buffer[LOG_BUFFER_SIZE+1];
    FILE *m_stream;

    void write(const char *color);
    LogFile(const LogFile&); // disable copy-constructor.
};

#endif
