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
#include <exception>
#include <ctime>

#include "logger.h"

const char *Logger::endl = "<br>\n";
static bool table = false;

///=================================================================================================
/// .
///=================================================================================================
Logger::Logger(const char *mFilename, const char *title): mFilename(mFilename)
{
  std::ofstream log_stream(mFilename, std::ios::out);
  if(!log_stream.is_open()) throw std::bad_exception();
  log_stream << "<html>\n<head>\n"\
  "<title>" << title << " - Logfile</title>\n</head>\n"\
  "<style>\n"\
  "td.Info {color:black;  }\n"\
  "td.Warn {color:orange; }\n"\
  "td.Error{color:red;    }\n"\
  "td.Ok   {color:#00ff00;}\n"\
  "</style>\n"\
  "<body>\n<h1>" << title << " - Logfile</h1>\n"\
  "<h2>Started: " << now() << "</h2>\n";
}

///=================================================================================================
/// .
///=================================================================================================
Logger::~Logger()
{
  std::ofstream log_stream(mFilename, std::ios::out | std::ios::app);
  if(log_stream.is_open())
  {
    if (table) {
      log_stream << "</table>\n"; table = false; }
    log_stream << "\n<hr><h2>Ended: " << now() << "</h2></body></html>";
  }
}


///=================================================================================================
/// .
///=================================================================================================
Logger::LogEntry::LogEntry(const char *mFilename, const char *type)
    : out(mFilename, std::ios::out | std::ios::app)
{
  if(!out.is_open()) throw std::bad_exception();
  if (!table) {
    out << "<table>\n"; table = true; }
  out << "  <tr><td class=\"" << type << "\">";
}

///=================================================================================================
/// .
///=================================================================================================
Logger::LogEntry::~LogEntry()
{
  out << "  </td></tr>\n";
}

///=================================================================================================
/// .
///=================================================================================================
void Logger::headline(const char *text)
{
  std::ofstream log_stream(mFilename, std::ios::out | std::ios::app);
  if(!log_stream.is_open()) throw std::bad_exception();
  if (table) {
    log_stream << "</table>\n"; table = false; }
  log_stream << "\n<hr><h2>" << text << "</h2>\n";
}

///=================================================================================================
/// .
///=================================================================================================
const char* Logger::now()
{
  time_t now;
  time(&now);
  return asctime(localtime(&now));
}

///=================================================================================================
/// .
///=================================================================================================
void Logger::success(bool status)
{
  std::ofstream log_stream(mFilename, std::ios::in | std::ios::out);
  if(!log_stream.is_open()) throw std::bad_exception();
  log_stream.seekp(-10, std::ios::end);
  if (status)
    log_stream << "<tr><td class=\"Ok\"> ok </td></tr>\n";
  else
    log_stream << "<tr><td class=\"Error\"> failed </td></tr>\n";
}
