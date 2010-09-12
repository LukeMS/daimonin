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
#include <sstream>
#include <iomanip>
#include "logger.h"

static const char *PRG_NAME = "Daimonin Ogre3d Client";
static const char *FILENAME = "./client_log.html";
static const char *STR_STYLE[] = {"Head", "List", "Info", "Warn", "Error", "Debug", "Attempt", "Ok" };
int Logger::mType = 0;
bool Logger::LogEntry::mAttemptActive = false;

//================================================================================================
// Constructor.
//================================================================================================
Logger::Logger()
{
    std::ofstream log_stream(FILENAME, std::ios::out);
    if (!log_stream.is_open()) return;
    log_stream  << "<html>\n<head><title>" << PRG_NAME << " - Logfile</title></head>\n" <<
                "<style>\n" <<
                "td."<< STR_STYLE[STYLE_HEADLINE]<< " {color:black; font-weight: bold;}\n" <<
                "td."<< STR_STYLE[STYLE_LIST    ]<< " {color:black;  }\n" <<
                "td."<< STR_STYLE[STYLE_INFO    ]<< " {color:black;  }\n" <<
                "td."<< STR_STYLE[STYLE_WARN    ]<< " {color:orange; }\n" <<
                "td."<< STR_STYLE[STYLE_ERROR   ]<< " {color:red;    }\n" <<
                "td."<< STR_STYLE[STYLE_DEBUG   ]<< " {color:violet; }\n" <<
                "td."<< STR_STYLE[STYLE_ATTEMPT ]<< " {color:black;  }\n" <<
                "td."<< STR_STYLE[STYLE_OK      ]<< " {color:#00ff00;}\n" <<
                "</style>\n\n" <<
                "<body>\n" <<
                "<table  width=\"100%\">\n" <<
                "<h1>" << PRG_NAME << " - Logfile</h1>\n" <<
                "<h2>Started: " << now() << "</h2>";
}

//================================================================================================
// Destructor.
//================================================================================================
Logger::~Logger()
{
    std::ofstream log_stream(FILENAME, std::ios::out | std::ios::app);
    if (log_stream.is_open())
    {
        log_stream << "\n</table>\n\n<hr><h2>Ended: " << now() << "</h2>\n</body>\n</html>";
    }
}

//================================================================================================
// Returns the actual date/time.
//================================================================================================
const std::string Logger::now()
{
    std::time_t timestamp;
    time(&timestamp);
    tm *tdata = localtime(&timestamp);
    std::ostringstream os;
    os.fill('0');
    os << std::setw(2) << tdata->tm_mday <<
       '.'<< std::setw(2) << tdata->tm_mon+1 <<
       '.'<< std::setw(2) << tdata->tm_year+1900 <<
       ' '<< std::setw(2) << tdata->tm_hour <<
       ':'<< std::setw(2) << tdata->tm_min <<
       ':'<< std::setw(2) << tdata->tm_sec;
    return os.str();
}

//================================================================================================
// Constructor.
//================================================================================================
Logger::LogEntry::LogEntry(int type)
{
    mOut.open(FILENAME, std::ios::out | std::ios::app);
    if (!mOut.is_open()) throw std::bad_exception();
    if (mAttemptActive)
    {
        if (type != STYLE_ERROR)
            mOut << "<font color=#00ff00>  ok</font></td></tr>";
        else
            mOut << "<font color=red>  failed</font></td></tr>";
        mAttemptActive = false;
    }
    mType = type;
    if (type == STYLE_HEADLINE)
        mOut << "\n</table>\n\n<hr>\n<table width=\"100%\">";
    if (type == STYLE_LIST)
        mOut << "\n<tr><td class=\"" << STR_STYLE[STYLE_LIST] << "\"><li>";
    else
        mOut << "\n<tr><td class=\"" << STR_STYLE[type] << "\">";
    if (type == STYLE_ATTEMPT)
        mAttemptActive = true;
}

//================================================================================================
// Destructor.
//================================================================================================
Logger::LogEntry::~LogEntry()
{
    if (!mOut.is_open()) return;
    if (mType == STYLE_LIST) mOut << "</li>";
    if (!mAttemptActive)     mOut << "</td></tr>";
    mOut.close();
}
