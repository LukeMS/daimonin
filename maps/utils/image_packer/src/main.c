/*
    Daimonin Updater, a service program for the Daimonin MMORPG.


  Copyright (C) 2002-2005 Michael Toennies

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to info@daimonin.net
*/

#include "include/include.h"

#define PACK_FILE "daimonin.0"
#define NEW_FILE "daimonin.p0"

#ifdef WIN32
#define PROCESS_PNGOUT "pngout.exe"
#else
#define PROCESS_PNGOUT "pngout"
#endif


#define MAX_IMAGE_SIZE 20000
#define HUGE_BUF 4096

#define MAX_DIR_PATH 2048 /* maximal full path we support.      */
char  process_path[MAX_DIR_PATH], output[4096],
prg_path[MAX_DIR_PATH], parms[MAX_DIR_PATH];

int main(int argc, char *argv[])
{
    char *string_pos;
    char    filename[400];
    char    buf[HUGE_BUF];
    char    copybuf[MAX_IMAGE_SIZE+1];
    char   *cp;
    FILE   *infile, *outfile, *out2;
    int     num, len, newlen, i;
    int     saved=0, min=99999999, max = 0, diff;

    printf("Daimonin Imageset Packer 0.1\n\n");


    if (strlen(argv[0]) + 256 >MAX_DIR_PATH)
        exit(0);

    strcpy(prg_path, argv[0]);

    string_pos = strrchr( prg_path, '/');
    if ( string_pos < strrchr(prg_path, '\\') )
        string_pos = strrchr( prg_path, '\\');

    if (string_pos)
        *(string_pos+1) = '\0';
    else
        prg_path[0]='\0';

    sprintf(filename, "%s%s",prg_path, PACK_FILE);

    printf("Loading image file %s\n", filename);

    if ((infile = fopen(filename, "rb")) == NULL)
        printf("Unable to open %s for reading\n", filename);


    if ((out2 = fopen(NEW_FILE, "wb")) == NULL)
        printf("Unable to open %s for writing\n", filename);

    while (fgets(buf, HUGE_BUF - 1, infile) != NULL)
    {
        if (strncmp(buf, "IMAGE ", 6) != 0)
            printf("read_daimonin.0:Bad image line - not IMAGE, instead\n%s", buf);
        num = atoi(buf + 6);
        if (num < 0)
            printf("read_daimonin.0: Corrupt Image num %d (%s)\n", num, buf);

        for (cp = buf + 6; *cp != ' '; cp++)
            ;

        len = atoi(cp);
        if (len == 0 || len > MAX_IMAGE_SIZE)
            printf("read_daimonin.0 length not valid: %d > %d \n%s", len, MAX_IMAGE_SIZE, buf);

        for (cp = cp+1; *cp != ' '; cp++)
            ;

//        printf("got: nr: %d, len: %d, name: %s", num, len, cp+1);
        if ((i = fread(copybuf, len, 1, infile)) != 1)
            printf("read_daimonin.0: Did not read desired amount of data, wanted %d, got %d\n%s", len, i,
                buf);

        sprintf(filename, "%stemp.png",prg_path);
        if ((outfile = fopen(filename, "wb")) == NULL )
            printf("could not open temp.png for write\n");

        if ((i = fwrite(copybuf, 1, len, outfile)) != len)
            printf("temp write error, should write %d, did write %d\n",len, i);
        fclose(outfile);


        output[0]='\0';

        sprintf(process_path,"%s%s", prg_path,PROCESS_PNGOUT);
        sprintf(parms,"temp.png");
        i = execute_process(process_path, PROCESS_PNGOUT, parms, output, 0);

        /* we need this hack, execute process comes back, but the file is not enterily written... */
        while ((outfile = fopen("temp.png","w")) == NULL) {}
        fclose(outfile);


        outfile = fopen("temp.png","rb");
        fseek(outfile, 0L, SEEK_END);
        newlen = ftell(outfile);
        fseek(outfile, 0L, SEEK_SET);
        fread(copybuf, 1, MAX_IMAGE_SIZE-1, outfile);
        fclose(outfile);

        fprintf(out2, "IMAGE %05d %d %s",num, newlen, cp+1);
        fwrite(copybuf, newlen, 1, out2);

        diff = len - newlen;

        saved += diff;
        if (diff<min)
            min = diff;

        if ((diff)>max)
            max = diff;

        printf("\r");

        if (i)
            printf("pngout returncode: %d\n\n",i);

//        printf("num %d, old: %d, new: %d, diff: %d, name: %s",num, len, newlen, diff, cp+1);

        if ((diff)<0)
        {
            printf("image was getting bigger: old: %d, new: %d, num: %d, name: %s",len, newlen, num, cp+1);
            printf("%s\n\n",output);
        }
        printf("Images: % 5d / Saved Space: % 7.2f KB (Min: % 3.2f, Max: % 3.2f Avg: % 3.2f)",num, (saved / 1024.0f), (min/1024.0f), (max/1024.0f), ((saved/(num+0.0f))/1024.0f));
    }
    fclose(infile);
    fclose(out2);

    return(0);
}
