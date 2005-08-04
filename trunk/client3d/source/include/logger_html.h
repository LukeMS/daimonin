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

#ifndef HTML_LOGGER_H
#define HTML_LOGGER_H

#include <string>

////////////////////////////////////////////////////////////
///
/// HtmlLogger provides support for logging information in
/// a specified file in HTML format.
/// This class is (now) completely isolated from the rest of
/// code.
///
/// Methods Info(), Error(), Success(), Headline()
/// and also the constructor HtmlLogger() and ~HtmlLogger() throw
/// an std::bad_exception if they fail to open the log file.
///
////////////////////////////////////////////////////////////
class HtmlLogger
		{
		public:
			/// Create/Overwrite  the file and
			/// initialize it with the html header.
			/// \param file_name A C-string with the name of the file where store log.
			/// \param title A C-string with the title of the log.
			HtmlLogger(const char *file_name, const char *title);
			/// Close the html file writing the closing header.
			~HtmlLogger();

			/// Append info the the log.
			/// Note: stdarg version is deprecated.
			/// \param message The printf-like string to print.
			void Info    (const std::string text);
			/// Append error the the log.
			/// Note: stdarg version is deprecated.
			/// \param message The printf-like string to print.
			void Error   (const std::string text);
			/// Append the Ok|Failed status-text to the log file.
			/// \param status True for "Ok", false for "Failed".
			void Success (bool status);
			/// Print a head line in the log file
			/// \param text The text to print
			void Headline(const std::string text);

		private:
			/// Here we put the code that manage files.
			/// The function opens and closes the file for each call.
			/// By now if the file can't be opened this funcion throws
			/// an std::bad_exception. In future
			/// this should be changed to something more specific.
			/// \param message The message to print.
			/// \todo Throw a more specific exception if can't open the log file.
			void write(const std::string &message);
			/// This is a filter that change all newline in "text" to
			/// "<br>" html tag.
			/// \param text The string to process.
			/// \return String with all newline sostituited by <br> tags.
			std::string newline_to_br(const std::string &text);
			/// Returns a string containing the current date and time.
			/// \return Const reference to the std::string.
			const std::string& now();

			// Set those private, so external users cannot copy objects.
			HtmlLogger(const HtmlLogger&);
			const HtmlLogger& operator=(const HtmlLogger&);

			const char *file_name;
		};

#endif

