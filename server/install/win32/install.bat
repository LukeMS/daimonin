rem Daimonin Install Script
rem This is the win32 installer.
rem It will generate the data folder with all needed files
rem and generate the lib folder(if not exits) by copying from default arch
rem (this weak base installer can't handle different /arch folders)
rem c) Michael Toennies 2002/2003

cd ..\..
md lib
copy ..\arch\*.* lib\*.*
md data
md data\tmp
md data\log
md data\players
md data\unique-items
copy install\*. data\*.
