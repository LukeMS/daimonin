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


/* NOTE: I have added the new commands as (slow) string stuff.
 * The only reason is that they are simple to debug.
 * We have at, 2 command system - this one and the stuff in commands.c.
 * I plan to rework the command system - in 2 steps (new protocol and then later UDP socket)
 * or one step - new protocol, based on UDP.
 */

/* This file deals with administrative commands from the client. */
#include <global.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

#ifndef tolower
#define tolower(C)      (((C) >= 'A' && (C) <= 'Z')? (C) - 'A' + 'a': (C))
#endif

#define MAP_POS_X 0
#define MAP_POS_Y 1

static int map_pos_array[][2] =
{
	{0,0},

	{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},

	{0,-2},{1,-2},{2,-2},{2,-1},{2,0},{2,1},{2,2},{1,2},{0,2},{-1,2},{-2,2},{-2,1},
	{-2,0},{-2,-1},{-2,-2},{-1,-2},

	{0,-3},{1,-3},{2,-3},
	{3,-3},{3,-2},{3,-1},{3,0},{3,1},{3,2},
	{3,3},{2,3},{1,3},{0,3},{-1,3},{-2,3},
	{-3,3},{-3,2},{-3,1},{-3,0},{-3,-1},{-3,-2},
	{-3,-3},{-2,-3},{-1,-3},

	{0,-4},{1,-4},{2,-4},{3,-4},
	{4,-4},{4,-3},{4,-2},{4,-1},{4,0},{4,1},{4,2},{4,3},
	{4,4},{3,4},{2,4},{1,4},{0,4},{-1,4},{-2,4},{-3,4},
	{-4,4},{-4,3},{-4,2},{-4,1},{-4,0},{-4,-1},{-4,-2},{-4,-3},
	{-4,-4},{-3,-4},{-2,-4},{-1,-4},

	{0,-5},{1,-5},{2,-5},{3,-5},{4,-5},
	{5,-5},{5,-4},{5,-3},{5,-2},{5,-1},{5,0},{5,1},{5,2},{5,3},{5,4},
	{5,5},{4,5},{3,5},{2,5},{1,5},{0,5},{-1,5},{-2,5},{-3,5},{-4,5},
	{-5,5},{-5,4},{-5,3},{-5,2},{-5,1},{-5,0},{-5,-1},{-5,-2},{-5,-3},{-5,-4},
	{-5,-5},{-4,-5},{-3,-5},{-2,-5}, {-1,-5},

	{0,-6},{1,-6},{2,-6},{3,-6},{4,-6},{5,-6},
	{6,-6},{6,-5},{6,-4},{6,-3},{6,-2},{6,-1},{6,0},{6,1},{6,2},{6,3},{6,4},{6,5},
	{6,6},{5,6},{4,6},{3,6},{2,6},{1,6},{0,6},{-1,6},{-2,6},{-3,6},{-4,6},{-5,6},
	{-6,6},{-6,5},{-6,4},{-6,3},{-6,2},{-6,1},{-6,0},{-6,-1},{-6,-2},{-6,-3},{-6,-4},{-6,-5},
	{-6,-6},{-5,-6},{-4,-6},{-3,-6},{-2,-6},{-1,-6},

	{0,-7},{1,-7},{2,-7},{3,-7},{4,-7},{5,-7},{6,-7},
	{7,-7},{7,-6},{7,-5},{7,-4},{7,-3},{7,-2},{7,-1},{7,0},{7,1},{7,2},{7,3},{7,4},{7,5},{7,6},
	{7,7},{6,7},{5,7},{4,7},{3,7},{2,7},{1,7},{0,7},{-1,7},{-2,7},{-3,7},{-4,7},{-5,7},{-6,7},
	{-7,7},{-7,6},{-7,5},{-7,4},{-7,3},{-7,2},{-7,1},{-7,0},{-7,-1},{-7,-2},{-7,-3},{-7,-4},{-7,-5},{-7,-6},
	{-7,-7},{-7,-6},{-7,-5},{-7,-4},{-7,-3},{-7,-2},{-7,-1},

	{0,-8},{1,-8},{2,-8},{3,-8},{4,-8},{5,-8},{6,-8},{7,-8},
	{8,-8},{8,-7},{8,-6},{8,-5},{8,-4},{8,-3},{8,-2},{8,-1},{8,0},{8,1},{8,2},{8,3},{8,4},{8,5},{8,6},{8,7},
	{8,8},{7,8},{6,8},{5,8},{4,8},{3,8},{2,8},{1,8},{0,8},{-1,8},{-2,8},{-3,8},{-4,8},{-5,8},{-6,8},{-7,8},
	{-8,8},{-8,7},{-8,6},{-8,5},{-8,4},{-8,3},{-8,2},{-8,1},{-8,0},{-8,-1},{-8,-2},{-8,-3},{-8,-4},{-8,-5},{-8,-6},{-8,-7},
	{-8,-8},{-8,-7},{-8,-6},{-8,-5},{-8,-4},{-8,-3},{-8,-2},{-8,-1}



};

#define NROF_MAP_NODE (sizeof(map_pos_array) /(sizeof(int)*2))


static int compare_A(const void *a, const void *b)
{
  return strcmp(((CommArray_s *)a)->name, ((CommArray_s *)b)->name);
}

static CommArray_s *find_command_element(char *cmd, CommArray_s *commarray,
    int commsize)
{
  CommArray_s *asp, dummy;
  char *cp;

  for (cp=cmd; *cp; cp++)
    *cp =tolower(*cp);

  dummy.name =cmd;
  asp =(CommArray_s *)bsearch((void *)&dummy,
			      (void *)commarray, commsize,
			      sizeof(CommArray_s), compare_A);
  return asp;
}

/* This function is called from the new client/server code.
 * pl is the player who is issuing the command, command is the
 * command.
 */
int execute_newserver_command(object *pl, char *command)
{
    CommArray_s *csp;
    char *cp;

    pl->contr->has_hit=0;

	/* remove the command from the parameters */
    cp=strchr(command, ' ');
    if (cp) 
	{
		*(cp++) ='\0';
		while (*cp==' ') 
			cp++;
    }

    csp = find_plugin_command(command,pl);

    if (!csp)
		csp = find_command_element(command, Commands, CommandsSize);

    if (!csp)
        csp = find_command_element(command, CommunicationCommands, CommunicationCommandSize);

    if (!csp && QUERY_FLAG(pl, FLAG_WIZ))
		csp = find_command_element(command, WizCommands, WizCommandsSize);

    if (csp==NULL) {
	    new_draw_info_format(NDI_UNIQUE, 0,pl,
		"'%s' is not a valid command.", command);
	    return 0;
    }

    pl->speed_left -= csp->time;

    /* A character time can never exceed his speed (which in many cases,
     * if wearing armor, is less than one.)  Thus, in most cases, if
     * the command takes 1.0, the player's speed will be less than zero.
     * it is only really an issue if time goes below -1
     * Due to various reasons that are too long to go into here, we will
     * actually still execute player even if his time is less than 0,
     * but greater than -1.  This is to improve the performance of the
     * new client/server.  In theory, it shouldn't make much difference.
     */

#ifdef DEBUG
    if (csp->time && pl->speed_left<-2.0)
		LOG(llevDebug,"execute_newclient_command: Player issued command that takes more time than he has left.\n");
#endif
	
    return csp->func(pl, cp);
}

int command_run(object *op, char *params)
{
    op->contr->run_on=1;
    return (move_player(op, params?atoi(params):0));
}

int command_run_stop(object *op, char *params)
{
    op->contr->run_on=0;
    return 1;
}


void send_target_command(player *pl)
{
	int aim_self_flag = FALSE;
    char tmp[256];

	if(!pl->ob->map)
		return;

	tmp[0]='t';
	tmp[1]='o';
	tmp[2]=' ';
	tmp[3]=pl->combat_mode;

	pl->ob->enemy=NULL;
	pl->ob->enemy_count=0;
	/* target still legal? */
	if(!pl->target_object || pl->target_object == pl->ob) /* thats we self */
		aim_self_flag = TRUE;
	else if(pl->target_object_count == pl->target_object->count)
	{
		/* ok, a last check... i put it here to have clear code:
		 * perhaps we have legal issues why we can't aim or attack 
		 * our target anymore... invisible & stuff are handled here.
		 * stuff like a out of pvp area moved player are handled different.
		 * we HOLD the target - perhaps the guy moved back.
		 * this special stuff is handled deeper in attack() functions.
		 */
		if(QUERY_FLAG(pl->target_object,FLAG_SYS_OBJECT) || 
					(QUERY_FLAG(pl->target_object,FLAG_IS_INVISIBLE) && !QUERY_FLAG(pl->ob,FLAG_SEE_INVISIBLE)))
			aim_self_flag = TRUE;
		else
		{
			if(pl->target_object->type == PLAYER || QUERY_FLAG(pl->target_object,FLAG_FRIENDLY) )
				tmp[4]=2; /* friend */
			else
			{
				tmp[4]=1; /* enemy */
				pl->ob->enemy=pl->target_object;
				pl->ob->enemy_count=pl->target_object_count;
			}
			if(pl->target_object->name)
				strcpy(tmp+5,pl->target_object->name);
			else
				strcpy(tmp+5,"(null)");
		}
	}
	else
		aim_self_flag = TRUE;

	/* ok... at last, target self */
	if(aim_self_flag)
	{
		tmp[4]=0; /* self */
		strcpy(tmp+5,pl->ob->name);
		pl->target_object = pl->ob;
		pl->target_object_count = 0;
		pl->target_map_pos =0;
	}

    Write_String_To_Socket(&pl->socket, tmp, strlen(tmp+5)+5);
}

int command_combat(object *op, char *params)
{
	if(!op || !op->map || !op->contr)
		return 1;

	if(op->contr->combat_mode)
		op->contr->combat_mode=0;
	else
		op->contr->combat_mode=1;

	send_target_command(op->contr);
	return 1;
}

/* enter combat mode and attack the object in front of us - IF we are in combat
 * and have a enemy/target, skip it and stop attacking.
 */
/* TODO: at some time, we should move the target stuff to the client. but for this,
 * we need a better and smarter client information strategy - MT2003 
 */
int command_target(object *op, char *params)
{
	mapstruct *m;
	object *tmp, *head;
	int n,nt,xt,yt, block, pvp_flag = FALSE;

	if(!op || !op->map || !op->contr || !params || params[0]==0)
		return 1;

	/* 0: we target enemy */
	if(params[0]=='0')
	{
		/* if our target before was a non enemy, start new search
		 * if it was an enemy, use old value.
		 */
		n=0;
		nt=-1;
		
		/* lets search for enemy object! */
		if(op->contr->target_object && op->contr->target_object_count==op->contr->target_object->count 
												&& !QUERY_FLAG(op->contr->target_object,FLAG_FRIENDLY ))
			n=op->contr->target_map_pos;
		else
			op->contr->target_object=NULL;
		
		/* now check where we are. IF we are on a PvP map or in a PvP area - then we can
		* target players on a PvP area too... TODO: group check for group PvP
		*/
		if(GET_MAP_FLAGS(op->map,op->x,op->y)&P_IS_PVP || op->map->map_flags&MAP_FLAG_PVP)
			pvp_flag = TRUE;

		for(;n<NROF_MAP_NODE && n!=nt;n++)
		{
			int xx,yy;
			if(nt==-1)
				nt=n;
			xt=op->x+(xx=map_pos_array[n][MAP_POS_X]);
			yt=op->y+(yy=map_pos_array[n][MAP_POS_Y]);
			block = op->contr->blocked_los[xx+op->contr->socket.mapx/2][yy+op->contr->socket.mapy/2];
			if(block>3 || block<0 ||!(m=out_of_map(op->map,&xt,&yt)))
			{
				if((n+1)==NROF_MAP_NODE)
					n=-1;
				continue;
			}
			/* we can have more as one possible target
			 * on a square - but i try this first without 
			 * handle it.
			 */
			for(tmp=get_map_ob(m,xt,yt);tmp!=NULL;tmp=tmp->above)
			{
				/* this is a possible target */
				tmp->head!= NULL?(head=tmp->head):(head=tmp); /* ensure we have head */
				if (QUERY_FLAG(head,FLAG_MONSTER) && !QUERY_FLAG(head,FLAG_FRIENDLY) ||
				  (pvp_flag && (head->type==PLAYER && (GET_MAP_FLAGS(m,xt,yt)&P_IS_PVP||m->map_flags&MAP_FLAG_PVP))))
				{
					/* this can happen when our old target has moved to next position */
					if(head == op->contr->target_object || head == op || QUERY_FLAG(head,FLAG_SYS_OBJECT) || 
								(QUERY_FLAG(head,FLAG_IS_INVISIBLE) && !QUERY_FLAG(op,FLAG_SEE_INVISIBLE)) )
						continue;
					op->contr->target_object=head;
					op->contr->target_object_count=head->count;
					op->contr->target_map_pos =n;
					goto found_target;
				}
			}
			if((n+1)==NROF_MAP_NODE) /* force a full loop */
				n=-1;
		}

	}
	else if(params[0]=='1') /* friend */
	{
		/* if /target friend but old target was enemy - target self first */
		if(op->contr->target_object && op->contr->target_object_count==op->contr->target_object->count 
						&& !QUERY_FLAG(op->contr->target_object,FLAG_FRIENDLY ))
		{
			op->contr->target_object=op;
			op->contr->target_object_count=op->count;
			op->contr->target_map_pos =0;
		}
		else /* ok - search for a friendly object now! */
		{
			/* if our target before was a non enemy, start new search
			 * if it was an enemy, use old value.
			 */
			n=1; /* don't start with ourself */
			nt=-1;
		
			/* lets search for last friendly object position! */
			if(op->contr->target_object && op->contr->target_object_count==op->contr->target_object->count 
												&& QUERY_FLAG(op->contr->target_object,FLAG_FRIENDLY ))
				n=op->contr->target_map_pos;
			else
				op->contr->target_object=NULL;
		
			/* now check where we are. IF we are on a PvP map or in a PvP area - then we can
			 * target players on a PvP area too... TODO: group check for group PvP
			 */
			if(GET_MAP_FLAGS(op->map,op->x,op->y)&P_IS_PVP || op->map->map_flags&MAP_FLAG_PVP)
				pvp_flag = TRUE;

			for(;n<NROF_MAP_NODE && n!=nt;n++)
			{
				int xx,yy;
				if(nt==-1)
					nt=n;
				xt=op->x+(xx=map_pos_array[n][MAP_POS_X]);
				yt=op->y+(yy=map_pos_array[n][MAP_POS_Y]);
				block = op->contr->blocked_los[xx+op->contr->socket.mapx/2][yy+op->contr->socket.mapy/2];
				if(block>3 || block<0|| !(m=out_of_map(op->map,&xt,&yt)))
				{
					if((n+1)==NROF_MAP_NODE)
						n=-1;
					continue;
				}
				/* we can have more as one possible target
				* on a square - but i try this first without 
				* handle it.
				*/
				for(tmp=get_map_ob(m,xt,yt);tmp!=NULL;tmp=tmp->above)
				{
					/* this is a possible target */
					tmp->head!=NULL?(head=tmp->head):(head=tmp); /* ensure we have head */
					if (QUERY_FLAG(head,FLAG_FRIENDLY) || 
						(!pvp_flag && head->type==PLAYER && 
						  !(GET_MAP_FLAGS(m,xt,yt)&P_IS_PVP) && !(m->map_flags&MAP_FLAG_PVP) ))
					{
							/* this can happen when our old target has moved to next position
						     * i have no tmp == op here to allow self targeting in the friendly chain 
						     */
							if(head == op->contr->target_object || QUERY_FLAG(head,FLAG_SYS_OBJECT) || 
								(QUERY_FLAG(head,FLAG_IS_INVISIBLE) && !QUERY_FLAG(op,FLAG_SEE_INVISIBLE)) )
								continue;
						op->contr->target_object=head;
						op->contr->target_object_count=head->count;
						op->contr->target_map_pos =n;
						goto found_target;
					}
				}
				if((n+1)==NROF_MAP_NODE) /* force a full loop */
					n=-1;
			}
		}
	}
	else if(params[0]=='2') /* self */
	{
		op->contr->target_object=op;
		op->contr->target_object_count=op->count;
		op->contr->target_map_pos =0;
	}
	else /* TODO: ok... try to use params as a name */
	{
		op->contr->target_object=NULL; /* dummy */
	}

	found_target:
	send_target_command(op->contr);
	return 1;
}

void command_face_request(char *params, int len,player *pl)
{
	int i, count;
	
	if(!params)
		return;
		count = *(uint8*)params;

	for(i=0;i<count;i++)
	{
		if(esrv_send_face(&pl->socket,*((short*)(params+1)+i),0) == SEND_FACE_OUT_OF_BOUNDS)
		{
			new_draw_info_format(NDI_UNIQUE|NDI_RED,0, pl->ob, "CLIENT ERROR: Your client requests bad face (#%d). Connection closed!",*((short*)(params+1)+i));
			LOG(llevInfo,"CLIENT BUG: command_face_request (%d) out of bounds. player: %s. close connection.\n",
													*((short*)(params+1)+i), pl->ob?pl->ob->name:"(->ob <no name>)");
			pl->socket.status = Ns_Dead; /* killl socket */
			return;
		}
	}
}

void command_fire(char *params, int len,player *pl)
{
    int dir=0, type, tag1, tag2; 
	object *op=pl->ob;
    
    if(!params)
		return;

    op->contr->fire_on=1;

    /* i submit all this as string for testing. if stable, we change this to a short
	 * and fancy binary format. MT-11-2002
	 */
    sscanf(params,"%d %d %d %d", &dir,&type, &tag1, &tag2);
   
    if(type == FIRE_MODE_SPELL)
    {
        char *tmp;
        tag2=-1;
        tmp = strchr(params,' ');
        tmp = strchr(tmp+1,' ');
        tmp = strchr(tmp+1,' ');
        strncpy(op->contr->firemode_name,tmp+1,60);
		if(!fire_cast_spell (op, op->contr->firemode_name))
		{
		    op->contr->fire_on=0;
		    op->contr->firemode_type = -1; /* marks no client fire action */
			return;
		}
            
    } 
    else if(type == FIRE_MODE_SKILL)
    {
        char *tmp;
        tag2=-1;
        tmp = strchr(params,' ');
        tmp = strchr(tmp+1,' ');
        tmp = strchr(tmp+1,' ');
        strncpy(op->contr->firemode_name,tmp+1,60);
    }
        
    op->contr->firemode_type = type; /* only here will this value be set */
    op->contr->firemode_tag1 = tag1;
    op->contr->firemode_tag2 = tag2;

    move_player(op, dir);
    op->contr->fire_on=0;
    op->contr->firemode_type = -1; /* marks no client fire action */
}

/* STILL IN TEST */
/* sends a mapstats cmd to the players client, after the player had entered the map.
 * Cmd sends map width / map height + mapinfo string.
 */
void send_mapstats_cmd(object *op, struct mapdef *map)
{
    char tmp[2024];

    op->contr->last_update = map; /* player: remember this is the map the client knows */    
    sprintf(tmp,"mapstats %d %d %d %d %s", map->width, map->height, op->x, op->y, map->name);
    Write_String_To_Socket(&op->contr->socket, tmp, strlen(tmp));
}


void send_spelllist_cmd(object *op, char *spellname, int mode)
{
    char tmp[1024*10]; /* we should careful set a big enough buffer here */
 
    sprintf(tmp,"splist %d ", mode);
    if(spellname) /* send single name */
    {
        strcat(tmp, "/");
        strcat(tmp, spellname);
    }
    else
    {
        int i,spnum;
        
        for (i=0; i<(QUERY_FLAG(op, FLAG_WIZ)?NROFREALSPELLS:op->contr->nrofknownspells); i++) 
        {	
            if (QUERY_FLAG(op,FLAG_WIZ)) 
                spnum=i;
            else 
                spnum = op->contr->known_spells[i];
            
            strcat(tmp, "/");
            strcat(tmp, spells[spnum].name);            
        }
    }
    Write_String_To_Socket(&op->contr->socket, tmp, strlen(tmp));    
}

void send_skilllist_cmd(object *op, object *skillp, int mode)
{
    object *tmp2;
    char buf[256];
    char tmp[1024*5]; /* we should careful set a big enough buffer here */
    
    if(skillp)
    {
        if(skillp->last_eat)
            sprintf(tmp,"sklist %d /%s|%d|%d", mode, skillp->name, skillp->level, skillp->stats.exp );
        else
            sprintf(tmp,"sklist %d /%s|%d|-1", mode, skillp->name, skillp->level);       
    }
    else
    {
        sprintf(tmp,"sklist %d ", mode);
        for (tmp2=op->inv;tmp2;tmp2=tmp2->below) 
        {
            if(tmp2->type==SKILL&&IS_SYS_INVISIBLE(tmp2))
            {
                if(tmp2->last_eat)
                    sprintf(buf,"/%s|%d|%d",tmp2->name, tmp2->level, tmp2->stats.exp );
                else
                    sprintf(buf,"/%s|%d|-1",tmp2->name,tmp2->level);       

                strcat(tmp, buf);
            }
        }
    }
    Write_String_To_Socket(&op->contr->socket, tmp, strlen(tmp));        
}

/* all this functions are not really bulletproof. filling tmp[] can be easily produce
 * a stack overflow. Doing here some more intelligent is needed. I do this here
 * with sprintf() only for fast beta implementation */

void send_ready_skill(object *op, char *skillname)
{
    char tmp[256]; /* we should careful set a big enough buffer here */
    
    sprintf(tmp,"skill_rdy %s", skillname);
    Write_String_To_Socket(&op->contr->socket, tmp, strlen(tmp));        
    
}

/* send to the client the golem face & name. Note, that this is only cosmetical
 * information to fill the range menu in the client.
 */
void send_golem_control(object *golem, int mode)
{
    char tmp[256]; /* we should careful set a big enough buffer here */
    
	if(mode == GOLEM_CTR_RELEASE)
	   sprintf(tmp,"gc %d %d %s", mode, 0, golem->name);
	else
		sprintf(tmp,"gc %d %d %s", mode, golem->face->number, golem->name);
    Write_String_To_Socket(&golem->owner->contr->socket, tmp, strlen(tmp));        
    
}

/* generate_ext_title() - get name and grap race/gender/profession from force objects */
void generate_ext_title(player *pl)
{
    object *walk;
    char *gender;
    char prof[32]="";
    char title[32]="";
    char rank[32]="";
    char align[32]="";
    
    /* collect all information from the force objects. Just walk one time through them*/
    for(walk=pl->ob->inv;walk!=NULL;walk=walk->below)
    {
        if (!strcmp(walk->name,"GUILD_FORCE") && !strcmp(walk->arch->name,"guild_force"))
        {
            if(walk->slaying)
                strcpy(prof,walk->slaying);
            if(walk->title)
            {
                strcpy(title," the ");
                strcat(title,walk->title);
            }
        }

        else if (!strcmp(walk->name,"RANK_FORCE") && !strcmp(walk->arch->name,"rank_force"))
        {
            if(walk->title)
            {
                strcpy(rank,walk->title);
                strcat(rank," ");
            }
        }
        
        else if (!strcmp(walk->name,"ALIGNMENT_FORCE") && !strcmp(walk->arch->name,"alignment_force"))
        {
            if(walk->title)
                strcpy(align,walk->title);
        }
    }

	if(QUERY_FLAG(pl->ob, FLAG_IS_MALE))
		gender = "male";
	else if(QUERY_FLAG(pl->ob, FLAG_IS_FEMALE))
		gender = "female";
	else
		gender = "neuter";
	strcpy(pl->quick_name, rank);
	strcat(pl->quick_name, pl->ob->name);
	strcat(pl->quick_name, title);
    sprintf(pl->ext_title,"%s\n%s %s\n%s\n%s\n%s\n%s\n%c\n", rank, pl->ob->name, title, pl->ob->race, prof, align,determine_god(pl->ob), *gender);
}
