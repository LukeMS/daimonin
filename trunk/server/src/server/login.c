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

    The author can be reached via e-mail to daimonin@nord-com.net
*/
#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <loader.h>

extern spell spells[NROFREALSPELLS];
extern void sub_weight (object *, signed long);
extern void add_weight (object *, signed long);
extern long pticks;

/* If flag is non zero, it means that we want to try and save everyone, but
 * keep the game running.  Thus, we don't want to free any information.
 */
void emergency_save(int flag) {
  player *pl;
#ifndef NO_EMERGENCY_SAVE
  trying_emergency_save = 1;

  LOG(llevSystem,"Emergency save:  ");
  for(pl=first_player;pl!=NULL;pl=pl->next) {
    if(!pl->ob) {
      LOG(llevSystem, "No name, ignoring this.\n");
      continue;
    }
    LOG(llevSystem,"%s ",pl->ob->name);
    new_draw_info(NDI_UNIQUE, 0,pl->ob,"Emergency save...");

/* If we are not exiting the game (ie, this is sort of a backup save), then
 * don't change the location back to the village.  Note that there are other
 * options to have backup saves be done at the starting village
 */
    if (!flag) {
	strcpy(pl->maplevel, first_map_path);
	if(pl->ob->map!=NULL)
	    pl->ob->map = NULL;
	pl->ob->x = -1;
	pl->ob->y = -1;
    }
	container_unlink(pl,NULL);
    if(!save_player(pl->ob,flag)) {
      LOG(llevSystem, "(failed) ");
      new_draw_info(NDI_UNIQUE, 0,pl->ob,"Emergency save failed, checking score...");
    }
    check_score(pl->ob);
  }
  LOG(llevSystem,"\n");
#else
  LOG(llevSystem,"Emergency saves disabled, no save attempted\n");
#endif
  /* If the game is exiting, remove the player locks */
  if (!flag) {
    for(pl=first_player;pl!=NULL;pl=pl->next) {
      if(pl->ob) {
	unlock_player(pl->ob->name);
      }
    }
  }
}

/* Delete character with name.  if new is set, also delete the new
 * style directory, otherwise, just delete the old style playfile
 * (needed for transition)
 */
void delete_character(const char *name, int new) {
    char buf[MAX_BUF];

    sprintf(buf,"%s/%s/%s.pl",settings.localdir,settings.playerdir,name);
    if(unlink(buf)== -1)
	{
		LOG(llevBug, "BUG:: delete_character(): unlink(%s) failed! (dir not removed too)\n", buf);
		return;
	}
    if (new) {
	sprintf(buf,"%s/%s/%s",settings.localdir,settings.playerdir,name);
	/* this effectively does an rm -rf on the directory */
	remove_directory(buf);
    }
}

/* Lock/unlock player functions.  In reality, the only time they are
 * really needed is if 2 players are logging in at the same time and
 * trying to use the same name to create characters (not very likely,
 * but...)  Otherwise, it checks against the existance of other active
 * players for duplicate names.  In reality, these functions could
 * probably be removed and it wouldn't create any problems.  However,
 * they are minor enough and the extra safety the grant is probably
 * worth keeping them around.
 */

/* Renamed from 'remove_lock' to unlock_player to better match with
 * lock_player below.  Basically, given player 'pl', it removes the
 * corresponding lock file.
 */

void unlock_player(const char *name) {
    char buf[MAX_BUF];

    sprintf(buf,"%s/%s/%s.lock",settings.localdir,settings.playerdir,name);
    if(!rmdir(buf)) {
		LOG(llevBug,"BUG: Couldn't remove lockfile(dir): %s\n", buf);
    }
}

#if 0
/* creates a lock for player 'name'.  returns 0 if the lock is successful,
 * 1 otherwise.
 */

static int lock_player(char *name) {
    char buf[MAX_BUF];

    sprintf(buf,"%s/%s/%s.lock",settings.localdir,settings.playerdir,name);
    if(!mkdir(buf,0770))
	return 0;
    if(errno != EEXIST) {
		LOG(llevBug,"BUG: Couldn't create lockfile(dir): %s\n", buf);
	return 1;
    }
    return 1;
}
#endif

/* This verify that a character of name exits, and that it matches
 * password.  It return 0 if there is match, 1 if no such player,
 * 2 if incorrect password.
 */

int verify_player(char *name, char *password)
{
    char buf[MAX_BUF];
    int comp;
    FILE *fp;

    sprintf(buf,"%s/%s/%s/%s.pl",settings.localdir,settings.playerdir,name,name);
    if ((fp=open_and_uncompress(buf,0,&comp))==NULL) return 1;

    /* Read in the file until we find the password line.  Our logic could
     * be a bit better on cleaning up the password from the file, but since
     * it is written by the program, I think it is fair to assume that the
     * syntax should be pretty standard.
     */
    while (fgets(buf, MAX_BUF-1, fp) != NULL) {
	if (!strncmp(buf,"password ",9)) {
	    buf[strlen(buf)-1]=0;	/* remove newline */
	    if (check_password(password, buf+9)) {
		close_and_delete(fp, comp);
		return 0;
	    }
	    else {
		close_and_delete(fp, comp);
		return 2;
	    }
	}
    }
    LOG(llevDebug,"Could not find a password line in player %s\n", name);
    close_and_delete(fp, comp);
    return 1;
}

/* Checks to see if anyone else by 'name' is currently playing.  We
 * do this by a lockfile (playername.lock) in the save directory.  If
 * we find that file or another character of some name is already in the
 * game, we don't let this person join.  We return 0 if the name is
 * in use, 1 otherwise.
 */

int check_name(player *me,char *name) {
    player *pl;

    for(pl=first_player;pl!=NULL;pl=pl->next)
	if(pl!=me&&pl->ob->name!=NULL&&!strcmp(pl->ob->name,name)) {
	    new_draw_info(NDI_UNIQUE, 0,me->ob,"That name is already in use.");
	    return 0;
	}

#if 0
    /* This is commented out - unexpected client/server crashes can leave the
     * lock files laying around.  In retrospect, I don't think the reason
     * give above is needed for lock files.  If two new people are creating
     * characters at the same time (or logging in), the server data
     * is still consistent (ie, it will get the name from player 1, store
     * it in the player object, get the name from player 2, check it against
     * current players and find a duplicated.)
     */
    if(lock_player(name)) {
	/* Include the 'locked' so that the error message is more descriptive.
	 * This way, players can include a slightly more detailed message
	 * on the problem they are having.
	 */
	new_draw_info(NDI_UNIQUE, 0,me->ob,"That name is already in use (locked).");
	return 0;
    }
#endif
    if(!playername_ok(name)) {
	unlock_player(name);
	new_draw_info(NDI_UNIQUE, 0,me->ob,"That name contains illegal characters.");
	return 0;
    }
    return 1;
}

int create_savedir_if_needed(char *savedir)
{
  struct stat *buf;

  if ((buf = (struct stat *) malloc(sizeof(struct stat))) == NULL) {
	LOG(llevError,"ERROR: Unable to save playerfile... out of memory!! %s\n", savedir);
    return 0;
  } else {
    stat(savedir, buf);
    if ((buf->st_mode & S_IFDIR) == 0)
#if defined(_IBMR2) || defined(___IBMR2)
      if (mkdir(savedir, S_ISUID|S_ISGID|S_IRUSR|S_IWUSR|S_IXUSR))
#else
      if (mkdir(savedir, S_ISUID|S_ISGID|S_IREAD|S_IWRITE|S_IEXEC))
#endif
	{
		LOG(llevBug,"BAD BUG: Unable to create player savedir: %s\n", savedir);
		return 0;
      }
    free(buf);
  }
 return 1;
}

void destroy_object (object *op)
{
    object *tmp;
    while ((tmp = op->inv))
	destroy_object (tmp);

    if (!QUERY_FLAG(op, FLAG_REMOVED))
	remove_ob(op);
    free_object(op);
}

/*
 * If flag is set, it's only backup, ie dont remove objects from inventory
 * If BACKUP_SAVE_AT_HOME is set, and the flag is set, then the player
 * will be saved at the emergency save location.
 * Returns non zero if successful.
 */

int save_player(object *op, int flag) {
  FILE *fp;
  char filename[MAX_BUF], *tmpfilename,backupfile[MAX_BUF];
  object *tmp;/* *container=NULL;*/
  player *pl = op->contr;
  int i,wiz=QUERY_FLAG(op,FLAG_WIZ);
  long checksum;
#ifdef BACKUP_SAVE_AT_HOME
  sint16 backup_x, backup_y;
#endif

  if (!op->stats.exp) return 0;	/* no experience, no save */

  flag&=1;

  if(!pl->name_changed||(!flag&&!op->stats.exp)) {
    if(!flag) {
      new_draw_info(NDI_UNIQUE, 0,op,"Your game is not valid,");
      new_draw_info(NDI_UNIQUE, 0,op,"Game not saved.");
    }
    return 0;
  }

    /* Sanity check - some stuff changes this when player is exiting */
    if (op->type != PLAYER) return 0;

    /* Prevent accidental saves if connection is reset after player has
     * mostly exited.
     */
    if (pl->state != ST_PLAYING && pl->state != ST_GET_PARTY_PASSWORD)
	return 0;

	/* perhaps we don't need it here?*/
  /*container_unlink(pl,NULL);*/

  if (flag == 0)
    terminate_all_pets(op);

  /* Delete old style file */
  sprintf(filename,"%s/%s/%s.pl",settings.localdir,settings.playerdir,op->name);
  unlink(filename);

  sprintf(filename,"%s/%s/%s/%s.pl",settings.localdir,settings.playerdir,op->name,op->name);
  make_path_to_file(filename);
  tmpfilename = tempnam_local(settings.tmpdir,NULL);
  fp=fopen(tmpfilename, "w");
  if(!fp) {
    new_draw_info(NDI_UNIQUE, 0,op, "Can't open file for save.");
    LOG(llevDebug,"Can't open file for save (%s).\n",tmpfilename);
    free(tmpfilename);
    return 0;
  }

/* Eneq(@csd.uu.se): If we have an open container hide it. */

  fprintf(fp,"password %s\n",pl->password);

#ifdef EXPLORE_MODE
  fprintf(fp,"explore %d\n",pl->explore);
#endif
  fprintf(fp,"gen_hp %d\n",pl->gen_hp);
  fprintf(fp,"gen_sp %d\n",pl->gen_sp);
  fprintf(fp,"gen_grace %d\n",pl->gen_grace);
  fprintf(fp,"listening %d\n",pl->listening);
  fprintf(fp,"spell %d\n",pl->chosen_spell);
  fprintf(fp,"shoottype %d\n",pl->shoottype);
  fprintf(fp,"digestion %d\n",pl->digestion);
  fprintf(fp,"pickup %d\n", pl->mode);
  /*
  fprintf(fp,"outputs_sync %d\n", pl->outputs_sync);
  fprintf(fp,"outputs_count %d\n", pl->outputs_count);
*/
  /* Match the enumerations but in string form */
  fprintf(fp,"usekeys %s\n", pl->usekeys==key_inventory?"key_inventory":
	  (pl->usekeys==keyrings?"keyrings":"containers"));

#ifdef BACKUP_SAVE_AT_HOME
  if (op->map!=NULL && flag==0)
#else
  if (op->map!=NULL)
#endif
    fprintf(fp,"map %s\n",op->map->path);
  else
    fprintf(fp,"map %s\n",EMERGENCY_MAPPATH);
  
  fprintf(fp,"savebed_map %s\n", pl->savebed_map);
  fprintf(fp,"bed_x %d\nbed_y %d\n", pl->bed_x, pl->bed_y);
  fprintf(fp,"Str %d\n",pl->orig_stats.Str);
  fprintf(fp,"Dex %d\n",pl->orig_stats.Dex);
  fprintf(fp,"Con %d\n",pl->orig_stats.Con);
  fprintf(fp,"Int %d\n",pl->orig_stats.Int);
  fprintf(fp,"Pow %d\n",pl->orig_stats.Pow);
  fprintf(fp,"Wis %d\n",pl->orig_stats.Wis);
  fprintf(fp,"Cha %d\n",pl->orig_stats.Cha);

  /* old lev_array code 
  fprintf(fp,"lev_array %d\n",op->level>10?10:op->level);
  for(i=1;i<=pl->last_level&&i<=10;i++) {
    fprintf(fp,"%d\n",pl->levhp[i]);
    fprintf(fp,"%d\n",pl->levsp[i]);
	 fprintf(fp,"%d\n",pl->levgrace[i]);
  }
  */
  /* save hp table */
  fprintf(fp,"lev_hp %d\n",op->level);
  for(i=1;i<=op->level;i++)
    fprintf(fp,"%d\n",pl->levhp[i]);

  /* save sp table */
  fprintf(fp,"lev_sp %d\n",op->contr->sp_exp_ptr->level);
  for(i=1;i<=op->contr->sp_exp_ptr->level;i++)
    fprintf(fp,"%d\n",pl->levsp[i]);

  fprintf(fp,"lev_grace %d\n",op->contr->grace_exp_ptr->level);
  for(i=1;i<=op->contr->grace_exp_ptr->level;i++)
    fprintf(fp,"%d\n",pl->levgrace[i]);


  for(i=0;i<pl->nrofknownspells;i++)
    fprintf(fp,"known_spell %s\n",spells[pl->known_spells[i]].name);
  fprintf(fp,"endplst\n");

  SET_FLAG(op, FLAG_NO_FIX_PLAYER);
  CLEAR_FLAG(op, FLAG_WIZ);
#ifdef BACKUP_SAVE_AT_HOME
  if (flag) {
    backup_x = op->x;
    backup_y = op->y;
    op->x = -1;
    op->y = -1;
  }
  /* Save objects, but not unpaid objects.  Don't remove objects from
   * inventory.
   */
  save_object(fp, op, 2);
  if (flag) {
    op->x = backup_x;
    op->y = backup_y;
  }
#else

  save_object(fp, op, 3); /* don't check and don't remove */
#endif

  if(!flag)
      while ((tmp = op->inv))
	  destroy_object (tmp);

  if (fclose(fp) == EOF) {	/* make sure the write succeeded */
	new_draw_info(NDI_UNIQUE, 0,op, "Can't save character.");
	unlink(tmpfilename);
	free(tmpfilename);
  CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
	return 0;
  }
  checksum = calculate_checksum(tmpfilename, 0);
  sprintf(backupfile, "%s.tmp", filename);
  rename(filename, backupfile);
  fp = fopen(filename,"w");
  if(!fp) {
    new_draw_info(NDI_UNIQUE, 0,op, "Can't open file for save.");
    unlink(tmpfilename);
    free(tmpfilename);
  CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
    return 0;
  }
  fprintf(fp,"checksum %lx\n",checksum);
  copy_file(tmpfilename, fp);
  unlink(tmpfilename);
  free(tmpfilename);
  if (fclose(fp) == EOF) {	/* got write error */
	new_draw_info(NDI_UNIQUE, 0,op, "Can't close file for save.");
	rename(backupfile, filename); /* Restore the original */
  CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
	return 0;
  }
  else
	unlink(backupfile);

  /* Eneq(@csd.uu.se): Reveal the container if we have one. */
  /*
  if (flag&&container!=NULL) 
    op->contr->container = container;
*/
  if (wiz) SET_FLAG(op,FLAG_WIZ);
  if(!flag)
	esrv_send_inventory(op, op);

  chmod(filename,SAVE_MODE);
  CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
  return 1;
}

/*
 * calculate_checksum:
 * Evil scheme to avoid tampering with the player-files 8)
 * The cheat-flag will be set if the file has been changed.
 */

long calculate_checksum(char *filename, int checkdouble) {
#ifdef USE_CHECKSUM
  long checksum = 0;
  int offset = 0;
  FILE *fp;
  char buf[MAX_BUF], *cp;
  if ((fp = fopen(filename,"r")) == NULL)
    return 0;
  while(fgets(buf,MAX_BUF,fp)) {
    if(checkdouble && !strncmp(buf,"checksum",8))
      continue;
    for(cp=buf;*cp;cp++) {
      if(++offset>28)
        offset = 0;
      checksum^=(*cp<<offset);
    }
  }
  fclose(fp);
  return checksum;
#else
  return 0;
#endif
}

void copy_file(char *filename, FILE *fpout) {
  FILE *fp;
  char buf[MAX_BUF];
  if((fp = fopen(filename,"r")) == NULL)
    return;
  while(fgets(buf,MAX_BUF,fp)!=NULL)
    fputs(buf,fpout);
  fclose(fp);
}

#if 1
static int spell_sort(const void *a1,const void *a2)
{
  return strcmp(spells[(int)*(sint16 *)a1].name,spells[(int)*(sint16 *)a2].name);
}
#else
static int spell_sort(const char *a1,const char *a2)
{
   LOG(llevDebug, "spell1=%d, spell2=%d\n", *(sint16*)a1, *(sint16*)a2);
  return strcmp(spells[(int )*a1].name,spells[(int )*a2].name);
}
#endif


/* this whole player loading routine is REALLY not optimized - 
 * just look for all these scanf()
 */
void check_login(object *op) {
    FILE *fp;
    char filename[MAX_BUF];
    char buf[MAX_BUF],bufall[MAX_BUF];
    int i,value,comp;
	int lev_array_flag;
    long checksum = 0;
    player *pl = op->contr;
    int correct = 0;
    time_t    elapsed_save_time=0;
    struct stat	statbuf;
	object *tmp;
#ifdef PLUGINS
    CFParm CFP;
    int evtid;
#endif
    strcpy (pl->maplevel,first_map_path);

	/* a good point to add this i to a 10 minute temp ban,,, 
	 * if needed, i add it... its not much work but i better
	 * want a real login server in the future 
	 */
	if(pl->state == ST_PLAYING)
	{
		LOG(llevSystem,"HACK-BUG: >%s< from ip %s - double login!\n", op->name, pl->socket.host);
	    new_draw_info_format(NDI_UNIQUE, 0,op,"You manipulated the login procedure.\nYour IP is ... >%s< - hack flag set!\nserver break",pl->socket.host);
		pl->socket.status = Ns_Dead;
		return;
	}


	LOG(llevInfo,"LOGIN: >%s< from ip %s\n", op->name, op->contr->socket.host);

    /* First, lets check for newest form of save */
    sprintf(filename,"%s/%s/%s/%s.pl",settings.localdir,settings.playerdir,op->name,op->name);
    if (access(filename, F_OK)==-1) {
	/* not there,  Try the old style */

	sprintf(filename,"%s/%s/%s.pl",settings.localdir,settings.playerdir,op->name);
	/* Ok - old style exists.  Lets make the new style directory */
	if (access(filename, F_OK)==0) {
	    sprintf(buf,"%s/%s/%s",settings.localdir,settings.playerdir,op->name);
	    make_path_to_file(buf);
	}
    }

    /* If no file, must be a new player, so lets get confirmation of
     * the password.  Return control to the higher level dispatch,
     * since the rest of this just deals with loading of the file.
     */
    if ((fp=open_and_uncompress(filename,1,&comp)) == NULL) {
	confirm_password(op);
	return;
    }
    if (fstat(fileno(fp), &statbuf)) {
	LOG(llevBug,"BUG: Unable to stat %s?\n", filename);
	elapsed_save_time=0;
    } else {
	elapsed_save_time = time(NULL) - statbuf.st_mtime;
	if (elapsed_save_time<0) {
	    LOG(llevBug,"BUG: Player file %s was saved in the future? (%d time)\n", filename, elapsed_save_time);
	    elapsed_save_time=0;
	}
    }

    if(fgets(bufall,MAX_BUF,fp) != NULL) {
	if(!strncmp(bufall,"checksum ",9)) {
	    checksum = strtol_local(bufall+9,(char **) NULL, 16);
	    (void) fgets(bufall,MAX_BUF,fp);
	}
	if(sscanf(bufall,"password %s\n",buf)) {
	    /* New password scheme: */
	    correct=check_password(pl->write_buf+1,buf);
	}
	/* Old password mode removed - I have no idea what it 
	 * was, and the current password mechanism has been used
	 * for at least several years.
	 */
    }
    if (!correct) {
	new_draw_info(NDI_UNIQUE, 0,op," ");
	new_draw_info(NDI_UNIQUE, 0,op,"Wrong Password!");
	new_draw_info(NDI_UNIQUE, 0,op," ");
	unlock_player(pl->ob->name);
	FREE_AND_COPY_HASH(op->name, "noname");
	pl->last_value= -1;
	get_name(op);
	return;	    /* Once again, rest of code just loads the char */
    }

    unlock_player(pl->ob->name);
#ifdef SAVE_INTERVAL
    pl->last_save_time=time(NULL);
#endif /* SAVE_INTERVAL */
    pl->party_number = (-1);

#ifdef SEARCH_ITEMS
    pl->search_str[0]='\0';
#endif /* SEARCH_ITEMS */
    pl->name_changed=1;
    pl->orig_stats.Str=0;
    pl->orig_stats.Dex=0;
    pl->orig_stats.Con=0;
    pl->orig_stats.Int=0;
    pl->orig_stats.Pow=0;
    pl->orig_stats.Wis=0;
    pl->orig_stats.Cha=0;
    strcpy(pl->savebed_map, first_map_path);
    pl->bed_x=0, pl->bed_y=0;
    
    /* Loop through the file, loading the rest of the values */
	lev_array_flag = FALSE;
    while (fgets(bufall,MAX_BUF,fp)!=NULL) {
	sscanf(bufall,"%s %d\n",buf,&value);
        if (!strcmp(buf,"endplst"))
          break;

#ifdef EXPLORE_MODE
	else if (!strcmp(buf,"explore"))
	    pl->explore = value;
#endif
	else if (!strcmp(buf,"gen_hp"))
	    pl->gen_hp=value;
        else if (!strcmp(buf,"shoottype"))
	    pl->shoottype=(rangetype)value;
        else if (!strcmp(buf,"gen_sp"))
	    pl->gen_sp=value;
        else if (!strcmp(buf,"gen_grace"))
	    pl->gen_grace=value;
        else if (!strcmp(buf,"spell"))
	    pl->chosen_spell=value;
        else if (!strcmp(buf,"listening"))
	    pl->listening=value;
        else if (!strcmp(buf,"digestion"))
	    pl->digestion=value;
	else if (!strcmp(buf,"pickup"))
	    pl->mode=value;
/*
	else if (!strcmp(buf,"outputs_sync"))
	    pl->outputs_sync = value;
	else if (!strcmp(buf,"outputs_count"))
	    pl->outputs_count = value;
*/
        else if (!strcmp(buf,"map"))
	    sscanf(bufall,"map %s", pl->maplevel);
        else if (!strcmp(buf,"savebed_map"))
	    sscanf(bufall,"savebed_map %s", pl->savebed_map);
	else if (!strcmp(buf,"bed_x"))
	    pl->bed_x=value;
	else if (!strcmp(buf,"bed_y"))
	    pl->bed_y=value;
        else if (!strcmp(buf,"Str"))
	    pl->orig_stats.Str=value;
        else if (!strcmp(buf,"Dex"))
	    pl->orig_stats.Dex=value;
        else if (!strcmp(buf,"Con"))
	    pl->orig_stats.Con=value;
        else if (!strcmp(buf,"Int"))
	    pl->orig_stats.Int=value;
        else if (!strcmp(buf,"Pow"))
	    pl->orig_stats.Pow=value;
        else if (!strcmp(buf,"Wis"))
	    pl->orig_stats.Wis=value;
        else if (!strcmp(buf,"Cha"))
	    pl->orig_stats.Cha=value;
	else if (!strcmp(buf,"usekeys")) {
	    if (!strcmp(bufall+8,"key_inventory\n"))
		pl->usekeys=key_inventory;
	    else if (!strcmp(bufall+8,"keyrings\n"))
		pl->usekeys=keyrings;
	    else if (!strcmp(bufall+8,"containers\n"))
		pl->usekeys=containers;
	    else LOG(llevDebug,"load_player: got unknown usekeys type: %s\n", bufall+8);
	}
    else if (!strcmp(buf,"lev_hp"))
	{
		int j;
		for(i=1;i<=value;i++) 
		{
			fscanf(fp,"%d\n",&j);
			pl->levhp[i]=j;
		}
	}
    else if (!strcmp(buf,"lev_sp"))
	{
		int j;
		for(i=1;i<=value;i++) 
		{
			fscanf(fp,"%d\n",&j);
			pl->levsp[i]=j;
		}
	}
    else if (!strcmp(buf,"lev_grace"))
	{
		int j;
		for(i=1;i<=value;i++) 
		{
			fscanf(fp,"%d\n",&j);
			pl->levgrace[i]=j;
		}
	}

		/* if we meet this identifier, then we have a BETA 1 data file.
		 * Now lets change it to BETA 2 version (note that old CVS version
		 * can say 0.95 version instead of 0.96)
		 */
        else if (!strcmp(buf,"lev_array"))
		{
			int j;
		    for(i=1;i<=value;i++) 
			{
				fscanf(fp,"%d\n",&j); /* read in and skip */
				fscanf(fp,"%d\n",&j);
				fscanf(fp,"%d\n",&j);
			}
			lev_array_flag = TRUE;
		/*
	    for(i=1;i<=value;i++) {
		int j;
		fscanf(fp,"%d\n",&j);
		pl->levhp[i]=j;
		fscanf(fp,"%d\n",&j);
		pl->levsp[i]=j;
		fscanf(fp,"%d\n",&j);
		pl->levgrace[i]=j;
	    }*/

	/* spell_array code removed - don't know when that was last used.
	 * Even the load code below will someday be replaced by spells being
	 * objects.
	 */
	} else if (!strcmp(buf,"known_spell")) {
	    char *cp=strchr(bufall,'\n');
	    *cp='\0';
	    cp=strchr(bufall,' ');
	    cp++;
	    for(i=0;i<NROFREALSPELLS;i++)
		if(!strcmp(spells[i].name,cp)) {
		    pl->known_spells[pl->nrofknownspells++]=i;
		    break;
		}
	    if(i==NROFREALSPELLS)
		LOG(llevDebug, "Error: unknown spell (%s)\n",cp);
	}
	/* Remove confkeys, pushkey support - very old */
    } /* End of loop loading the character file */
    /*leave_map(op);*/
	LOG(llevDebug,"load obj for player: %s\n", op->name);
    clear_object(op);
    op->contr = pl;
    pl->ob = op;
    /* this loads the standard objects values. */
    load_object(fp, op, LO_NEWFILE,0);
    close_and_delete(fp, comp);

    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);

    strncpy(pl->title, op->arch->clone.name,MAX_NAME);
    
    /* If the map where the person was last saved does not exist,
     * restart them on their home-savebed. This is good for when
     * maps change between versions
     * First, we check for partial path, then check to see if the full
     * path (for unique player maps)
     */
    if (check_path(pl->maplevel,1)==-1) {
      if (check_path(pl->maplevel,0)==-1) {
	strcpy(pl->maplevel, pl->savebed_map);
	op->x = pl->bed_x, op->y = pl->bed_y;
      }
    }

    /* If player saved beyond some time ago, and the feature is
     * enabled, put the player back on his savebed map.
     */
    if ((settings.reset_loc_time >0) && (elapsed_save_time > settings.reset_loc_time)) {
	strcpy(pl->maplevel, pl->savebed_map);
	op->x = pl->bed_x, op->y = pl->bed_y;
    }

    /* make sure he's a player--needed because of class change. */
    op->type = PLAYER;

	/* this is a funny thing: what happens when the autosave function saves a player
	 * with negative hp? (never thought thats possible but happens in a 0.95 server)
	 * Well, the sever tries to create a gravestone and heals the player... and then
	 * server tries to insert gravestone and anim on a map - but player is still in login!
	 * So, we are nice and set hp to 1 if here negative-.
	 */
	if(op->stats.hp <0)
		op->stats.hp=1;

    pl->name_changed=1;
    pl->state = ST_PLAYING;
#ifdef AUTOSAVE
    pl->last_save_tick = pticks;
#endif
    op->carrying = sum_weight (op);
    /* Need to call fix_player now - program modified so that it is not
     * called during the load process (FLAG_NO_FIX_PLAYER set when
     * saved)
     * Moved ahead of the esrv functions, so proper weights will be
     * sent to the client.
     */

    (void) init_player_exp(op);
    (void) link_player_skills(op);


	/* old 0.95b char - create a modern 0.96 one! */
	/* our rule is: for the first 3 levels we use maxXXX,
	 * for the next levels we simply use full random throw.
	 */
	/* our second mission here is to change all old players deity from Eldath
	 * to the Tabernacle.
	 */
	if(lev_array_flag == TRUE)
	{
		int i;

		/* if this is a 0.95 char, we adjust the exp of every skill or exp_obj NOW */
		/* in the first step, we clear out for recalculation the players level & exp and
		 * every EXPERIENCE object type we found.
		 * we don't use the normal exp functions here to avoid player messages like 
		 * "you lose a level".
		 */
		op->level = 1;
		op->stats.exp = 0;
		for(tmp=op->inv;tmp!=NULL;tmp=tmp->below) 
		{
			if (tmp->type == EXPERIENCE)
			{
				tmp->level = 1;
				tmp->stats.exp = 0;
				/* now check for deity change is this is the Wis exp_obj */
				if (tmp->stats.Wis)
				{
					/* if we have old beta 1 deity Eldath - change to Tabernacle! */
					if(tmp->title && !strcmp(tmp->title,"Eldath"))
						FREE_AND_COPY_HASH(tmp->title, "the Tabernacle");
				}
			}
		}
		/* now we collect all skills and recalculate the level - the exp are untouched here.
		 * then we look in the releated EXPERIENCE object - its lower exp, we set it to
		 * the releated skill. adjust main level/exp of the player on the fly too.
		 */
		for(tmp=op->inv;tmp!=NULL;tmp=tmp->below) 
		{
			if(tmp->type == SKILL)
			{
				for(i=0;i<=MAXLEVEL;i++)
				{
					/* if exp < exp from i+1, our level is i */
					if((uint32)tmp->stats.exp <new_levels[i+1])
					{
						tmp->level = i;
						break;
					}
				}
				if(tmp->exp_obj->stats.exp < tmp->stats.exp)
				{
					tmp->exp_obj->stats.exp = tmp->stats.exp;
					tmp->exp_obj->level = tmp->level;
					/* and lets adjust our main level in the same way! */
					if(op->stats.exp < tmp->exp_obj->stats.exp)
					{
						op->stats.exp = tmp->exp_obj->stats.exp;
						op->level = tmp->exp_obj->level;
					}
				}
			}
		}

		/* first we generate the hp table */
		for(i=1;i<=op->level;i++)
		{
			if(i<=3)
				pl->levhp[i]=(char) op->arch->clone.stats.maxhp;
			else
				pl->levhp[i]=(char)((RANDOM()%op->arch->clone.stats.maxhp)+1);
		}

		/* now the sp chain */
		for(i=1;i<=pl->sp_exp_ptr->level;i++)
		{
			if(i<=3)
				pl->levsp[i]=(char)op->arch->clone.stats.maxsp;
			else
				pl->levsp[i]=(char)((RANDOM()%op->arch->clone.stats.maxsp)+1);
		}

		/* and the grace chain */
		for(i=1;i<=pl->grace_exp_ptr->level;i++)
		{
			if(i<=3)
				pl->levgrace[i]=(char)op->arch->clone.stats.maxgrace;
			else
				pl->levgrace[i]=(char)((RANDOM()%op->arch->clone.stats.maxgrace)+1);
		}
	}

	if ( ! legal_range (op, op->contr->shoottype))
    op->contr->shoottype = range_none;
    
    fix_player (op);
    
    /* if it's a dragon player, set the correct title here */
    if (is_dragon_pl(op) && op->inv != NULL) {
        object *tmp, *abil=NULL, *skin=NULL;
        for (tmp=op->inv; tmp!=NULL; tmp=tmp->below) {
	    if (tmp->type == FORCE) {
	        if (strcmp(tmp->arch->name, "dragon_ability_force")==0)
		    abil = tmp;
		else if (strcmp(tmp->arch->name, "dragon_skin_force")==0)
		    skin = tmp;
	    }
	}
	set_dragon_name(op, abil, skin);
    }

    new_draw_info(NDI_UNIQUE, 0,op,"Welcome Back!");
    new_draw_info_format(NDI_UNIQUE | NDI_ALL, 5, NULL,
	     "%s has entered the game.",pl->ob->name);
#ifdef PLUGINS
    /* GROS : Here we handle the LOGIN global event */
    evtid = EVENT_LOGIN;
    CFP.Value[0] = (void *)(&evtid);
    CFP.Value[1] = (void *)(pl);
    CFP.Value[2] = (void *)(pl->socket.host);
    GlobalEvent(&CFP);
#endif
#ifdef ENABLE_CHECKSUM
    LOG(llevDebug,"Checksums: %x %x\n", checksum,calculate_checksum(filename,1));
    if(calculate_checksum(filename,1) != checksum) {
	new_draw_info(NDI_UNIQUE, 0,op,"Since your savefile has been tampered with,");
	new_draw_info(NDI_UNIQUE, 0,op,"you will not be able to save again.");
	set_cheat(op);
    }
#endif
    /* If the player should be dead, call kill_player for them
     * Only check for hp - if player lacks food, let the normal
     * logic for that to take place.  If player is permanently
     * dead, and not using permadeath mode, the kill_player will
     * set the play_again flag, so return.
     */
    if (op->stats.hp<0) {
	new_draw_info(NDI_UNIQUE, 0,op,"Your character was dead last your played.");
	kill_player(op);
	if (pl->state != ST_PLAYING) return;
    }

    /* Do this after checking for death - no reason sucking up bandwidth if
     * the data isn't needed.
     */
    esrv_new_player(op->contr,op->weight+op->carrying);
    esrv_send_inventory(op, op);

    pl->last_value= -1;

    /* This seems to compile without warnings now.  Don't know if it works
     * on SGI's or not, however.
     */
    qsort((void *)pl->known_spells,pl->nrofknownspells,
	sizeof(pl->known_spells[0]),(void*)(int (*)())spell_sort);


	/* ok, we are done with the login.
	 * Lets put the player on the map and send all player lists to the client.
	 * The player is active now.
	 */
    enter_exit(op,NULL); /* kick player on map - load map if needed */

    pl->socket.update_tile=0;
    pl->socket.look_position=0;
    pl->socket.ext_title_flag = 1;
	
	pl->ob->direction = 4;
    esrv_new_player(op->contr,op->weight+op->carrying);
	send_spelllist_cmd(op, NULL, SPLIST_MODE_ADD); /* send the known spells as list to client */
    send_skilllist_cmd(op, NULL, SPLIST_MODE_ADD);
    return;
}
