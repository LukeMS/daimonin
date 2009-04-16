#include <global.h>
#include <stdarg.h>

/*
 * STATS_EVENT writes a stats event line to a file in settings.statsdir
 * filename is YYYYMMDD-HH-XX.stats, depending on the current time
 * XX will be 00 if no other file is found from the same hour
 * otherwise the new XX will be max(XX) + 1
 * All the other files will be moved to an archived
 * There it can be used for our database
 * Format of log lines in file:
 * event_id <tab> current time in seconds <tab> gametime in ticks <tab> messagetype (int) <tab> type dependent data
 */
void stats_event(stats_event_type type, ...)
{
    time_t              time_now;
    struct              tm* ts;
    char                buf[HUGE_BUF];          /* buffer for writing to stats file */
    char                filename_base[MAX_BUF]; /* stats filename without dir */
    char                filename_full[MAX_BUF]; /* includes path */
    char                filename_source[MAX_BUF]; /* filename for rename */
    char                filename_dest[MAX_BUF]; /* filename for rename */
    static short        last_year = 0;
    static short        last_month = 0;
    static short        last_day = 0;
    static short        last_hour = 0;
    static FILE         *fp = NULL;
    long unsigned int   event_id = 0;
    int                 counter;
    int                 counter_tmp;
    va_list             ap;
    DIR                 *dp;
    struct dirent       *dirp;
    struct stat         statbuf;

    /* The format strings correspond to the type enums
     * in stats.h */
    const char *format[] =
    {
        "",         /* FIRST */
        "",         /* STARTUP needs no parameters */
        "",         /* SHUTDOWN needs no parameters */
        "\t%s",     /* player name */
        "\t%s\t%s", /* PvP names */
        "\t%s",     /* Message string */
        "\t%d",     /* stats event id */
        ""          /* LAST */
    };

    if((type <= STATS_EVENT_FIRST) || (type >= STATS_EVENT_LAST))
    {
        LOG(llevBug, "BUG: Unknown stats_event_type: %d\n", type);
        return;
    }
    if(!event_id)
    {
        /* TODO: get event_id from meta data file */
        /* Create directories */
        sprintf(filename_full, "%s/", settings.statsdir);
        make_path_to_file(filename_full);
        sprintf(filename_full, "%s/", settings.statsarchivedir);
        make_path_to_file(filename_full);
    }
    event_id++;
    time_now = time(NULL);
    ts = gmtime(&time_now);
    /* This will get true once every hour */
    if(   (ts->tm_hour != last_hour)
       || (ts->tm_mday != last_day)
       || (ts->tm_mon  != last_month)
       || (ts->tm_year != last_year))
    {
        if(fp)
        {
            fclose(fp);
            fp = NULL;
        }
        /* format: YYYYMMDD-HH- */
        sprintf(filename_base, "%.4d%.2d%.2d-%.2d-", ts->tm_year + 1900, ts->tm_mon + 1, ts->tm_mday, ts->tm_hour);
        /* Find files with same base name and get counter (XX) */
        if((dp = opendir(settings.statsdir)) == NULL)
        {
            LOG(llevBug, "BUG: Cannot open stats dir %s\n", settings.statsdir);
            return;
        }
        counter = -1;
        while((dirp = readdir(dp)) != NULL)
        {
            if(!strncmp(filename_base, dirp->d_name, 12))
            {
                /* Found file with same base name */
                /* Parse counter */
                if(isdigit(dirp->d_name[12]) && isdigit(dirp->d_name[13]))
                {
                    counter_tmp  = (dirp->d_name[12] - '0') * 10;
                    counter_tmp +=  dirp->d_name[13] - '0';
                    /* get max counter */
                    if(counter_tmp > counter)
                        counter = counter_tmp;
                }
            }
        }
        if(closedir(dp))
            LOG(llevBug, "BUG: Could not close stats dir %s\n", settings.statsdir);
        if(counter < 99)
        {
            sprintf(&filename_base[12], "%.2d.stats", ++counter);
        }
        else
        {
            /* Too many restarts of server in this hour */
            LOG(llevBug, "BUG: Too many stats files/hour\n");
            return;
        }
        sprintf(filename_full, "%s/%s", settings.statsdir, filename_base);
        if((fp = fopen(filename_full, "w")) == NULL)
        {
            LOG(llevBug, "BUG: Cannot open %s for writing event %d\n", filename_full, event_id);
            return;
        }
        /* Move all the other files into archive */
        if((dp = opendir(settings.statsdir)) == NULL)
        {
            LOG(llevBug, "BUG: Cannot open stats dir %s\n", settings.statsdir);
            return;
        }
        else
        {
            while((dirp = readdir(dp)) != NULL)
            {
                /* Don't move active logfile */
                if(strcmp(dirp->d_name, filename_base))
                {
                    sprintf(filename_source, "%s/%s", settings.statsdir, dirp->d_name);
                    if(stat(filename_source, &statbuf))
                        LOG(llevBug, "BUG: Cannot read status of %s\n", filename_source);
                    else
                        if(!S_ISDIR(statbuf.st_mode))
                        {
                            sprintf(filename_dest, "%s/%s", settings.statsarchivedir, dirp->d_name);
                            if(rename(filename_source, filename_dest))
                                LOG(llevBug, "BUG: Cannot move %s to %s\n", filename_source, filename_dest);
                        }
                }
            }
        }
        if(closedir(dp))
            LOG(llevBug, "BUG: Could not close archive %s\n", settings.statsarchivedir);
        last_hour  = ts->tm_hour;
        last_day   = ts->tm_mday;
        last_month = ts->tm_mon;
        last_year  = ts->tm_year;
    }
    va_start(ap, type);
    vsprintf(buf, format[type], ap);
    va_end(ap);
    fprintf(fp, "%ld\t%ld\t%ld\t%d%s\n", event_id, (long)time_now, todtick, type, buf);
}
