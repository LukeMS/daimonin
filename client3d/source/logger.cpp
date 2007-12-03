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

#include <exception>
#include <ctime>
#include "logger.h"

const char *Logger::mFilename = FILE_LOGGING;
bool Logger::mTable = false;
bool Logger::mList = false;

//================================================================================================
// .
//================================================================================================
Logger::Logger()
{
    std::ofstream log_stream(mFilename, std::ios::out);
    if (!log_stream.is_open()) throw std::bad_exception();
    log_stream
    << "<html><head><title>" << PRG_NAME << " - Logfile</title></head>" <<
    "<style>\n" <<
    "td.Info {color:black;  }\n" <<
    "td.List {color:black;  }\n" <<
    "td.Warn {color:orange; }\n" <<
    "td.Error{color:red;    }\n" <<
    "td.Ok   {color:#00ff00;}\n" <<
    "</style>\n" <<
    "<body>\n<h1>" << PRG_NAME << " - Logfile</h1>\n" <<
    "<h2>Started: " << now() << "</h2>\n";
}

//================================================================================================
// .
//================================================================================================
Logger::~Logger()
{
    std::ofstream log_stream(mFilename, std::ios::out | std::ios::app);
    if (log_stream.is_open())
    {
        if (mTable)
        {
            log_stream << "</table>\n";
            mTable = false;
        }
        log_stream << "\n<hr><h2>Ended: " << now() << "</h2></body></html>";
    }
}

//================================================================================================
// .
//================================================================================================
void Logger::headline(const char *text)
{
    std::ofstream log_stream(mFilename, std::ios::out | std::ios::app);
    if (!log_stream.is_open()) throw std::bad_exception();
    if (mTable)
    {
        log_stream << "\n</table>\n";
        mTable = false;
    }
    log_stream << "\n<hr><h2>" << text << "</h2>\n";
}

//================================================================================================
// .
//================================================================================================
void Logger::success(bool status)
{
    std::ofstream log_stream(mFilename, std::ios::out | std::ios::in| std::ios::binary);
    if (!log_stream.is_open()) throw std::bad_exception();
    log_stream.seekp(-5, std::ios::end);
    if (status)
        log_stream << "<td width=\"5%\" class=\"Ok\"> ok </td></tr>";
    else
        log_stream << "<td width=\"5%\" class=\"Error\"> failed </td></tr>";
}

//================================================================================================
// .
//================================================================================================
const char* Logger::now()
{
    static char dateStr[50];
#ifdef WIN32
    _strdate(dateStr);
    _strtime(dateStr+9);
    dateStr[8] = ' ';
    return dateStr;
#else
    struct tm newtime;
    time_t ltime;
    ltime=time(&ltime);
    localtime_r(&ltime, &newtime);
    asctime_r(&newtime, dateStr);
    return dateStr;
#endif
}

//================================================================================================
// .
//================================================================================================
Logger::LogEntry::LogEntry(const char *type)
{
    out.open(mFilename, std::ios::out | std::ios::app);
    if (!out.is_open())  throw std::bad_exception();
    if (!mTable)
    {
        out << "<table  width=\"100%\">";
        mTable = true;
    }
    out << "\n<tr><td width= \"95%\" class=\"" << type << "\">";
    if (!mList && type && type[0]=='L') //"List"
    {
        out << "<li>";
        mList = true;
        return;
    }
}

//================================================================================================
// .
//================================================================================================
Logger::LogEntry::~LogEntry()
{
    if (!out.is_open())  throw std::bad_exception();
    if (mList)
    {
        out << "</li>";
        mList = false;
    }
    out << "</td></tr>";
}
