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
#include "include/md5.h"
#include "include/xdelta3.h"

#define UPDATE_URL "http://daimonin.sourceforge.net/patch/"

#define UPDATE_PATCH_FILE "patch.file"
#define UPDATE_FILE "patch.master"
#define UPDATE_VERSION "version"
#define FOLDER_UPDATE "update"
#define FOLDER_PATCH "update/patch/"

#ifdef WIN32
#define PROCESS_UPDATER "daimonin_start.exe"
#define SYSTEM_OS_TAG 'w'
#define PROCESS_CLIENT "client.exe"
#else
#define PROCESS_UPDATER "daimonin_start"
#define SYSTEM_OS_TAG 'l'
#define PROCESS_CLIENT "./daimonin"
#endif

#define MAX_DIR_PATH 2048 /* maximal full path we support.      */
char version_path[MAX_DIR_PATH], process_path[MAX_DIR_PATH], output[4096],
prg_path[MAX_DIR_PATH], file_path[MAX_DIR_PATH], parms[MAX_DIR_PATH];

#define COPY_BUFFER_SIZE 1024*512
char *copy_buffer = NULL, *argv0;

int update_flag = FALSE;

FILE *version_handle=NULL;

/* our easy_curl handle */
CURL *curlhandle = NULL;



extern void clear_directory(char* start_dir);
extern void copy_patch_files(char* start_dir);
extern int process_patch_file(char *patch_file, int mode);
extern void copy_patch(char *src, char *dest);
extern int  download_file(char *url, char *remotefilename, char *destfolder, char *destfilename);
int curl_progresshandler(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
extern int  bunzip2(char *infile, char *outfile);
int process_xdelta3(FILE*  patchFile, FILE*  oldFile, FILE* destFile, int BufSize);
extern int  apply_xdelta3(char *patchfile, char *oldfile, char *destfile);
extern int  calc_md5(char *filename, char *outputbuf);

static void free_resources(void)
{
    if (copy_buffer)
        free(copy_buffer);
    if (curlhandle)
        curl_easy_cleanup(curlhandle);
}


static void updater_error(const char *msg)
{
    printf("\nUpdater Error: %s\n", msg);
    printf("\nPRESS RETURN");
    getchar();
    free_resources();
    if (version_handle)
        fclose(version_handle);
    exit(-1);
}

static void start_client_and_close(char *p_path)
{
    printf("Starting client...\n");
//    getchar();
    strcpy(process_path, p_path);
    strcat(process_path, PROCESS_CLIENT); /* '/' will work in windows too */
    execute_process(process_path, PROCESS_CLIENT, "", NULL, 0);
    free_resources();
    if (version_handle)
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
    if (fputs(buf,version_handle)<0)
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
    /*    struct flock fl = { F_RDLCK, SEEK_SET, 0,       0,     0 };*/
#endif
    printf("Daimonin AutoUpdater 1.0a\n\n");

    curlhandle = curl_easy_init();
    if (!curlhandle)
        updater_error("Error initializing curl...");

//        printf("%s\n",argv[0]);
//        if(argc>1)
//            printf("%s\n",argv[1]);
//        if(argc>2)
//            printf("%s\n",argv[2]);
//        printf("\n");

    argv0 = argv[0];
    /* prepare pathes */
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

    /* check for install or update keywords */
    if (argc>1)
    {
        if (!strcmp("install", argv[1]))
        {
            if (argc>2)
            {
                FILE *tmp=NULL;

                /* copy us over older installer */
                /* access has problems under windows in this special case - this works */
                while (!(tmp = fopen(argv[2], "w"))){}

                fclose(tmp);
                copy_patch(argv[0], argv[2]);
                sprintf(parms,"update \"%s\"", argv[0]);
                execute_process(argv[2], argv[2], parms, NULL, 0);

                printf("updated... restarting.\n");
                free_resources();
                exit(0);

            }
        }
        else if (!strcmp("update", argv[1]))
        {
            FILE *tmp=NULL;
            if (argc>2)
            {
                update_flag = TRUE;
                while (!(tmp = fopen(argv[2], "w"))){}
                fclose(tmp);
                unlink(argv[2]);
            }
        }
    }

    /* we use the version file as lock/unlock to avoid different instances of the updater running at once.
    */
    if (update_flag==FALSE)
        printf("Loading version info.... ");
    sprintf(version_path,"%s%s/%s", prg_path, FOLDER_UPDATE, UPDATE_VERSION);
    if ((version_handle = fopen(version_path, "r+t")) == NULL)
    {
        /* check for access/locked */
        updater_error("\nCan't find version info.\nRun file check.\nPRESS RETURN\n");
    }
    /* test & set the lock - if file is locked, exit*/
#ifdef WIN32
    if ( _locking(_fileno(version_handle), _LK_NBLCK, 1L ) == -1 )
#else
    if (lockf(fileno(version_handle), F_TLOCK,1) == -1)
#endif
    {
        fclose(version_handle);
        exit(-2);
    }

    fgets(version, 128 - 1, version_handle);
    /* we don't close the version file here because we need to hold the lock of it */

    adjust_string(version);
    string_pos = strrchr(version, ' ');
    if (!string_pos)
        updater_error("\nError in version file.\nRun file check.\nPRESS RETURN\n");
    *string_pos = '\0';
    version_def_nr = atoi(string_pos+1);

    /* we try to load the version info first.
     * If we don't have a valid version info, we can't
     * do a packer based patch. We have force file to file check.
     * This will also restore a valid version info for the next time.
     */
    if (update_flag==FALSE)
    {
        /* clearing up is always a good idea to start - also ensure its there */
        mkdir(FOLDER_PATCH, 0777);
        clear_directory(FOLDER_PATCH);
    }

    if (update_flag==FALSE)
    {
        printf("version %s\n", version);
        printf("Get update info....\n");

        if (!download_file(UPDATE_URL, UPDATE_FILE, FOLDER_UPDATE, UPDATE_FILE))
        {
            /* failed to get a update... lets start the client anyway and try to connect to a server */
            printf("Update failed!\nStarting client without update.\nPRESS RETURN\n");
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
        printf("Can't find update file!\nUpdate failed!\nStarting client without update.\nPRESS RETURN\n");
        getchar();
        start_client_and_close(prg_path);
    }

    /* check every entry */
    while (fgets(buf, 256 - 1, stream) != NULL)
    {
        adjust_string(buf);

        sscanf(buf, "%s %s %d %s", file_name, md5, &version_nr, version);

        if (version_nr <= version_def_nr) /* ignore same or lesser versions! */
            continue;

        /* we have something new. patch it */
        /* if we have the update flag set, we assume this is our old patch file */
        if (update_flag==FALSE)
        {

            if (!download_file(UPDATE_URL, file_name, FOLDER_UPDATE, file_name))
            {
                /* failed to get a update... lets start the client anyway and try to connect to a server */
                printf("Update failed!\nStarting client without update.\nPRESS RETURN\n");
                getchar();
                start_client_and_close(prg_path);
            }

            printf("Applying patch %s (%d).... \n", version, version_nr);

            sprintf(parms,"%s/%s", FOLDER_UPDATE,file_name);
            calc_md5(parms, output);

            if (strcmp(output, md5) )
                updater_error("MD5 check - FAILED.\nBad File - Restart Updater.\n");
            printf("MD5 check - ok.\n");

            /* unpack pach file */

            printf("extracting patch...\n");

            sprintf(parms,"%s/%s", FOLDER_UPDATE,file_name);
            sprintf(buf,"%s/%s", FOLDER_UPDATE,file_name);

            /* we test for .bz2 ending which means we have a .zip.bz2 file (what a combination) */

            string_pos = strrchr(buf, '.');
            if ((string_pos) && (!strcmp(string_pos, ".bz2")))
            {
                *string_pos = '\0';
                if (!bunzip2(parms, buf))
                {
                    printf("Error extracting file: %s\n",file_name);
                }
            }

            zip_extract(buf,FOLDER_UPDATE);

            if (strcmp(parms, buf))
                unlink(buf);

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

    if (patched)
        printf("Patched to version %s (%d)\n", version, version_nr);
    else
        printf("No new patches found...\n");

#ifdef _DEBUG
	printf("Starting client...\n(debug mode: press RETURN to confirm)\n");
	getchar();
#else
	printf("Starting client...\n");
#endif

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
    DIR* dir;        /* pointer to the scanned directory. */
    struct dirent* entry=NULL;     /* pointer to one directory entry.   */
    char cwd[MAX_DIR_PATH+1]; /* current working directory.        */
    struct stat dir_stat;       /* used by stat().                   */

    /* first, save path of current working directory */
    if (!getcwd(cwd, MAX_DIR_PATH+1))
    {
        perror("getcwd:");
        return;
    }

    /* open the directory for reading */
    if (start_dir)
    {
        dir = opendir(start_dir);
        chdir(start_dir);
    }
    else
        dir = opendir(".");

    if (!dir)
    {
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
            if (rmdir(entry->d_name)!=0)
            {
                fprintf(stderr, "Cannot delete directory '%s': ", entry->d_name);
                perror("");
            }
        }
        else
            unlink(entry->d_name); /* remove a file */
    }

    closedir(dir);

    if (start_dir) /* clean restore */
        chdir(cwd);
}


/* copy a (patched) file over the older original */
void copy_patch(char *src, char *dest)
{
    FILE *src_file, *dest_file;
    int num_read, num_write;

    if (!copy_buffer)
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

    for (;;)
    {
        if (!(num_read = fread(copy_buffer, sizeof( char) ,COPY_BUFFER_SIZE, src_file)))
        {
            /* no error handling so far */
            break;
        }
        if (!(num_write = fwrite(copy_buffer, sizeof( char) ,num_read, dest_file)))
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
    static char base[MAX_DIR_PATH+1]; /* current working directory.        */
    DIR* dir;           /* pointer to the scanned directory. */
    struct dirent* entry;     /* pointer to one directory entry.   */
    char cwd[MAX_DIR_PATH+1]; /* current working directory.        */
    struct stat dir_stat;       /* used by stat().                   */
    char buf[MAX_DIR_PATH+1]; /* current working directory.        */

    /* open the directory for reading */
    if (start_dir)
    {
        /* first, save path of current working directory */
        if (!getcwd(base, MAX_DIR_PATH+1))
        {
            perror("getcwd:");
            return;
        }

        dir = opendir(start_dir);
        chdir(start_dir);
    }
    else
        dir = opendir(".");

    if (!dir)
    {
        fprintf(stderr, "\nCannot read directory '%s': ", cwd);
        perror("");
        return;
    }

    /* first, save path of current working directory */
    if (!getcwd(cwd, MAX_DIR_PATH+1))
    {
        perror("getcwd:");
        return;
    }

    if (start_dir)
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
            /*    int l = strlen(entry->d_name);*/

            sprintf(buf,"%s/%s/%s", base,cwd+patch_dir_len,entry->d_name);
            printf(".");
            copy_patch(entry->d_name, buf);
        }
    }
    closedir(dir);

    if (start_dir) /* clean restore */
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
        if (output[0]=='#' || output[0]=='\0')
            continue;

        sscanf(output, "%s %s %s %s %s", cmd, os_tag, src_path, target_path, dest_path);
        if (mode == FALSE)
        {

            if (!strcmp(cmd, "install"))
            {
                if (update_flag)
                {
                    update_flag = FALSE;
                    continue;
                }
                if (!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    fclose(stream);
                    /* start the new installer and stop, so it can copy over us */
                    sprintf(process_path,"%s%s%s", prg_path, FOLDER_PATCH, src_path);
                    sprintf(parms,"install \"%s\"", argv0);
                    fclose(version_handle); /* important: allow instances */
                    execute_process(process_path, process_path, parms, NULL, 0);

                    printf("Patching Updater!\nleaving old instance....\n");
                    free_resources();
                    exit(0);
                }
            }

            if (update_flag == TRUE)
                continue;

            if (!strcmp(cmd, "xdelta"))
            {
                /* allowed on this system/os? */
                if (!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("XDELTA: %s %s %s\n", src_path, target_path, dest_path);*/

                    sprintf(file_path, "%s%s", FOLDER_PATCH, dest_path);
                    sprintf(parms, "%s%s", FOLDER_PATCH,src_path);
                    if (!apply_xdelta3(parms, target_path ,file_path))
                    {
                        /* be sure we don't left bogus files */
                        unlink(file_path);
                    }
                }
                sprintf(file_path, "%s%s", FOLDER_PATCH, src_path);
                unlink(file_path);
            }
            else if (!strcmp(cmd, "check"))
            {
                /* allowed on this system/os? */
                if (strcmp(os_tag,"x") && !strchr(os_tag,SYSTEM_OS_TAG))
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
            if (!strcmp(cmd, "mkdir"))
            {
                if (!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("MKDIR: %s\n", src_path);*/
                    if (mkdir(src_path, 0777)!=0)
                    {
                        fprintf(stderr, "Cannot create directory '%s': ", src_path);
                        perror("");
                    }
                }
            }
            /* remove a folder in the patch target directory - remove all content/subdir too */
            else if (!strcmp(cmd, "rmdir"))
            {
                if (!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("RMDIR: %s\n", src_path);*/
                    /* rmdir xx is equal to "rm -fr xx" */
                    clear_directory(src_path);
                    if (rmdir(src_path)!=0)
                    {
                        fprintf(stderr, "Cannot delete directory '%s': ", src_path);
                        perror("");
                    }
                }
            }
            /* copy/move a file in patch target directory from one position to another */
            else if (!strcmp(cmd, "move") || !strcmp(cmd, "copy") )
            {
                if (!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("MOVE/COPY: %s\n", src_path);*/
                    copy_patch(src_path, target_path);/* copy file from a to b */
                    if (!strcmp(cmd, "move"))
                        unlink(src_path); /* and delete old src file (move only) */
                }
            }
            /* delete (unlink) a file in the patch target directory */
            else if (!strcmp(cmd, "del"))
            {
                if (!strcmp(os_tag,"x") || strchr(os_tag,SYSTEM_OS_TAG))
                {
                    /*printf("DELETE: %s\n", src_path);*/
                    unlink(src_path);
                }
            }
        }
    }

    fclose(stream);

    /* delete at last the patch.file so we have a clean patch directory */
    if (mode == TRUE)
    {
        sprintf(file_path,"%s/%s", FOLDER_PATCH, patch_file);
        unlink(file_path);
    }


    return(0);
}

int download_file(char *url, char *remotefilename, char *destfolder, char *destfilename)
{
    CURLcode    res;
    FILE        *outfile;
    char        filename[MAX_DIR_PATH];
    char        fullurl[MAX_DIR_PATH];

    if (!curlhandle)
        updater_error("curl error.");

    sprintf(filename,"%s/%s", destfolder, destfilename);
    sprintf(fullurl,"%s%s", url, remotefilename);

    outfile=fopen(filename, "wb+");
    if (!outfile)
        updater_error("dl: could not open file for writing.");

    curl_easy_setopt(curlhandle,CURLOPT_URL, fullurl);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, outfile);
//    curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, FALSE);
    curl_easy_setopt(curlhandle, CURLOPT_PROGRESSFUNCTION, curl_progresshandler);

    printf("start downloading file: %s\n",destfilename);

    res = curl_easy_perform(curlhandle);
    printf("\n");
    if (res)
    {

        printf("curl error: %s\n",curl_easy_strerror(res));
        fclose(outfile);
        return FALSE;
    }

    fclose(outfile);

    printf("%s downloaded succesful.\n",destfilename);
    return TRUE;
}

int curl_progresshandler(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    if (dltotal>0)
    {
        int step = 0;
        char outstring[51];
        int i = 0;

        step = (int) ((dlnow / dltotal)*50.0f +0.5);

        outstring[0]='\0';
        for (i=1;i<=step;i++)
            strcat(outstring,"#");
        for (i=1;i<=(50-step);i++)
            strcat(outstring," ");
        outstring[50]='\0';
        printf("\r[%s] %3.2f%%",outstring, (dlnow/dltotal)*100.0f);

    } else {
        printf("\r[                                                  ]   0.00%%");
    }

    return 0;
}

extern int bunzip2(char *infile, char *outfile)
{
#define BZ_BUFSIZE 1024*512
    FILE    *in = NULL;
    FILE    *out = NULL;
    BZFILE  *bzf = NULL;
    char    buf[BZ_BUFSIZE];
    int     bzer = 0, readlen = 0;
    int     error = FALSE;

    out=fopen(outfile, "wb+");
    if (!out)
        updater_error("unpack: could not open file for writing.");
    in = fopen(infile, "rb");
    if (!in)
    {
        printf("unpack: could not open input file.\n ");
        error = TRUE;
        goto cleanup;
    }
    bzf = BZ2_bzReadOpen(&bzer, in, BZ_VERBOSE, 0, NULL, 0);
    if (bzer!=BZ_OK)
    {
        error = TRUE;
        printf("unpack: bz_open error: %d\n", bzer);
        goto cleanup_both;
    }

    readlen = BZ2_bzRead(&bzer, bzf, &buf, BZ_BUFSIZE);
    while (bzer==BZ_OK)
    {
        fwrite(&buf, readlen, 1, out);
        readlen = BZ2_bzRead(&bzer, bzf, &buf, BZ_BUFSIZE);
    }

    if (bzer==BZ_STREAM_END)
        fwrite(&buf, readlen, 1, out);
    else
    {
        printf("unpack: bz_read error: %d\n",bzer);
        error = TRUE;
    }

    BZ2_bzReadClose(&bzer,bzf);

cleanup_both:
    fclose(in);
cleanup:
    fclose(out);

    if (error)
        return FALSE;

    return TRUE;
#undef BZ_BUFSIZE
}

int process_xdelta3(FILE*  patchFile, FILE*  oldFile, FILE* destFile, int BufSize)
{
  int r, ret;
  struct stat statbuf;
  xd3_stream stream;
  xd3_config config;
  xd3_source source;
  void* Input_Buf;
  int Input_Buf_Read;

  if (BufSize < XD3_ALLOCSIZE)
    BufSize = XD3_ALLOCSIZE;

  memset (&stream, 0, sizeof (stream));
  memset (&source, 0, sizeof (source));

  xd3_init_config(&config, XD3_ADLER32);
  config.winsize = BufSize;
  xd3_config_stream(&stream, &config);

  if (oldFile)
  {
    r = fstat(fileno(oldFile), &statbuf);
    if (r)
      return r;
    source.size = statbuf.st_size;
    source.blksize = BufSize;
    source.curblk = malloc(source.blksize);

    /* Load 1st block of stream. */
    r = fseek(oldFile, 0, SEEK_SET);
    if (r)
      return r;
    source.onblk = fread((void*)source.curblk, 1, source.blksize, oldFile);
    source.curblkno = 0;
    /* Set the stream. */
    xd3_set_source(&stream, &source);
  }

  Input_Buf = malloc(BufSize);

  fseek(patchFile, 0, SEEK_SET);
  do
  {
    Input_Buf_Read = fread(Input_Buf, 1, BufSize, patchFile);
    if (Input_Buf_Read < BufSize)
    {
      xd3_set_flags(&stream, XD3_FLUSH);
    }
    xd3_avail_input(&stream, Input_Buf, Input_Buf_Read);

process:
    ret = xd3_decode_input(&stream);

    switch (ret)
    {
    case XD3_INPUT:
      {
//        printf("XD3_INPUT\n");
        continue;
      }

    case XD3_OUTPUT:
      {
//        printf("XD3_OUTPUT\n");
        r = fwrite(stream.next_out, 1, stream.avail_out, destFile);
        if (r != (int)stream.avail_out)
          return r;
	xd3_consume_output(&stream);
        goto process;
      }

    case XD3_GETSRCBLK:
      {
//        printf("XD3_GETSRCBLK %d\n", source.getblkno);
        if (oldFile)
        {
          r = fseek(oldFile, source.blksize * source.getblkno, SEEK_SET);
          if (r)
            return r;
          source.onblk = fread((void*)source.curblk, 1,
			       source.blksize, oldFile);
          source.curblkno = source.getblkno;
        }
        goto process;
      }

    case XD3_GOTHEADER:
      {
//        printf("XD3_GOTHEADER\n");
        goto process;
      }

    case XD3_WINSTART:
      {
//        printf("XD3_WINSTART\n");
        goto process;
      }

    case XD3_WINFINISH:
      {
//        printf("XD3_WINFINISH\n");
        goto process;
      }

    default:
      {
        printf("!!! INVALID %s %d !!!\n",
		stream.msg, ret);
        return ret;
      }

    }

  }
  while (Input_Buf_Read == BufSize);

  free(Input_Buf);

  free((void*)source.curblk);
  xd3_close_stream(&stream);
  xd3_free_stream(&stream);

  return 0;

}

int apply_xdelta3(char *patchfile, char *oldfile, char *destfile)
{
    FILE   *oldFile = NULL;
    FILE   *patchFile = NULL;
    FILE   *destFile = NULL;
    int ret = 0;

    printf("applying patch: %s...",patchfile);

    oldFile = fopen(oldfile, "rb");
    patchFile = fopen(patchfile, "rb");
    destFile = fopen(destfile, "wb");

    ret = process_xdelta3(patchFile, oldFile, destFile, 1024*32);

    fclose(oldFile);
    fclose(patchFile);
    fclose(destFile);

    if (ret==0)
    {
        printf("ok\n");
        return TRUE;
    } else {
        printf("failed: %s\n",xd3_strerror(ret));
        return FALSE;
    }

}

int calc_md5(char *filename, char *outputbuf)
{
    FILE    *stream;
    char    buf[32];
    int     i;

    stream = fopen(filename, "rb");
    if (!stream)
    {
        printf("could not open file for reading: %s\n",filename);
        return FALSE;
    }
    if (md5_stream(stream, buf)==1)
    {
        printf("error with md5 calc: %s\n",filename);
        fclose(stream);
        return FALSE;
    }
    /* convert the hex to string */
    for (i=0;i<16;i++)
    {
        outputbuf[(i*2)]='\0';
        sprintf(outputbuf, "%s%02x",outputbuf, buf[i]);
    }

    fclose(stream);
    return TRUE;
}
