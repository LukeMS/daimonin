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

#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <Ogre.h>

/**
** This class provides the reloading of manual created resources.
*****************************************************************************/
class ManResourceLoader
{
private:
    class ResourceLoader : public Ogre::ManualResourceLoader
    {
    public:
        ResourceLoader() {};
        ~ResourceLoader() {};

        /**
        ** This function is called by Ogre
        ** when a manual created resource needs to be reloaded.
        ** All Managers, with manual loaded resources, are called
        ** until the.responsible one was found.
        *****************************************************************************/
        void loadResource(Ogre::Resource *resource);
    } Loader;
    ManResourceLoader()  {}
    ~ManResourceLoader() {}
    ManResourceLoader(const ManResourceLoader&); // disable copy-constructor.
public:
    static ManResourceLoader &getSingleton()
    {
        static ManResourceLoader Singleton; return Singleton;
    }
    ResourceLoader *getLoader()
    {
        return &Loader;
    }
};

#endif
