/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>
#include "logfile.h"
#include "mainwindow.h"
#include "editor.h"
#include "mesh.h"
#include "skeleton.h"

using namespace std;

MainWindow   *gpMainWin =0;



void Editor::loadModel()
{
    char *filename = fl_file_chooser("Open Mesh XML-File", "*.mesh.xml", "", Fl_File_Chooser::SINGLE);
    if (!filename) return;
    Mesh::getSingleton().load(filename); 
    Skeleton::getSingleton().load(Mesh::getSingleton().getSkeletonName());
}

//*******************************************************************************************
// Don't let ESC-Button end the program.
//*******************************************************************************************
inline int handle(int e) { return (e == FL_SHORTCUT); }

//*******************************************************************************************
// Main.
//*******************************************************************************************
int main(int argc, char *argv[])
{
    if (!(LogFile::getSingleton().Init())) { return 0; }
    gpMainWin = new MainWindow;

    // Init the mainwindow.
    Fl::add_handler(handle);
    gpMainWin->MWindow->position(1,1);
    gpMainWin->MWindow->show( argc, argv );
//  gpMainWin->MWindow->add(gpGLWin);

    LogFile::getSingleton().Headline("Init System");
    Mesh    ::getSingleton().Init();
    Skeleton::getSingleton().Init();    
    LogFile::getSingleton().Headline("Mainloop");
    while (Fl::check())
    {
    }
    
    if (gpMainWin) { delete gpMainWin; }
    return 0;
}
