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
const char *Logger::_success[] = { "<b class=\"Failed\"> Failed </p>",
																	 "<b class=\"Ok\"> Ok </b>" };

Logger::Logger(const char *file_name, const char *title)
		: file_name(file_name)
	{
	std::ofstream log_stream(file_name, std::ios::out);
	if(!log_stream.is_open()) throw std::bad_exception();
	log_stream << "<html>\n<head>\n"\
	"<title>" << title << " - Logfile</title>\n</head>\n"\
	"<style>\n"\
	"th { vertical-align:top; text-align:left; }\n"\
	"th.Info { background-color:white; }\n"\
	"th.Warn { background-color:orange; }\n"\
	"th.Error{ background-color:red; }\n"\
	"td.Info { color:black; }\n"\
	"td.Warn { color:orange; }\n"\
	"td.Error{ color:red; }\n"\
	"b.Ok { background-color:green; }\n"\
	"b.Failed { background-color:red; }\n"\
	"</style>\n"\
	"<body>\n<h1>" << title << " - Logfile</h1>\n"\
	"<h2>Started: " << now() << "</h2>\n"\
	"<table>";
	}

Logger::~Logger()
	{
	std::ofstream log_stream(file_name, std::ios::out | std::ios::app);
	if(log_stream.is_open())
		log_stream << "</table><hr>\n<h2>Ended: " << now() << "</h2>\n</body>\n</html>";
	}


Logger::LogEntry::LogEntry(const char *file_name, const char *type)
		: out(file_name, std::ios::out | std::ios::app)
	{
	if(!out.is_open()) throw std::bad_exception();
	out << "\n<tr><th class=\"" << type << "\">["
	<< type << "]</th><td class=\"" << type << "\">";
	}

Logger::LogEntry::~LogEntry()
	{ out << "</td></tr>"; }

void Logger::headline(const char *text)
	{
	std::ofstream log_stream(file_name, std::ios::out | std::ios::app);
	if(!log_stream.is_open()) throw std::bad_exception();
	log_stream << "</table><hr><h2>" << text << "</h2><table>";
	}

const char* Logger::now() {
	time_t now;
	time(&now);
	return asctime(localtime(&now));
	}
