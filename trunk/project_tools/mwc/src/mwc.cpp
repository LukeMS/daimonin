/***********************************
 ***                             ***
 ***     Mahewa World Creator    ***
 ***           (MWC)             ***
 ***  (C) 2007 by Sammy Broock   ***
 ***                             ***
 ***      Version :   0.01       ***
 ***                             ***
 ***********************************/

/*  This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

//#pragma warning (disable:4430)
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <time.h>
#include "Bitmap.h"

using namespace std;

    /* deklaration of globals*/
    string      author,                 // author of the mapset
                area,                   // name of the maps (shown in client)
                filename,               // major filename of the maps
                current_filename,       // temporary filenames
                temp_filename, 
                input_filename,
                level;                  // specifies an overground or underground level

    int         bitmap_height,          // height of an input bitmap
                bitmap_width,           // width of an input bitmap
                map_size_x, 
                map_size_y,             // defines the tiles-width and -heigth of a map
                area_size_x, 
                area_size_y,            // defines the map-width and -heigth of a mapset
                sublevel,               // specifies an overground or underground level
                map_x,
                map_y,                  // holds the current position of a map
                area_x,
                area_y,                 // holds the current position of a maparea
                maps,                   // number of maps
                terrain,
				difficulty;				// Difficulty level of the map

    int         help,                   // dummyvariables
                x,
                y,
                r,
                g,
                b;

    char        field[24][24];          // array to hold data to create map from a textfile
    char        token;                  // data from a textfile
    bool        outdoor,                // is the mapset an outdoor or indoor area
                map_with_bitmap;        // should mwc create the maps using a bitmap

    int         pic_map[360][360][3];   // array to hold the bitmapdata (max 15x15 maps)

    ofstream    dat_aus;                // outputstream
    ifstream    dat_ein;                // inputstream

/* utilityfunctions */

string stringify_int (unsigned int x)   // converts an unsigned int into a string
  {
    string res = "";
    while (x > 0)
      {
        res = char('0'+(x-(x/10)*10))+res;
        x = x/10;
      }
    return res;
  }

string stringtolower (string stri)      // sets a string into lowercase
  {
    for (unsigned int i=0;i<stri.length();i++)
      {
        stri[i] = tolower(stri[i]);
      }
    return stri;
  }

/*
   Normalize the input string into a compatible map path.
   E.g. User may name a map "Lost City". So the name for all
   in the set "Lost City", but the file name should be 
   "lost_city_0101" or whatever.
*/
string normalize_path(string path)
{
    path = stringtolower(path);

	for(unsigned int i = 0; i < path.length(); i++)
	{
		if(path[i] == ' '){
			path[i] = '_';
		}
	}

	return path;
  }

string leadingzero (string stri)        // adds a leading "0" in front of stringnumbers < 10
  {
    if (stri.length() == 1)
      return "0"+stri;
    else
      return stri;
  }

string timeasstring ()                  // returns the system time as a string
  {
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    return asctime(timeinfo);
  }

int mwc_random (int area)               // returns a random number out of a given area
  {
    return 1+(int)((float)area*(rand()/(RAND_MAX+1.0)));
  }

/* bitmap functions */
/* a special thank you to Mark Bernard
   on GameDev.net: Captain Jester
   e-mail: mark.bernard@rogers.com 
   You started me off on the right track ( for sure !!! ;-) )
   Bitmap.h is as well a portion from Mark
*/

//basic constructor
Bitmap::Bitmap()
  {
    reset();
  }

//constructor loads the bitmap when it is created
Bitmap::Bitmap(char *file)
  {
    reset();
    loadBMP(file);
  }

//destructor
Bitmap::~Bitmap()
  {
    if(colours!=0) {
        delete[] colours;
    }
    if(data!=0) {
        delete[] data;
    }
  }

//load a bitmap from a file and represent it correctly
//in memory
bool Bitmap::loadBMP(char *file)
  {
    FILE *in;                           //file stream for reading
    char *tempData;                     //temp storage for image data
    int numColours;                     //total available colours

    //bitmap is not loaded yet
    loaded=false;

    //make sure memory is not lost
    if(colours!=0) 
      {
        delete[] colours;
      }
    if(data!=0) 
      {
        delete[] data;
      }

    //open the file for reading in binary mode
    in=fopen(file,"rb");

    //if the file does not exist return in error
    if(in==NULL) 
      {
        error="File not found";
        fclose(in);
        return false;
      }

    //read in the entire BITMAPFILEHEADER
    fread(&bmfh,sizeof(BitmapFileHeader),1,in);
    cout << "sizeof(BitmapFileHeader)=" << sizeof(BitmapFileHeader) << endl;

    //check for the magic number that says this is a bitmap
    if(bmfh.bfType!=BITMAP_MAGIC_NUMBER) 
      {
        error="File is not in DIB format";
        fclose(in);
        return false;
      }

    //read in the entire BITMAPINFOHEADER
    fread(&bmih,sizeof(BitmapInfoHeader),1,in);
    cout << "sizeof(BitmapInfoHeader)=" << sizeof(BitmapInfoHeader) << endl;

    //save the width, height and bits per pixel for external use
    width=bmih.biWidth;
    height=bmih.biHeight;
    bpp=bmih.biBitCount;

    cout << "biBitCount      =" << bmih.biBitCount << endl;
    cout << "biClrImportant  =" << bmih.biClrImportant << endl;
    cout << "biClrUsed       =" << bmih.biClrUsed << endl;
    cout << "biCompression   =" << bmih.biCompression << endl;
    cout << "biHeight        =" << bmih.biHeight << endl;
    cout << "biPlanes        =" << bmih.biPlanes << endl;
    cout << "biSize          =" << bmih.biSize << endl;
    cout << "biSizeImage     =" << bmih.biSizeImage << endl;
    cout << "biWidth         =" << bmih.biWidth << endl;
    cout << "biXPelsPerMeter =" << bmih.biXPelsPerMeter << endl;
    cout << "biYPelsPerMeter =" << bmih.biYPelsPerMeter << endl;

    //calculate the size of the image data with padding
    dataSize=(width*height*(unsigned int)(bmih.biBitCount/8.0));

    //calculate the number of available colours
    numColours=1<<bmih.biBitCount;

    //if the bitmap is not 8 bits per pixel or more
    //return in error
    if(bpp<8) 
      {
        error="File is not 8 or 24 bits per pixel";
        fclose(in);
        return false;
      }

    //load the palette for 8 bits per pixel
    if(bpp==8) 
      {
        colours=new RGBQuad[numColours];
        fread(colours,sizeof(RGBQuad),numColours,in);
      }

    //set up the temporary buffer for the image data
    tempData=new char[dataSize];

    //exit if there is not enough memory
    if(tempData==NULL) 
      {
        error="Not enough memory to allocate a temporary buffer";
        fclose(in);
        return false;
      }

    //read in the entire image
    fread(tempData,sizeof(char),dataSize,in);

    //close the file now that we have all the info
    fclose(in);

    //calculate the witdh of the final image in bytes
    byteWidth=padWidth=(int)((float)width*(float)bpp/8.0);

    //adjust the width for padding as necessary
    while(padWidth%4!=0) 
      {
        padWidth++;
      }

    //change format from GBR to RGB
    if(bpp==8) 
      {
        loaded=convert8(tempData);
      }
    else 
      if(bpp==24) 
        {
          loaded=convert24(tempData);
        }

    //clean up memory
    delete[] tempData;

    //bitmap is now loaded
    error="Bitmap loaded";

    //return success
    return loaded;
  }

//function to set the inital values
void Bitmap::reset(void) 
  {
    loaded=false;
    colours=0;
    data=0;
    error="";
  }

bool Bitmap::convert24(char* tempData) 
  {
    int offset,diff;
    diff=width*height*RGB_BYTE_SIZE;

    //allocate the buffer for the final image data
    data=new char[diff];

    //exit if there is not enough memory
    if(data==NULL) 
      {
        error="Not enough memory to allocate an image buffer";
        delete[] data;
        return false;
      }

    if(height>0) 
      {
        offset=padWidth-byteWidth;

        //count backwards so you start at the front of the image
        for(unsigned int i=0;i<dataSize;i+=3) 
          {
            //jump over the padding at the start of a new line
            if((i+1)%padWidth==0) 
              {
                i+=offset;
              }

            //transfer the data
            *(data+i+2)=*(tempData+i);
            *(data+i+1)=*(tempData+i+1);
            *(data+i)=*(tempData+i+2);
          }
      }

    //image parser for a forward image
    else 
      {
        offset=padWidth-byteWidth;
        int j=dataSize-3;
        //count backwards so you start at the front of the image
        //here you can start from the back of the file or the front,
        //after the header  The only problem is that some programs
        //will pad not only the data, but also the file size to
        //be divisible by 4 bytes.
        for(unsigned int i=0;i<dataSize;i+=3) 
          {
            //jump over the padding at the start of a new line
            if((i+1)%padWidth==0) 
              {
                i+=offset;
              }

            //transfer the data
            *(data+j+2)=*(tempData+i);
            *(data+j+1)=*(tempData+i+1);
            *(data+j)=*(tempData+i+2);
            j-=3;
          }
      }

    return true;
}

bool Bitmap::convert8(char* tempData) 
  {
    int offset,diff;

    diff=width*height*RGB_BYTE_SIZE;

    //allocate the buffer for the final image data
    data=new char[diff];

    //exit if there is not enough memory
    if(data==NULL) 
      {
        error="Not enough memory to allocate an image buffer";
        delete[] data;
        return false;
      }

    if(height>0) 
      {
        offset=padWidth-byteWidth;
        int j=0;

        //count backwards so you start at the front of the image
        for(unsigned int i=0;i<dataSize*RGB_BYTE_SIZE;i+=3) 
          {
            //jump over the padding at the start of a new line
            if((i+1)%padWidth==0) 
              {
                i+=offset;
              }
            //transfer the data
            *(data+i)=colours[*(tempData+j)].rgbRed;
            *(data+i+1)=colours[*(tempData+j)].rgbGreen;
            *(data+i+2)=colours[*(tempData+j)].rgbBlue;
            j++;
          }
      }

    //image parser for a forward image
    else 
      {
        offset=padWidth-byteWidth;
        int j=dataSize-1;

        //count backwards so you start at the front of the image
        for(unsigned int i=0;i<dataSize*RGB_BYTE_SIZE;i+=3) 
          {
            //jump over the padding at the start of a new line
            if((i+1)%padWidth==0) 
              {
                i+=offset;
              }
            //transfer the data
            *(data+i)=colours[*(tempData+j)].rgbRed;
            *(data+i+1)=colours[*(tempData+j)].rgbGreen;
            *(data+i+2)=colours[*(tempData+j)].rgbBlue;
            j--;
          }
      }

    return true;
  }


/* this function is modified by me to get the rgb-data in the pic_map array
   and the width and the height into the global variables bitmap_width / bitmap_height */

bool getfeld(char *bitmapname)
  {
    Bitmap *image;
    image=new Bitmap();
    int fx,fy,fz;

    if (image==NULL)
      {
        return false;
      }

    if (image->loadBMP(bitmapname))
      {
        cout << bitmapname << " loaded successfully" << endl;;
        fx = 0;
        fy = image->height-1;
        fz = 0;
        for(unsigned int i=0;i<image->dataSize;i+=3) 
          {
            if (*(image->data+i)<0) 
              pic_map[fx][fy][fz] = (256 + *(image->data+i));
            else 
              pic_map[fx][fy][fz] = *(image->data+i);
            if (*(image->data+i+1)<0) 
              pic_map[fx][fy][fz+1] = (256 + *(image->data+i+1));
            else 
              pic_map[fx][fy][fz+1] = *(image->data+i+1);
            if (*(image->data+i+2)<0) 
              pic_map[fx][fy][fz+2] = (256 + *(image->data+i+2));
            else 
              pic_map[fx][fy][fz+2] = *(image->data+i+2);
            fx++;
            if (fx >= image->width)
              {
                fx = 0;
                fy--;
              }
          }
        bitmap_width = image->width;
        bitmap_height = image->height;
      }
    else
      {
        return false;
      }
    if (image)
      {
        delete image;
      }
    return true;
  }

/* Main routine */

int main() {
    /* set the randomize seed according to the systemtime */
    srand(time(NULL));

    /* title */
    cout << "****************************" << endl;
    cout << "*                          *" << endl;
    cout << "*   Mahewa World Creator   *" << endl;
    cout << "*                          *" << endl;
    cout << "* (C) 2007 by Sammy Broock *" << endl;
    cout << "*                          *" << endl;
    cout << "****************************" << endl;
    cout << endl;

    /* datainput */
	/*
	  Area and author use getline() because plain ol' cin
	  Doesn't work with spaces in names. So "The Hills of Rome"
	  would end up named "the". 
	*/
    cout << "Please enter the name of the mapset : ";
	getline(cin, area);
	cout << endl;
    cout << "Please enter the author of the mapset : ";
	getline(cin, author);
	cout << endl;
    filename = normalize_path(area)+"_";
    cout << "Please enter the areasize (max 15) (x * maps) : ";
    cin >> area_size_x;
    cout << endl;
    cout << "Please enter the areasize (max 15) (y * maps) : ";
    cin >> area_size_y;
	cout << endl;
	cout << "Please enter the difficulty level of the area (1-110) : ";
	cin >> difficulty;
    cout << endl;
    cout << "Is this an outdoor mapset ? (1-yes / 2-no) : ";
    cin >> help;
    cout << endl;
    if (help != 1)
      outdoor = false;
    else
      outdoor = true;
    cout << "Please enter the heightlevel : (0 for groundlevel / 1-5 for sublevel / 11-15 for upper level) :";
    cin >> sublevel;
    cout << endl;
    switch (sublevel)
      {
        case 1 : level = "b_"; break;
        case 2 : level = "c_"; break;
        case 3 : level = "d_"; break;
        case 4 : level = "e_"; break;
        case 5 : level = "f_"; break;
        case 11 : level = "1_"; break;
        case 12 : level = "2_"; break;
        case 13 : level = "3_"; break;
        case 14 : level = "4_"; break;
        case 15 : level = "5_"; break;
        default : level = ""; break;
      }
    map_size_x = 24;                    // standard height and width for a map
    map_size_y = 24;
    cout << "Create maps with a bitmap ? (max 360x360 pixel)(1-yes / 2-no)";
    cin >> help;
    cout << endl;
    if (help != 1)
      map_with_bitmap = false;
    else
      {
        cout << "Enter name of bitmap (must be a 24 bit BMP";
        cin >> temp_filename;
        cout << endl;
        char *bitmap_filename = new char[temp_filename.length()+1];
        strcpy(bitmap_filename, temp_filename.c_str());
        map_with_bitmap = false;
        if (getfeld(bitmap_filename))
          {
            if ((bitmap_height >= area_size_y*map_size_y) && (bitmap_width >= area_size_x*map_size_x))
              {
                map_with_bitmap = true;
              }
          }
        if (!map_with_bitmap)
          cout << "Error comparing bitmapsize with areasize !" << endl;
      }

    /* verfiy data */
    cout << endl << endl << "*********************************" << endl;
    cout << "Author     : " << author << endl;
    cout << "Mapset     : " << area << endl;
    cout << "Filename   : " << filename+level << endl;
    cout << "Areasize   : " << area_size_x << "x" << area_size_y << " maps" << endl;
    cout << "Mapsize    : " << map_size_x << "x" << map_size_y << " tiles" << endl;
    cout << "Difficulty : " << difficulty << endl;
    if (outdoor)
      cout << "Outdoor    : YES" << endl;
    else
      cout << "Outdoor    : NO" << endl;

    if (map_with_bitmap)
      cout << "BMP-Map    : " << temp_filename << endl;

    cout << "*********************************" << endl;
    cout << "Continue with this data ? (1-yes / 2-no) :"; 
    cin >> help;
    cout << endl;
    if (help != 1)
      {
        cout << "Interrupt by user !" << endl;
        return 0;
      }

    maps = 0;

    for (area_y = 1; area_y <= area_size_y; area_y++)
      {
        for (area_x = 1; area_x <= area_size_x; area_x++)
          {
            current_filename = filename +level +leadingzero(stringify_int(area_x)) +leadingzero(stringify_int(area_y));
            dat_aus.open(current_filename.c_str(),ios_base::out);
            if (!dat_aus)
              {
                cout << current_filename << " konnte nicht geöffnet werden !" << endl;
                return 1;
              }
            //write data into file
            dat_aus << "arch map\n";
            dat_aus << "name " << area << "\n";
            dat_aus << "msg\n";
            dat_aus << "Creator: " << author << "\n";
            dat_aus << "Date: " << timeasstring();
            dat_aus << "Created with MWC\n";
            dat_aus << "endmsg\n";
            dat_aus << "width " << stringify_int(map_size_x) << "\n";
            dat_aus << "height " << stringify_int(map_size_y) << "\n";
            dat_aus << "difficulty " << difficulty << "\n";
            if (outdoor)
              dat_aus << "outdoor 1\n";

            // set tile_paths if necessary

            if (area_y < area_size_y)         // is there a map in the north ?
              {
                temp_filename = filename +level +leadingzero(stringify_int(area_x)) +leadingzero(stringify_int(area_y+1));
                dat_aus << "tile_path_1 " << temp_filename << "\n";
              }
            if (area_x < area_size_x)         // is there a map in he east ?
              {
                temp_filename = filename +level +leadingzero(stringify_int(area_x+1)) +leadingzero(stringify_int(area_y));
                dat_aus << "tile_path_2 " << temp_filename << "\n";
              }
            if (area_y > 1)                   // is there a map in the south ?
              {
                temp_filename = filename +level +leadingzero(stringify_int(area_x)) +leadingzero(stringify_int(area_y-1));
                dat_aus << "tile_path_3 " << temp_filename << "\n";
              }
            if (area_x > 1)                   // is there a map in the west ?
              {
                temp_filename = filename +level +leadingzero(stringify_int(area_x-1)) +leadingzero(stringify_int(area_y));
                dat_aus << "tile_path_4 " << temp_filename << "\n";
              }
            if (area_x < area_size_x)         // map in north east ?
              {
                if (area_y < area_size_y)
                  {
                    temp_filename = filename +level +leadingzero(stringify_int(area_x+1)) +leadingzero(stringify_int(area_y+1));
                dat_aus << "tile_path_5 " << temp_filename << "\n";
                  }
              }
            if (area_x < area_size_x)         // map in the south east ?
              {
                if (area_y > 1)
                  {
                    temp_filename = filename +level +leadingzero(stringify_int(area_x+1)) +leadingzero(stringify_int(area_y-1));
                dat_aus << "tile_path_6 " << temp_filename << "\n";
                  }
              }
            if (area_x > 1)                   // map in the south west ?
              {
                if (area_y > 1)
                  {
                    temp_filename = filename +level +leadingzero(stringify_int(area_x-1)) +leadingzero(stringify_int(area_y-1));
                dat_aus << "tile_path_7 " << temp_filename << "\n";
                  }
              }
            if (area_x > 1)                   // map in the north west ?
              {
                if (area_y < area_size_y)
                  {
                    temp_filename = filename +level +leadingzero(stringify_int(area_x-1)) +leadingzero(stringify_int(area_y+1));
                dat_aus << "tile_path_8 " << temp_filename << "\n";
                  }
              }
            dat_aus << "end\n";

            cout << "******************" << endl;

            // creatiuon of the mapdata

            if (!map_with_bitmap)      // mapcreation without a bitmap
              {
                cout << current_filename << " : (1-grass / 2-stone / 3-water / 4-earth / 5-forrest / 6-from file) :";
                cin >> terrain;
                cout << endl;

                if (terrain != 6)     // creation of randommaps with a given terrain
                  {
                    for (x=0; x < map_size_x; x++)
                      for (y=0; y < map_size_y; y++)
                        {
                          dat_aus << "arch ";
                          switch (terrain)
                            {
                              case 1 : dat_aus << "grassd_"+stringify_int(mwc_random(8)) << "\n"; break;
                              case 2 : dat_aus << "floor_stone_b"+stringify_int(mwc_random(4)) << "\n"; break;
                              case 3 : dat_aus << "water_still_deepest\n"; break;
                              case 4 : dat_aus << "earth_brown"+stringify_int(mwc_random(3)) << "\n"; break;
                              case 5 :
                                dat_aus << "grassd_"+stringify_int(mwc_random(8)) << "\n";
                                if (mwc_random(100)<10)
                                  {
                                    if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                                    if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                                    dat_aus << "end\n" << "arch lo_tree"+stringify_int(mwc_random(5)) << "\n";
                                  }
                              break;
                              default : dat_aus << "grassd_"+stringify_int(mwc_random(8)) << "\n"; break;
                            }
                          if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                          if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                          dat_aus << "end\n";
                        }
                  }
                else                  // creation of a map according to a textfile
                  {
                    input_filename = current_filename+".txt";
                    dat_ein.open(input_filename.c_str(), ios_base::in);
                    if (!dat_ein)
                      {
                        cout << "Couldn't open : " << input_filename << endl;
                        for (x=0;x<24;x++)
                          for (y=0;y<24;y++)
                            field[x][y] = 'g';
                      }
                    else
                      {
                        x = 0;
                        y = 0;
                        while (!dat_ein.eof())
                          {
                            dat_ein.get(token);
                            if (x<24)
                              {
                                field[x][y] = token;
                                x++;
                              }
                            else
                              {
                                x = 0;
                                y++;
                              }
                          }
                        dat_ein.close();
                      }
                    for (x=0;x<24;x++)
                      for (y=0;y<24;y++)
                        {
                          switch (field[x][y])
                            {
                              case 'g' : dat_aus << "arch grassd_"+stringify_int(mwc_random(8)) << "\n"; break;
                              case 'G' : dat_aus << "arch grass_thick"+stringify_int(mwc_random(3)) << "\n"; break;
                              case 'w' : dat_aus << "arch water_still\n"; break;
                              case 'W' : dat_aus << "arch water_still_deep\n"; break;
                              case '0' : dat_aus << "arch water_still_deepest\n"; break;
                              case 's' : dat_aus << "arch floor_stone_b"+stringify_int(mwc_random(4)) << "\n"; break;
                              case 'S' : dat_aus << "arch floor_nstone"+stringify_int(mwc_random(3)) << "\n"; break;
                              case 'f' : dat_aus << "arch fstone1";
                                switch (mwc_random(3))
                                  {
                                    case 1 : dat_aus << "a";break;
                                    case 2 : dat_aus << "b";break;
                                    case 3 : dat_aus << "c";break;
                                    default : dat_aus << "a";break;
                                  }
                                dat_aus << "\n"; 
                              break;
                              case 'F' : dat_aus << "arch floor_flagm"+stringify_int(mwc_random(4)) << "\n"; break;
                              case 'b' : dat_aus << "arch floor_fs_grey1";
                                switch (mwc_random(4))
                                  {
                                    case 1 : dat_aus << "a";break;
                                    case 2 : dat_aus << "b";break;
                                    case 3 : dat_aus << "c";break;
                                    case 4 : dat_aus << "d";break;
                                    default : dat_aus << "a";break;
                                  }
                                dat_aus << "\n"; 
                              break;
                              case 'B' : dat_aus << "arch floor_fstone2";
                                switch (mwc_random(10))
                                  {
                                    case 1 : dat_aus << "a";break;
                                    case 2 : dat_aus << "b";break;
                                    case 3 : dat_aus << "c";break;
                                    case 4 : dat_aus << "d";break;
                                    case 5 : dat_aus << "e";break;
                                    case 6 : dat_aus << "f";break;
                                    case 7 : dat_aus << "g";break;
                                    case 8 : dat_aus << "h";break;
                                    case 9 : dat_aus << "i";break;
                                    case 10 : dat_aus << "j";break;
                                    default : dat_aus << "a";break;
                                  }
                                dat_aus << "\n"; 
                              break;
                              case 'r' : dat_aus << "arch floor_sblack"+stringify_int(mwc_random(3)) << "\n"; break;
                              case 'R' : dat_aus << "arch floor_sblack"+stringify_int(mwc_random(3)) << "b\n"; break;
                              case 'e' : dat_aus << "arch earth_brown"+stringify_int(mwc_random(3)) << "b\n"; break;
                              case 'E' : dat_aus << "arch earth_dark"+stringify_int(mwc_random(3)) << "b\n"; break;
                              default : dat_aus << "arch grassd_"+stringify_int(mwc_random(8)) << "\n"; break;
                            }
                          if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                          if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                          dat_aus << "end\n";
                        }
                  }
              }
            else
              {
                /* read data from pic_map array */

                cout << current_filename << endl;
                for (x=0; x < map_size_x; x++)
                  {
                    for (y=0; y < map_size_y; y++)
                      {
                        // get red / green / blue data from pic_map array
                        r = pic_map[(area_x-1)*map_size_x+x][((bitmap_height)-(area_y*map_size_y))+y][0];
                        g = pic_map[(area_x-1)*map_size_x+x][((bitmap_height)-(area_y*map_size_y))+y][1];
                        b = pic_map[(area_x-1)*map_size_x+x][((bitmap_height)-(area_y*map_size_y))+y][2];
                        cout << r << "/" << g << "/" << b << " ";

                        if (r==68&&g==136&&b==68)
                          {
                            dat_aus << "arch grassd_"+stringify_int(mwc_random(8)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==51&&g==102&&b==51)
                          {
                            dat_aus << "arch grass_thick"+stringify_int(mwc_random(2)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==34&&g==68&&b==34)
                          {
                            dat_aus << "arch grass_leaf"+stringify_int(mwc_random(2)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==102&&g==187&&b==102)
                          {
                            dat_aus << "arch grass"+stringify_int(mwc_random(3)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==153&&g==153&&b==153)
                          {
                            dat_aus << "arch floor_sblack"+stringify_int(mwc_random(3)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==102&&g==102&&b==102)
                          {
                            dat_aus << "arch floor_sblack"+stringify_int(mwc_random(3)) << "b\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==68&&g==68&&b==68)
                          {
                            dat_aus << "arch fstone1";
                            switch (mwc_random(3))
                              {
                                case 1 : dat_aus << "a\n"; break;
                                case 2 : dat_aus << "b\n"; break;
                                case 3 : dat_aus << "c\n"; break;
                              }
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==187&&g==187&&b==187)
                          {
                            dat_aus << "arch floor_fs_grey1";
                            switch (mwc_random(4))
                              {
                                case 1 : dat_aus << "a\n"; break;
                                case 2 : dat_aus << "b\n"; break;
                                case 3 : dat_aus << "c\n"; break;
                                case 4 : dat_aus << "d\n"; break;
                              }
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==119&&g==85&&b==85)
                          {
                            dat_aus << "arch floor_fstone2";
                            switch (mwc_random(10))
                              {
                                case 1 : dat_aus << "a\n"; break;
                                case 2 : dat_aus << "b\n"; break;
                                case 3 : dat_aus << "c\n"; break;
                                case 4 : dat_aus << "d\n"; break;
                                case 5 : dat_aus << "e\n"; break;
                                case 6 : dat_aus << "f\n"; break;
                                case 7 : dat_aus << "g\n"; break;
                                case 8 : dat_aus << "h\n"; break;
                                case 9 : dat_aus << "i\n"; break;
                                case 10 : dat_aus << "j\n"; break;
                              }
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==170&&g==119&&b==119)
                          {
                            dat_aus << "arch floor_stone_b"+stringify_int(mwc_random(4)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==102&&g==51&&b==51)
                          {
                            dat_aus << "arch earth_brown"+stringify_int(mwc_random(3)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==136&&g==136&&b==102)
                          {
                            dat_aus << "arch floor_sand_d"+stringify_int(mwc_random(4)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==136&&g==136&&b==170)
                          {
                            dat_aus << "arch floor_nstone"+stringify_int(mwc_random(3)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==153&&g==255&&b==153)
                          {
                            dat_aus << "arch grass_leaf"+stringify_int(mwc_random(2)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                            dat_aus << "arch lo_tree"+stringify_int(mwc_random(5)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==85&&g==85&&b==255)
                          {
                            dat_aus << "arch water_still_deepest\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==153&&g==153&&b==255)
                          {
                            dat_aus << "arch water_still_deep\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==204&&g==204&&b==255)
                          {
                            dat_aus << "arch water_still\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                        if (r==170&&g==221&&b==221)
                          {
                            dat_aus << "arch floor_sblack"+stringify_int(mwc_random(3)) << "\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                            dat_aus << "arch fwall_fstone2\n";
                            if (x>0) dat_aus << "x " << stringify_int(x) << "\n";
                            if (y>0) dat_aus << "y " << stringify_int(y) << "\n";
                            dat_aus << "end\n";
                          }
                    }
                    cout << endl;
                  }
              }
            dat_aus.close();
            maps++;
          }
      }

    // write how many maps are created

    cout << maps << " maps succesfully created" << endl;
  }

/* EOF MWC */
