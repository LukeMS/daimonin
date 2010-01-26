/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

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

#ifdef __WIN_32
#include <direct.h>
#include <stdlib.h>
#endif

#include <include.h>

/**
* Create the directory @p path.  If it already exists as a directory
* we succeed.
* Recursive mkdir is from the Gal project's e_mkdir_hier() function.
**/

static int mkdir_recurse(const char *path)
{
    char *copy, *p;
	char cSlash;

#ifdef __WIN_32
	cSlash = '\\';
#define F_OK	(0)
#define R_OK	(4)
#define W_OK	(2)
#else
	cSlash = '/';
#endif

    p = copy = strdup(path);

    while(p){
        p = strchr (p + 1, cSlash);
        if (p)
            *p = '\0';
        if(access (copy, F_OK) == -1)
        {
            if (mkdir (copy, 0755) == -1)
            {
				free(copy);
                return -1;
            }
        }
        if (p)
            *p = cSlash;
    }
	if(copy){
		free(copy);
	}
    return 0;
}

#ifdef __WIN_32
void slash_to_backslash(char *s){
int i, l;
	l = strlen(s);

	for(i=0;i < l;i++){
		if(s[i] == '/'){
			s[i] = '\\';
		}
	}
}

char *mygetenv(const char *sVarName){
size_t nSize;
char *sRetVal;

	getenv_s(&nSize, NULL, 0, sVarName); /* retrieve size of var value */

	if(!nSize){
		return NULL;
	}

	if(!(sRetVal = (char *) malloc(nSize * sizeof(char)))){ /* out of memory problem */
		return NULL;
	}

	// Get the value of the environment variable.
	if(!getenv_s(&nSize, sRetVal, nSize, sVarName)){
		return sRetVal;
	}

	free(sRetVal);
	return NULL;
}

/* use the following prgma to disable warnings C4996 */
#pragma warning(disable : 4996)


int determine_best_dir(char *tmp, const char *fname){
char *stmp;

	stmp = mygetenv("APPDATA");
	if(!stmp){ /* APPDATA not defined - win98 ??? */
		if(stmp = mygetenv("WINDIR")){ /* WINDIR defined ? */
			strcpy(tmp, stmp);
			free(stmp);
			if((tmp[strlen(tmp)-1] != '/') && (tmp[strlen(tmp)-1] != '\\')){
				strcat(tmp, "\\");
			}
			strcat(tmp, "Application Data\\Daimonin\\");
		} else { /* WINDIR not defined, let's leave */
			return 0;
		}
		if(access(tmp, W_OK) == -1){
			return 0;
		}
	} else { /* APPDATA defined */
		strcpy(tmp, stmp);
		free(stmp);
		if((tmp[strlen(tmp)-1] != '/') && (tmp[strlen(tmp)-1] != '\\')){
			strcat(tmp, "\\");
		}
		strcat(tmp, "Daimonin\\");
	}
	strcat(tmp, fname);
	
	slash_to_backslash(tmp);
	return 1;
}


#endif

char *file_path(const char *fname, const char *mode)
{
    static char tmp[256];
    char *stmp;
    char ctmp;

	char cslash = '/';

#ifdef __WIN_32
	if(!determine_best_dir(tmp, fname)){ /* no best dir, let's use the less worst dir */
		sprintf(tmp, "%s%s", SYSPATH, fname);
		return tmp;
	}
	cslash = '\\';
#else
	sprintf(tmp, "%s/.daimonin/%s", getenv("HOME"), fname);
#endif

    if (strchr(mode, 'w'))
    { // overwrite (always use file in home dir)
        if ((stmp=strrchr(tmp, cslash)))
        {
            ctmp = stmp[0];
            stmp[0] = 0;
            mkdir_recurse(tmp);
            stmp[0] = ctmp;
        }
    }
    else if (strchr(mode, '+') || strchr(mode, 'a'))
    { // modify (copy base file to home dir if not exists)
        if (access(tmp, W_OK))
        {
            char otmp[256];
            char shtmp[517];
            int  dummy; // purely to suppress GCC's warn_unused_result warning

            sprintf(otmp, "%s%s", SYSPATH, fname);
            if ((stmp=strrchr(tmp, cslash)))
            {
                ctmp = stmp[0];
                stmp[0] = 0;
                mkdir_recurse(tmp);
                stmp[0] = ctmp;
            }

            /* Copy base file to home directory */
#ifdef __WIN_32
            sprintf(shtmp, "copy %s %s", otmp, tmp);
#else
            sprintf(shtmp, "cp %s %s", otmp, tmp);
#endif
            dummy = system(shtmp);
        }
    }
    else
    { // just read (check home dir first, then system dir)
        if (access(tmp, R_OK))
            sprintf(tmp, "%s%s", SYSPATH, fname);
    }

    //    printf("file_path: %s (%s) => %s\n", fname, mode, tmp);

    return tmp;
}

FILE *fopen_wrapper(const char *fname, const char *mode)
{
    return fopen(file_path(fname, mode), mode);
}

SDL_Surface *IMG_Load_wrapper (const char *file)
{
    return IMG_Load(file_path(file, "r"));
}

#ifdef INSTALL_SOUND
Mix_Chunk *Mix_LoadWAV_wrapper(const char *fname)
{
    return Mix_LoadWAV(file_path(fname, "r"));
}

Mix_Music *Mix_LoadMUS_wrapper(const char *file)
{
    return Mix_LoadMUS(file_path(file, "r"));
}
#endif
