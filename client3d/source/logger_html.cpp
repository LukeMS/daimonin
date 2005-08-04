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

#include "logger_html.h"

#include <fstream>
#include <ctime>
#include <string>
#include <exception>

using namespace std;

// Note:
//
// "Both HTML 4.01 Transitional as well as HTML 4.01 Strict
// allow the omission of the HTML and BODY start and end tag."
// (many thanks to Christian!)
//
// So we can safely skip the problem of adding the closing header
// to the log file during the play.

HtmlLogger::HtmlLogger(const char *file_name, const char *title):file_name(file_name)
{
	std::ofstream log_stream(file_name, std::ios::out);
	log_stream.close();
	write("<html>\n<head>\n"\
			"<title>" + string(title) + " - Logfile</title>\n</head>\n"\
			"<style>\n"\
			"body, p { font-family:arial, helvetica, sans-serif; color:Black;}\n"\
			"h1,h2   { font-family:arial, helvetica, sans-serif; color:Blue; }\n"\
			"h1 { font-size:18pt; line-height:8pt; }\n"\
			"h2 { font-size:16pt; line-height:8pt; }\n"\
			"p  { font-size:10pt; }\n"\
			"</style>\n"\
			"<body>\n<h1>" + title + " - Logfile</h1>\n"\
			"<h2>Started: " + now() + "</h2>\n<p>");
}

HtmlLogger::~HtmlLogger()
{
	write("</p>\n<hr>\n<h2>Ended: " + now() + "</h2>\n</body>\n</html>"); 
}

const string& HtmlLogger::now()
{
	static string s;
	time_t now;
	time(&now);
	s = asctime(localtime(&now));
	return s;
}

void HtmlLogger::write(const string &message)
{
	std::ofstream log_stream(file_name, std::ios::out | std::ios::app);
	if(log_stream.is_open()) { log_stream << message; }
	else throw std::bad_exception();
}

std::string HtmlLogger::newline_to_br(const string &text)
{
	string message;
	string::size_type left = 0, right;
	while(true)
	{
		right = text.find('\n', left);
		if(right == string::npos) break;
		message.append(text, left, right-left).append("<br>");
		left = right + 1;
	}
	message.append(text, left, text.size() - left); 
	return message;
}

void HtmlLogger::Info(const string text)
{
	write( "\n<br><font color=\"#000000\"><strong>[Info]: </strong>"\
	+ newline_to_br(text) + "</font>" );
}

void HtmlLogger::Error(const string text)
{
	write( "\n<br><font color=\"#FF0000\"><strong>[Error]: </strong>"\
		+ newline_to_br(text) + "</font>" );
}

void HtmlLogger::Success(bool status)
{
	if(status) write("<font color=\"#00FF00\"><strong>Ok</strong></font>");
	else write("<font color=\"#FF0000\"><strong>Failed</strong></font>");
}

void HtmlLogger::Headline(const string text)
{
	write( "</p><hr><h2>" + text + "</h2><p>" );
}
