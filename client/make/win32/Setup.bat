@echo off
rem Daimonin SDL client, a client program for the Daimonin MMORPG.
rem
rem Copyright (C) 2012 Michael Toennies
rem
rem This program is free software; you can redistribute it and/or modify
rem it under the terms of the GNU General Public License as published by
rem the Free Software Foundation; either version 2 of the License, or
rem (at your option) any later version.
rem
rem This program is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU General Public License for more details.
rem
rem You should have received a copy of the GNU General Public License
rem along with this program; if not, write to the Free Software
rem Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
rem
rem The author can be reached via e-mail to info@daimonin.org

setlocal enableextensions

rem Unzip 3rdparty code to current folder.
unxutils\unzip.exe 3rdparty.zip

rem Move DLs to client root folder.
move dll\*.dll ..\..
rmdir dll

rem Find revision number (defaults to exported).
set revision=exported

if exist ..\..\.svn (
    for /F "skip=5 tokens=2" %%a in ('svn info ..\..') do (
        set revision=%%a
        goto :break
    )
)

:break

rem Write revision.h file.
echo #define REVISION "%revision%" > include\revision.h
endlocal
