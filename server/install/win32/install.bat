rem Daimonin Install Script
rem This is the win32 installer.
rem It will generate the data folder with all needed files
rem and generate the lib folder(if not exits) by copying from default arch
rem (this weak base installer can't handle different /arch folders)
rem c) Michael Toennies 2002/2003

cd ..\..
if not exist data xcopy install\data data /i /s /e
md lib
copy ..\arch\*.* lib\*.*
