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

#ifndef TILE_ATLAS_H
#define TILE_ATLAS_H

class TileAtlas
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {ATLAS_LAND_COLS = 6 /**< Cols of Landtiles in the altlastexture. **/ };
    enum {ATLAS_LAND_ROWS = 7 /**< Rows of Landtiles in the altlastexture. **/ };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static TileAtlas &getSingleton()
    {
        static TileAtlas Singleton; return Singleton;
    }
    /// Create the atlastexture in different resolutions.
    /// @param filenamePrefix The prefix of the atlas texture filename.
    /// @param maxTextureSize The maximum size of the atlas to create.
    /// @param groupNr        How many atlas groups to create. -1 to create all groups found in the media folder.
    /// @return true if the atlas-texture was created successful.
    void createAtlasTexture(Ogre::String &filenamePrefix, Ogre::uint32 groupNr = -1);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::uint32 mTileSize;
    Ogre::uint32 mBorderSize;
    Ogre::String mPathGfxTiles;
    Ogre::uint32 mMaxTextureSize;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileAtlas() {}
    ~TileAtlas(){}
    TileAtlas(const TileAtlas&);               ///< disable copy-constructor.
    TileAtlas &operator=(const TileAtlas&);    ///< disable assignment operator.
    bool copyTileToAtlas(Ogre::uchar *dstBuf); ///< 41 Standard tiles.
    void copyFlowToAtlas(Ogre::uchar *dstBuf); ///<  3 Special tiles with a flow effect (Water/Lava/etc).
    void copySpotToAtlas(Ogre::uchar *dstBuf);
    void copyMaskToAtlas(Ogre::uchar *dstBuf);
    bool loadImage(Ogre::Image &image, const Ogre::String &filename, bool logErrors);
    /// Create a template to make it easier for the artists to create a maskset.
    void createMaskTemplate();
    /// Set the path string with the parsed entry of the 'resources.cfg' file.
    /// Reimplemented
    /// @param resource The directory name to be found in the resources file.
    /// @param path     The path string to be set.
    int setResourcePath(Ogre::String resource, Ogre::String &path);
};

#endif
