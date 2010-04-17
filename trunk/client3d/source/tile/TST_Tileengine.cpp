#include "ExampleApplication.h"
#include "tile/tile_manager.h"
#include "tile/tile_decal.h"
#include "logger.h"

class Demo : public ExampleApplication
{
public:
    Demo()  {}
    ~Demo() {}
private:
    void createScene(void)
    {
        const Real CAMERA_POS_Z = TileManager::TILE_RENDER_SIZE * TileManager::CHUNK_SIZE_Z;
        mCamera->pitch(Degree(-35));
        //mCamera->setFOVy(Degree(50)); // Height of the camera.
        //mCamera->setAspectRatio(Real(VP->getActualWidth()) / Real(VP->getActualHeight()));
        mCamera->setPosition(TileManager::TILE_RENDER_SIZE * (TileManager::CHUNK_SIZE_X+1), CAMERA_POS_Z, 2*CAMERA_POS_Z);
        int lod = 1;
        int queryMaskLand  = 1 << 2;
        int queryMaskSprite= 1 << 1;
        int sizeMap = 32;
        TileManager::getSingleton().Init(mSceneMgr, queryMaskLand, queryMaskSprite, lod, false);
        TileManager::getSingleton().setGrid(true);
        //TileManager::getSingleton().setGrid(false);

        for (int y=0; y < sizeMap; ++y)
            for (int x=0; x < sizeMap; ++x)
                TileManager::getSingleton().setMap(x, y, 60, 1, 0);
        // Mask demo.
        int gfxBG = 1, gfx0  = 2, gfx1  = 7;
        int x = 2, y = 2; // Mask 0
        TileManager::getSingleton().setMap(x+0, y+0, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfx0 , 0);
        TileManager::getSingleton().setMap(x+0, y+1, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfx1 , 0);
        TileManager::getSingleton().setMap(x+0, y+2, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfx0 , 0);
        x+= 4;            // Mask 1
        TileManager::getSingleton().setMap(x+0, y+0, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfx1 , 0);
        TileManager::getSingleton().setMap(x+0, y+1, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfx0 , 0);
        TileManager::getSingleton().setMap(x+0, y+2, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfx1 , 0);
        y+= 4; x= 2;      // Mask 2
        TileManager::getSingleton().setMap(x+0, y+0, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfxBG, 0);
        TileManager::getSingleton().setMap(x+0, y+1, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfx1 , 0);
        TileManager::getSingleton().setMap(x+0, y+2, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfxBG, 0);
        x+= 4;            // Mask 3
        TileManager::getSingleton().setMap(x+0, y+0, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfxBG, 0);
        TileManager::getSingleton().setMap(x+0, y+1, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfx0 , 0);
        TileManager::getSingleton().setMap(x+0, y+2, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfxBG, 0);
        y+= 4;  x= 2;     // Mask 4
        TileManager::getSingleton().setMap(x+0, y+0, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfx1 , 0);
        TileManager::getSingleton().setMap(x+0, y+1, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfxBG, 0);
        TileManager::getSingleton().setMap(x+0, y+2, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfx1 , 0);
        x+= 4;            // Mask 5
        TileManager::getSingleton().setMap(x+0, y+0, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfx0 , 0);
        TileManager::getSingleton().setMap(x+0, y+1, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfxBG, 0);
        TileManager::getSingleton().setMap(x+0, y+2, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfx0 , 0);
        // Some height.
        TileManager::getSingleton().setMap(16,  4,  99, 4, 0);
        TileManager::getSingleton().setMap(17,  4, 111, 4, 0);
        TileManager::getSingleton().setMap(16,  5,  60, 4, 0);
        TileManager::getSingleton().setMap(17,  5,  80, 4, 0);
        TileManager::getSingleton().setMap(18,  5,  80, 4, 0);

        // Some water.
        /*
                TileManager::getSingleton().setMap(10,  8, 30, 6, 48);
                TileManager::getSingleton().setMap(11,  8, 30, 6, 48);
                TileManager::getSingleton().setMap(10,  9, 30, 6, 48);
                TileManager::getSingleton().setMap(11,  9, 30, 6, 48);
                TileManager::getSingleton().setMap(12,  9, 30, 6, 48);
                TileManager::getSingleton().setMap(11,  4, 30, 6, 48);
                TileManager::getSingleton().setMap(12,  4, 30, 6, 48);
                TileManager::getSingleton().setMap(11,  5, 30, 6, 48);
                TileManager::getSingleton().setMap(12,  5, 30, 6, 48);
                TileManager::getSingleton().setMap(13,  5, 30, 6, 48);
        */
        //                                  x,  z    h, gfx, h, shadow, hard edge, spot
        TileManager::getSingleton().setMap(17,  8,  60,   1, 0, 0,      0, true);
        TileManager::getSingleton().setMap(18,  9,  60,   2, 0, 0,      0, true);
        TileManager::getSingleton().updateChunks();
    }

    void destroyScene(void)
    {
        TileManager::getSingleton().freeRecources();
    }
};

int main(int argc, char **argv)
{
    Demo terrain;
    terrain.go();
    return 0;
}
