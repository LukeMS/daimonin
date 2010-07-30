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

    The author can be reached via e-mail to info@daimonin.org
*/

#include <string.h>

#ifdef __WIN_32
#include <direct.h>
#include <stdlib.h>

#ifndef F_OK
#define F_OK	(0)
#endif

#ifndef R_OK
#define R_OK	(4)
#endif

#ifndef W_OK
#define W_OK	(2)
#endif

#define CSLASH  '\\'

/* use the following pragma to disable warnings C4996 - this pragma is used by Visual Studio */
#ifndef MINGW
#pragma warning(disable : 4996)
#endif

#else
#define CSLASH  '/'

#endif

void char_to_anotherchar(char *s, char c1, char c2){
int i, l;
	l = strlen(s);

	for(i=0;i < l;i++){
		if(s[i] == c1){
			s[i] = c2;
		}
	}
}

#define slash_to_backslash(s) char_to_anotherchar((s), '/', '\\')
#define backslash_to_slash(s) char_to_anotherchar((s), '\\', '/')

#include <include.h>


/* concat two paths adding the necessary slash.
(spath must be large enough to contain the result)
*/
void append_to_path(char *spath, const char *s){
int l = strlen(spath);

    if(l){
        if(spath[l-1] != CSLASH){
            spath[l] = CSLASH;
            l++;
        }
    }
    strcpy(&spath[l], s);

#ifdef __WIN_32
    slash_to_backslash(&spath[l]);
#endif
}

/**
* Create the directory @p path.  If it already exists as a directory
* we succeed.
* Recursive mkdir is from the Gal project's e_mkdir_hier() function.
**/

static int mkdir_recurse(const char *path)
{
    char *copy, *p;

    p = copy = strdup(path);

    while(p){
        p = strchr (p + 1, CSLASH);
        if (p)
            *p = '\0';
        if(access(copy, F_OK) == -1){
            if (mkdir (copy, 0755) == -1){
				FREE(copy);
                return -1;
            }
        }
        if(p)
            *p = CSLASH;
    }
	if(copy){
		FREE(copy);
	}
    return 0;
}

#define FILECOPY_BUFSIZE    (32 * 1024)

int filecopy(char *sfilei, char *sfileo){
FILE *pfi, *pfo;
char s[FILECOPY_BUFSIZE];
size_t nRead;
int nRetVal = 0;

    if((pfi = fopen(sfilei, "rb"))){
        if((pfo = fopen(sfileo, "wb"))){
            while(!nRetVal){
                if((nRead = fread(s, 1, FILECOPY_BUFSIZE, pfi)) < FILECOPY_BUFSIZE){
                    if(ferror(pfi)){
                        fprintf(stderr, "Failed to read input file %s (%s).\n", sfilei, strerror(errno));
                        break;
                    }
                    nRetVal = 1;
                    if(nRead <= 0){
                        break;
                    }
                }
                if(fwrite(s, 1, nRead, pfo) != nRead){
                    fprintf(stderr, "Failed to write output file %s (%s).\n", sfileo, strerror(errno));
                    nRetVal = 0;
                    break;
                }
            }
            fclose(pfo);
        } else {
            fprintf(stderr, "Failed to open output file %s (%s).\n", sfileo, strerror(errno));
        }
        fclose(pfi);
    } else {
        fprintf(stderr, "Failed to open input file %s (%s).\n", sfilei, strerror(errno));
    }
    return nRetVal;
}

#define VERSION_MAXSIZE (32)

const char *getversion(void){
FILE *pfi;
static char sVersion[VERSION_MAXSIZE];
char *sRet = NULL;
int nRead, i;
char sfile[256] = "";

    append_to_path(sfile, SYSPATH);
    append_to_path(sfile, "update/version");

    if((pfi = fopen(sfile, "r"))){
        if(((nRead = fread(sVersion, 1, VERSION_MAXSIZE - 1, pfi)) > 0)){
            sVersion[nRead] = '\0';
            for(i=0;i < nRead;i++){ /* only keep the major version information */
                if(sVersion[i] == ' '){
                    sVersion[i] = '\0';
                    break;
                }
            }
            sRet = sVersion;
        }
        fclose(pfi);
    }
    return sRet;
}

/*
returns 0 if we failed to use a user dir location - in this case we use the SYSPATH location
returns 1 if we maange to use a user dir location
*/
int determine_best_location(char *tmp, const char *fname){
#ifdef __WIN_32 
char *stmp;
#endif
const char *sVer;

*tmp = '\0';

#ifdef __WIN_32
	if((stmp = getenv("APPDATA")) && *stmp){ /* APPDATA defined */
        append_to_path(tmp, stmp);
		append_to_path(tmp, "Daimonin");
	} else { /* Windows 98 ??? */
		if((stmp = getenv("WINDIR")) && *stmp){ /* WINDIR defined ? */
			append_to_path(tmp, stmp);
			append_to_path(tmp, "Application Data\\Daimonin");
		} else { /* WINDIR not defined, let's use the the daimonin path */
			append_to_path(tmp, SYSPATH);
			append_to_path(tmp, fname);
			return 0;
		}
	}
#else
	append_to_path(tmp, getenv("HOME"));
	append_to_path(tmp, ".daimonin");
#endif

	if((sVer = getversion())){ /* Append the short version */
	    append_to_path(tmp, sVer);
	}
	append_to_path(tmp, fname);
	return 1;
}


char *file_path(const char *fname, const char *mode)
{
    static char tmp[256];
    char *stmp;
    char ctmp;

    if(!determine_best_location(tmp, fname)){ /* let's use the file in the daimonin dir */
        return tmp;
    }

    if(strchr(mode, 'w')){ /* overwrite (always use file in home dir) */
        if((stmp=strrchr(tmp, CSLASH))){
            ctmp = stmp[0];
            stmp[0] = 0;
            mkdir_recurse(tmp);
            stmp[0] = ctmp;
        }
    }
    else if (strchr(mode, '+') || strchr(mode, 'a')){ /* modify (copy base file to home dir if not exists) */
        if(access(tmp, W_OK)){
            char otmp[256] = "";

            if ((stmp=strrchr(tmp, CSLASH))){
                ctmp = stmp[0];
                stmp[0] = 0;
                mkdir_recurse(tmp);
                stmp[0] = ctmp;
            }

            append_to_path(otmp, SYSPATH);
            append_to_path(otmp, fname);

            /* Copy base file to home directory */
            if(!access(otmp, R_OK)){ /* if source file exists */
                if(!filecopy(otmp,tmp)){ /* if we failed to copy file */
                    *tmp = '\0';
                    append_to_path(tmp, SYSPATH);
                    append_to_path(tmp, fname);
                    return tmp;
                }
            }
        }
    }
    else
    { /* just read (check home dir first, then system dir) */
        if (access(tmp, R_OK)){
            *tmp = '\0';
            append_to_path(tmp, SYSPATH);
            append_to_path(tmp, fname);
        }
    }

    /*    printf("file_path: %s (%s) => %s\n", fname, mode, tmp); */
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
