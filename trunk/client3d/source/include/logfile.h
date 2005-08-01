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

#ifndef LOG_FILE_H
#define LOG_FILE_H

#include <cstdarg>
#include "define.h"
#include "html_logger.h"

////////////////////////////////////////////
/// 
/// This class realizes
/// the logging facility for the program. 
/// The LogFile is implemented in terms of HtmlLogger
/// through layering. 
/// In this class is added only the support for variable argument number. 
/// The separation of the code, that manages logging, in two distinct classes
/// comes from the need of isolating the code that manages html 
/// from the rest of the program. 
/// Indeed LogFile provides an interface compatible with the old code. 
/// 
////////////////////////////////////////////
class LogFile
{
public:
	/// This function returns a reference to 
	/// the unique instance of LogFile.
	/// Note that there is no need to call explicitly a function to initialize the object.
	/// All initialization, like printing html-header, is done in the constuctor of the
	/// HtmlLogger object, which is automatically called the first time getSingleton()
	/// is invoked.
	inline static LogFile& getSingleton()
	{
		static LogFile singleton;
		return singleton;
	}

	void Info (const char *text, ...);
	void Error (const char *text, ...);
	inline void Headline (const char *text) { log.Headline(text); }
	inline void Success (bool state) { log.Success(state); }

private:
	HtmlLogger log;
	// Make those private so user cannot
	LogFile() : log(FILE_LOGGING, PRG_NAME) {}
	~LogFile() {}
	LogFile(const LogFile&);
	const LogFile& operator=(const LogFile&);

	// Local buffer to do the dirty work.
	enum { LOG_BUFFER_SIZE = 255 };
	char m_buffer[LOG_BUFFER_SIZE+1];
};

/// If you want a cleaner sintax use this function
/// to retieve the LogFile instance
/// \return The LogFile (singleton) instance.
inline LogFile& Log()
{ return LogFile::getSingleton(); }

#endif
