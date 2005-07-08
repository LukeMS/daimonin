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

#include <include.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/locking.h>
#include <share.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define UPDATE_URL "http://daimonin.sourceforge.net/patch/"

#define UPDATE_PATCH_FILE "patch.file"
#define UPDATE_FILE "patch.master"
#define UPDATE_VERSION "version"
#define FOLDER_UPDATE "update"
#define FOLDER_PATCH "update/patch/"
#ifdef WIN32
#define ACCESS_OK 6
#define PROCESS_UPDATER "daimonin.exe"
#define SYSTEM_OS_TAG 'w'
#define FOLDER_TOOLS "tools/"
#else
#define ACCESS_OK 6
#define PROCESS_UPDATER "daimonin"
#define SYSTEM_OS_TAG 'l'
#endif
#define PROCESS_WGET "wget.exe"
#define PROCESS_MD5 "md5sum.exe"
#define PROCESS_BZ2 "bunzip2.exe"
#define PROCESS_TAR "tar.exe"
#define PROCESS_XDELTA "xdelta.exe"
#define PROCESS_CLIENT "client.exe"

#define MAX_DIR_PATH 2048	/* maximal full path we support.      */
char version_path[MAX_DIR_PATH], process_path[MAX_DIR_PATH], output[4096],
     prg_path[MAX_DIR_PATH], file_path[MAX_DIR_PATH], parms[MAX_DIR_PATH];

#define COPY_BUFFER_SIZE 1024*512
char *copy_buffer = NULL, *argv0;

int update_flag = FALSE;

FILE *version_handle=NULL;

extern void clear_directory(char* start_dir);
extern void copy_patch_files(char* start_dir);
extern int process_patch_file(char *patch_file, int mode);
extern void copy_patch(char *src, char *dest);

static free_resources(void)
{
    if(copy_buffer)
        free(copy_buffer);
}

static int execute_process(char *p_path, char *exe_name, char *parms, char *output, int seconds_to_wait) 
{
    STARTUPINFO siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;
    HANDLE hChildStdoutRd, hChildStdoutWr; 
    SECURITY_ATTRIBUTES saAttr; 

    DWORD dwExitCode;
    int ret = 0;
    char cmd[4096];

    /* CreateProcess API initialization */
    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));
    siStartupInfo.cb = sizeof(siStartupInfo);

    if(output)/* create a pipe and redirect stdout to output */ 
    {
        *output='\0';
        // Set the bInheritHandle flag so pipe handles are inherited. 
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
        saAttr.bInheritHandle =TRUE; 
        saAttr.lpSecurityDescriptor = NULL; 

        if (! CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
        {
            output = NULL;
            printf("Stdout pipe creation failed\n"); 
        }

        if(output)
        {
            siStartupInfo.hStdError = GetStdHandle(STD_OUTPUT_HANDLE);
            //siStartupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            siStartupInfo.hStdOutput = hChildStdoutWr;
            siStartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            siStartupInfo.dwFlags |= STARTF_USESTDHANDLES;
        }
    }

    sprintf(cmd,"%s %s", exe_name, parms);
    /*printf("CMD.... %s (%s)\n", cmd, prg_path);*/

    /* Execute */
    if (CreateProcess(p_path, cmd, 0, 0, TRUE,
        CREATE_DEFAULT_ERROR_MODE, 0, 0, &siStartupInfo,
        &piProcessInfo) != FALSE)
    {
        if(!seconds_to_wait)
        {
            CloseHandle(piProcessInfo.hProcess);
            CloseHandle(piProcessInfo.hThread);
            if(output)
            {
                DWORD dwRead; 

                CloseHandle(hChildStdoutWr);
                ReadFile(hChildStdoutRd, output, 4000, &dwRead, NULL);
                output[dwRead]='\0';
                CloseHandle(hChildStdoutRd);
            }
            return 0;
        }

        GetExitCodeProcess(piProcessInfo.hProcess, &dwExitCode);
        while (dwExitCode == STILL_ACTIVE && seconds_to_wait != 0)
        {
            GetExitCodeProcess(piProcessInfo.hProcess, &dwExitCode);
            Sleep(50);
        }

        /*printf("RET: %d\n", dwExitCode);*/
        ret = dwExitCode;
    }

    CloseHandle(piProcessInfo.hProcess);
    CloseHandle(piProcessInfo.hThread);

    if(output)
    {
        DWORD dwRead; 

        CloseHandle(hChildStdoutWr);
        ReadFile(hChildStdoutRd, output, 4000, &dwRead, NULL);
        output[dwRead]='\0';
        CloseHandle(hChildStdoutRd);
    }
    return ret;
}

static void updater_error(const char *msg)
{
    printf("\nUpdater Error!!\n");
    printf(msg);
    printf("\nPRESS RETURN");
    getchar();
    free_resources();
    fclose(version_handle);
    exit(-1);
}

static void start_client_and_close(char *p_path)
{
    printf("Starting client....\n");
    strcpy(process_path, p_path);
    strcat(process_path, PROCESS_CLIENT); /* '/' will work in windows too */
    execute_process(process_path, PROCESS_CLIENT, "", NULL, 0);
    free_resources();
    fclose(version_handle);
    exit(0);
}

/* after we applied successful a patch, we
 * adjusting the version marker file, so we
 * can track the applied patches.
 */
static void write_version_file(char *version, int version_nr)
{
    char buf[256];

    sprintf(buf,"%s %d", version, version_nr);
    fseek(version_handle, 0L, SEEK_SET);
    if(fputs(buf,version_handle)<0)
    {
        perror("fputs():");
        printf("\nCan't write version info.\nForcing file check.\nPRESS RETURN\n");
        printf("This version has no file check\n");
        getchar();
        start_client_and_close(prg_path);
    }
}

/* removes whitespace from right side */
static char *adjust_string(char *buf)
{
    int i, len = strlen(buf);

    for (i = len - 1; i >= 0; i--)
    {
        if (!isspace(buf[i]))
            return buf;

        buf[i] = 0;
    }
    return buf;
}

int main(int argc, char *argv[])
{
    FILE   *stream;
    int version_nr, version_def_nr, patched=FALSE;
    char version[256], buf[256], *string_pos;
    char file_name[256], md5[64]; 
#ifndef WIN32
    struct flock fl = { F_RDLCK, SEEK_SET, 0,       0,     0 };
#endif

    printf("Daimonin AutoUpdater 1.0\n\n");

    argv0 = argv[0];
    /* prepare pathes */
    if(strlen(argv[0]) + 256 >MAX_DIR_PATH)
        exit(0);

	strcpy(prg_path, argv[0]);

	string_pos = strrchr( prg_path, '/');
	if(	string_pos < strrchr(prg_path, '\\') )
		string_pos = strrchr( prg_path, '\\');

    if(string_pos)
        *(string_pos+1) = '\0';

    /* check for install or update keywords */
    if(argc>1)
    {
        if(!strcmp("install", argv[1]))
        {
            if(argc>2)
            {
                FILE *tmp=NULL;

                /* copy us over older installer */
                /* access has problems under windows in this special case - this works */
                while(!(tmp = fopen(argv[2], "w"))){}

                fclose(tmp);
                copy_patch(argv[0], argv[2]);
                sprintf(parms,"update %s", argv[0]);
                execute_process(argv[2], argv[2], parms, NULL, 0);

                printf("updated... restarting.\n");
                free_resources();
                exit(0);

            }
        }
        else if(!strcmp("update", argv[1]))
        {
            FILE *tmp=NULL;
            if(argc>2)
            {
                update_flag = TRUE;
                while(!(tmp = fopen(argv[2], "w"))){}
                fclose(tmp);
                unlink(argv[2]);
            }
        }
    }

    /* we use the version file as lock/unlock to avoid different instances of the updater running at once.
    * We don't use the guid or something because we the self instance start when the updater
    * updates itself.
    */
    if(update_flag==FALSE)
        printf("Loading version info.... ");
    sprintf(version_path,"%s%s/%s", prg_path, FOLDER_UPDATE, UPDATE_VERSION);
    if ((version_handle = fopen(version_path, "r+t")) == NULL)
    {
        /* check for access/locked */
        updater_error("\nCan't find version info.\nRun file check.\nPRESS RETURN\n");
    }
#ifdef WIN32
    if( _locking(_fileno(version_handle), _LK_NBLCK, 1L ) == -1 )
    {
        fclose(version_handle);
        exit(-2);
    }
#else
    //_fcntl(fd, F_SETLKW, &fl);  /* set the lock, waiting if necessary */
#endif

    fgets(version, 128 - 1, version_handle);
    /* we don't close the version file here because we need to hold the lock of it */

    adjust_string(version);
    string_pos = strrchr(version, ' ');
    if(!string_pos)
        updater_error("\nError in version file.\nRun file check.\nPRESS RETURN\n");
    *string_pos = '\0';
    version_def_nr = atoi(string_pos+1);

    /* we try to load the version info first.
     * If we don't have a valid version info, we can't
     * do a packer based patch. We have force file to file check.
     * This will also restore a valid version info for the next time.
     */
    if(update_flag==FALSE)
    {
        /* clearing up is always a good idea to start - also ensure its there */
        mkdir(FOLDER_PATCH, 0777);
        clear_directory(FOLDER_PATCH);    
    }

    if(update_flag==FALSE)
    {
        printf("version %s\n", version);
        printf("Get update info....\n");

        /* prepare wget for general update info and retrieve the updating base info */
        sprintf(process_path,"%s%s%s", prg_path, FOLDER_TOOLS, PROCESS_WGET);
        sprintf(parms,"-N -P%s %s%s", FOLDER_UPDATE, UPDATE_URL, UPDATE_FILE);
        if( execute_process(process_path, PROCESS_WGET, parms, NULL, 100) != 0)
        {
            /* failed to get a update... lets start the client anyway and try to connect to a server */
            printf("Update failed!\nTry starting client without update.\nPRESS RETURN\n");
            getchar();
            start_client_and_close(prg_path);
        }

        /* check we have the update or we must get it */
        printf("Check update info....\n");
    }
    /* open the patch info file and process the single entries */
    sprintf(file_path,"%s%s/%s", prg_path, FOLDER_UPDATE, UPDATE_FILE);
    if ((stream = fopen(file_path, "rt")) == NULL)
    {
        printf("Can't find update file!!\nUpdate failed!\nTry starting client without update.\nPRESS RETURN\n");
        getchar();
        start_client_and_close(prg_path);
    }

    /* check every entry */
    while (fgets(buf, 256 - 1, stream) != NULL)
    {
        adjust_string(buf);
     
        sscanf(buf, "%s %s %d %s", file_name, md5, &version_nr, version); 

        if(version_nr <= version_def_nr) /* ignore same or lesser versions! */
            continue;

        /* we have something new. patch it */
        /* if we have the update flag set, we assume this is our old patch file */
        if(update_flag==FALSE)
        {
            sprintf(process_path,"%s%s%s", prg_path, FOLDER_TOOLS, PROCESS_WGET);
            sprintf(parms,"-N -P%s %s%s", FOLDER_UPDATE, UPDATE_URL, file_name);
            if( execute_process(process_path, PROCESS_WGET, parms, NULL, 100) != 0)
            {
                /* failed to get a update... lets start the client anyway and try to connect to a server */
                fclose(stream);
                printf("Update failed!\nTry starting client without update.\nPRESS RETURN\n");
                getchar();
                start_client_and_close(prg_path);
            }

            printf("Applying patch %s (%d).... \n", version, version_nr);

            sprintf(process_path,"%s%s%s", prg_path, FOLDER_TOOLS, PROCESS_MD5);
            sprintf(parms,"-b %s/%s", FOLDER_UPDATE,file_name);
            execute_process(process_path, PROCESS_MD5, parms, output, 1);

            /* check MD5 of current download/patch file */
            string_pos = strchr(output, ' ');
            if(string_pos)
                *string_pos = '\0';
            if(strcmp(output, md5) )
                updater_error("MD5 check - FAILED.\nBad File - Restart Updater.\n");
            printf("MD5 check - ok.\n");

            /* unpack pach file */
            sprintf(process_path,"%s%s%s", prg_path, FOLDER_TOOLS, PROCESS_BZ2);
            sprintf(parms,"-k %s/%s", FOLDER_UPDATE,file_name);
            execute_process(process_path, PROCESS_BZ2, parms, NULL, 1);
            sprintf(process_path,"%s%s%s", prg_path, FOLDER_TOOLS, PROCESS_TAR);
            sprintf(buf,"%s/%s", FOLDER_UPDATE,file_name);
            string_pos = strrchr(buf, '.'); /* kill the .bz2 */
            if(string_pos)
                *string_pos = '\0';
            sprintf(parms,"-xvf %s %s", buf, FOLDER_UPDATE);
            execute_process(process_path, PROCESS_TAR, parms, NULL, 1);
            unlink(buf); /* delete the tar file - we still have the bz2 */
            /* now process the patch files */
            printf("prepare patch...\n");
        }
        process_patch_file(UPDATE_PATCH_FILE, FALSE);
        process_patch_file(UPDATE_PATCH_FILE, TRUE);
        printf("copy files... ");
        copy_patch_files(FOLDER_PATCH);    
        clear_directory(FOLDER_PATCH);    

        /* now make it "offical" */
        version_def_nr = version_nr;
        patched = TRUE;
        write_version_file(version, version_nr);
        printf("patch applied.\n");


    }
    fclose(stream);

    if(patched)
        printf("Patched to version %s (%d)\n", version, version_nr);
    else
        printf("No new patches found...\n");


    //printf("Starting client...\n");

    printf("Starting client...\n(debug mode: press RETURN to confirm)\n");
    getchar();

    start_client_and_close(prg_path);

    /* thats only for security here */
    fclose(version_handle);
    free_resources();
    return(0);
}


/*
* recusively traverse the given directory and clear it
* (delete all files and subdirectories of the given directory
* but not the directory itself)
*/
void clear_directory(char* start_dir)
{
    DIR* dir;			        /* pointer to the scanned directory. */
    struct dirent* entry;	    /* pointer to one directory entry.   */
    char cwd[MAX_DIR_PATH+1];	/* current working directory.        */
    struct stat dir_stat;       /* used by stat().                   */

    /* first, save path of current working directory */
    if (!getcwd(cwd, MAX_DIR_PATH+1)) {
        perror("getcwd:");
        return;
    }

    /* open the directory for reading */
    if(start_dir)
    {
        dir = opendir(start_dir);
        chdir(start_dir);
    }
    else
        dir = opendir(".");

    if (!dir) {
        fprintf(stderr, "Cannot read directory '%s': ", cwd);
        perror("");
        return;
    }

    /* scan the directory, traversing each sub-directory, and */
    /* matching the pattern for each file name.               */
    while ((entry = readdir(dir))) 
    {
        /* check if the given entry is a directory. */
        /* skip the "." and ".." entries, to avoid loops. */
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        if (stat(entry->d_name, &dir_stat) == -1) 
        {
            perror("stat:");
            continue;
        }

        /* is this a directory? */
        if (S_ISDIR(dir_stat.st_mode))
        {
            /* Change into the new directory */
            if (chdir(entry->d_name) == -1) 
            {
                fprintf(stderr, "Cannot chdir into '%s': ", entry->d_name);
                perror("");
                continue;
            }
            /* check this directory */
            clear_directory(NULL);

            /* finally, restore the original working directory. */
            if (chdir("..") == -1) 
            {
                fprintf(stderr, "Cannot chdir back to '%s': ", cwd);
                perror("");
                fclose(version_handle);
                exit(1);
            }

            /* remove the (cleared) directory */
            if(rmdir(entry->d_name)!=0)
            {
                fprintf(stderr, "Cannot delete directory '%s': ", entry->d_name);
                perror("");
            }
        }
        else
            unlink(entry->d_name); /* remove a file */
    }

    closedir(dir);

    if(start_dir) /* clean restore */
        chdir(cwd);
}


/* copy a (patched) file over the older original */
void copy_patch(char *src, char *dest)
{
    FILE *src_file, *dest_file;
    int num_read, num_write;

    if(!copy_buffer)
        copy_buffer = malloc(COPY_BUFFER_SIZE);

    unlink(dest);
    if ((dest_file = fopen(dest, "wb")) == NULL)
    {
        fprintf(stderr, "Cannot open file for write :: '%s'\n", dest);
        perror("");
        updater_error("");
    }

    if ((src_file = fopen(src, "rb")) == NULL)
    {
        fprintf(stderr, "Cannot open file for read :: '%s'\n", src);
        perror("");
        fclose(dest_file);
        updater_error("");
    }

    for(;;)
    {
        if(!(num_read = fread(copy_buffer, sizeof( char) ,COPY_BUFFER_SIZE, src_file)))
        {
            /* no error handling so far */
            break;
        }
        if(!(num_write = fwrite(copy_buffer, sizeof( char) ,num_read, dest_file)))
        {
            /* no error handling so far */
            break;
        }
    }

    fclose(dest_file);
    fclose(src_file);

}


/*
* recusively traverse the given directory,
* and copy all files 1:1 to the client root folder
* ( = apply the patch physically)
*/
void copy_patch_files(char* start_dir)
{
    static int patch_dir_len;
    static char base[MAX_DIR_PATH+1];	/* current working directory.        */
    DIR* dir;			        /* pointer to the scanned directory. */
    struct dirent* entry;	    /* pointer to one directory entry.   */
    char cwd[MAX_DIR_PATH+1];	/* current working directory.        */
    struct stat dir_stat;       /* used by stat().                   */
    char buf[MAX_DIR_PATH+1];	/* current working directory.        */

    /* open the directory for reading */
    if(start_dir)
    {
        /* first, save path of current working directory */
        if (!getcwd(base, MAX_DIR_PATH+1)) {
            perror("getcwd:");
            return;
        }

        dir = opendir(start_dir);
        chdir(start_dir);
    }
    else
        dir = opendir(".");

    if (!dir) {
        fprintf(stderr, "\nCannot read directory '%s': ", cwd);
        perror("");
        return;
    }

    /* first, save path of current working directory */
    if (!getcwd(cwd, MAX_DIR_PATH+1)) {
        perror("getcwd:");
        return;
    }

    if(start_dir)
        patch_dir_len = strlen(cwd);


    /* scan the directory, traversing each sub-directory, and */
    /* matching the pattern for each file name.               */
    while ((entry = readdir(dir))) 
    {
        /* check if the given entry is a directory. */
        /* skip the "." and ".." entries, to avoid loops. */
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        if (stat(entry->d_name, &dir_stat) == -1) 
        {
            perror("stat:");
            continue;
        }

        /* is this a directory? */
        if (S_ISDIR(dir_stat.st_mode))
        {
            /* Change into the new directory */
            if (chdir(entry->d_name) == -1) 
            {
                fprintf(stderr, "\nCannot chdir into '%s': ", entry->d_name);
                perror("");
                continue;
            }
            /* check this directory */
            copy_patch_files(NULL);

            /* finally, restore the original working directory. */
            if (chdir("..") == -1) 
            {
                fprintf(stderr, "\nCannot chdir back to '%s': ", cwd);
                perror("");
                fclose(version_handle);
                exit(1);
            }
        }
        else
        {
            int l = strlen(entry->d_name);

            sprintf(buf,"%s/%s/%s", base,cwd+patch_dir_len,entry->d_name);
            printf(".");
            copy_patch(entry->d_name, buf);
        }
    }
    closedir(dir);

    if(start_dir) /* clean restore */
    {
        printf(" done.\n");
        chdir(base);
    }
}


/* read in the patch file and execute the given commands.
* mode = FALSE: only process xdelta and local commands.
* mode = TRUE: execute commands like removing folder/files
* inside the base folders.
*/
int process_patch_file(char *patch_file, int mode)
{
    FILE *stream;
    char cmd[32], os_tag[32], src_path[MAX_DIR_PATH], dest_path[MAX_DIR_PATH], target_path[MAX_DIR_PATH];

    sprintf(file_path,"%s/%s", FOLDER_PATCH, patch_file);
    if ((stream = fopen(file_path, "rt")) == NULL)
        updater_error("Can't find patch command file!!\nUpdate failed!\n");

    /* check every entry */
    while (fgets(output, 4096 - 1, stream) != NULL)
    {
        /* remove whitespaces, skip clear lines and comment lines */
        adjust_string(output);
        if(output[0]=='#' || output[0]=='\0')
            continue;

        sscanf(output, "%s %s %s %s %s", cmd, os_tag, src_path, target_path, dest_path); 
        if(mode == FALSE)
        {

            if(!strcmp(cmd, "install"))
            {
                if(update_flag)
                {
                    update_flag = FALSE;
                    continue;
                }
                if(!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    fclose(stream); 
                    /* start the new installer and stop, so it can copy over us */
                    sprintf(process_path,"%s%s%s", prg_path, FOLDER_PATCH, src_path);
                    sprintf(parms,"install %s", argv0);
                    fclose(version_handle); /* important: allow instances */
                    execute_process(process_path, process_path, parms, NULL, 0);
                
                    printf("Patching Updater!\nleaving old instance....\n");
                    free_resources();
                    exit(0);
                }
            }

            if(update_flag == TRUE)
                continue;

            if(!strcmp(cmd, "xdelta"))
            {
                /* allowed on this system/os? */
                if(!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("XDELTA: %s %s %s\n", src_path, target_path, dest_path);*/

                    sprintf(file_path, "%s%s", FOLDER_PATCH, dest_path);
                    sprintf(process_path,"%s%s%s", prg_path, FOLDER_TOOLS, PROCESS_XDELTA);
                    sprintf(parms, "patch %s%s %s %s", FOLDER_PATCH,src_path, target_path, file_path );
                    if( execute_process(process_path, PROCESS_XDELTA, parms, NULL, 100) != 0)
                    {
                        /* be sure we don't left bogus files */
                        unlink(file_path);
                    }
                }
                sprintf(file_path, "%s%s", FOLDER_PATCH, src_path);
                unlink(file_path);
            }
            else if(!strcmp(cmd, "check"))
            {
                /* allowed on this system/os? */
                if(strcmp(os_tag,"x") && !strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /* we don't want apply this file, delete it from patch folder */
                    sprintf(file_path, "%s%s", FOLDER_PATCH, src_path);
                    /*printf("CHECK unlink: %s\n", file_path);*/
                    unlink(file_path);
                }
            }
        }
        else 
        {
            /* create a new folder in the patch target directory */
            if(!strcmp(cmd, "mkdir"))
            {
                if(!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("MKDIR: %s\n", src_path);*/
                    if(mkdir(src_path, 0777)!=0)
                    {
                        fprintf(stderr, "Cannot create directory '%s': ", src_path);
                        perror("");
                    }
                }
            }
            /* remove a folder in the patch target directory - remove all content/subdir too */
            else if(!strcmp(cmd, "rmdir"))
            {
                if(!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("RMDIR: %s\n", src_path);*/
                    /* rmdir xx is equal to "rm -fr xx" */
                    clear_directory(src_path);
                    if(rmdir(src_path)!=0)
                    {
                        fprintf(stderr, "Cannot delete directory '%s': ", src_path);
                        perror("");
                    }
                }
            }
            /* copy/move a file in patch target directory from one position to another */
            else if(!strcmp(cmd, "move") || !strcmp(cmd, "copy") )
            {
                if(!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("MOVE/COPY: %s\n", src_path);*/
                    copy_patch(src_path, target_path);/* copy file from a to b */
                    if(!strcmp(cmd, "move"))
                        unlink(src_path); /* and delete old src file (move only) */
                }
            }
            /* delete (unlink) a file in the patch target directory */
            else if(!strcmp(cmd, "del"))
            {
                if(!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("DELETE: %s\n", src_path);*/
                    unlink(src_path);
                }
            }
        }
    }

    fclose(stream);

    /* delete at last the patch.file so we have a clean patch directory */
    if(mode == TRUE)
    {
        sprintf(file_path,"%s/%s", FOLDER_PATCH, patch_file);
        unlink(file_path);
    }


    return(0);
}
