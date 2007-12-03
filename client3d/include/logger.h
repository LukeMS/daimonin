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

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include "define.h"

/**
 ** This class provides logging facility.
 ** Logs are stored on a file in HTML format.
 *****************************************************************************/
class Logger
{
public:
    /**
     ** This static method returns a reference to
     ** a static Logger object. This allows users to
     ** use this class in a singleton-like manner.
     ** Note that the constructor of this static Logger
     ** object is called automatically the first time this
     ** method is invoked.
     ** @return Reference to a static singleton Logger object.
     *****************************************************************************/
    static Logger &log()
    {
        static Logger Singleton; return Singleton;
    }
    /**
     ** The constructor create the log file and initalizes it
     ** with the html header. If a file with the given file name
     ** already exists, ctor overwrites it.
     ** If ctor can't open file a std::bad_exception is thrown.
     ** @todo If ctor fails in opening file throw a more specific exception.
     *****************************************************************************/
    class LogEntry
    {
    public:
        /**
         ** This template function provides to LogEntry a
         ** stream-like behaviour. Note that it returns a
         ** reference to this object so many calls to
         ** operator<< can be chained in the usual manner.
         ** @param in Takes a const reference to the object to log.
         ** @return Reference to *this.
         *****************************************************************************/
        template <typename T>
        LogEntry& operator<<(const T &in)
        {
            out << in;
            return *this;
        }
        ~LogEntry();
    private:
        /**
         ** The ctor writes the line header.
         ** If it can't open file throws an std::bad_exception().
         ** @param file_name The log file name.
         ** @param type The type of information to log. This is
         **        shown at the begining of the line.
         ** @todo If it fails to open file throw a more specific exception.
         *****************************************************************************/
        LogEntry(const char *type);
        friend class Logger;
        LogEntry(const LogEntry&)
        {}
        std::ofstream out;
    };

    /**
     ** Logs an list entry.
     ** @see LogEntry::LogEntry()
     *****************************************************************************/
    LogEntry list()
    {
        return LogEntry("List");
    }

    /**
     ** Logs an information.
     ** @see LogEntry::LogEntry()
     *****************************************************************************/
    LogEntry info()
    {
        return LogEntry("Info");
    }

    /**
     ** Logs a warning.
     ** @see LogEntry::LogEntry()
     *****************************************************************************/
    LogEntry warning()
    {
        return LogEntry("Warn");
    }

    /**
     ** Logs an error.
     ** @see LogEntry::LogEntry()
     *****************************************************************************/
    LogEntry error()
    {
        return LogEntry("Error");
    }

    /**
     ** Write a headline to the log.
     ** Again it throws an std::bad_exeption if it can't open file.
     ** @param text The headline to show.
     ** @todo If it fails to open file throw a more specific exception.
     *****************************************************************************/
    void headline(const char *text);

    /**
     ** Helper function. Returns an Ok | Failed string depending
     ** on the value of the argument.
     ** @param status True means "Ok", false means "Failed".
     ** @return The string equivalent to status.
     *****************************************************************************/
    void success(bool status);

private:
    Logger();
    /**
     ** Destructor closes the log writing a closing html header.
     ** If it fails in opening the file it ingnores the error.
     ** Throwing an exception here would be useless an dangerous if
     ** the object is static.
     ** Both HTML 4.01 Transitional as well as HTML 4.01 Strict
     ** allow the omission of the HTML and BODY start and end tag.
     ** So we can consider not an error leaving the log
     ** without the closing header.
     ** In the same manner we can skip the problem of adding the closing header
     ** to the log file during the play.
     *****************************************************************************/
    ~Logger();
    /**
     ** Helper function. Returns a formatted representation
     ** of the time when called.
     *****************************************************************************/
    const char* now();

    static bool mTable;
    static bool mList;
    static const char *mFilename;
};

#endif
