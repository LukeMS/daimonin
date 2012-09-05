REM *** this should start the editor **
cd ..\..\editor
IF EXIST DaimoninEditor.jar.tmp (
rename DaimoninEditor.jar DaimoninEditor.jar.bak
rename DaimoninEditor.jar.tmp DaimoninEditor.jar)
javaw.exe -Xmx1024m -Xms1024m -jar DaimoninEditor.jar 
cd ..\bin\win32
