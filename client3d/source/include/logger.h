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

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include "define.h"

/**
 *      This class provides logging facility.
 *      Logs are stored on a file in HTML format.
 */
class Logger
		{
		public:
			/**
			 *      This static method returns a reference to 
			 *      a static Logger object. This allows users to
			 *      use this class in a singleton-like manner.
			 *      Note that the constructor of this static Logger 
			 *      object is called automatically the first time this 
			 *      method is invoked.
			 *      @return Reference to a static singleton Logger object.
			 */     
			static Logger& log()
				{ static Logger l(FILE_LOGGING, PRG_NAME); return l; }
			/**
			 *      The constructor create the log file and initalizes it
			 *      with the html header. If a file with the given file name 
			 *      already exists, ctor overwrites it. 
			 *      If ctor can't open file a std::bad_exception is thrown.
			 *      @param file_name A pointer to the C-string containing the log file name.
			 *      @param title A pointer to the C-string contaning the title of the log.
			 *      @todo If ctor fails in opening file throw a more specific exception.
			 */
			Logger(const char *file_name, const char *title);
			/**
			 *      Destructor closes the log writing a closing html header.
			 *      If it fails in opening the file it ingnores the error.
			 *      Throwing an exception here would be useless an dangerous if
			 *      the object is static.
			 *      Note also that:
			 *      "Both HTML 4.01 Transitional as well as HTML 4.01 Strict
			 *      allow the omission of the HTML and BODY start and end tag."
			 *      (many thanks to Cher!)
			 *      So we can consider not an error leaving the log 
			 *      without the closing header.
			 *      In the same manner we can skip the problem of adding the closing header
			 *      to the log file during the play.         
			 */
			~Logger();
			/**
			 *      This nested class provides a way for using Logger objects with 
			 *      a stream-like sintax. LogEntry objects can't be created by users of 
			 *      Logger and can't be copied. 
			 */
			class LogEntry {
					public:
						/**
						 *      This template function provides to LogEntry a
						 *      stream-like behaviour. Note that it returns a 
						 *      reference to this object so many calls to 
						 *      operator<< can be chained in the usual manner.
						 *      @param in Takes a const reference to the object to log.
						 *      @return Reference to *this.
						 */
						template <typename T>
						LogEntry& operator<<(const T &in) { out << in; return *this; }
						~LogEntry();
					private:
						/**
						 *      The ctor writes the line header.
						 *      If it can't open file throws an std::bad_exception().
						 *      @param file_name The log file name.
						 *      @param type The type of information to log. This is
						 *                      shown at the begining of the line.
						 *      @todo If it fails to open file throw a more specific exception.
						 */
						LogEntry(const char *file_name, const char *type);
						friend class Logger;
						LogEntry(const LogEntry&) {}
						std::ofstream out;
					};
			/**
			 *      Logs an information.
			 *      @see LogEntry::LogEntry()
			 */
			LogEntry info() { return LogEntry(file_name, "Info"); }
			/**
			 *      Logs a warning.
			 *      @see LogEntry::LogEntry()
			 */
			LogEntry warning() { return LogEntry(file_name, "Warn"); }
			/**
			 *      Logs an error.
			 *      @see LogEntry::LogEntry()
			 */
			LogEntry error() { return LogEntry(file_name, "Error"); }
			/**
			 *      Write a headline to the log.
			 *      Again it throws an std::bad_exeption if it can't open file.
			 *      @param text The headline to show.
			 *      @todo If it fails to open file throw a more specific exception.
			 */
			void headline(const char *text);
			/**
			 *      Helper function. Returns an Ok | Failed string depending
			 *      on the value of the argument.
			 *      @param status True means "Ok", false means "Failed".
			 *      @return The string equivalent to status.
			 */
			static const char* success(bool status) { return _success[status]; }
			/**
			 *      Helper function. Returns a formatted representation
			 *      of the time when called.
			 */
			static const char* now();
			/**
			 *      Use this to break the line instead of std::endl;
			 */
			static const char* endl;
		private:
			const char *file_name;
			static const char *_success[];
		};

#endif
