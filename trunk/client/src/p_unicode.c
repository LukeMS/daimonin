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


    p_unicode.c
    Copyright (C) 2013 DarK AvengeR

*/

#include <include.h>


/*	Stores on dst an UTF8 byte array containing the current
	executable full path. Remember that an Unicode byte
	array could be double as the size of an ASCII one
	due to being mostly using 2 bytes per char.*/
void UTF_CopyExecutablePath (char * dst, size_t maxlen) {

	/*	This will store our path in wide char */
	wchar_t	  path[UNICODE_MAX_PATH_BUFFER];

	/*	Get the executable name, wide char aware */
	GetModuleFileNameW(0, (LPWSTR)&path, UNICODE_MAX_PATH_BUFFER);

	/*	Convert a Windows wide char array into a plain byte array,
		given its max storage size */
	PHYSFS_utf8FromUcs2((Uint16 *)path, dst, maxlen);

}

/*	Returns an UT8 byte array containing the current
	executable full path. Remember: it has to be freed! */
char * UTF_GetExecutablePath () {

	/*	This will store our path in UTF8 format */
	char	*	retpath;

	/*	Allocate memory to store the UTF converted path */
	MALLOC(retpath, PATH_BACKBUFFER);

	/*	Execute the function UTF_CopyExecutablePath to avoid
		redundant code */
	UTF_CopyExecutablePath (retpath, PATH_BACKBUFFER);

	return retpath;
}


/*	Get the Daimonin Version. */
void GetDaimoninVersion(char * dst, size_t maxlen) {

		snprintf(dst, maxlen, "%d.%d", DAI_VERSION_RELEASE, DAI_VERSION_MAJOR);

}

/*	Get the Daimonin Base Folder, for PhysFS use. */
void GetDaimoninBase(char * dst, const char * separator, size_t maxlen) {

		snprintf(dst, maxlen, "Daimonin%s%d.%d", separator, DAI_VERSION_RELEASE, DAI_VERSION_MAJOR);

}

/*	Searches for Home directory and stores on dst an UTF8 byte array
	containing the Home directory if found or just the base if not.
	Remember that an Unicode byte array could be double as
	the size of an ASCII one due to being mostly using 2bytes per char.*/
void UTF_FindUserDir(char * dst, size_t maxlen) {

    wchar_t	*		env;                            /*  Temporary storage for _wgetenv */
    wchar_t         home[UNICODE_MAX_PATH_BUFFER];  /*  and temporary storage for home path.
                                                        This is needed because we need to convert
                                                        wide chars into utf8 using proper PHYFS
                                                        functions. (PHYSFS_utf8FromUcs2, as of
                                                                    PHYFS docs.) */
	/*  env type. I did not like to use two different
        swprintf just to accomplish the same task */
	uint8			type=0;

    /*  If APPDATA is not in the ENV, get the WINDIR
        else set the type to 1 */
	if (!(env = _wgetenv(L"APPDATA")))
		env = _wgetenv(L"WINDIR");
	else
		type=1;

    /* If we have a valid ENV */
	if (env)
    {
        /*  Depending on the type, home is either set to env or env
            plus \\Application Data (thats for the WINDIR case) */
        swprintf(home, UNICODE_MAX_PATH_BUFFER, L"%ls%ls", env, (type == 1 ? L"" : APPDATA));

        /* Convert the wide char array into UTF8 */
		PHYSFS_utf8FromUcs2((Uint16 *)home, dst, maxlen);

	} else /* We dont have any valid ENV, so we use the base directory */
		snprintf(dst, maxlen, "%s", PHYSFS_getBaseDir());

}


/*	Searches for a suitable output directory and stores on dst an UTF8 byte array
	containing the User Dir if found or just the base if not.
	Remember that an Unicode byte array could be double as
	the size of an ASCII one due to being mostly using 2bytes per char.*/
void UTF_FindOutputPath(char * dst, size_t maxlen) {

    char			home[PATH_BACKBUFFER], /* Holds the home dir, already converted to UTF8 */
					DaimoninHome[LARGE_BUF]; /* Holds the DAIMONIN home */

    /*  We get the platform separator */
	const char	*	Separator = PHYSFS_getDirSeparator();

    /*  We need to know what is the home directory so
        a call to this function will hopefully find a
        suitable path for our Daimonin Home Directory */
	UTF_FindUserDir(home, PATH_BACKBUFFER);

    /*  If our home is not the Base Dir, we need to
        refer to the usual Daimonin/RELEASE.MAJOR
        path, else use the base dir as storage folder.
        Remember: if its not writable, it will crash
        the client. A check could be done and
        warn the player.*/
	if (strcmp(PHYSFS_getBaseDir(), home))
		GetDaimoninBase(DaimoninHome, Separator, LARGE_BUF);
	else
		DaimoninHome[0]='\0';

    /* Build the final destination on dst */
	snprintf (dst, maxlen, "%s%s%s", home, Separator, DaimoninHome);

}

/* Redirects STDOUT and STDERR to a writable directory */
void UTF_Redirect_std() {

    /*  Wide char array for full destinations path. We
        cannot use PHYSFS mount points directly */
	wchar_t d_stderr[UNICODE_MAX_PATH_BUFFER];
	wchar_t d_stdout[UNICODE_MAX_PATH_BUFFER];

    /*  Temporary wide char array for writedir path. We
        cannot use PHYSFS_getWrideDir() directly */
	wchar_t tmp[UNICODE_MAX_PATH_BUFFER];

    /*  Convert from the UTF8 WriteDir path to a wide char
        and store the result into tmp */
	PHYSFS_utf8ToUcs2(PHYSFS_getWriteDir(), (uint16 *)tmp, UNICODE_MAX_PATH_BUFFER);

    /*  Build the final destination path. This assumes that DIR_LOGS
        has already been created, and perhaps it should be at this point */
     swprintf(d_stderr, UNICODE_MAX_PATH_BUFFER, L"%ls\\%S\\stderr.log", tmp, DIR_LOGS);
     swprintf(d_stdout, UNICODE_MAX_PATH_BUFFER, L"%ls\\%S\\stdout.log", tmp, DIR_LOGS);

    /*  Do the actual redirection */
	_wfreopen(d_stderr, L"w", stderr);
	_wfreopen(d_stdout, L"w", stdout);

}

