How to Compile a Daimonin Server on Windows:

Note:
The daimonin_server.dsp/The daimonin_server.dsw files are for Visual Studio 6.
The daimonin_server.vcproj is a Visual Studio 7 (Visual Studio .NET 2002/2003)
project file.

Please follow the steps in the right order!
(Note: The Visual C++ project is our default one. The way to compile
       will be of course different for other compilers. Browse the make/win32
       folder for your system and read the README.txt inside for more infos.)

o First step: You must compile the program.
  After unpacking the Daimonin package, run the "daimonin_server.dsw".
  There are some projects in your workspace: You need to compile all.
  Easiest way is to select the daimonin_server - ReleaseLog as active
  project, this will compile all others automatically too.
  Then - in visual studio 6 - press <F7> to compile.

  In this folder is a flex.exe. This is the flex
  program, used to generate loader.c from loader.l and reader.c from reader.l.
  The cb/vc project setup will run this automatically using custom build commands.
  Don't change the position of the flex.exe or the folder tree of the projects or
  the flex run will fail! You can change the content of loader.l/reader.l in the
  vc studio like a normal c-file. The compiler will look at the depencies and generate
  the .c files when the .l files are from a newer date. Just compile normal with F7.

o Last step: If the compile was successful, you should have now a
  daimonin_server.exe inside your Daimonin/server folder and a
  lua_plugin.dll inside your Daimonin/server/plugins folder
  (the vc studio will copy this files automatically after a succesfull
  compile to this folders).

  If the files are on the right place, you are done!

  Change to the directory Daimonin/server/install/win32 and read the
  README.txt there.

If you still have troubles, go to the Daimonin home page and
search there for more help. You can also find there mailing lists
for more infos.

http://daimonin.sourceforge.net

Tell us exactly what version of Daimonin you've used.
If it's an old version, update first and see if that solves your problem.
