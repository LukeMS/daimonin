%{

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
#include <loader.h>
#include <newserver.h>
#include <sproto.h>

#define YY_NO_UNPUT
#define YY_NO_TOP_STATE

/* got a stupid signedess problem in line
 * for ( n = 0; n < (size_t) max_size
 * because the missing (size_t) cast. If you experience
 * problems with this definition, disable this macro here,
 * flex will fallback to its original
 */
#ifdef LINUX
#define YY_INPUT(buf,result,max_size) \
	if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) \
		{ \
		int c = '*'; \
		size_t n; \
		for ( n = 0; n < (size_t) max_size && \
			     (c = getc( yyin )) != EOF && c != '\n'; ++n ) \
			buf[n] = (char) c; \
		if ( c == '\n' ) \
			buf[n++] = (char) c; \
		if ( c == EOF && ferror( yyin ) ) \
			YY_FATAL_ERROR( "input in flex scanner failed" ); \
		result = n; \
		} \
	else \
		{ \
		errno=0; \
		while ( (result = fread(buf, 1, max_size, yyin))==0 && ferror(yyin)) \
			{ \
			if( errno != EINTR) \
				{ \
				YY_FATAL_ERROR( "input in flex scanner failed" ); \
				break; \
				} \
			errno=0; \
			clearerr(yyin); \
			} \
		}\
\

#endif

#define YY_DECL int lex_load(object *op, int map_flags)
static char *yval();
static object *singularity;
static artifact *amask_art;

static void *cur_buffer; /* thats needed to track the used buffers for recursive calling */
static int object_load_flag;
static int lex_error, msg_flag;
static char msgbuf[HUGE_BUF*5];

/* SET_RESIST is really only really needed for transition code.  We normally
 * don't care about multiple values overwriting each other, but this is
 * to catch items that have multiple protection/immune/vulnerable.
 * This can be simplified later on to just do the set after all the archs
 * and maps have been updated.
 * We always keep the last value because otherwise the value from the
 * arch may take precedence.
 * Unfortunately, we will report warnings here simply because an object has
 * been modified from the arch.
 */
#define SET_RESIST(op, type, val)  op->resist[type] = val;
#define SET_ATTACK(op, type, val)  op->attack[type] = val;

#define IVAL	atoi(yval())
#define FVAL	(float) atof(yval())

%}

LF	\x0A
CR	\x0D
NL	({LF}|{CR}{LF}?)

S	[ \t]+.+{NL}?
WS	[ \t]*{NL}?

%x MESSAGE

/* Don't have to link with -lfl with this */
%option noyywrap

/* need yy_push_state, yy_pop_state */
%option stack

%%

%{
/* Declare some local variables */
    int ismore=0;

    lex_error=0;

%}

^arch{S}         {	/* If op->arch has been set, then this new object
			 * must be part of the inventory.  So process
			 * appropriately.
			 */
			 /* i changed here copy_object() to copy_object() - we have no idea what with the
			  * next commands to speed & speed_left - so its redundant and very likely a bug
			  * to adjust here speed.
			  */
			if (op->arch) {
				artifact *tmp_art;
			    object *tmp = NULL, *tmp2 = NULL;
				float old_sleft,old_speed;

			    tmp=get_object();
				tmp_art = amask_art;
				amask_art = NULL;
			    tmp->arch = find_archetype(yval());
			    if (tmp->arch)
					copy_object_data(&tmp->arch->clone,tmp);
				else
				    tmp2 = create_singularity(yval());

			    strcpy(msgbuf, "");
				old_sleft = tmp->speed_left;
				old_speed = tmp->speed;

				lex_load(tmp, map_flags);
			    if (!tmp->arch)
				    tmp = tmp2;

				/* we set here AND in load_object the material real values.
				   * we do: If item_quality == 0 AND material defines a base
				   * material, then use material_real to set quality & condition.
                   * So, we don't set it for floor or walls or some else objects
                   * were we don't need it.
                   */
				if(tmp->material && !tmp->item_quality && tmp->material_real>=0)
				{
					tmp->item_quality = material_real[tmp->material_real].quality;
	                tmp->item_condition = tmp->item_quality;
				}

				if(QUERY_FLAG(tmp,FLAG_IDENTIFIED))
				{
					SET_FLAG(tmp,FLAG_KNOWN_MAGICAL);
					SET_FLAG(tmp,FLAG_KNOWN_CURSED);
				}
				/* artificate the item */
				if(amask_art)
					give_artifact_abilities(tmp, amask_art);

				if (tmp->type == CONTAINER) /* do some safety for containers */
				{
					tmp->attacked_by = NULL; /* used for containers as link to players viewing it */
					tmp->attacked_by_count = 0;
				}

				if(tmp->inv && !QUERY_FLAG(tmp,FLAG_SYS_OBJECT))
					tmp->carrying = sum_weight(tmp);			/* ensure right weight for inventories */
				else
					tmp->carrying = 0; /* sanity setting... this means too - NO double use of ->carrying! */

			    if(QUERY_FLAG(op, FLAG_NO_INVENTORY) )
			    {
                    /* destroy_object(tmp);
                       Don't call destroy_object(). tmp will be
                       garbage collected automatically!
                       Gecko 2006-07-12
                     */
				}
				else
				{
			    insert_ob_in_ob(tmp,op);
				amask_art = tmp_art;

				if (!(map_flags & MAP_STATUS_STYLE))
				{
					/* first adjust speed */
					/* ok - ONLY change something when we
					 * a.) don't have fix set speed_left in the loading
					 * b.) we have negative speed
					 */
					if(old_sleft==tmp->speed_left && tmp->speed<0.0f)
					{
						/* if we have now a negative speed - use default arch
						 * speed left for new random set
						 */
						if(tmp->speed<0.0f)
							tmp->speed_left=tmp->arch->clone.speed_left+(float)(RANDOM()%90)/100.0f;
					}
					if(tmp->speed)
						update_ob_speed(tmp); /* kick it now on active list*/

				/* we must do this after the insert_ob_in_ob().
				 * now we look we have a treasure or something else.
				 * If so, we transform it now to real items.
				 * also we adjust here auto_apply.
				 * This was moved from fix_auto_apply.
				 * This here only handles inventory items, the "base"
				 * items are handled in load_objects() .
				 */
					if(QUERY_FLAG(tmp,FLAG_AUTO_APPLY)) /* remember: auto_apply will remove FLAG_AUTO_APPLY */
						auto_apply(tmp);
					/* here we can have a interesting problem when we have kind of
					 * creators (like spawn points) which use the inv object as a clone
					 * kind object. Then we don't want substitute the treasure here or the
					 * generator will clone objects with same treasures over and over.
					 */
					 /* i excluded here only objects inside spawn_points. Add here more
					  * what you need to exclude at object/map loading time.
					  */
					else if(tmp->randomitems && (map_flags & MAP_STATUS_ORIGINAL) &&(op->type != SPAWN_POINT ))
						create_treasure_list(tmp->randomitems, tmp, 0, get_enviroment_level(tmp),ART_CHANCE_UNSET,0);

                    if(object_initializers[tmp->type] != NULL)
                        object_initializers[tmp->type](tmp);
				}
				}
			}
			/* This is the actual archetype definition then */
			else {
					op->arch=find_archetype(yval());
					if (op->arch)
						copy_object_data(&op->arch->clone,op);
					else
						singularity = create_singularity(yval());
					/* note: speed will handled in load_object() for this */
				}
		    }

^object{S}	    {	char *yv=yval();

			if (*yv=='\0') {
			    LOG(llevError,"Object lacks name.\n");
			    return LL_IGNORED;
			}
			if (op->arch!=NULL)
				FREE_AND_COPY_HASH(op->arch->name, yv);
			FREE_AND_COPY_HASH(op->name, yv);
			object_load_flag = 1; /* ok - now we know the pre speed values are valid */
		    }

^msg{WS}	    {	BEGIN( MESSAGE ); msgbuf[0]='\0'; msg_flag=0;}
<MESSAGE>^endmsg{WS} {	BEGIN( INITIAL );
    			if (strlen(msgbuf) > HUGE_BUF-16 )
	    		    LOG(llevMapbug, "\n\tMAPBUG: SKIPPED msg/endmsg -> buffer overflow!! (obj: %s) %d (max allowed=%d)\n", query_name(op), strlen(msgbuf),HUGE_BUF-16);
                else
    		    	FREE_AND_COPY_HASH(op->msg, msgbuf);
                }
			/*<MESSAGE>.*	    {if(msg_flag)strcat(msgbuf,"\n");msg_flag=1;strcat(msgbuf, yytext); }*/
<MESSAGE>.*{WS}	strcat(msgbuf, yytext);

^name{S}	    {	char *yv=yval();
			if (*yv=='\0') LOG(llevError,"Name without val\n");
			else FREE_AND_COPY_HASH(op->name, yv);
		    }
^race{S}	    FREE_AND_COPY_HASH(op->race,yval());
^slaying{S}	    FREE_AND_COPY_HASH(op->slaying, yval());

^other_arch{S}        op->other_arch=find_archetype(yval());
^animation{S}	    {
			if (strcmp (yval(), "NONE") == 0)
			    op->animation_id = 0;
			else
			    op->animation_id = find_animation (yval());
		    }
^inv_animation{S}	    {
			if (strcmp (yval(), "NONE") == 0)
			    op->inv_animation_id = 0;
			else
			    op->inv_animation_id = find_animation (yval());
		    }

^more{WS}	    { /* We need to record that this is a multipart object,
		       * so the calling function can glue things back together
		       */
			ismore=1;
		    }

^end{WS}	    { if (ismore) return LL_MORE; else return LL_NORMAL; }
^object_int1{S}		op->enemy_count = IVAL;
^object_int2{S}		op->attacked_by_count = IVAL;
^object_int3{S}		op->owner_count = IVAL;
^last_heal{S}	    op->last_heal = IVAL;
^last_sp{S}			op->last_sp = IVAL;
^last_grace{S}		op->last_grace = IVAL;
^last_eat{S}	    op->last_eat = IVAL;
^speed{S}			op->speed = FVAL;

^speed_left{S}		op->speed_left = FVAL;
^amask{S}		{
					if(!(amask_art = find_artifact(yval())))
					    LOG(llevMapbug,"MAPBUG: amask->find_artifact(): unknown artifact %s.\n", STRING_SAFE(yval()));
				}
^face{S}		{
					register int _face_ = FindFace(yval(), 0);
					op->face = &new_faces[_face_ ];

					if(!_face_)
					    LOG(llevMapbug,"MAPBUG: Object %s - can't find face %s.\n",
							op->arch?(op->arch->name?op->arch->name:"<no arch name>"):"<no ->arch>",yval());
				}
^inv_face{S}	{
					register int _face_ = FindFace(yval(), 0);
					op->inv_face = &new_faces[_face_ ];

					if(!_face_)
					    LOG(llevMapbug,"MAPBUG: Object %s - can't find inv_face %s.\n",
							op->arch?(op->arch->name?op->arch->name:"<no arch name>"):"<no ->arch>",yval());
				}

^str{S}			op->stats.Str = IVAL;
^dex{S}			op->stats.Dex = IVAL;
^con{S}			op->stats.Con = IVAL;
^wis{S}			op->stats.Wis = IVAL;
^cha{S}			op->stats.Cha = IVAL;
^int{S}			op->stats.Int = IVAL;
^pow{S}			op->stats.Pow = IVAL;
^hp{S}			op->stats.hp = IVAL;
^maxhp{S}		op->stats.maxhp = IVAL;
^sp{S}			op->stats.sp = IVAL;
^maxsp{S}		op->stats.maxsp = IVAL;
^grace{S}		op->stats.grace = IVAL;
^maxgrace{S}	op->stats.maxgrace = IVAL;
^exp{S}			op->stats.exp = atol(yval());
^food{S}		op->stats.food = IVAL;
^dam{S}			op->stats.dam = IVAL;
^dam_add{S}		op->stats.dam += IVAL;
^wc{S}			op->stats.wc = IVAL;
^wc_add{S}		op->stats.wc += IVAL;
^ac{S}			op->stats.ac = IVAL;
^ac_add{S}		op->stats.ac += IVAL;
^thac0{S}		op->stats.thac0 = IVAL;
^thacm{S}		op->stats.thacm = IVAL;

^x{S}		{op->x = IVAL;
#ifdef POSITION_DEBUG
op->ox= op->x;
#endif
}
^y{S}		{op->y = IVAL;
#ifdef POSITION_DEBUG
op->oy= op->y;
#endif
}
^z{S}		{
#ifdef USE_TILESTRETCHER
op->z = IVAL;
#ifdef POSITION_DEBUG
op->oz= op->z;
#endif
#endif
}
^can_stack{S}				SET_OR_CLEAR_FLAG(op, FLAG_CAN_STACK, IVAL);
^nrof{S}					op->nrof= atol(yval());
^level{S}					op->level = IVAL;
^direction{S}				op->facing=op->anim_last_facing =op->direction = (IVAL%9);
^type{S}					op->type = IVAL;
^material{S}				op->material = IVAL;
^value{S}					op->value = atoll(yval());
^weight{S}					op->weight = atol(yval());
^carrying{S}				op->carrying = atol(yval());
^path_attuned{S}			op->path_attuned = IVAL;
^path_repelled{S}			op->path_repelled = IVAL;
^path_denied{S}				op->path_denied = IVAL;
^magic{S}					op->magic = IVAL;
^state{S}					op->state = IVAL;
^hidden{S}					op->hide = IVAL;
^layer{S}					op->layer = IVAL;
^ego{S}						SET_OR_CLEAR_FLAG(op, FLAG_IS_EGOITEM, IVAL);
^egobound{S}				SET_OR_CLEAR_FLAG(op, FLAG_IS_EGOBOUND, IVAL);
^egoclan{S}					SET_OR_CLEAR_FLAG(op, FLAG_IS_EGOCLAN, IVAL);
^egolock{S}					SET_OR_CLEAR_FLAG(op, FLAG_IS_EGOITEM, IVAL);
^no_inv{S}					SET_OR_CLEAR_FLAG(op, FLAG_NO_INVENTORY, IVAL);
^door_closed{S}				SET_OR_CLEAR_FLAG(op, FLAG_DOOR_CLOSED, IVAL);
^cursed_perm{S}				SET_OR_CLEAR_FLAG(op, FLAG_PERM_CURSED, IVAL);
^damned_perm{S}				SET_OR_CLEAR_FLAG(op, FLAG_PERM_DAMNED, IVAL);
^one_drop{S}				SET_OR_CLEAR_FLAG(op, FLAG_ONE_DROP, IVAL);
^is_traped{S}				SET_OR_CLEAR_FLAG(op, FLAG_IS_TRAPED, IVAL);
^quest_item{S}				SET_OR_CLEAR_FLAG(op, FLAG_QUEST_ITEM, IVAL);
^player_only{S}				SET_OR_CLEAR_FLAG(op, FLAG_PLAYER_ONLY, IVAL);
^is_named{S}				SET_OR_CLEAR_FLAG(op, FLAG_IS_NAMED, IVAL);
^is_player{S}				SET_OR_CLEAR_FLAG(op, FLAG_IS_PLAYER, IVAL);
^is_aged{S}					SET_OR_CLEAR_FLAG(op, FLAG_IS_AGED, IVAL);
^sys_object{S}				SET_OR_CLEAR_FLAG(op, FLAG_SYS_OBJECT, IVAL);
^generator{S}				SET_OR_CLEAR_FLAG(op, FLAG_GENERATOR, IVAL);
^is_thrown{S}				SET_OR_CLEAR_FLAG(op, FLAG_IS_THROWN, IVAL);
^auto_apply{S}				SET_OR_CLEAR_FLAG(op, FLAG_AUTO_APPLY, IVAL);
^treasure{S}				SET_OR_CLEAR_FLAG(op, FLAG_TREASURE, IVAL);
^was_reflected{S}			SET_OR_CLEAR_FLAG(op, FLAG_WAS_REFLECTED, IVAL);
^is_assassin{S}				SET_OR_CLEAR_FLAG(op, FLAG_IS_ASSASSINATION, IVAL);
^is_missile{S}				SET_OR_CLEAR_FLAG(op, FLAG_IS_MISSILE, IVAL);
^see_invisible{S}			SET_OR_CLEAR_FLAG(op, FLAG_SEE_INVISIBLE, IVAL);
^make_invisible{S}			SET_OR_CLEAR_FLAG(op, FLAG_MAKE_INVISIBLE, IVAL);
^make_ethereal{S}			SET_OR_CLEAR_FLAG(op, FLAG_MAKE_ETHEREAL, IVAL);
^can_roll{S} 				SET_OR_CLEAR_FLAG(op, FLAG_CAN_ROLL, IVAL);
^is_turnable{S}				SET_OR_CLEAR_FLAG(op, FLAG_IS_TURNABLE, IVAL);
^is_used_up{S}				SET_OR_CLEAR_FLAG(op, FLAG_IS_USED_UP, IVAL);
^is_invisible{S}			SET_OR_CLEAR_FLAG(op, FLAG_IS_INVISIBLE, IVAL);
^alive{S}					SET_OR_CLEAR_FLAG(op, FLAG_ALIVE, IVAL);
^applied{S}					SET_OR_CLEAR_FLAG(op, FLAG_APPLIED, IVAL);
^unpaid{S}					SET_OR_CLEAR_FLAG(op, FLAG_UNPAID, IVAL);
^no_pick{S}					SET_OR_CLEAR_FLAG(op, FLAG_NO_PICK, IVAL);
^no_pass{S}					SET_OR_CLEAR_FLAG(op, FLAG_NO_PASS, IVAL);
^no_teleport{S}				SET_OR_CLEAR_FLAG(op, FLAG_NO_TELEPORT, IVAL);
^corpse{S}					SET_OR_CLEAR_FLAG(op, FLAG_CORPSE, IVAL);
^corpse_forced{S}			SET_OR_CLEAR_FLAG(op, FLAG_CORPSE_FORCED, IVAL);

^walk_on{S}					SET_OR_CLEAR_FLAG(op, FLAG_WALK_ON, IVAL);
^walk_off{S}				SET_OR_CLEAR_FLAG(op, FLAG_WALK_OFF, IVAL);
^fly_on{S}					SET_OR_CLEAR_FLAG(op, FLAG_FLY_ON, IVAL);
^fly_off{S}					SET_OR_CLEAR_FLAG(op, FLAG_FLY_OFF, IVAL);
^is_animated{S}				{
								SET_OR_CLEAR_FLAG(op, FLAG_ANIMATE, IVAL);
								/*SET_FLAG(op,FLAG_CLIENT_SENT);*/ /* we using this flag for debugging - ignore */
							}
^flying{S}					SET_OR_CLEAR_FLAG(op, FLAG_FLYING, IVAL);
^levitate{S}				SET_OR_CLEAR_FLAG(op, FLAG_LEVITATE, IVAL);
^monster{S}					SET_OR_CLEAR_FLAG(op, FLAG_MONSTER, IVAL);
^no_attack{S}				SET_OR_CLEAR_FLAG(op, FLAG_NO_ATTACK, IVAL);
^invulnerable{S}			SET_OR_CLEAR_FLAG(op, FLAG_INVULNERABLE, IVAL);
^friendly{S}			    SET_OR_CLEAR_FLAG(op, FLAG_FRIENDLY, IVAL);
^identified{S}	    {	if (IVAL) {
			    SET_FLAG(op, FLAG_IDENTIFIED);
			    CLEAR_FLAG(op, FLAG_KNOWN_MAGICAL);
			}
			else CLEAR_FLAG(op, FLAG_IDENTIFIED);
		    }
^reflecting{S}	    SET_OR_CLEAR_FLAG(op, FLAG_REFLECTING, IVAL);
^changing{S} 	    SET_OR_CLEAR_FLAG(op, FLAG_CHANGING, IVAL);
^splitting{S}	    SET_OR_CLEAR_FLAG(op, FLAG_SPLITTING, IVAL);
^hitback{S}  	    SET_OR_CLEAR_FLAG(op, FLAG_HITBACK, IVAL);
^startequip{S}	    SET_OR_CLEAR_FLAG(op, FLAG_STARTEQUIP, IVAL);
^blocksview{S}	    SET_OR_CLEAR_FLAG(op, FLAG_BLOCKSVIEW, IVAL);
^undead{S}  	    SET_OR_CLEAR_FLAG(op, FLAG_UNDEAD, IVAL);
^scared{S}  	    SET_OR_CLEAR_FLAG(op, FLAG_SCARED, IVAL);
^surrendered{S}  	SET_OR_CLEAR_FLAG(op, FLAG_SURRENDERED, IVAL);
^unaggressive{S}    SET_OR_CLEAR_FLAG(op, FLAG_UNAGGRESSIVE, IVAL);
^reflect_missile{S} SET_OR_CLEAR_FLAG(op, FLAG_REFL_MISSILE, IVAL);
^can_reflect_missile{S} SET_OR_CLEAR_FLAG(op, FLAG_CAN_REFL_MISSILE, IVAL);
^reflect_spell{S}   SET_OR_CLEAR_FLAG(op, FLAG_REFL_SPELL, IVAL);
^can_reflect_spell{S}   SET_OR_CLEAR_FLAG(op, FLAG_CAN_REFL_SPELL, IVAL);
^use_dmg_info{S}	SET_OR_CLEAR_FLAG(op, FLAG_USE_DMG_INFO, IVAL);
^no_magic{S} 	    SET_OR_CLEAR_FLAG(op, FLAG_NO_MAGIC, IVAL);
^no_cleric{S} 	    SET_OR_CLEAR_FLAG(op, FLAG_NO_CLERIC, IVAL);
^wiz{S}  	    {	if (IVAL) {
			    SET_FLAG(op, FLAG_WIZ);
			    SET_FLAG(op, FLAG_WIZPASS);
			}
			else {
			    CLEAR_FLAG(op, FLAG_WIZ);
			    CLEAR_FLAG(op, FLAG_WIZPASS);
			}
		    }
^no_fix_player{S}   SET_OR_CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER, IVAL);
^tear_down{S}	    SET_OR_CLEAR_FLAG(op, FLAG_TEAR_DOWN, IVAL);
^run_away{S}	    op->run_away = IVAL;
^pass_thru{S}	    SET_OR_CLEAR_FLAG(op, FLAG_PASS_THRU, IVAL);
^pass_ethereal{S}	    SET_OR_CLEAR_FLAG(op, FLAG_PASS_ETHEREAL, IVAL);
^can_pass_thru{S}   SET_OR_CLEAR_FLAG(op, FLAG_CAN_PASS_THRU, IVAL);
^anim_speed{S}	    op->anim_speed = IVAL;
^container{S}	    op->weight_limit = IVAL;
^no_drop{S}	    SET_OR_CLEAR_FLAG(op, FLAG_NO_DROP, IVAL);
^is_ethereal{S}			SET_OR_CLEAR_FLAG(op, FLAG_IS_ETHEREAL, IVAL);
^can_open_door{S}	    SET_OR_CLEAR_FLAG(op, FLAG_CAN_OPEN_DOOR, IVAL);
^has_ready_bow{S}		SET_OR_CLEAR_FLAG(op, FLAG_READY_BOW, IVAL);
^has_ready_spell{S}	    SET_OR_CLEAR_FLAG(op, FLAG_READY_SPELL, IVAL);
^can_use_armour{S}	    SET_OR_CLEAR_FLAG(op, FLAG_USE_ARMOUR, IVAL);
^can_use_weapon{S}	    SET_OR_CLEAR_FLAG(op, FLAG_USE_WEAPON, IVAL);
^can_use_ring{S}	    SET_OR_CLEAR_FLAG(op, FLAG_USE_RING, IVAL);
^xrays{S}	    SET_OR_CLEAR_FLAG(op, FLAG_XRAYS, IVAL);
^no_save{S}			SET_OR_CLEAR_FLAG(op, FLAG_NO_SAVE, IVAL);
^is_male{S}		   SET_OR_CLEAR_FLAG(op, FLAG_IS_MALE, IVAL);
^is_female{S}	   SET_OR_CLEAR_FLAG(op, FLAG_IS_FEMALE, IVAL);
^is_evil{S}		   SET_OR_CLEAR_FLAG(op, FLAG_IS_EVIL, IVAL);
^is_good{S}		   SET_OR_CLEAR_FLAG(op, FLAG_IS_GOOD, IVAL);
^is_neutral{S}	   SET_OR_CLEAR_FLAG(op, FLAG_IS_NEUTRAL, IVAL);
^lifesave{S}	    SET_OR_CLEAR_FLAG(op, FLAG_LIFESAVE, IVAL);
^reg_f{S}			SET_OR_CLEAR_FLAG(op, FLAG_FIGHT_HPREG, IVAL);
^sleep{S}	    {
			SET_OR_CLEAR_FLAG(op, FLAG_SLEEP, IVAL);
			/*(LOG(llevDebug," Warning: Object %s has sleep set in arch.\n",op->name);*/
		    }
^stand_still{S}	    SET_OR_CLEAR_FLAG(op, FLAG_STAND_STILL, IVAL);
^random_move{S}	    SET_OR_CLEAR_FLAG(op, FLAG_RANDOM_MOVE, IVAL);
^only_attack{S}	    SET_OR_CLEAR_FLAG(op, FLAG_ONLY_ATTACK, IVAL);
^berserk{S}	    SET_OR_CLEAR_FLAG(op, FLAG_BERSERK, IVAL);
^is_magical{S}	    SET_OR_CLEAR_FLAG(op, FLAG_IS_MAGICAL, IVAL);

    /* Start of various attacktypes */
^attack_impact{S}	SET_ATTACK(op, ATNR_PHYSICAL, IVAL);
^attack_fire{S}		SET_ATTACK(op, ATNR_FIRE, IVAL);
^attack_electricity{S}	SET_ATTACK(op, ATNR_ELECTRICITY, IVAL);
^attack_cold{S}		SET_ATTACK(op, ATNR_COLD, IVAL);
^attack_confusion{S}	SET_ATTACK(op, ATNR_CONFUSION, IVAL);
^attack_acid{S}		SET_ATTACK(op, ATNR_ACID, IVAL);
^attack_drain{S}	SET_ATTACK(op, ATNR_DRAIN, IVAL);
^attack_weaponmagic{S}	SET_ATTACK(op, ATNR_WEAPONMAGIC, IVAL);
^attack_poison{S}	SET_ATTACK(op, ATNR_POISON, IVAL);
^attack_slow{S}		SET_ATTACK(op, ATNR_SLOW, IVAL);
^attack_paralyze{S}	SET_ATTACK(op, ATNR_PARALYZE, IVAL);
^attack_fear{S}		SET_ATTACK(op, ATNR_FEAR, IVAL);
^attack_cancellation{S}	SET_ATTACK(op, ATNR_CANCELLATION, IVAL);
^attack_depletion{S}	SET_ATTACK(op, ATNR_DEPLETION, IVAL);
^attack_death{S}	SET_ATTACK(op, ATNR_DEATH, IVAL);
^attack_chaos{S}	SET_ATTACK(op, ATNR_CHAOS, IVAL);
^attack_countermagic{S}	SET_ATTACK(op, ATNR_COUNTERMAGIC, IVAL);
^attack_godpower{S}	SET_ATTACK(op, ATNR_GODPOWER, IVAL);
^attack_internal{S}	SET_ATTACK(op, ATNR_INTERNAL, IVAL);
^attack_lifesteal{S}	SET_ATTACK(op, ATNR_LIFESTEAL, IVAL);
^attack_slash{S}	SET_ATTACK(op, ATNR_SLASH, IVAL);
^attack_cleave{S}	SET_ATTACK(op, ATNR_CLEAVE, IVAL);
^attack_pierce{S}	SET_ATTACK(op, ATNR_PIERCE, IVAL);
^attack_nether{S}	SET_ATTACK(op, ATNR_NETHER, IVAL);
^attack_sonic{S}	SET_ATTACK(op, ATNR_SONIC, IVAL);
^attack_psionic{S}	SET_ATTACK(op, ATNR_PSIONIC, IVAL);

^attack_light{S}	SET_ATTACK(op, ATNR_LIGHT, IVAL);
^attack_magic{S}	SET_ATTACK(op, ATNR_MAGIC, IVAL);
^attack_aether{S}	SET_ATTACK(op, ATNR_AETHER, IVAL);
^attack_snare{S}		SET_ATTACK(op, ATNR_SNARE, IVAL);
^attack_shadow{S}		SET_ATTACK(op, ATNR_SHADOW, IVAL);
^attack_corruption{S}	SET_ATTACK(op, ATNR_CORRUPTION, IVAL);

	/* Start of various resisttypes */
^resist_impact{S}	SET_RESIST(op, ATNR_PHYSICAL, IVAL);
^resist_fire{S}		SET_RESIST(op, ATNR_FIRE, IVAL);
^resist_electricity{S}	SET_RESIST(op, ATNR_ELECTRICITY, IVAL);
^resist_cold{S}		SET_RESIST(op, ATNR_COLD, IVAL);
^resist_confusion{S}	SET_RESIST(op, ATNR_CONFUSION, IVAL);
^resist_acid{S}		SET_RESIST(op, ATNR_ACID, IVAL);
^resist_drain{S}	SET_RESIST(op, ATNR_DRAIN, IVAL);
^resist_weaponmagic{S}	SET_RESIST(op, ATNR_WEAPONMAGIC, IVAL);
^resist_poison{S}	SET_RESIST(op, ATNR_POISON, IVAL);
^resist_slow{S}		SET_RESIST(op, ATNR_SLOW, IVAL);
^resist_paralyze{S}	SET_RESIST(op, ATNR_PARALYZE, IVAL);
^resist_fear{S}		SET_RESIST(op, ATNR_FEAR, IVAL);
^resist_cancellation{S}	SET_RESIST(op, ATNR_CANCELLATION, IVAL);
^resist_depletion{S}	SET_RESIST(op, ATNR_DEPLETION, IVAL);
^resist_death{S}	SET_RESIST(op, ATNR_DEATH, IVAL);
^resist_chaos{S}	SET_RESIST(op, ATNR_CHAOS, IVAL);
^resist_countermagic{S}	SET_RESIST(op, ATNR_COUNTERMAGIC, IVAL);
^resist_godpower{S}	SET_RESIST(op, ATNR_GODPOWER, IVAL);
^resist_internal{S}	SET_RESIST(op, ATNR_INTERNAL, IVAL);
^resist_lifesteal{S}	SET_RESIST(op, ATNR_LIFESTEAL, IVAL);
^resist_slash{S}	SET_RESIST(op, ATNR_SLASH, IVAL);
^resist_cleave{S}	SET_RESIST(op, ATNR_CLEAVE, IVAL);
^resist_pierce{S}	SET_RESIST(op, ATNR_PIERCE, IVAL);
^resist_nether{S}	SET_RESIST(op, ATNR_NETHER, IVAL);
^resist_sonic{S}	SET_RESIST(op, ATNR_SONIC, IVAL);
^resist_psionic{S}	SET_RESIST(op, ATNR_PSIONIC, IVAL);

^resist_magic{S}	SET_RESIST(op, ATNR_MAGIC, IVAL);
^resist_aether{S}	SET_RESIST(op, ATNR_AETHER, IVAL);
^resist_light{S}	SET_RESIST(op, ATNR_LIGHT, IVAL);
^resist_shadow{S}	SET_RESIST(op, ATNR_SHADOW, IVAL);
^resist_snare{S}		SET_RESIST(op, ATNR_SNARE, IVAL);
^resist_corruption{S}	SET_RESIST(op, ATNR_CORRUPTION, IVAL);


	/* outdate beta 3 attack/protection/resists  WE HOTMAP THEM TO NEW ONES HERE - remove this for 1.0 */
^protect_impact{S}	SET_RESIST(op, ATNR_PHYSICAL, IVAL);
^protect_slash{S}	SET_RESIST(op, ATNR_SLASH, IVAL);
^protect_cleave{S}	SET_RESIST(op, ATNR_CLEAVE, IVAL);
^protect_pierce{S}	SET_RESIST(op, ATNR_PIERCE, IVAL);
^protect_fire{S}		SET_RESIST(op, ATNR_FIRE, IVAL);
^protect_cold{S}		SET_RESIST(op, ATNR_COLD, IVAL);
^protect_electricity{S}	SET_RESIST(op, ATNR_ELECTRICITY, IVAL);
^protect_poison{S}		SET_RESIST(op, ATNR_POISON, IVAL);
^protect_acid{S}		SET_RESIST(op, ATNR_ACID, IVAL);
^protect_sonic{S}		SET_RESIST(op, ATNR_SONIC, IVAL);
^protect_psionic{S}		SET_RESIST(op, ATNR_PSIONIC, IVAL);
^protect_force{S}		SET_RESIST(op, ATNR_MAGIC, IVAL);
^protect_death{S}		SET_RESIST(op, ATNR_DEATH, IVAL);
^protect_light{S}		SET_RESIST(op, ATNR_LIGHT, IVAL);
^protect_shadow{S}		SET_RESIST(op, ATNR_SHADOW, IVAL);
^protect_aether{S}		SET_RESIST(op, ATNR_AETHER, IVAL);
^protect_nether{S}		SET_RESIST(op, ATNR_NETHER, IVAL);
^protect_chaos{S}		SET_RESIST(op, ATNR_CHAOS, IVAL);
^protect_lifesteal{S}	SET_RESIST(op, ATNR_LIFESTEAL, IVAL);
^protect_weaponmagic{S}	SET_RESIST(op, ATNR_WEAPONMAGIC, IVAL);
^protect_godpower{S}	SET_RESIST(op, ATNR_GODPOWER, IVAL);

^protect_magic{S}		SET_RESIST(op, ATNR_SONIC, IVAL); /* old: PROTECT_MAGIC */
^protect_mind{S}		SET_RESIST(op, ATNR_LIGHT, IVAL); /* old: PROTECT_MIND */
^protect_body{S}		SET_RESIST(op, ATNR_SHADOW, IVAL); /* old: PROTECT_BODY */
^protect_spiritual{S}   SET_RESIST(op, ATNR_LIFESTEAL, IVAL); /* old: PROTECT_SPIRITUAL */
^protect_corruption{S}  SET_RESIST(op, ATNR_AETHER, IVAL);/* old: PROTECT_CORRUPTION */
^resist_force{S}	SET_RESIST(op, ATNR_MAGIC, IVAL);
^resist_time{S}		SET_RESIST(op, ATNR_SHADOW, IVAL);
^resist_demonic{S}	SET_RESIST(op, ATNR_AETHER, IVAL);
^resist_blind{S}	SET_RESIST(op, ATNR_LIGHT, IVAL);
^resist_purity{S}	SET_RESIST(op, ATNR_CORRUPTION, IVAL);
^resist_ghosthit{S}	SET_RESIST(op, ATNR_SNARE, IVAL);
^attack_blind{S}	SET_ATTACK(op, ATNR_LIGHT, IVAL);
^attack_force{S}	SET_ATTACK(op, ATNR_MAGIC, IVAL);
^attack_demonic{S}	SET_ATTACK(op, ATNR_AETHER, IVAL);
^attack_purity{S}	SET_ATTACK(op, ATNR_CORRUPTION, IVAL);
^attack_time{S}		SET_ATTACK(op, ATNR_SHADOW, IVAL);
^attack_ghosthit{S}	SET_ATTACK(op, ATNR_SNARE, IVAL);
^resist_life_steal{S}	SET_RESIST(op, ATNR_LIFESTEAL, IVAL);
^attack_life_steal{S}	SET_ATTACK(op, ATNR_LIFESTEAL, IVAL);
^resist_counterspell{S}	SET_RESIST(op, ATNR_COUNTERMAGIC, IVAL);
^attack_counterspell{S}	SET_ATTACK(op, ATNR_COUNTERMAGIC, IVAL);
^protect_weapon_magic{S}	SET_RESIST(op, ATNR_WEAPONMAGIC, IVAL);
^is_vul_elemental{S}	{};
^is_vul_magic{S}	{};
^is_vul_physical{S}	{};
^is_vul_sphere{S}	{};
^is_indestructible{S}	{};
^unique{S}			{};
^attacktype{S}			{};
^wc_range{S}			{};
^has_ready_skill{S}			{};
	/* end outdate */

^attack_movement{S} LOG(llevMapbug, "MAPBUG: loader(): deprecated attack_movement setting. op=%s\n", STRING_OBJ_NAME(op));
^move_state{S}	    LOG(llevMapbug, "MAPBUG: loader(): deprecated move_state setting. op=%s\n", STRING_OBJ_NAME(op));
^confused{S}	    SET_OR_CLEAR_FLAG(op, FLAG_CONFUSED, IVAL);
^paralyzed{S}	    SET_OR_CLEAR_FLAG(op, FLAG_PARALYZED, IVAL);
^feared{S}			SET_OR_CLEAR_FLAG(op, FLAG_FEARED, IVAL);
^rooted{S}			SET_OR_CLEAR_FLAG(op, FLAG_ROOTED, IVAL);
^slowed{S}			SET_OR_CLEAR_FLAG(op, FLAG_SLOWED, IVAL);
^stealth{S}			SET_OR_CLEAR_FLAG(op, FLAG_STEALTH, IVAL);
^connected{S}	    add_button_links(op, op->map, yval());
^cursed{S}	    SET_OR_CLEAR_FLAG(op, FLAG_CURSED, IVAL);
^damned{S}	    SET_OR_CLEAR_FLAG(op, FLAG_DAMNED, IVAL);
^see_anywhere{S}    SET_OR_CLEAR_FLAG(op, FLAG_SEE_ANYWHERE, IVAL);
^known_magical{S}   SET_OR_CLEAR_FLAG(op, FLAG_KNOWN_MAGICAL, IVAL);
^known_cursed{S}    SET_OR_CLEAR_FLAG(op, FLAG_KNOWN_CURSED, IVAL);
^been_applied{S}    SET_OR_CLEAR_FLAG(op, FLAG_BEEN_APPLIED, IVAL);
^title{S}	    {	char *y=yval();
			if (*y=='\0') LOG(llevMapbug,"MAPBUG: loader(): Title without value. op=%s\n", STRING_OBJ_NAME(op));
			else FREE_AND_COPY_HASH(op->title, y);
		    }
^inv_locked{S}		SET_OR_CLEAR_FLAG(op, FLAG_INV_LOCKED, IVAL);
^is_wooded{S}		SET_OR_CLEAR_FLAG(op, FLAG_IS_WOODED, IVAL);
^is_hilly{S}		SET_OR_CLEAR_FLAG(op, FLAG_IS_HILLY, IVAL);
^has_ready_weapon{S}	SET_OR_CLEAR_FLAG(op, FLAG_READY_WEAPON, IVAL); /* internal player only flag */
^no_skill_ident{S}	SET_OR_CLEAR_FLAG(op, FLAG_NO_SKILL_IDENT, IVAL);
^glow_radius{S}		op->glow_radius = IVAL;
^is_blind{S}		SET_OR_CLEAR_FLAG(op, FLAG_BLIND, IVAL);
^can_see_in_dark{S}	SET_OR_CLEAR_FLAG(op, FLAG_SEE_IN_DARK, IVAL);
^is_cauldron{S}		SET_OR_CLEAR_FLAG(op, FLAG_IS_CAULDRON, IVAL);
^randomitems{S}		op->randomitems = link_treasurelists(yval(), OBJLNK_FLAG_REF);
^is_dust{S}		SET_OR_CLEAR_FLAG(op, FLAG_DUST, IVAL);
^no_steal{S}		SET_OR_CLEAR_FLAG(op, FLAG_NO_STEAL, IVAL);
^one_hit{S}		SET_OR_CLEAR_FLAG(op, FLAG_ONE_HIT, IVAL);

^proof_phy{S}		SET_OR_CLEAR_FLAG(op, FLAG_PROOF_PHYSICAL, IVAL);
^proof_ele{S}		SET_OR_CLEAR_FLAG(op, FLAG_PROOF_ELEMENTAL, IVAL);
^proof_mag{S}		SET_OR_CLEAR_FLAG(op, FLAG_PROOF_MAGICAL, IVAL);
^proof_sph{S}		SET_OR_CLEAR_FLAG(op, FLAG_PROOF_SPHERICAL, IVAL);


^editor_folder{S}	{ }
^sub_type{S}		op->sub_type1 = IVAL;
^terrain_flag{S}	op->terrain_flag = IVAL;
^terrain_type{S}	op->terrain_type = IVAL;
^item_quality{S}	op->item_quality = IVAL;
^item_condition{S}	op->item_condition = IVAL;
^item_race{S}		op->item_race = IVAL;
^item_skill{S}		op->item_skill = IVAL;
^item_level{S}		op->item_level = IVAL;
^item_level_art{S}	{ /* artifact list token - NEVER USE IN REAL ARCHES OR MAPS */
					int ival_tmp = IVAL;
				    /* when we have a artifact which has a min. use level of 10
					 * but we give it a mithril armour of default use level 50,
					 * we will create artifact mithril armour usable for level 10!
					 * this command prevent it
					 */
					if(ival_tmp <=0 ) /* this is forcing the value! */
						op->item_level = -ival_tmp;
					else if(ival_tmp >op->item_level)
						op->item_level = ival_tmp;
					}
^material_real{S}	op->material_real = IVAL;
^mpart_id{S}		op->quick_pos |= (((char)IVAL)<<4);
^mpart_nr{S}		op->quick_pos |= (char)IVAL;
^weapon_speed{S}	{
						op->weapon_speed = FVAL;
						op->weapon_speed_left = 0;
					}


<*>(^{WS})|{NL}	{/* ignore empty lines, newlines we don't do above */}
#.*{NL}			{}

<<EOF>>			{/* If we got an error, return the error.  Otherwise, return that we got EOF */
			    if (lex_error!=0) return lex_error; else return LL_EOF;}
.*			{ yyerror( "MAPBUG: loader(): Unrecognized string"); lex_error= -1; }
%%


int yyerror(char *s)
{
  LOG(llevMapbug, "%s: %s\n", s, yytext);
  return -1;
}


/* Our save file syntax is very simple, so we can use a very simple
 * processing mechanism here instead using something like bison
 * This skips over the space and returns the value, or "" if no value
 * is found.  Modified 4/26/2000 to also strip spaces at end of
 * line
 */
static char *yval()
{
    static char *em="";
    char *cp,*end;

    /* First skip over start of line, like animation or name */
   for (cp=yytext; *cp!=' '; cp++) {
	if (*cp=='\0') {
	    return em;
	}
   }

    /* Skip over whitespace */
    for (; *cp==' '; cp++) {
	if (*cp=='\0') {
	    return em;
	}
    }
    /* Got last character before null and strip
     * off tailing whitespace
     */
    for (end=cp+strlen(cp)-1; *end==' ' || *end == 0x0A || *end==0x0D; end--) {
	if (end==cp) return em;
	*end='\0';
    }
    return cp;
}

/* buffer functions - needed to make load_object()
 * recursive save.
 */
void delete_loader_buffer(void *buffer)
{
	if(buffer)
		yy_delete_buffer(buffer);
	else
		yy_delete_buffer(YY_CURRENT_BUFFER);
	cur_buffer = NULL;
}

void *create_loader_buffer(void *fp)
{
	cur_buffer = yy_create_buffer(fp, YY_BUF_SIZE);
	yy_switch_to_buffer(cur_buffer);
	return cur_buffer;
}


/*
 * Loads an object from the given file-pointer.
 * Variables will be read and parsed and patched into the object
 * until the string "end" is reached, or the end of the file.
 *
 * bufstat is used to determine various file attributes:
 *  LO_REPEAT (0): We are reading from the same buffer as the last call.
 *  LO_LINEMODE (1): file that is being read from is multi purpose (ie, other functions
 *	will also be reading from this (treasure file, artifacts.)
 *  LO_NEWFILE (2): disabled
 *  LO_NOREAD (3): disabled
 *  LO_MEMORYMODE (4): read from a memory buffer instead of a file.
 */
/* load_object() is now recursive safe - this kind of call can happens when a multi
 * arch is expanded after loading in a map and out_of_map() triggers a map loading for
 * parts in a different map. Because we don't need any order of objects when we load a map,
 * this will work fine.
 * Note: load_object can be called with fp = FILE * and fp = char * - so we have here void used
 */
int load_object(void *fp, object *op, void *mybuffer, int bufstate, int map_flags) {
    int retval = 0;
	float old_speed, old_sleft;
    char inbuf[MAX_BUF];

	/* note: at this point we have perhaps a "arch" parse or a "object"
	 * parse - we don't know. "arch" parse means, arches are loaded - we
	 * hit a arch command and pre-load our object data - then we go on and
	 * parse the difference in. If we hit a "object" command, we had outside
	 * this function pre-loaded the arch. in the first case, the next 2
	 * values are undefined - in the last case, these values are the arch
	 * pre sets. We need to track this down to adjust for the "real ingame"
	 * object the right speed & speed_left.
	 */
	singularity = NULL;
	amask_art = NULL;
	old_speed=op->speed;
	old_sleft=op->speed_left;
	object_load_flag = 0; /* we assume a "arch" load as default */

	*msgbuf='\0';

    if (bufstate==LO_REPEAT)
	{
		if(cur_buffer != mybuffer) /* be sure we work on the right buffer */
		{
			cur_buffer = mybuffer;
			yy_switch_to_buffer(mybuffer);
		}
	    retval=lex_load(op, map_flags);
	}
    else if (bufstate==LO_MEMORYMODE)
	{
		char *strptr = (char*)fp;
		YY_BUFFER_STATE  yybufstate;
		int lcount=0;
		object_load_flag = 1; /* we are object load but object is not called */
		while (1) /* we can be sure this will work - InitArtifact has tested this before */
		{
			strptr +=lcount;
			lcount = strlen(strptr)+1; /*we can even put this value at string start to save the strlen */
			yybufstate=yy_scan_string(strptr);
			retval=lex_load(op, map_flags); /* hopefully we have here a "end" - or we stay in this while a bit */
			yy_delete_buffer(yybufstate);
			if (retval==LL_NORMAL)
				break;
		}
		if(cur_buffer) /* be sure we work on the right buffer */
			yy_switch_to_buffer(cur_buffer);
    }
    else /* if (bufstate==LO_LINEMODE) */
	{
		YY_BUFFER_STATE  yybufstate;
		while (fgets(inbuf, MAX_BUF-3, (FILE *)fp))
		{
			yybufstate=yy_scan_string(inbuf);
			retval=lex_load(op, map_flags);
			yy_delete_buffer(yybufstate);
			if (retval==LL_NORMAL)
			{
				if(cur_buffer) /* be sure we work on the right buffer */
					yy_switch_to_buffer(cur_buffer);
				return retval;
			}
		}
		LOG(llevMapbug,"MAPBUG: Got eof while scanning strings (%d)\n",retval);
		if(cur_buffer) /* be sure we work on the right buffer */
			yy_switch_to_buffer(cur_buffer);
		return LL_EOF;
    }

	if(singularity) /* we have a singularity - create one (new NEED the op->arch set after load_object()! */
	{
		singularity->x = op->x; /* we want them on the right map spot so we can watch them in DM mode */
		singularity->y = op->y;
		copy_object_data(singularity,op); /* yep, this is right - op is a reference from outside */
	}

	if(QUERY_FLAG(op,FLAG_IDENTIFIED))
	{
		SET_FLAG(op,FLAG_KNOWN_MAGICAL);
		SET_FLAG(op,FLAG_KNOWN_CURSED);
	}
  /* we set here the material real values.
   * we do: If item_quality == 0 AND material defines a base
   * material, then use material_real to set quality & condition.
   * So, we don't set it for floor or walls or some else objects
   * were we don't need it.
   */
  if(op->material && !op->item_quality && op->material_real >= 0)
  {
	  op->item_quality = material_real[op->material_real].quality;
	  op->item_condition = op->item_quality;
  }

	/* ok... now we have our old speed values and our new ones.
	 */
	if (!(map_flags & MAP_STATUS_STYLE))
	{
	  /* if set, our old speed values comes from a valid source (arches?) outside. */
	  if(object_load_flag)
	  {
			/* ok - ONLY change something when we
			 * a.) don't have fix set speed_left in the loading
			 * b.) we have negative speed
			 */
			if(old_sleft==op->speed_left && op->speed<0.0f)
			{
				/* if we have now a negative speed - use default arch
				 * speed left for new random set
				 */
				if(op->speed<0.0f)
					op->speed_left=op->speed_left+(float)(RANDOM()%90)/100.0f;
			}
	  }
	  else /* arch load... perhaps a map */
	  {
	    /* only do this when we have not loaded a fix speed_left and speed is <0 */
		if(op->speed<0.0f && op->speed_left==op->arch->clone.speed_left)
			op->speed_left=op->speed_left+(float)(RANDOM()%90)/100.0f;
	  }

	  if(amask_art)
		give_artifact_abilities(op, amask_art);

	  update_ob_speed(op); /* now we have build a new object - and its not arch loading
	                        * nor artifact list or style map. (MAP_STYLE is used to exlude them)
							*/

      if(object_initializers[op->type] != NULL)
          object_initializers[op->type](op);
	}


    /*LOG(llevDebug," load completed, object=%s %s (%d - %d)\n",op->name,op->title, op->item_quality, op->item_condition);*/
    return retval;
}


/* This takes a buffer, scans it for variables, and sets those variables
 * as appropriate in op.
 *
 * This function appears to be used in only 2 places - in crossedit to
 * override values and in c_wiz to mutate values.
 */
int set_variable(object *op,char *buf) {
    YY_BUFFER_STATE  yybufstate,yycurbuf=YY_CURRENT_BUFFER;
    int retval;

    strcpy(msgbuf, "");
    yy_push_state(INITIAL);
    yybufstate=yy_scan_string(buf);
    retval=lex_load(op,0);
	if(yycurbuf)
    	yy_switch_to_buffer(yycurbuf);
    yy_delete_buffer(yybufstate);
    yy_pop_state();
    return retval;
}


/* This is a list of pointers taht correspond to the FLAG_.. values.
 * This is a simple 1:1 mapping - if FLAG_FRIENDLY is 15, then
 * the 15'th element of this array should match that name.
 * If an entry is NULL, that is a flag not to loaded/saved.
 */
static const char *flag_names[NUM_FLAGS+1] = {
"sleep", "confused", "paralyzed", "scared", NULL /*eating flag */,"is_invisible" , "is_ethereal" ,
"is_good", "no_pick", "walk_on", "no_pass",		/* 10 */
"is_animated", NULL /*free*/, "flying", "monster", "friendly",
NULL /*is_removed*/,"been_applied", NULL /* internal flag: HAS_MOVED */, "treasure", "is_neutral",	/* 20 */
"see_invisible", "can_roll", "generator", "is_turnable", "walk_off",
"fly_on", "fly_off", "is_used_up", "identified", "reflecting",	/* 30 */
"changing", "splitting", "hitback", "startequip",
"blocksview", "undead", NULL /*fix_player*/, "unaggressive",
"reflect_missile", "reflect_spell",				/* 40 */
"no_magic", "no_fix_player", "is_evil", "tear_down", "run_away",
"pass_thru", "can_pass_thru", "feared", "is_blind", "no_drop",	/* 50 */
 "reg_f", "has_ready_spell", "surrendered", "rooted",
 "slowed", "can_use_armour", "can_use_weapon",
"can_use_ring", NULL /* in_activlist*/, "has_ready_bow",		/* 60 */
"xrays", NULL /*no_apply*/, "can_stack", "lifesave", "is_magical", "alive",
"stand_still", "random_move", "only_attack", "wiz",	/* 70 */
 "stealth", NULL /*wizpass*/, NULL /*linked*/, "cursed", "damned",
"see_anywhere", "known_magical", "known_cursed", "can_open_door", "is_thrown",				/* 80 */
NULL /*free*/, NULL /*free*/, "is_male",
"is_female", "applied",  "inv_locked", "is_wooded",
"is_hilly", "levitate", "has_ready_weapon",		/* 90 */
"no_skill_ident","use_dmg_info", "can_see_in_dark", "is_cauldron",
"is_dust", "no_steal", "one_hit", NULL /*client_sent*/, "berserk", "no_attack",	/* 100 */
"invulnerable", "quest_item", "is_traped", "proof_phy",  "proof_ele", /* 105 */
"proof_mag", "proof_sph", "no_inv", NULL /*free*/, "sys_object", /* 110 */
NULL /*free*/,"unpaid","is_aged","make_invisible" , "make_ethereal","is_player",
"is_named",NULL /* spawn mob flag */, "no_teleport", "corpse", "corpse_forced",
"player_only", "no_cleric", "one_drop", "cursed_perm", "damned_perm", "door_closed",
"was_reflected", "is_missile","can_reflect_missile","can_reflect_spell","is_assassin",
"auto_apply", "no_save", "pass_ethereal",
"ego", "egobound", "egoclan", "egolock"
};



void save_double(char *buf,char *name,double v)
{
  char tbuf[200];

  sprintf(tbuf,"%s %f\n",name,v);
  strcat(buf,tbuf);
}

/*
 * Initialises the array of variable-names.  Needed before any
 * objects can be loaded.  Called by init_library().
 */

void init_vars() {
}

/* Append a single-line string like name, title, race or slaying.
 * Makes sure no newlines makes it into the destination buffer.
 * Returns the new end of the destination buffer */
static inline char *append_line(char *buf, const char *str)
{
    char c;
    while((c = *buf++ = *str++))
        if(c == '\n')
        {
            LOG(llevBug, "BUG: Detected (and replaced) '\\n' in basic object string while saving.\n");
            buf[-1] = ' ';
        }
    return buf-1;
}

/* Append a multi-line string like message.
 * Newlines are allowed, but not stand-alone "endmsg" strings.
 * Returns the new end of the destination buffer */
static inline char *append_multiline(char *buf, const char *str)
{
    char c;
    char first = *str;

    while((c = *buf++ = *str++))
    {
        if(c == '\n')
        {
            while(strncasecmp(str, "endmsg", 6) == 0)
            {
                LOG(llevBug, "BUG: Detected 'endmsg' in msg string.\n");
                str += 6; /* Just skip it... */
            }
        }
    }

    /* Mantis bug #000050 */
    /* Ensure there's a newline before endmsg */
    if(first == '\0' || buf[-2] != '\n') {
        buf[-1] = '\n'; /* Replace old null */
        *buf++ = '\0';
    }

    return buf-1;
}

/* Append any null-terminated string and return the new end of buf */
static inline char *append_simple(char *buf, const char *str)
{
    while((*buf++ = *str++))
        ;
    return buf-1;
}

/*
 * Returns a pointer to a static string which contains all variables
 * which are different in the two given objects.  op is the what object
 * the different values will be taken from.  This function is
 * typically used to dump objects (op2=empty object), or to save objects
 * (op2 is the objects original archetype)
 */

char *get_ob_diff(const object *op, const object *op2) {
    /* I plan to optimize this heavily */
    /* TODO: One good optimization would be to keep track of the end of buf
     * instead of using strcat() which must find the end at every call
     * /Gecko
     */
  static char buf2[HUGE_BUF];
  static char buf[HUGE_BUF * 6]; /* There can be up to 5 HUGE_BUF long strings AFAIK */
  char *buf_end;
  int tmp;

  buf[0]='\0';
  buf_end = buf;
  if(op->name && op->name!=op2->name)
    buf_end = append_simple(append_line(append_simple(buf_end, "name "), op->name), "\n");
  if(op->title && op->name!=op2->title)
    buf_end = append_simple(append_line(append_simple(buf_end, "title "), op->title), "\n");
  if(op->race && op->race!=op2->race)
    buf_end = append_simple(append_line(append_simple(buf_end, "race "), op->race), "\n");
  if(op->slaying && op->slaying!=op2->slaying)
    buf_end = append_simple(append_line(append_simple(buf_end, "slaying "), op->slaying), "\n");
  if(op->msg && op->msg!=op2->msg)
    buf_end = append_simple(append_multiline(append_simple(buf_end,"msg\n"), op->msg), "endmsg\n");
  if(op->other_arch!=op2->other_arch&&op->other_arch!=NULL &&
     op->other_arch->name) {
    sprintf(buf2,"other_arch %s\n",op->other_arch->name);
    strcat(buf,buf2);
  }
  if(op->face!=op2->face) {
      sprintf(buf2,"face %s\n", op->face->name);
      strcat(buf,buf2);
  }
  if(op->inv_face!=op2->inv_face) {
      sprintf(buf2,"inv_face %s\n", op->inv_face->name);
      strcat(buf,buf2);
  }
  if (op->animation_id != op2->animation_id) {
    if (op->animation_id)
      sprintf(buf2,"animation %s\n", animations[GET_ANIM_ID(op)].name);
    else
      sprintf (buf2, "animation NONE\n");
    strcat (buf, buf2);
  }

  if (op->inv_animation_id != op2->inv_animation_id) {
    if (op->inv_animation_id)
      sprintf(buf2,"inv_animation %s\n", animations[GET_INV_ANIM_ID(op)].name);
    else
      sprintf (buf2, "inv_animation NONE\n");
    strcat (buf, buf2);

  }

  if(op->stats.Str!=op2->stats.Str)
    save_long(buf, "str", op->stats.Str);
  if(op->stats.Dex!=op2->stats.Dex)
    save_long(buf, "dex", op->stats.Dex);
  if(op->stats.Con!=op2->stats.Con)
    save_long(buf, "con", op->stats.Con);
  if(op->stats.Wis!=op2->stats.Wis)
    save_long(buf, "wis", op->stats.Wis);
  if(op->stats.Pow!=op2->stats.Pow)
    save_long(buf, "pow", op->stats.Pow);
  if(op->stats.Cha!=op2->stats.Cha)
    save_long(buf, "cha", op->stats.Cha);
  if(op->stats.Int!=op2->stats.Int)
    save_long(buf, "int", op->stats.Int);
  if(op->stats.hp!=op2->stats.hp)
    save_long(buf, "hp", op->stats.hp);
  if(op->stats.maxhp!=op2->stats.maxhp)
    save_long(buf, "maxhp", op->stats.maxhp);
  if(op->stats.sp!=op2->stats.sp)
    save_long(buf, "sp", op->stats.sp);
  if(op->stats.maxsp!=op2->stats.maxsp)
    save_long(buf, "maxsp", op->stats.maxsp);
  if(op->stats.grace!=op2->stats.grace)
    save_long(buf, "grace", op->stats.grace);
  if(op->stats.maxgrace!=op2->stats.maxgrace)
    save_long(buf, "maxgrace", op->stats.maxgrace);
  if(op->stats.exp!=op2->stats.exp)
    save_long(buf, "exp", op->stats.exp);
  if(op->stats.food!=op2->stats.food)
    save_long(buf, "food", op->stats.food);
  if(op->stats.dam!=op2->stats.dam)
    save_long(buf, "dam", op->stats.dam);
  if(op->stats.wc!=op2->stats.wc)
    save_long(buf, "wc", op->stats.wc);
  if(op->stats.ac!=op2->stats.ac)
    save_long(buf, "ac", op->stats.ac);
  if(op->stats.thac0!=op2->stats.thac0)
    save_long(buf, "thac0", op->stats.thac0);
  if(op->stats.thacm!=op2->stats.thacm)
    save_long(buf, "thacm", op->stats.thacm);
  if(op->x!=op2->x)
    save_long(buf, "x", op->x);
  if(op->y!=op2->y)
    save_long(buf, "y", op->y);
#ifdef USE_TILESTRETCHER
  if(op->z!=op2->z)
    save_long(buf, "z", op->z);
#endif

  if(op->speed!=op2->speed) {
    sprintf(buf2,"speed %f\n",op->speed);
    strcat(buf,buf2);
  }
  if(op->speed_left!=op2->speed_left || op->speed_left) {
    sprintf(buf2,"speed_left %f\n",op->speed_left);
    strcat(buf,buf2);
  }

  if(op->material_real!=op2->material_real) {
    sprintf(buf2,"material_real %d\n",op->material_real);
    strcat(buf,buf2);
  }
  if(op->sub_type1!=op2->sub_type1) {
    sprintf(buf2,"sub_type %d\n",op->sub_type1);
    strcat(buf,buf2);
  }
  if(op->terrain_flag!=op2->terrain_flag) {
    sprintf(buf2,"terrain_flag %d\n",op->terrain_flag);
    strcat(buf,buf2);
  }
  if(op->terrain_type!=op2->terrain_type) {
    sprintf(buf2,"terrain_type %d\n",op->terrain_type);
    strcat(buf,buf2);
  }
  if(op->item_quality!=op2->item_quality) {
    sprintf(buf2,"item_quality %d\n",op->item_quality);
    strcat(buf,buf2);
  }
  if(op->item_condition!=op2->item_condition) {
    sprintf(buf2,"item_condition %d\n",op->item_condition);
    strcat(buf,buf2);
  }
  if(op->item_race!=op2->item_race) {
    sprintf(buf2,"item_race %d\n",op->item_race);
    strcat(buf,buf2);
  }
  if(op->item_skill!=op2->item_skill) {
    sprintf(buf2,"item_skill %d\n",op->item_skill);
    strcat(buf,buf2);
  }
  if(op->item_level!=op2->item_level) {
    sprintf(buf2,"item_level %d\n",op->item_level);
    strcat(buf,buf2);
  }
  if(op->weapon_speed!=op2->weapon_speed) {
    sprintf(buf2,"weapon_speed %f\n",op->weapon_speed);
    strcat(buf,buf2);
  }
  if(op->enemy_count!=op2->enemy_count) {
    sprintf(buf2,"object_int1 %d\n",op->enemy_count);
    strcat(buf,buf2);
  }
  if(op->attacked_by_count !=op2->attacked_by_count) {
    sprintf(buf2,"object_int2 %d\n",op->attacked_by_count);
    strcat(buf,buf2);
  }

  if(op->value!=op2->value)
  {
#ifndef WIN32
		sprintf(buf2,"value %lld\n", op->value);
#else
		sprintf(buf2,"value %I64d\n", op->value);
#endif

		strcat(buf,buf2);
	}

  if(op->owner_count !=op2->owner_count) {
    sprintf(buf2,"object_int3 %d\n",op->owner_count);
    strcat(buf,buf2);
  }

  if(op->nrof!=op2->nrof)
    save_long(buf, "nrof", op->nrof);
  if(op->level!=op2->level)
    save_long(buf, "level", op->level);
  if(op->direction!=op2->direction)
    save_long(buf, "direction", op->direction);
  if(op->type!=op2->type)
    save_long(buf, "type", op->type);

  for (tmp=0; tmp < NROFATTACKS; tmp++) {
   if (op->resist[tmp] != op2->resist[tmp]) {
	sprintf(buf2,"resist_%s %d\n",attack_name[tmp], op->resist[tmp]);
	strcat(buf,buf2);
    }
  }
  /* resist_save[] holds the attack form also for attack_xx */
  for (tmp=0; tmp < NROFATTACKS; tmp++) {
   if (op->attack[tmp] != op2->attack[tmp]) {
	sprintf(buf2,"attack_%s %d\n",attack_name[tmp], op->attack[tmp]);
	strcat(buf,buf2);
    }
  }

  if(op->layer!=op2->layer)
    save_long(buf, "layer", op->layer);
  if(op->hide!=op2->hide)
    save_long(buf, "hidden", op->hide);
  if(op->path_attuned!=op2->path_attuned)
    save_long(buf, "path_attuned", op->path_attuned);
  if(op->path_repelled!=op2->path_repelled)
    save_long(buf, "path_repelled", op->path_repelled);
  if(op->path_denied!=op2->path_denied)
    save_long(buf, "path_denied", op->path_denied);
  if(op->material!=op2->material)
    save_long(buf, "material", op->material);
  if(op->carrying!=op2->carrying)
    save_long(buf, "carrying", op->carrying);
  if(op->weight!=op2->weight)
    save_long(buf, "weight", op->weight);
  if(op->state!=op2->state)
    save_long(buf, "state", op->state);
  if(op->magic!=op2->magic)
    save_long(buf, "magic", op->magic);
  if(op->last_heal!=op2->last_heal)
    save_long(buf, "last_heal", op->last_heal);
  if(op->last_sp!=op2->last_sp)
    save_long(buf, "last_sp", op->last_sp);
  if(op->last_grace!=op2->last_grace)
    save_long(buf, "last_grace", op->last_grace);
  if(op->last_eat!=op2->last_eat)
    save_long(buf, "last_eat", op->last_eat);
  if(QUERY_FLAG(op,FLAG_IS_LINKED) && op->map)
  {
      /* Generate comma separated list of links */
      oblinkpt   *obp;
      objectlink *ol;
      int foundone = 0;

      strcpy(buf2, "connected ");
      for (obp = op->map->buttons; obp; obp = obp->next)
          for (ol = obp->objlink.link; ol; ol = ol->next)
              if (ol->objlink.ob == op && ol->id == op->count)
              {
                  sprintf(buf2 + strlen(buf2), "%s%d", foundone?",":"", obp->value);
                  foundone = 1;
              }

      strcat(buf,buf2);
      strcat(buf,"\n");
  }
  if(op->glow_radius!=op2->glow_radius)
    save_long(buf, "glow_radius", op->glow_radius);

  if (op->randomitems!=op2->randomitems)
   {
		strcpy(buf2, "randomitems ");

		/* special case: randomitems was set to null by "none" */
		if(!op->randomitems)
		    strcat(buf2, "none\n");
		else /* we have different lists - generate the "list1;list2;list3" string */
		{
			struct oblnk *ol = op->randomitems;
			if(ol->parmlink.tl_tweak) /* tlist with or without '&' parameter list* */
				strcat(buf2,ol->parmlink.tl_tweak->name); /* save name and '&' tail */
			else
				strcat(buf2,ol->objlink.tl->listname); /* clean default list name */
			ol=ol->next;

			for(;ol;ol=ol->next)
			{
				strcat(buf2,";");
				if(ol->parmlink.tl_tweak)
					strcat(buf2,ol->parmlink.tl_tweak->name);
				else
					strcat(buf2,ol->objlink.tl->listname);
			}

			strcat(buf,buf2);
			strcat(buf,"\n");
		}
  }

  if(op->run_away!=op2->run_away)
    save_long(buf, "run_away", op->run_away);
  if(op->weight_limit!=op2->weight_limit)
    save_long(buf, "container", op->weight_limit);

  for (tmp=0; tmp <= NUM_FLAGS; tmp++) {
    if (flag_names[tmp] && (QUERY_FLAG(op, tmp) != QUERY_FLAG(op2, tmp)))
	 {
		if(QUERY_FLAG(op, tmp))
			sprintf(buf2, "%s 1\n", flag_names[tmp]);
		else
			sprintf(buf2, "%s 0\n", flag_names[tmp]);

		strcat(buf, buf2);

    }
  }

  if(buf[0]=='\0')
    return NULL;
  return buf;
}

/*
 * Dumps all variables in an object to a file.
 * If bit 0 of flag is set, unpaid objects will be saved.  As of now,
 * the only place this is not set is when saving the player.
 * If bit 1 of flag is set, don't remove the object after save.  As of now,
 * all of the callers are setting this.
 */

void save_object(FILE *fp,object *op, int flag) {
    archetype *at;
    char *cp;
    object *tmp,*old;

    /* Even if the object does have an owner, it would seem that we should
     * still save it.
     */
    /*if(op->owner!=NULL || fp == NULL) this will wreck item we throw - they get player as owner*/

    /* If it is unpaid and we don't want to save those, just return. */
    if(!fp || QUERY_FLAG(op, FLAG_NO_SAVE) || (!(flag&1) && (QUERY_FLAG(op, FLAG_UNPAID))))
		return;

    if((at=op->arch)==NULL)
		at=empty_archetype;
	fprintf(fp,"arch %s\n",at->name);

    if((cp=get_ob_diff(op,&at->clone)))
		fputs(cp,fp);	/* We really should do some status checking on this */

    old=NULL;

    if (flag & 2 )
	{
		if(!QUERY_FLAG(op, FLAG_NO_INVENTORY) )
		{
			for(tmp=op->inv;tmp;tmp=tmp->below)
				save_object(fp,tmp,flag);
		}
	    /* Slightly different logic because tmp/op will be removed by
		* the save_object we call.  So we just keep looking at op->inv
		* until there is nothing left.  In theory, the variable old
		* should not be needed, as recursive loops shouldn't happen.
		*/
	}
    else while ((tmp=op->inv))
	{
		if(old==tmp)
		{
			LOG(llevError," Recursive loop in inventory\n");
			break;
		}
		save_object(fp,tmp,flag);
		old=tmp;
    }

    /* Save a player's pets */
    if(op->type==PLAYER)
        save_all_pets(fp, op, flag);

    if (!(flag&2))
		remove_ob(op);

    fprintf(fp,"end\n");
}
