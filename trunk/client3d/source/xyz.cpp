#include <stdio.h>
#include <sys/stat.h> 
#include "logfile.h"
#include "logfile.h"
#include "network.h"
#include "xyz.h"
#include "zlib.h"

struct _spell_list      spell_list[SPELL_LIST_MAX]; /* skill list entries */
struct _skill_list      skill_list[SKILL_LIST_MAX]; /* skill list entries */

struct _dialog_list_set spell_list_set;
struct _dialog_list_set skill_list_set; 

#define HUGE_BUF 1024 


_bmaptype          *bmap_table[BMAPTABLE]; 

int                 bmaptype_table_size; 

_anim_table         anim_table[MAXANIM]; /* the stored "anim commands" we created out of anims.tmp */
Animations          animations[MAXANIM]; /* get this from commands.c to this place*/
 _bmaptype_table bmaptype_table[MAX_BMAPTYPE_TABLE];


void SYSTEM_End()
{

}




static int load_anim_tmp(void)
{
    int     i, anim_len = 0, new_anim = TRUE;
    unsigned char   faces   = 0;
    unsigned short  count = 0, face_id;
    FILE   *stream;
    char    buf[HUGE_BUF];
    unsigned char anim_cmd[2048];


    // clear both animation tables
    // this *must* be reloaded every time we connect
    // - remember that different servers can have different
    // animations!
    for (i = 0; i < MAXANIM; i++)
    {
        if (animations[i].faces)
            delete[] (animations[i].faces);
        if (anim_table[i].anim_cmd)
            delete[] (anim_table[i].anim_cmd);
    }
    memset(animations, 0, sizeof(animations));

    // animation #0 is like face id #0 a bug catch - if ever
    // appear in game flow its a sign of a uninit of simply
    // buggy operation.
    anim_cmd[0] = (unsigned char) ((count >> 8) & 0xff);
    anim_cmd[1] = (unsigned char) (count & 0xff);
    anim_cmd[2] = 0; /* flags ... */
    anim_cmd[3] = 1;
    anim_cmd[4] = 0; /* face id o */
    anim_cmd[5] = 0;
    anim_table[count].anim_cmd = new char[6];
    memcpy(anim_table[count].anim_cmd, anim_cmd, 6);
    anim_table[count].len = 6;
    // end of dummy animation #0

    count++;
    if ((stream = fopen(FILE_ANIMS_TMP, "rt")) == NULL)
    {
        LogFile::getSingelton().Error("load_anim_tmp: Error reading anim.tmp!");
        SYSTEM_End(); /* fatal */
        exit(0);
    }

    while (fgets(buf, HUGE_BUF - 1, stream) != NULL)
    {
        if (new_anim == TRUE) /* we are outside a anim body ? */
        {
            if (!strncmp(buf, "anim ", 5))
            {
                new_anim = FALSE;
                faces = 0;
                anim_cmd[0] = (unsigned char) ((count >> 8) & 0xff);
                anim_cmd[1] = (unsigned char) (count & 0xff);
                faces = 1;
                anim_len = 4;
            }
            else /* we should never hit this point */
            {
                LogFile::getSingelton().Error("load_anim_tmp:Error parsing anims.tmp - unknown cmd: >%s<!\n", buf);
            }
        }
        else /* no, we are inside! */
        {
            if (!strncmp(buf, "facings ", 8))
            {
                faces = atoi(buf + 8);
            }
            else if (!strncmp(buf, "mina", 4))
            {
                /*LOG(LOG_DEBUG,"LOAD ANIM: #%d - len: %d (%d)\n", count, anim_len, faces);*/
                anim_cmd[2] = 0; /* flags ... */
                anim_cmd[3] = faces; /* facings */
                anim_table[count].len = anim_len;
                anim_table[count].anim_cmd = new char[anim_len];
                memcpy(anim_table[count].anim_cmd, anim_cmd, anim_len);
                count++;
                new_anim = TRUE;
            }
            else
            {
                face_id = (unsigned short) atoi(buf);
                anim_cmd[anim_len++] = (unsigned char) ((face_id >> 8) & 0xff);
                anim_cmd[anim_len++] = (unsigned char) (face_id & 0xff);
            }
        }
    }


    fclose(stream);
    return 1;
}
 

unsigned long hashbmap(char *str, int tablesize)
{
    unsigned long hash = 0;
    int     i = 0, rot = 0;
    char   *p;

    for (p = str; i < MAXHASHSTRING && *p; p++, i++)
    {
        hash ^= (unsigned long) * p << rot;
        rot += 2;
        if (rot >= (sizeof(long) - sizeof(char)) * 8)
            rot = 0;
    }
    return (hash % tablesize);
}
 

void add_bmap(_bmaptype *at)
{
    int index = hashbmap(at->name,  BMAPTABLE),org_index = index;

    for (; ;)
    {
        if (bmap_table[index] && !strcmp(bmap_table[index]->name, at->name))
        {
            LogFile::getSingelton().Error("ERROR: add_bmap(): double use of bmap name %s\n", at->name);
        }
        if (bmap_table[index] == NULL)
        {
            bmap_table[index] = at;
            return;
        }
        if (++index == BMAPTABLE)
            index = 0;
        if (index == org_index)
            LogFile::getSingelton().Error("ERROR: add_bmap(): bmaptable to small\n");
    }
} 





// after we tested and/or created bmaps.p0 - load the data from it 
static void load_bmaps_p0(void)
{
    char    buf[HUGE_BUF];
    char    name[HUGE_BUF];
    int     len, pos, num;
    unsigned int crc;
    _bmaptype  *at;
    FILE       *fbmap;

    // clear bmap hash table
    memset((void *) bmap_table, 0, BMAPTABLE * sizeof(_bmaptype *));

    // try to open bmaps_p0 file
    if ((fbmap = fopen(FILE_BMAPS_P0, "rb")) == NULL)
    {
        LogFile::getSingelton().Error("FATAL: Error loading bmaps.p0!");
        SYSTEM_End(); // fatal
        unlink(FILE_BMAPS_P0);
        exit(0);
    }
    while (fgets(buf, HUGE_BUF - 1, fbmap) != NULL)
    {
        sscanf(buf, "%d %d %x %d %s", &num, &pos, &crc, &len, name);

        at = (_bmaptype *) malloc(sizeof(_bmaptype));
        at->name = (char *) malloc(strlen(name) + 1);
        strcpy(at->name, name);
        at->crc = crc;
        at->num = num;
        at->len = len;
        at->pos = pos;
        add_bmap(at);
    }
    fclose(fbmap);
}
 
int read_anim_tmp(void)
{
    FILE       *stream, *ftmp;
    int         i, new_anim = TRUE, count = 1;
    char        buf[HUGE_BUF], cmd[HUGE_BUF];
    struct stat stat_bmap, stat_anim, stat_tmp;

    /* if this fails, we have a urgent problem somewhere before */
    if ((stream = fopen(FILE_BMAPS_TMP, "rb")) == NULL)
    {
        LogFile::getSingelton().Error("read_anim_tmp:Error reading bmap.tmp for anim.tmp!");
        SYSTEM_End(); // fatal
        exit(0);
    }
    fstat(fileno(stream), &stat_bmap);
    fclose(stream);

    if ((stream = fopen(FILE_CLIENT_ANIMS, "rb")) == NULL)
    {
        LogFile::getSingelton().Error("read_anim_tmp:Error reading bmap.tmp for anim.tmp!");
        SYSTEM_End(); // fatal
        exit(0);
    }
    fstat(fileno(stream), &stat_anim);
    fclose(stream);

    if ((stream = fopen(FILE_ANIMS_TMP, "rb")) != NULL)
    {
        fstat(fileno(stream), &stat_tmp);
        fclose(stream);

        /* our anim file must be newer as our default anim file */
        if (difftime(stat_tmp.st_mtime, stat_anim.st_mtime) > 0.0f)
        {
            /* our anim file must be newer as our bmaps.tmp */
            if (difftime(stat_tmp.st_mtime, stat_bmap.st_mtime) > 0.0f)
                return load_anim_tmp(); /* all fine - load file */
        }
    }

    unlink(FILE_ANIMS_TMP); /* for some reason - recreate this file */
    if ((ftmp = fopen(FILE_ANIMS_TMP, "wt")) == NULL)
    {
        LogFile::getSingelton().Error("read_anim_tmp:Error opening anims.tmp!");
        SYSTEM_End(); /* fatal */
        exit(0);
    }

    if ((stream = fopen(FILE_CLIENT_ANIMS, "rt")) == NULL)
    {
        LogFile::getSingelton().Error("read_anim_tmp:Error reading client_anims for anims.tmp!");
        SYSTEM_End(); /* fatal */
        exit(0);
    }
    while (fgets(buf, HUGE_BUF - 1, stream) != NULL)
    {
        sscanf(buf, "%s", cmd);
        if (new_anim == TRUE) /* we are outside a anim body ? */
        {
            if (!strncmp(buf, "anim ", 5))
            {
                sprintf(cmd, "anim %d -> %s", count++, buf);
                fputs(cmd, ftmp); /* safe this key string! */
                new_anim = FALSE;
            }
            else /* we should never hit this point */
            {
                LogFile::getSingelton().Error("read_anim_tmp:Error parsing client_anim - unknown cmd: >%s<!\n", cmd);
            }
        }
        else /* no, we are inside! */
        {
            if (!strncmp(buf, "facings ", 8))
            {
                fputs(buf, ftmp); /* safe this key word! */
            }
            else if (!strncmp(cmd, "mina", 4))
            {
                fputs(buf, ftmp); /* safe this key word! */
                new_anim = TRUE;
            }
            else
            {
                /* this is really slow when we have more pictures - we
                         * browsing #anim * #bmaps times the same table -
                         * pretty bad - when we stay to long here, we must create
                         * for bmaps.tmp entries a hash table too.
                         */
                for (i = 0; i < bmaptype_table_size; i++)
                {
                    if (!strcmp(bmaptype_table[i].name, cmd))
                        break;
                }
                if (i >= bmaptype_table_size)
                {
                    // if we are here then we have a picture name in the anims file
                    // which we don't have in our bmaps file! Pretty bad. But because
                    // face #0 is ALWAYS bug.101 - we simply use it here!
                    i = 0;
                    LogFile::getSingelton().Error("read_anim_tmp: Invalid anim name >%s< - set to #0 (bug.101)!\n", cmd);
                }
                sprintf(cmd, "%d\n", i);
                fputs(cmd, ftmp);
            }
        }
    }

    fclose(stream);
    fclose(ftmp);
    return load_anim_tmp(); // all fine - load file
} 




void read_settings(void)
{
    FILE       *stream;
    struct stat statbuf;

    srv_client_files[SRV_CLIENT_SETTINGS].len = 0;
    srv_client_files[SRV_CLIENT_SETTINGS].crc = 0;
    LogFile::getSingelton().Info("Reading %s...", FILE_CLIENT_SETTINGS);
    if ((stream = fopen(FILE_CLIENT_SETTINGS, "rb")) != NULL)
    {
        // temp load the file and get the data we need for compare with server.
        fstat(fileno(stream), &statbuf);
        int i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_SETTINGS].len = i;
        unsigned char *temp_buf = new unsigned char[i];
        fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_SETTINGS].crc = crc32(1L, temp_buf, i);
        delete[] temp_buf;
        fclose(stream);
        LogFile::getSingelton().Info(" found file!(%.6d/%x)", srv_client_files[SRV_CLIENT_SETTINGS].len,
            srv_client_files[SRV_CLIENT_SETTINGS].crc);
    }
    LogFile::getSingelton().Success(true);
}

void read_spells(void)
{
    int         i, ii, panel;
    char        type, nchar, *tmp, *tmp2;
    struct stat statbuf;
    FILE       *stream;
    char        line[255], name[255], d1[255], d2[255], d3[255], d4[255], icon[128];

    for (i = 0; i < SPELL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            spell_list[i].entry[0][ii].flag = LIST_ENTRY_UNUSED;
            spell_list[i].entry[1][ii].flag = LIST_ENTRY_UNUSED;
            spell_list[i].entry[0][ii].name[0] = 0;
            spell_list[i].entry[1][ii].name[0] = 0;
        }
    }
    spell_list_set.class_nr = 0;
    spell_list_set.entry_nr = 0;
    spell_list_set.group_nr = 0;

    srv_client_files[SRV_CLIENT_SPELLS].len = 0;
    srv_client_files[SRV_CLIENT_SPELLS].crc = 0;
    LogFile::getSingelton().Info("Reading %s......", FILE_CLIENT_SPELLS);
    if ((stream = fopen(FILE_CLIENT_SPELLS, "rb")) != NULL)
    {
        /* temp load the file and get the data we need for compare with server */
        fstat(fileno(stream), &statbuf);
        i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_SPELLS].len = i;
        unsigned char *temp_buf = new unsigned char[i];
        fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_SPELLS].crc = crc32(1L, temp_buf, i);
        delete[] temp_buf;
        rewind(stream);

        for (i = 0; ; i++)
        {
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(name, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            sscanf(line, "%c %c %d %s", &type, &nchar, &panel, icon);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d1, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d2, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d3, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d4, tmp + 1);
            panel--;
            spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].flag = LIST_ENTRY_USED;
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].icon_name, icon);
//            sprintf(line, "%s%s", GetIconDirectory(), icon);
//            spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].icon = sprite_load_file(line, 0);

            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].name, name);
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].desc[0], d1);
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].desc[1], d2);
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].desc[2], d3);
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].desc[3], d4);
        }
        fclose(stream);
        LogFile::getSingelton().Info(" found file!(%.6d/%x)", srv_client_files[SRV_CLIENT_SPELLS].len,
            srv_client_files[SRV_CLIENT_SPELLS].crc);
    }
    LogFile::getSingelton().Success(true);
}

void read_skills(void)
{
    int         i, ii, panel;
    char        nchar, *tmp, *tmp2;
    struct stat statbuf;
    FILE       *stream;
    char        line[255], name[255], d1[255], d2[255], d3[255], d4[255], icon[128];

    for (i = 0; i < SKILL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            skill_list[i].entry[ii].flag = LIST_ENTRY_UNUSED;
            skill_list[i].entry[ii].name[0] = 0;
        }
    }

    skill_list_set.group_nr = 0;
    skill_list_set.entry_nr = 0;

    srv_client_files[SRV_CLIENT_SKILLS].len = 0;
    srv_client_files[SRV_CLIENT_SKILLS].crc = 0;

    LogFile::getSingelton().Info("Reading %s.......", FILE_CLIENT_SKILLS);
    if ((stream = fopen(FILE_CLIENT_SKILLS, "rb")) != NULL)
    {
        // temp load the file and get the data we need for compare with server
        fstat(fileno(stream), &statbuf);
        i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_SKILLS].len = i;
        unsigned char *temp_buf = new unsigned char[i];
        fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_SKILLS].crc = crc32(1L, temp_buf, i);
        delete[] temp_buf;
        rewind(stream);

        for (i = 0; ; i++)
        {
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(name, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            sscanf(line, "%d %c %s", &panel, &nchar, icon);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d1, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d2, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d3, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d4, tmp + 1);

            skill_list[panel].entry[nchar - 'a'].flag = LIST_ENTRY_USED;
            skill_list[panel].entry[nchar - 'a'].exp = 0;
            skill_list[panel].entry[nchar - 'a'].exp_level = 0;

            strcpy(skill_list[panel].entry[nchar - 'a'].icon_name, icon);
//            sprintf(line, "%s%s", GetIconDirectory(), icon);
//            skill_list[panel].entry[nchar - 'a'].icon = sprite_load_file(line, 0);

            strcpy(skill_list[panel].entry[nchar - 'a'].name, name);
            strcpy(skill_list[panel].entry[nchar - 'a'].desc[0], d1);
            strcpy(skill_list[panel].entry[nchar - 'a'].desc[1], d2);
            strcpy(skill_list[panel].entry[nchar - 'a'].desc[2], d3);
            strcpy(skill_list[panel].entry[nchar - 'a'].desc[3], d4);
        }
        fclose(stream);
        LogFile::getSingelton().Info(" found file!(%.6d/%x)", srv_client_files[SRV_CLIENT_SKILLS].len,
            srv_client_files[SRV_CLIENT_SKILLS].crc);
    }
    LogFile::getSingelton().Success(true);
}

void read_anims(void)
{
	FILE       *stream;
    struct stat statbuf;
    int         i;

    LogFile::getSingelton().Info("Loading %s......", FILE_CLIENT_ANIMS);
    srv_client_files[SRV_CLIENT_ANIMS].len = 0;
    srv_client_files[SRV_CLIENT_ANIMS].crc = 0;
    if ((stream = fopen(FILE_CLIENT_ANIMS, "rb")) != NULL)
    {
        /* temp load the file and get the data we need for compare with server */
        fstat(fileno(stream), &statbuf);
        i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_ANIMS].len = i;
        unsigned char *temp_buf = new unsigned char[i];
        fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_ANIMS].crc = crc32(1L, temp_buf, i);
        delete[] temp_buf;
        fclose(stream);
        LogFile::getSingelton().Info(" found file!(%.6d/%x)", srv_client_files[SRV_CLIENT_ANIMS].len,
            srv_client_files[SRV_CLIENT_ANIMS].crc);
    }
	LogFile::getSingelton().Success(true);
}

bool read_bmaps(void)
{
	FILE   *fbmap, *fpic;
    char   *cp;
    int     bufsize, len, num, pos;
    unsigned int crc;
    char        buf[HUGE_BUF];
    struct stat bmap_stat, pic_stat;

    LogFile::getSingelton().Info("Loading %s...................  ", FILE_DAIMONIN_P0);
    if ((fpic = fopen(FILE_DAIMONIN_P0, "rb")) == NULL)
    {
        LogFile::getSingelton().Success(false);
        SYSTEM_End(); // fatal.
        unlink(FILE_BMAPS_P0);
        return false;
    }
    // get time stamp of the file daimonin.p0 
    fstat(fileno(fpic), &pic_stat);

    // try to open bmaps_p0 file 
    if ((fbmap = fopen(FILE_BMAPS_P0, "r")) == NULL)
	{
		LogFile::getSingelton().Success(true);
        goto create_bmaps;
    }
    // get time stamp of the file
    fstat(fileno(fbmap), &bmap_stat);
    fclose(fbmap);

    if (difftime(pic_stat.st_mtime, bmap_stat.st_mtime) > 0.0f)
        goto create_bmaps;

    fclose(fpic);
    load_bmaps_p0();
	LogFile::getSingelton().Success(true);
    return true;

  create_bmaps: // if we are here, then we have to (re)create the bmaps.p0 file
    LogFile::getSingelton().Info("Creating %s......................", FILE_BMAPS_P0);
    if ((fbmap = fopen(FILE_BMAPS_P0, "w")) == NULL)
    {
        LogFile::getSingelton().Error("FATAL: Can't create bmaps.p0 file!");
        SYSTEM_End(); // fatal
        fclose(fbmap);
        unlink(FILE_BMAPS_P0);
        return false;
    }
    unsigned char *temp_buf = new unsigned char[(bufsize = 24 * 1024)];

    while (fgets(buf, HUGE_BUF - 1, fpic) != NULL)
    {
        if (strncmp(buf, "IMAGE ", 6) != 0)
        {
            LogFile::getSingelton().Success(false);
            LogFile::getSingelton().Error("read_client_images:Bad image line - not IMAGE, instead\n%s", buf);
            SYSTEM_End(); // fatal
            fclose(fbmap);
            fclose(fpic);
            unlink(FILE_BMAPS_P0);
            return false;
        }

        num = atoi(buf + 6);
        /* Skip accross the number data */
        for (cp = buf + 6; *cp != ' '; cp++)
            ;
        len = atoi(cp);

        strcpy(buf, cp);
        pos = (int) ftell(fpic);

        if (len > bufsize) // dynamic buffer adjustment
        {
            delete[] temp_buf;
            // we assume thats this is nonsense
            if (len > 128 * 1024)
            {
                LogFile::getSingelton().Success(false);
                LogFile::getSingelton().Error("read_client_images:Size of picture out of bounds!(len:%d)(pos:%d)", len, pos);
                SYSTEM_End(); /* fatal */
                fclose(fbmap);
                fclose(fpic);
                unlink(FILE_BMAPS_P0);
                return false;
            }
            bufsize = len;
            temp_buf = new unsigned char[bufsize];
        }

        fread(temp_buf, 1, len, fpic);
        crc = crc32(1L, temp_buf, len);

        // now we got all we needed!
        sprintf((char*)temp_buf, "%d %d %x %s", num, pos, crc, buf);
        fputs((char*)temp_buf, fbmap);
    }
    delete[] temp_buf;
    fclose(fbmap);
    fclose(fpic);
    load_bmaps_p0();
	LogFile::getSingelton().Success(true);
    return true; 
}

