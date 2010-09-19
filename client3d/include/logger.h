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

/// @brief This singleton class provides logging facility in HTML format.
/// @details
class Logger
{
private:
    enum
    {
        STYLE_HEADLINE, ///< @brief Write a headline.
        STYLE_LIST,     ///< @brief Write a list entry.
        STYLE_INFO,     ///< @brief Write a text line with black text color.
        STYLE_WARN,     ///< @brief Write a text line with orange text color.
        STYLE_ERROR,    ///< @brief Write a text line with red text color.
        STYLE_DEBUG,    ///< @brief Write a text line with violet text color.
        STYLE_ATTEMPT,  ///< @brief Write a text line with black text color.
        STYLE_OK,       ///< @brief Write a text line with black text color.
        STYLE_SUM       ///< @brief Number of elements in this enum.
    };
    static int mType;
    /// @brief   Default constructor.
    /// @details Creates the log file and initalizes it with the html header.
    Logger();

    /// @brief    Default destructor.
    /// @details  Writes a closing html header and closes the logfile.
    /// Both HTML 4.01 Transitional as well as HTML 4.01 Strict
    /// allow the omission of the HTML and BODY start and end tag.
    /// So we can consider not an error leaving the log without the closing header.
    ~Logger();

    /// @brief Returns the actual date and time.
    const std::string now();

    /// @brief Helper class that writes to the HTML-file.
    class LogEntry
    {
    public:
        /// @brief The Constructor writes the line header.
        /// @param type The type of information to log (e.g. warnings).
        LogEntry(int type);

        /// @brief This template function provides to LogEntry a stream-like behaviour.
        /// @details Note that it returns a reference to this object so many calls to
        ///          operator<< can be chained in the usual manner.
        /// @param in Takes a const reference to the object to log.
        /// @return Reference to *this.
        template <typename T>
        LogEntry& operator<<(const T &in)
        {
            if (mOut.is_open()) mOut << in;
            return *this;
        }

        LogEntry(const LogEntry&) {} /// @brief Default constructor.
        ~LogEntry();                 /// @brief Default destructor.
    private:
        static bool mAttemptActive;
        std::ofstream mOut;
        LogEntry &operator=(const LogEntry&); ///< @brief disable assignment operator.
    };

public:
    static const char *ICON_CAUDIO;
    static const char *ICON_OGRE3D;
    static const char *ICON_CLIENT;
    /// @brief Returns the reference to this singleton class.
    static Logger &log()
    {
        static Logger Singleton; return Singleton;
    }
    /// @brief Writes an attempt message and adds a status message to it.
    /// @details First it outputs the attempt message (e.g. "Starting the Gui-Manager...").
    ///          If the next logger cmd is an error() it adds "failed" else "ok" to the end of the line.
    ///          If you don't want to log an error on a failed attempt just add an empty error message.
    LogEntry attempt()  { return LogEntry(STYLE_ATTEMPT);  }
    LogEntry headline() { return LogEntry(STYLE_HEADLINE); } ///< @brief Writes a headline.
    LogEntry list()     { return LogEntry(STYLE_LIST);     } ///< @brief Writes a list entry.
    LogEntry warning()  { return LogEntry(STYLE_WARN);     } ///< @brief Writes a warning message.
    LogEntry debug()    { return LogEntry(STYLE_DEBUG);    } ///< @brief Writes a debug message.
    LogEntry error()    { return LogEntry(STYLE_ERROR);    } ///< @brief Writes an error message.
    LogEntry info()     { return LogEntry(STYLE_INFO);     } ///< @brief Writes an info message.
};

#endif
