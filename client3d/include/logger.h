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

/**
 ** This class provides logging facility.
 ** Logs are stored in HTML format.
 *****************************************************************************/
class Logger
{
private:
    enum
    {
        STYLE_LIST,
        STYLE_INFO,
        STYLE_WARN,
        STYLE_ERROR,
        STYLE_SUCCESS,
        STYLE_HEADLINE,
        STYLE_SUM
    };
    static int mType;

    /**
     ** Cconstructor creates the log file and initalizes it with the html header.
     *****************************************************************************/
    Logger();

    /**
     ** Destructor writes a closing html header and closes the logfile.
     ** Both HTML 4.01 Transitional as well as HTML 4.01 Strict
     ** allow the omission of the HTML and BODY start and end tag.
     ** So we can consider not an error leaving the log without the closing header.
     *****************************************************************************/
    ~Logger();

    /**
     ** Returns the actual date and time.
     *****************************************************************************/
    const char* now();

    class LogEntry
    {
    public:
        /**
         ** The Constructor writes the line header.
         ** @param type The type of information to log (e.g. warnings).
         *****************************************************************************/
        LogEntry(int type);
        /**
         ** This template function provides to LogEntry a stream-like behaviour.
         ** Note that it returns a reference to this object so many calls to
         ** operator<< can be chained in the usual manner.
         ** @param in Takes a const reference to the object to log.
         ** @return Reference to *this.
         *****************************************************************************/
        template <typename T>
        LogEntry& operator<<(const T &in)
        {
            if (mOut.is_open()) mOut << in;
            return *this;
        }
        LogEntry(const LogEntry&) {}
        ~LogEntry();
    private:
        std::ofstream mOut;
    };

public:
    /**
     ** This static method returns a reference to a static Logger object.
     ** This allows users to use this class in a singleton-like manner.
     ** @return Reference to a static singleton Logger object.
     *****************************************************************************/
    static Logger &log()
    {
        static Logger Singleton; return Singleton;
    }

    /**
     ** Writes a status message ("Ok" or "Failed") to the end of a line.
     ** @param status True means "Ok", false means "Failed".
     *****************************************************************************/
    void success(bool status);

    /**
     ** Writes a list entry.
     *****************************************************************************/
    LogEntry list()
    {
        return LogEntry(STYLE_LIST);
    }

    /**
     ** Writes an information.
     *****************************************************************************/
    LogEntry info()
    {
        return LogEntry(STYLE_INFO);
    }

    /**
     ** Writes a warning.
     *****************************************************************************/
    LogEntry warning()
    {
        return LogEntry(STYLE_WARN);
    }

    /**
     ** Writes an error.
     *****************************************************************************/
    LogEntry error()
    {
        return LogEntry(STYLE_ERROR);
    }

    /**
     ** Writes a headline.
     *****************************************************************************/
    LogEntry headline()
    {
        return LogEntry(STYLE_HEADLINE);
    }
};

#endif
