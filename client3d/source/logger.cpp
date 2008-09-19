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

#include <ctime>
#include <exception>
#include "logger.h"

const char *PRG_NAME = "Daimonin Ogre3d Client";
const char *FILENAME = "./client_log.html";
const char *STR_STYLE[] = {"List", "Info", "Warn", "Error", "Success", "Headline"};
int Logger::mType = 0;

//================================================================================================
// Constructor.
//================================================================================================
Logger::Logger()
{
    std::ofstream log_stream(FILENAME, std::ios::out);
    if (!log_stream.is_open()) return;
    log_stream  << "<html>\n<head><title>" << PRG_NAME << " - Logfile</title></head>\n" <<
    "<style>\n" <<
    "td."<< STR_STYLE[STYLE_LIST    ]<< " {color:black;  }\n" <<
    "td."<< STR_STYLE[STYLE_INFO    ]<< " {color:black;  }\n" <<
    "td."<< STR_STYLE[STYLE_WARN    ]<< " {color:orange; }\n" <<
    "td."<< STR_STYLE[STYLE_ERROR   ]<< " {color:red;    }\n" <<
    "td."<< STR_STYLE[STYLE_SUCCESS ]<< " {color:#00ff00;}\n" <<
    "td."<< STR_STYLE[STYLE_HEADLINE]<< " {color:black; font-size: 18pt; font-weight: bold;}\n" <<
    "</style>\n" <<
    "<body>\n" <<
    "<table  width=\"100%\">\n\n" <<
    "<h1>" << PRG_NAME << " - Logfile</h1>\n" <<
    "<h2>Started: " << now() << "</h2>\n";
}

//================================================================================================
// Destructor.
//================================================================================================
Logger::~Logger()
{
    std::ofstream log_stream(FILENAME, std::ios::out | std::ios::app);
    if (log_stream.is_open())
    {
        log_stream << "\n\n</table>\n<hr><h2>Ended: " << now() << "</h2>\n</body>\n</html>";
    }
}

//================================================================================================
// Returns the actual date/time.
//================================================================================================
const char* Logger::now()
{
    static char dateStr[50];
#ifdef WIN32
    _strdate(dateStr);
    _strtime(dateStr+9);
    dateStr[8] = ' ';
#else
    struct tm newtime;
    time_t ltime;
    ltime = time(&ltime);
    localtime_r(&ltime, &newtime);
    asctime_r(&newtime, dateStr);
#endif
    return dateStr;
}

//================================================================================================
// Writes a status message to the end of line.
//================================================================================================
void Logger::success(bool status)
{
    std::ofstream log_stream(FILENAME, std::ios::out | std::ios::in| std::ios::binary);
    if (log_stream.is_open())
    {
        log_stream.seekp(-5, std::ios::end); // position before </tr> key.
        if (status)
            log_stream << "<td width=\"5%\" class=\"" << STR_STYLE[STYLE_SUCCESS] << "\"> ok </td></tr>";
        else
            log_stream << "<td width=\"5%\" class=\"" << STR_STYLE[STYLE_ERROR] << "\"> failed </td></tr>";
    }
}

//================================================================================================
// Constructor.
//================================================================================================
Logger::LogEntry::LogEntry(int type)
{
    mOut.open(FILENAME, std::ios::out | std::ios::app);
    if (!mOut.is_open())  throw std::bad_exception();
    if ((mType = type) == STYLE_HEADLINE)
        mOut << "\n</table>\n\n<hr>\n<table  width=\"100%\">";
    if (type == STYLE_LIST)
        mOut << "\n<tr><td width= \"95%\" class=\"" << STR_STYLE[STYLE_LIST] << "\"><li>";
    else
        mOut << "\n<tr><td width= \"95%\" class=\"" << STR_STYLE[type] << "\">";
}

//================================================================================================
// Destructor.
//================================================================================================
Logger::LogEntry::~LogEntry()
{
    if (!mOut.is_open()) return;
    if (mType == STYLE_LIST) mOut << "</li>";
    mOut << "</td></tr>";
    mOut.close();
}
