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

#ifndef ASSERT_MANAGER_H
#define ASSERT_MANAGER_H

#  ifdef _DEBUG
#    define Assert(exp, description, breakpoint) {}
#  else
#    include "logger.h"
#    define Assert(exp, description, breakpoint) AssertManager::getSingleton().logAssert(exp, description, breakpoint, __LINE__, __FILE__);
#    if OGRE_COMPILER == OGRE_COMPILER_MSVC
#      define BREAKPOINT __asm {int 3};
#    else
#      define BREAKPOINT __asm ("int3");
#    endif

/// @brief This singleton class provides an assert function.
/// @details If compiled in release mode, all Assert() calls will be ignored.
class AssertManager
{
public:
    /// @brief Returns the reference to this singleton class.
    static AssertManager &getSingleton()
    {
        static AssertManager Singleton; return Singleton;
    }
    /// @brief Logs an assert (if compiled in debug modus).
    /// @details You can also use the Assert macro to call this function directly: Assert(exp, description, breakpoint)
    /// @param expression    The boolean test expression. If false an assert is logged.
    /// @param description   The despription for the assert.
    /// @param setBreakpoint If true, a breakpoint is set.
    /// @param line          The linenumber where this assert was triggered.
    /// @param file          The file where this assert was triggered.
    void logAssert(bool expression, std::string description, bool setBreakpoint, int line, std::string file)
    {
        if (expression) return;
        Logger::log().debug() << "Assert: " << description << " [line: " << line << " in " << file << "]";
        if (setBreakpoint) { BREAKPOINT }
    }

private:
    /// @brief Default constructor.
    AssertManager() {}
    /// @brief Default destructor.
    ~AssertManager() {}
};

#  endif
#endif
