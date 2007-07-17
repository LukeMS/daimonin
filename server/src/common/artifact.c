/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

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

#include <global.h>

static hashtable *art_table;

/* quick table to access artifact list for types */
static artifactlist *art_type_table[ARCH_MAX_TYPES];

/*
* Allocate and return the pointer to an empty artifactlist structure.
*/
static artifactlist * get_empty_artifactlist(void)
{
    artifactlist   *tl  = (artifactlist *) malloc(sizeof(artifactlist));

    if (tl == NULL)
        LOG(llevError, "ERROR: get_empty_artifactlist(): OOM!\n");
    memset(tl, 0, sizeof(artifactlist));

    return tl;
}

/*
 * Allocate and return the pointer to an empty artifact structure.
 */
static artifact * get_empty_artifact(void)
{
    artifact   *t   = (artifact *) malloc(sizeof(artifact));

    if (t == NULL)
        LOG(llevError, "ERROR: get_empty_artifact(): OOM!\n");
    memset(t, 0, sizeof(artifact));

    return t;
}

/* fill the artifacts table */
static void fill_artifact_table(void)
{
    artifactlist   *al;
    artifact       *art;

    art_table = string_hashtable_new(4096);

    for (al = first_artifactlist; al != NULL; al = al->next)
    {
        for (art = al->items; art != NULL; art = art->next)
        {
            if (! hashtable_insert(art_table, art->name, art))
            {
                LOG(llevError, "ERROR: add_artifact_hash(): double use of artifact name %s\n", STRING_SAFE(art->name));
            }
        }
    }
}


/*
* Builds up the lists of artifacts from the file in the libdir.
*/
/* Remember: other_arch & treasurelists defined in the artifacts file
* will be parsed in a second parse by hand - like the normal arches.
*/

/* first pass artifacts load */
static void init_artifacts(FILE *fp)
{
    archetype      *atemp;
    long            old_pos, file_pos;
    char            buf[MAX_BUF], *cp, *next;
    artifact       *art             = NULL;
    linked_char    *tmp;
    int             lcount, value, none_flag = 0, editor_flag = 0;
    artifactlist   *al;
    char            buf_text[10 * 1024]; /* ok, 10k arch text... if we bug here, we have a design problem */
    object            *dummy_obj=get_object(), *parse_obj;

    /* start read in the artifact list */
    while (fgets(buf, MAX_BUF, fp) != NULL)
    {
        if (*buf == '#')
            continue;
        cp = buf + (strlen(buf) - 1);
        while(cp > buf && isspace(*cp))
            --cp;
        cp[1] = '\0';
        cp = buf;
        while (isspace(*cp)) /* Skip blanks */
            cp++;

        /* we have a single artifact */
        if (!strncmp(cp, "Allowed", 7))
        {
            art = get_empty_artifact();
            editor_flag = FALSE;
            nrofartifacts++;
            none_flag = FALSE;
            cp+=7;
            if (!strcasecmp(cp, "all"))
                continue;
            if (!strcasecmp(cp, "none"))
            {
                none_flag = TRUE;
                continue;
            }
            do
            {
                while (isspace(*cp)) /* Skip blanks */
                    cp++;
                nrofallowedstr++;
                if ((next = strchr(cp, ',')) != NULL)
                    *(next++) = '\0';
                tmp = (linked_char *) malloc(sizeof(linked_char));
                tmp->name = NULL;
                FREE_AND_COPY_HASH(tmp->name, cp);
                tmp->next = art->allowed;
                art->allowed = tmp;
            }
            while ((cp = next) != NULL);
        }
        else if (sscanf(cp, "t_style %d", &value))
            art->t_style = value;
        else if (sscanf(cp, "chance %d", &value))
            art->chance = (uint16) value;
        else if (sscanf(cp, "difficulty %d", &value))
            art->difficulty = (uint8) value;
        else if (!strncmp(cp, "artifact", 8))
        {
            FREE_AND_COPY_HASH(art->name, cp + 9);
        }
        else if (!strncmp(cp, "def_arch", 8)) /* chain a default arch to this treasure */
        {
            FREE_AND_COPY_HASH(art->def_at_name, cp + 9); /* store the def archetype name (real base arch) */
        }
        else if (!strncmp(cp, "Object", 6)) /* all text after Object is now like a arch file until a end comes */
        {
            if(editor_flag == FALSE)
                LOG(llevError, "ERROR: Init_Artifacts: Artifact %s (%s) has no 'editor x:...' line!\n", STRING_SAFE(art->name),STRING_SAFE(art->def_at_name));

            if (!art->name)
                LOG(llevError, "ERROR: Init_Artifacts: Artifact %s has no arch id name\n", STRING_SAFE(art->def_at_name));

            old_pos = ftell(fp);
            if(art->flags&ARTIFACT_FLAG_HAS_DEF_ARCH) /* we have & use a default arch */
            {
                if (!art->def_at_name)
                    LOG(llevError, "ERROR: Init_Artifacts: Artifact %s has no def arch\n", art->name);

                /* we patch this .clone object after Object read with the artifact data.
                 * in find_artifact, this archetype object will be returned. For the server,
                 * it will be the same as it comes from the arch list, defined in the arches.
                 * This will allow us the generate for every artifact a "default one" and we
                 * will have always a non-magical base for every artifact
                 */
                if ((atemp = find_archetype(art->def_at_name)) == NULL)
                    LOG(llevError, "ERROR: Init_Artifacts: Can't find def_arch %s.\n", art->def_at_name);
                memcpy(&art->def_at, atemp, sizeof(archetype)); /* copy the default arch */
                art->def_at.base_clone = &atemp->clone;
                ADD_REF_NOT_NULL_HASH(art->def_at.clone.name);
                ADD_REF_NOT_NULL_HASH(art->def_at.clone.title);
                ADD_REF_NOT_NULL_HASH(art->def_at.clone.race);
                ADD_REF_NOT_NULL_HASH(art->def_at.clone.slaying);
                ADD_REF_NOT_NULL_HASH(art->def_at.clone.msg);
                art->def_at.clone.arch = &art->def_at;
                parse_obj = &art->def_at.clone;
            }
            else
            {
                parse_obj = dummy_obj;
                parse_obj->type = 0; /* we need the dummy obj to get the type info of the artifact! */
            }

            /* parse the new fake arch clone with the artifact values */
            if (!load_object(fp, parse_obj, NULL, LO_LINEMODE, MAP_STATUS_STYLE))
                LOG(llevError, "ERROR: Init_Artifacts: Could not load object.\n");

            /* ok, now lets catch & copy the commands to our artifacts buffer.
             * lets do some file magic here - thats the easiest way.
             */
            file_pos = ftell(fp);

            if (fseek(fp, old_pos, SEEK_SET))
                LOG(llevError, "ERROR: Init_Artifacts: Could not fseek(fp,%d,SEEK_SET).\n", old_pos);

            /* the lex reader will bug when it don't get feed with a <text>+0x0a+0 string.
             * so, we do it here and in the lex part we simple do a strlen and point
             * to every part without copy it.
             */
            lcount = 0;
            while (fgets(buf, MAX_BUF - 3, fp))
            {
                strcpy(buf_text + lcount, buf);
                lcount += strlen(buf) + 1;
                if (ftell(fp) == file_pos)
                    break;
                if (ftell(fp) > file_pos) /* should not possible! */
                    LOG(llevError, "ERROR: Init_Artifacts: fgets() read to much data! (%d - %d)\n", file_pos, ftell(fp));
            };

            /* now store the parse text in the artifacts list entry */
            if ((art->parse_text = malloc(lcount)) == NULL)
                LOG(llevError, "ERROR: Init_Artifacts: out of memory in ->parse_text (size %d)\n", lcount);

            memcpy(art->parse_text, buf_text, lcount);

            if(art->flags&ARTIFACT_FLAG_HAS_DEF_ARCH)
                FREE_AND_COPY_HASH(art->def_at.name, art->name); /* finally, change the archetype name of
                                                                  * our fake arch to the fake arch name.
                                                                  * without it, treasures will get the
                                                                  * original arch, not this (hm, this
                                                                  * can be a glitch in treasures too...)
                                                                  */
            /* now handle the <Allowed none> in the artifact to create
             * unique items or add them to the given type list.
             */
            al = find_artifactlist(none_flag == FALSE ? parse_obj->type : -1);
            if (al == NULL)
            {
                al = get_empty_artifactlist();
                al->type = none_flag == FALSE ? parse_obj->type : -1;
                al->next = first_artifactlist;
                first_artifactlist = al;
                /* init the quick jump table */
                if(!art_type_table[al->type+1])
                    art_type_table[al->type+1] = al;
            }

            art->next = al->items;
            al->items = art;
        }
        else if (!strncmp(cp, "editor", 6))
        {
            /* QUICKHACK: we don't won't delete the pre-beta 4 fake arch items on the players */
            art->flags |= ARTIFACT_FLAG_HAS_DEF_ARCH; /* remove this quickhack for 1.0 */

            editor_flag = TRUE;
            if(!strncmp(cp+7,"2:",2)) /* mask only */
                /* Do nothing */;
            else if(!strcmp(cp+7,"0") || !strncmp(cp+7,"1:",2) || !strncmp(cp+7,"3:",2)) /* use def arch def_at */
                art->flags |= ARTIFACT_FLAG_HAS_DEF_ARCH;
            else
                LOG(llevError, "\nERROR: Invalid editor line in artifact file: %s\n", cp);
        }
        else
            LOG(llevBug, "\nBUG: Unknown line in artifact file: %s\n", buf);
    }
}

void second_artifact_pass(FILE *fp)
{
    char            buf[MAX_BUF], *variable = buf, *argument, *cp;
    archetype      *at =  NULL, *other;

    while (fgets(buf, MAX_BUF, fp) != NULL)
    {
        if (*buf == '#')
            continue;
        if ((argument = strchr(buf, ' ')) != NULL)
        {
            *argument = '\0',argument++;
            cp = argument + strlen(argument) - 1;
            while (isspace(*cp))
            {
                *cp = '\0';
                cp--;
            }
        }

        /* now we get our artifact. if we hit "def_arch", we first copy from it
         * other_arch and treasure list to our artifact.
         * then we search the object for other_arch and randomitems - perhaps we override them here.
         */
        if (!strcmp("artifact", variable)) /* be sure "artifact" command is before def_arch cm! */
        {
            at = find_archetype(argument);
        }
        else if (at && !strcmp("def_arch", variable))
        {
            if ((other = find_archetype(argument)) == NULL)
            {
                LOG(llevBug, "BUG: second artifacts pass: failed to find def_arch %s from artifact %s\n",
                        STRING_SAFE(argument), STRING_ARCH_NAME(at));
                continue;
            }
            /* now copy from real arch the stuff from above to our "fake" arches */
            at->clone.other_arch = other->clone.other_arch;
            if (at->clone.randomitems)
                unlink_treasurelists(at->clone.randomitems, FALSE);
            at->clone.randomitems = other->clone.randomitems;
            if (at->clone.randomitems && (at->clone.randomitems->flags & OBJLNK_FLAG_REF))
                at->clone.randomitems->ref_count++;
        }
        else if (!strcmp("other_arch", variable))
        {
            if ((other = find_archetype(argument)) == NULL)
                LOG(llevBug, "BUG: second artifacts pass: failed to find other_arch %s\n", STRING_SAFE(argument));
            else if (at != NULL)
                at->clone.other_arch = other;
        }
        else if (at && !strcmp("randomitems", variable))
        {
            if (at->clone.randomitems)
                unlink_treasurelists(at->clone.randomitems, TRUE);
            at->clone.randomitems = link_treasurelists(argument, OBJLNK_FLAG_STATIC);
        }
    }
}

/*
* recusively traverse the given directory and search for *.art files
* and process them like they are part of the original artifact file
*/
static void traverse_artifact_files(char* start_dir, int mode)
{
    DIR* dir;                    /* pointer to the scanned directory. */
    struct dirent* entry=NULL;   /* pointer to one directory entry.   */
    char *fptr, cwd[HUGE_BUF+1]; /* current working directory.        */
    struct stat dir_stat;        /* used by stat().                   */

    /* first, save path of current working directory */
    if (!getcwd(cwd, HUGE_BUF+1)) {
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
        /* skip all ".*" entries, to avoid loops and forbidden directories. */
        if (entry->d_name[0] == '.')
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
            traverse_artifact_files(NULL, mode);

            /* finally, restore the original working directory. */
            if (chdir("..") == -1)
            {
                fprintf(stderr, "Cannot chdir back to '%s': ", cwd);
                perror("");
            }
        }
        else
        {
            /* lets check its a valid, local artifacts file */
            if(entry->d_name[0] != '.' && (fptr = strrchr(entry->d_name, '.')) && !strcmp(fptr, ".art") )
            {
                FILE *fp;

                LOG(llevDebug, " adding local artifacts from %s...", entry->d_name);
                if ((fp = fopen(entry->d_name, "r")) == NULL)
                {
                    LOG(llevError, "ERROR: Can't open %s.\n", entry->d_name);
                    exit(global_exit_return);
                }

                if(mode == ARTIFACTS_FIRST_PASS)
                    init_artifacts(fp);
                else
                    second_artifact_pass(fp);

                fclose(fp);
                LOG(llevDebug, "done.\n");
            }
        }
    }

    closedir(dir);

    if(start_dir) /* clean restore */
        chdir(cwd);
}

void load_artifacts(int mode)
{
    static int      has_been_inited = 0;
    FILE           *fp;
    artifactlist   *al;
    artifact       *art;
    char            filename[MAX_BUF];

    if (has_been_inited >= mode)
        return;
    has_been_inited = mode;

    /* load default file */
    sprintf(filename, "%s/artifacts", settings.datadir);
    LOG(llevDebug, " reading artifacts from %s...", filename);
    if ((fp = fopen(filename, "r")) == NULL)
    {
        LOG(llevError, "ERROR: Can't open %s.\n", filename);
        return;
    }

    if(mode == ARTIFACTS_FIRST_PASS)
    {
        int i;

        for(i=0;i<ARCH_MAX_TYPES;i++)
            art_type_table[i] = NULL;

        init_artifacts(fp);
    }
    else
        second_artifact_pass(fp);

    fclose(fp);
    LOG(llevDebug, "done.\n");

    /* now traverse the maps directory and the single map sets for *.art files */
    traverse_artifact_files(settings.mapdir, mode);

    if(mode == ARTIFACTS_FIRST_PASS)
    {
        for (al = first_artifactlist; al != NULL; al = al->next)
        {
            for (art = al->items; art != NULL; art = art->next)
            {
                /*add_arch();
                  LOG(llevDebug,"art: %s (%s %s)\n", art->name, art->def_at.name, query_name(&art->def_at.clone));*/

                if (al->type == -1) /* we don't use our unique artifacts as pick table */
                    continue;
                if (!art->chance)
                    LOG(llevBug, "BUG: artifact with no chance: %s\n", art->name);
                else
                    al->total_chance += art->chance;
            }
        }
    }
    else
        fill_artifact_table(); /* last action - populate the artifacts hash table for fast access */
}

/*
* Searches the artifact lists and returns one that has the same type
* of objects on it.
*/
inline artifactlist * find_artifactlist(int type)
{
    return art_type_table[type+1];
}

/*
 * find a artifact entry by name
 */
artifact *find_artifact(const char *name)
{
    if (name == NULL)
        return NULL;

    return (artifact *)hashtable_find(art_table, name);
}

void add_artifact_archtype(void)
{
    artifactlist   *al;
    artifact       *art = NULL;

    for (al = first_artifactlist; al != NULL; al = al->next)
    {
        art = al->items;
        do
        {
            if (art->flags&ARTIFACT_FLAG_HAS_DEF_ARCH && art->name)
            {
                add_arch(&art->def_at);
            }
            art = art->next;
        }
        while (art != NULL);
    }
}

/*
 * Fixes the given object, giving it the abilities and titles
 * it should have due to the second artifact-template.
 */
void give_artifact_abilities(object *op, artifact *art)
{
    sint64 tmp_value   = op->value;

    op->value = 0;
    if (!load_object(art->parse_text, op, NULL, LO_MEMORYMODE, MAP_STATUS_ARTIFACT))
        LOG(llevError, "ERROR: give_artifact_abilities(): load_object() error (ob: %s art: %s).\n", op->name, art->name);

    /* this will solve the problem to adjust the value for different items
     * of same artification. Also we can safely use negative values.
     */
    op->value += tmp_value;
    if (op->value < 0)
        op->value = 0;

#if 0 /* Bit verbose, but keep it here until next time I need it... */
    {
        char identified = QUERY_FLAG(op, FLAG_IDENTIFIED);
        SET_FLAG(op, FLAG_IDENTIFIED);
        LOG(llevDebug, "Generated artifact %s %s [%s]\n",
                op->name, op->title, describe_item(op));
        if (!identified)
            CLEAR_FLAG(op, FLAG_IDENTIFIED);
    }
#endif
    return;
}

static int legal_artifact_combination(object *op, artifact *art)
{
    int             neg, success = 0;
    linked_char    *tmp;
    const char     *name;

    tmp = art->allowed;
    if (!strcmp(tmp->name, "all"))
        return 1; /* Ie, "all" */
    for (; tmp; tmp = tmp->next)
    {
#ifdef TREASURE_VERBOSE
        LOG(llevDebug, "legal_art: %s\n", tmp->name);
#endif
        if (*tmp->name == '!')
            name = tmp->name + 1, neg = 1;
        else
            name = tmp->name, neg = 0;

        /* If we match name, then return the opposite of 'neg' */
        if (!strcmp(name, op->name) || (op->arch && !strcmp(name, op->arch->name)))
            return !neg;

        /* Set success as true, since if the match was an inverse, it means
         * everything is allowed except what we match
         */
        else if (neg)
            success = 1;
    }
    return success;
}

/*
 * Decides randomly which artifact the object should be
 * turned into.  Makes sure that the item can become that
 * artifact (means magic, difficulty, and Allowed fields properly).
 * Then calls give_artifact_abilities in order to actually create
 * the artifact.
 */
int generate_artifact(object *op, int difficulty, int t_style, int a_chance)
{
    artifactlist   *al;
    artifact       *art;
    artifact       *art_tmp = NULL;
    int             i, style_abs, chance_tmp = 0;

    al = find_artifactlist(op->type);
    if (al == NULL)
    {
#ifdef TREASURE_VERBOSE
        LOG(llevDebug, "Couldn't change %s into artifact - no table.\n", op->name);
#endif
        return 0;
    }

    style_abs = ABS(t_style);

    for (i = 0; i < ARTIFACT_TRIES; i++)
    {
        int roll    = RANDOM() % al->total_chance;

        for (art = al->items; art != NULL; art = art->next)
        {
            roll -= art->chance;
            if (roll < 0)
                break;
        }

        if (art == NULL || roll >= 0)
        {
            LOG(llevBug, "BUG: Got null entry and non zero roll in generate_artifact, type %d\n", op->type);
            return 0;
        }

        /* Map difficulty not high enough OR the t_style is set and don't match */
        if (difficulty < art->difficulty || ( t_style != T_STYLE_UNSET &&
                    (  (t_style > 0 && art->t_style != t_style) /* if style > 0 only same style is valid */
                       || (t_style ==0 && (art->t_style && art->t_style != T_STYLE_UNSET) ) /* style = 0: only art style 0 or unset */
                       || (t_style < 0 && (art->t_style && art->t_style != T_STYLE_UNSET && art->t_style != style_abs) ) /* 0, unset or same style */
                    )))
            continue;

        if (!legal_artifact_combination(op, art))
        {
#ifdef TREASURE_VERBOSE
            LOG(llevDebug, "%s of %s was not a legal combination.\n", op->name, art->item->name);
#endif
            continue;
        }
        give_artifact_abilities(op, art);
        return 1;
    }

    /* if we are here then we failed to generate a artifact by chance.
     * the reasons can be many - most times we just skipped over the
     * useful one.
     * If (and only if) a a_chance  - then now we force our way:
     * - lets get (if there are one) a legal artifact with the highest chance.
     */
    if (a_chance > 0)
    {
        for (art = al->items; art != NULL; art = art->next)
        {
            if (art->chance <= chance_tmp)
                continue;
            if (difficulty < art->difficulty || ( t_style != T_STYLE_UNSET &&
                        (  (t_style > 0 && art->t_style != t_style) /* if style > 0 only same style is valid */
                           || (t_style ==0 && (art->t_style && art->t_style != T_STYLE_UNSET) ) /* style = 0: only art style 0 or unset */
                           || (t_style < 0 && (art->t_style && art->t_style != T_STYLE_UNSET && art->t_style != style_abs) ) /* 0, unset or same style */
                        )))
                continue;
            if (!legal_artifact_combination(op, art))
                continue;
            art_tmp = art; /* there we go! */
        }
    }

    /* now we MUST have one - if there was at last one legal possible artifact */
    if (art_tmp)
        give_artifact_abilities(op, art_tmp);

    return 1;
}

static void free_charlinks(linked_char *lc)
{
    linked_char *tmp, *next;
    for(tmp = lc; tmp; tmp = next)
    {
        next = tmp->next;
        FREE_AND_CLEAR_HASH(tmp->name);;
        free(tmp);
    }
}

static void free_artifact(artifact *at)
{
    FREE_AND_CLEAR_HASH2(at->name);
    FREE_AND_CLEAR_HASH2(at->def_at.name);
    if (at->next)
        free_artifact(at->next);
    if (at->allowed)
        free_charlinks(at->allowed);
    if (at->parse_text)
        free(at->parse_text);
    if(at->flags&ARTIFACT_FLAG_HAS_DEF_ARCH)
    {
        FREE_AND_CLEAR_HASH2(at->def_at.clone.name);
        FREE_AND_CLEAR_HASH2(at->def_at.clone.race);
        FREE_AND_CLEAR_HASH2(at->def_at.clone.slaying);
        FREE_AND_CLEAR_HASH2(at->def_at.clone.msg);
        FREE_AND_CLEAR_HASH2(at->def_at.clone.title);
    }
    free(at);
}

/*
 * TODO: this should also clean up the hashtable
 */
void free_artifactlist(artifactlist *al)
{
    artifactlist   *nextal;
    for (al = first_artifactlist; al != NULL; al = nextal)
    {
        nextal = al->next;
        if (al->items)
        {
            free_artifact(al->items);
        }
        free(al);
    }
}

/*
 * For debugging purposes.  Dumps all tables.
 */

void dump_artifacts()
{
    artifactlist   *al;
    artifact       *art;
    linked_char    *next;

    for (al = first_artifactlist; al != NULL; al = al->next)
    {
        LOG(llevInfo, "Artifact has type %d, total_chance=%d\n", al->type, al->total_chance);
        for (art = al->items; art != NULL; art = art->next)
        {
            LOG(llevInfo, "Artifact %-30s Difficulty %3d T-Style %d Chance %5d\n", art->name, art->difficulty,
                    art->t_style, art->chance);
            if (art->allowed != NULL)
            {
                LOG(llevInfo, "\tAllowed combinations:");
                for (next = art->allowed; next != NULL; next = next->next)
                    LOG(llevInfo, "%s,", next->name);
                LOG(llevInfo, "\n");
            }
        }
    }
    LOG(llevInfo, "\n");
}

/*** end of artifact.c ***/
