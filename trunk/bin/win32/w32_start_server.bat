rem echo off
rem LFNOR ON is needed for windows 98
rem if a warning about unknown command is
rem given out, your windows system already
rem can handle long file names on default (XP, ...)
LFNFOR ON
copy ..\..\arch\*.* ..\..\server\lib\*.*
mkdir ..\..\server\data\tmp
cd ..\..\server\data\tmp
del ?*.*
cd ..\..
daimonin_server.exe -log logfile.log
cd ..\bin\win32