/***************************************************************************
 *   Copyright (C) 2004 by Andreas Seidel                                  *
 *   3999@freenet.de                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


// This was imported from one of my older projects - so forgive me the c-functions (polyveg)


#include "logfile.h"

#define PRG_NAME "Daimonin Ogre Client"

//=================================================================================================
// Create/Overwrite the Logfile.
//=================================================================================================
bool LogFile::Init(char *filename)
{
  m_filename = filename;
  m_stream   = fopen(m_filename, "w");
  if (!m_stream) { return false; }
  time(&m_time);
  m_localtime = localtime(&m_time);
  fputs("<html>\n", m_stream);
  // Head.
  fputs("<head>\n", m_stream);
  fputs("<title>"PRG_NAME" - Logfile</title>\n", m_stream);
  fputs("</head>\n", m_stream);
  // Style.
  fputs("<style>\n", m_stream);
  fputs("body, p { font-family:arial, helvetica, sans-serif; color:Black;}\n", m_stream);
  fputs("h1,h2   { font-family:arial, helvetica, sans-serif; color:Blue; }\n", m_stream);
  fputs("h1 { font-size:18pt; line-height:8pt; }\n", m_stream);
  fputs("h2 { font-size:16pt; line-height:8pt; }\n", m_stream);
  fputs("p  { font-size:10pt; }\n", m_stream);
  fputs("</style>\n", m_stream);
  // Body.
  fputs("<body>\n", m_stream);
  fputs("<h1>"PRG_NAME" - Logfile</h1>\n", m_stream);
  fputs("<h2>Started: ", m_stream);
  sprintf(m_buffer,"%.2d.%.2d.%4d - ",
    m_localtime->tm_mday, m_localtime->tm_mon+1, m_localtime->tm_year+1900);
  fputs(m_buffer, m_stream);
  sprintf(m_buffer,"%.2d:%.2d:%.2d",
    m_localtime->tm_hour, m_localtime->tm_min, m_localtime->tm_sec);
  fputs(m_buffer, m_stream);
  fputs("</h2>\n<p>\n", m_stream);
  fclose(m_stream);
  return true;
}

//=================================================================================================
// End of Logging.
//=================================================================================================
LogFile::~LogFile()
{
  m_stream = fopen(m_filename, "a");
  if (!m_stream)
    return;
  time(&m_time);
  m_localtime = localtime(&m_time);
  fputs("</p>\n<hr>\n<h2>Ended: ", m_stream);
  sprintf(m_buffer,"%.2d.%.2d.%4d - ",
    m_localtime->tm_mday, m_localtime->tm_mon+1, m_localtime->tm_year+1900);
  fputs(m_buffer, m_stream);
  sprintf(m_buffer,"%.2d:%.2d:%.2d",
    m_localtime->tm_hour, m_localtime->tm_min, m_localtime->tm_sec);
  fputs(m_buffer, m_stream);
  fputs("</h2>\n", m_stream);

  fputs("</body>\n</html>\n", m_stream);
  fclose(m_stream);
}

//=================================================================================================
// Return the instance.
//=================================================================================================
LogFile &LogFile::getSingelton()
{
   static LogFile singelton;
   return singelton;
}

//=================================================================================================
// Append text to the logfile.
//=================================================================================================
void LogFile::write(const char *color)
{
  m_stream = fopen(m_filename, "a");
  if (!m_stream)
    return;

  // set the color.
  fputs("<font color=\"", m_stream);
  fputs(color, m_stream);
  fputs("\">", m_stream);

  // change '\n' to <br>
  char *info, *info2;
  info = info2 = m_buffer;
  while (*info)
  {
    if (*info++ == '\n')
    {
      *(info-1) = 0;
      fputs(info2, m_stream);
      info2= info;
      fputs("<br>\n", m_stream);
    }
  }
  fputs(info2, m_stream);
  fclose(m_stream);
}

//=================================================================================================
// Append an info to the Logfile.
//=================================================================================================
void LogFile::Info(char *info, ...)
{
  va_list args;
  va_start(args, info);
  vsnprintf(m_buffer, LOG_BUFFER_SIZE, info, args);
  va_end(args);
  write("#000000");  // Color black.
}

//=================================================================================================
// Append an error to the Logfile.
//=================================================================================================
void LogFile::Error(char *error, ...)
{
  va_list args;
  va_start(args, error);
  vsnprintf(m_buffer, LOG_BUFFER_SIZE, error, args);
  va_end(args);
  write("#ff0000");  // Color red.
}

//=================================================================================================
// Append the a ok|failed status-text to the Logfile.
//=================================================================================================
void LogFile::Success(bool status)
{
  m_stream = fopen(m_filename, "r+" );
  if (!m_stream)
    return;
  fseek( m_stream, 0, SEEK_END);
  if (status) fputs  (" <font color=\"#00ff00\">ok<br>\n", m_stream);
  else        fputs  (" <font color=\"#ff0000\">failed<br>\n", m_stream);
  fclose(m_stream);
}

//=================================================================================================
// Append a headline.
//=================================================================================================
void LogFile::Headline(const char *text)
{
  m_stream = fopen(m_filename, "a" );
  if (!m_stream)
    return;
  fputs  ("<hr>\n<h2>", m_stream);
  fputs(text, m_stream);
  fputs("</h2><p>\n", m_stream);
  fclose(m_stream);
}
