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

#ifndef TILE_MAP_WRAPPER_H
#define TILE_MAP_WRAPPER_H

#include <iostream>
#include <fstream>
#include <vector>
#include "define.h"
#include "logger.h"

// QUICK HACK
// This will be replaced when the new map protocol is ready.

class ObjectWrapper
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static ObjectWrapper &getSingleton()
    {
        static ObjectWrapper Singleton; return Singleton;
    }

    /** Read the bmaps file from 2d client. All animations states are cut.
        That means there will be only 1 entry per object. */
    void extractObjects()
    {
        loadOrgBmp();
        // ////////////////////////////////////////////////////////////////////
        // Animations are client sided - so we need only the name of the
        // base gfx (skip all animations but the first).
        // ////////////////////////////////////////////////////////////////////
        std::ofstream fbmapOut(FILE_BMAPS_UNIQUE, std::ios::out | std::ios::binary);
        if (!fbmapOut)
        {
            Logger::log().error() << "Error on file " << FILE_BMAPS_UNIQUE;
            return;
        }
        int sumPics=0;
        std::string strTmp;
        for (std::vector<bmpType*>::iterator i = mvBmpEntry.begin(); i < mvBmpEntry.end(); ++i)
        {
            if (strTmp!= (*i)->name2d)
            {
                ++sumPics;
                fbmapOut << (*i)->name2d << std::endl;
            }
            strTmp = (*i)->name2d;
        }
        fbmapOut.close();
    }

    /** Read the bmaps file from 2d client.
        Add all mesh names from the client3d.p0 to the list. */
    void add3dNames()
    {
        loadOrgBmp();
        // ////////////////////////////////////////////////////////////////////
        // Load the bmp converter file.
        // ////////////////////////////////////////////////////////////////////
        std::ifstream fbmapIn;
        fbmapIn.open(FILE_BMAPS_CONVERT, std::ios::in | std::ios::binary);
        if (!fbmapIn)
        {
            Logger::log().error() << "Error on file " << FILE_BMAPS_CONVERT;
        }
        char *name2d = new char[256];
        char *name3d = new char[256];
        while (getline(fbmapIn, mStrBuf))
        {
            name3d[0]=0;
            convNames *entry = new convNames;
            sscanf(mStrBuf.c_str(), "%s %s", name2d, name3d);
            entry->name2d = name2d;
            entry->name3d = name3d;
            mvConvEntry.push_back(entry);
        }
        fbmapIn.close();
        delete[] name2d;
        delete[] name3d;

        // ////////////////////////////////////////////////////////////////////
        // Put the 3d mesh names into the struct.
        // ////////////////////////////////////////////////////////////////////
        for (std::vector<bmpType*>::iterator i = mvBmpEntry.begin(); i < mvBmpEntry.end(); ++i)
        {
            for (std::vector<convNames*>::iterator j = mvConvEntry.begin(); j < mvConvEntry.end(); ++j)
            {
                if ((*i)->name2d == (*j)->name2d)
                {
                    (*i)->name3d = (*j)->name3d;
                    break;
                }
            }
        }

        std::ofstream fbmapOut(FILE_BMAPS_CLIENT3D, std::ios::out | std::ios::binary);
        if (!fbmapOut) return;
        mStrBuf ="";
        int sumPics =0;
        // save it to file.
        for (std::vector<bmpType*>::iterator i = mvBmpEntry.begin(); i < mvBmpEntry.end(); ++i)
        {
            fbmapOut << sumPics++ << " " << (*i)->name2d << " " << (*i)->name3d << std::endl;
            mStrBuf = (*i)->name2d;
        }
        fbmapOut.close();

        // ////////////////////////////////////////////////////////////////////
        // Cleanup.
        // ////////////////////////////////////////////////////////////////////
        for (std::vector<convNames*>::iterator i = mvConvEntry.begin(); i < mvConvEntry.end(); ++i)
            delete (*i);
        mvConvEntry.clear();
    }

    void readObjects()
    {
        std::ifstream fbmapIn(FILE_BMAPS_CLIENT3D, std::ios::in | std::ios::binary);
        if (!fbmapIn)
        {
            std::cout << "error\n";
            return;
        }
        char *name2d = new char[256];
        char *name3d = new char[256];

        while (getline(fbmapIn, mStrBuf))
        {
            name2d[0]=0;
            name3d[0]=0;
            bmpType *entry = new bmpType;
            sscanf(mStrBuf.c_str(), "%d %s %s", &entry->num, name2d, name3d);
            // Cut the animation number.
            if (name2d && name2d[0])
                entry->name2d = name2d;
            else
                entry->name2d = "";
            if (name3d && name3d[0])
                entry->name3d = name3d;
            else
                entry->name3d = "";
            mvBmpEntry.push_back(entry);
        }
        fbmapIn.close();
        delete[] name2d;
        delete[] name3d;
    }

    const char *getMeshName(unsigned int objectNr)
    {
        if (objectNr >= mvBmpEntry.size()) return "";
        return mvBmpEntry[objectNr]->name3d.c_str();
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        int  len, pos, num;
        std::string name2d;
        std::string name3d;
        unsigned int crc;
    }
    bmpType;

    typedef struct
    {
        std::string name2d;
        std::string name3d;
    }
    convNames;
    std::vector<bmpType*> mvBmpEntry;
    std::vector<convNames*> mvConvEntry;
    std::string mStrBuf;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectWrapper()
    {}

    ~ObjectWrapper()
    {
        for (std::vector<bmpType*>::iterator i = mvBmpEntry.begin(); i < mvBmpEntry.end(); ++i)
            delete (*i);
        mvBmpEntry.clear();
    }

    ObjectWrapper( const ObjectWrapper& ); // disable copy-constructor.

    void loadOrgBmp()
    {
        // ////////////////////////////////////////////////////////////////////
        // Load the original bmp file.
        // ////////////////////////////////////////////////////////////////////
        std::ifstream fbmapIn(FILE_CLIENT_BMAPS, std::ios::in | std::ios::binary);
        if (!fbmapIn)
        {
            Logger::log().error() << "Error on file " << FILE_CLIENT_BMAPS;
            return;
        }
        char *name = new char[256];
        char *nameT = new char[256];
        std::string strTmp;
        while (getline(fbmapIn, mStrBuf))
        {
            bmpType *entry = new bmpType;
            sscanf(mStrBuf.c_str(), "%s %s %s", nameT, nameT, name);
            entry->name2d = name;
            if (entry->name2d.find("wall", 0) == std::string::npos && entry->name2d.find("door", 0) == std::string::npos)
            {
                // Cut the animation number.
                entry->name2d = entry->name2d.substr(0, entry->name2d.find( '.'));
            }
            mvBmpEntry.push_back(entry);
            entry->name3d = "";
        }
        fbmapIn.close();
        delete[] name;
        delete[] nameT;
    }
};

#endif
