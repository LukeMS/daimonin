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

/* This containes item logic for client/server.  IT doesn't contain
 * the actual commands that send the data, but does contain
 * the logic for what items should be sent.
 */


#include <global.h>
#include <object.h>	/* LOOK_OBJ */
#include <newclient.h>
#include <newserver.h>
#include <sproto.h>

static object *esrv_get_ob_from_count_DM(object *pl, tag_t count);

/* This is the maximum number of bytes we expect any one item to take up */
#define MAXITEMLEN  300

/*******************************************************************************
 *
 * Functions related to sending object data to the client.
 *
 ******************************************************************************/

/* This is more or less stolen from the query_weight function. */
#define WEIGHT(op) (op->nrof?op->weight:op->weight+op->carrying)

/* This is a simple function that we use a lot here.  It basically
 * adds the specified buffer into the socklist, but prepends a
 * single byte in length.  If the data is longer than that byte, it is
 * truncated approprately.
 */
inline void add_stringlen_to_sockbuf(char *buf, SockList *sl)
{
    int len;

    len=strlen(buf);
    if (len>255) len=255;
    SockList_AddChar(sl, (char) len);
    strncpy((char*)sl->buf+sl->len, buf,len);
    sl->len += len;
}

/* 
 *  This is a similar to query_name, but returns flags
 *  to be sended to client. 
 */
unsigned int query_flags (object *op)
{
    unsigned int flags = 0;

    if(QUERY_FLAG(op,FLAG_APPLIED)) {
	switch(op->type) {
	  case BOW:
	  case WAND:
	  case ROD:
	  case HORN:
	    flags = a_readied;
	    break;
	  case WEAPON:
	    flags = a_wielded;
	    break;
	  case SKILL:
	  case ARMOUR:
	  case HELMET:
	  case SHIELD:
	  case RING:
	  case BOOTS:
	  case GLOVES:
	  case AMULET:
	  case GIRDLE:
	  case BRACERS:
	  case CLOAK:
	    flags = a_worn;
	    break;
	  case CONTAINER:
	    flags = a_active;
	    break;
	  default:
	    flags = a_applied;
	    break;
	}
    }

    if (op->type == CONTAINER && ((op->env && op->env->container == op) || 
	(!op->env && QUERY_FLAG(op,FLAG_APPLIED))))
		flags |= F_OPEN;
   
    if (QUERY_FLAG(op,FLAG_KNOWN_CURSED)) {
	if(QUERY_FLAG(op,FLAG_DAMNED))
	    flags |= F_DAMNED;
	else if(QUERY_FLAG(op,FLAG_CURSED))
	    flags |= F_CURSED;
    }
    if ((QUERY_FLAG(op,FLAG_KNOWN_MAGICAL)&&QUERY_FLAG(op,FLAG_IS_MAGICAL)) ||
								(QUERY_FLAG(op,FLAG_IS_MAGICAL)&& QUERY_FLAG(op,FLAG_IDENTIFIED)))
	flags |= F_MAGIC;
    if (QUERY_FLAG(op,FLAG_UNPAID))
	flags |= F_UNPAID;
    if (QUERY_FLAG(op,FLAG_INV_LOCKED))
	flags |= F_LOCKED;
    if (QUERY_FLAG(op,FLAG_IS_INVISIBLE))
		flags |= F_INVISIBLE;
    if (QUERY_FLAG(op,FLAG_IS_ETHEREAL))
		flags |= F_ETHEREAL;
    return flags;
}


/* draw the look window.  Don't need to do animations here 
 * This sends all the faces to the client, not just updates.  This is
 * because object ordering would otherwise be inconsistent
 */

void esrv_draw_look(object *pl)
{
	char *tmp_sp;
    object *head, *tmp, *last;
    int len, flags, got_one=0,anim_speed, start_look=0, end_look=0;
    SockList sl;
    char buf[MAX_BUF];

    if (!pl->contr->socket.update_look)
	{
		LOG(llevDebug,"esrv_draw_look called when update_look was not set\n");
		return;
    } 
	else
		pl->contr->socket.update_look=0;

	/* change out_of_map to OUT_OF_REAL_MAP(). we don't even think here about map crossing */
    if(QUERY_FLAG(pl, FLAG_REMOVED) || pl->map == NULL || pl->map->in_memory != MAP_IN_MEMORY 
													|| OUT_OF_REAL_MAP(pl->map,pl->x,pl->y))
	    return;

	/* another layer feature: grap last (top) object without browsing the objects */
	tmp = GET_MAP_OB_LAST(pl->map,pl->x,pl->y);

    sl.buf=malloc(MAXSOCKBUF);

    strcpy((char*)sl.buf,"itemx ");
    sl.len=strlen((char*)sl.buf);

    SockList_AddInt(&sl, 0); /* delinv 0 */
    SockList_AddInt(&sl, 0);

    if (pl->contr->socket.look_position)
	{
		SockList_AddInt(&sl, 0x80000000 | (pl->contr->socket.look_position- NUM_LOOK_OBJECTS));
		SockList_AddInt(&sl, 0);
		SockList_AddInt(&sl, -1);
		SockList_AddInt(&sl, prev_item_face->number);
		SockList_AddChar(&sl, 0);
		sprintf(buf,"Click here to see %d previous items", NUM_LOOK_OBJECTS);
		add_stringlen_to_sockbuf(buf, &sl);
		SockList_AddShort(&sl,0);
		SockList_AddChar(&sl, 0);
		SockList_AddInt(&sl, 0);
    }

    for (last=NULL; tmp!=last; tmp=tmp->below) 
	{
		if(tmp==pl)
			continue;

		/* skip map mask, sys_objects and invisible objects when we can't see them */
		if(tmp->layer == 2 || IS_SYS_INVISIBLE(tmp) ||
			(!QUERY_FLAG(pl,FLAG_SEE_INVISIBLE) && QUERY_FLAG(tmp,FLAG_IS_INVISIBLE)) )
		{
			/* but only when we are not a active DM */
			if(!QUERY_FLAG(pl,FLAG_WIZ))
				continue;
		}

		/* skip all items we had send before of the 'max shown items of a tile space' */
		if (++start_look < pl->contr->socket.look_position)
			continue;
		/* if we have to much items to send, send a 'next group' object and leave here */
		if (++end_look > NUM_LOOK_OBJECTS)
		{
			SockList_AddInt(&sl, 0x80000000 | (pl->contr->socket.look_position+ NUM_LOOK_OBJECTS));
			SockList_AddInt(&sl, 0);
			SockList_AddInt(&sl, -1);
			SockList_AddInt(&sl, next_item_face->number);
			SockList_AddChar(&sl, 0);
			sprintf(buf,"Click here to see next group of items");
			add_stringlen_to_sockbuf(buf, &sl);

			SockList_AddShort(&sl,0);
			SockList_AddChar(&sl, 0);
			SockList_AddInt(&sl, 0);
			break;
		}

		/* ok, now we start sending this item here */

		flags = query_flags (tmp);
		if (QUERY_FLAG(tmp, FLAG_NO_PICK))
			flags |=  F_NOPICK;
			
		/* we send never the inventory animation here */
		if (QUERY_FLAG(tmp,FLAG_ANIMATE) && !pl->contr->socket.anims_sent[tmp->animation_id])
			esrv_send_animation(&pl->contr->socket, tmp->animation_id);

		SockList_AddInt(&sl, tmp->count);
		SockList_AddInt(&sl, flags);
		SockList_AddInt(&sl, QUERY_FLAG(tmp, FLAG_NO_PICK) ? -1 : WEIGHT(tmp));
		if(tmp->head)
			SockList_AddInt(&sl, tmp->head->face->number);
		else
			SockList_AddInt(&sl, tmp->face->number);
		SockList_AddChar(&sl, tmp->facing);

		if(tmp->head)
			head = tmp->head;
		else 				
			head = tmp;

		len=strlen((tmp_sp=query_base_name(head)))+1; /* +1 = 0 marker for string end */
		if(len >128)
		{
			len = 128; /* 127 chars + 0 marker */
			SockList_AddChar(&sl, (char ) len);
			strncpy(sl.buf+sl.len,tmp_sp,127);
			sl.len += len;
			*(sl.buf+sl.len)=0;
		}
		else
		{
			SockList_AddChar(&sl, (char ) len);
    		strcpy(sl.buf+sl.len,tmp_sp);
			sl.len += len;
		}

		/* handle animations... this will change 100% when we add client
		 * sided animations.
		 */
		 SockList_AddShort(&sl,tmp->animation_id);
		anim_speed=0;
		if (QUERY_FLAG(tmp,FLAG_ANIMATE))
		{
			if (tmp->anim_speed) 
				anim_speed=tmp->anim_speed;
			else 
			{
				if (FABS(tmp->speed)<0.001) 
					anim_speed=255;
				else if (FABS(tmp->speed)>=1.0)
					anim_speed=1;
				else 
					anim_speed = (int) (1.0/FABS(tmp->speed));
			}
			if (anim_speed>255)
				anim_speed=255;
		}
		SockList_AddChar(&sl, (char) anim_speed);

		SockList_AddInt(&sl, tmp->nrof);
		SET_FLAG(tmp, FLAG_CLIENT_SENT);
		got_one++;

		if (sl.len > (MAXSOCKBUF-MAXITEMLEN))
		{
			Send_With_Handling(&pl->contr->socket, &sl);
			strcpy((char*)sl.buf,"itemx ");
			sl.len=strlen((char*)sl.buf);
			SockList_AddInt(&sl, -2); /* do no delinv */
			SockList_AddInt(&sl, 0);
			got_one=0;
		}

		/* We do a special for DMs - forcing the
		 * inventory of ALL objects we send here... This is a 
		 * wonderful feature for controling & find bugs.
		 */
		if(QUERY_FLAG(pl,FLAG_WIZ))
		{
			if(tmp->inv)
				got_one = esrv_draw_DM_inv(pl,&sl,tmp);
		}

	} /* for loop */

    if (got_one)
		Send_With_Handling(&pl->contr->socket, &sl);

    free(sl.buf);
}

/* used for a active DM - implicit sending the inventory of all
 * items we see in inventory & in below. For controling & debug.
 * Do a examine cmd over the item and you will see a dump.
 */
int esrv_draw_DM_inv(object *pl, SockList *sl, object *op)
{
	char *tmp_sp;
	object *tmp, *head;
	int got_one,flags,len,anim_speed;

	SockList_AddInt(sl, 0);
	SockList_AddInt(sl, 0);
	SockList_AddInt(sl, -1);
	SockList_AddInt(sl, blank_face->number);
	len = strlen("in inventory")+1;
	SockList_AddChar(sl, (char)len);
	add_stringlen_to_sockbuf("in inventory", sl);
	SockList_AddShort(sl,0);
	SockList_AddChar(sl, 0);
	SockList_AddInt(sl, 0);

	for (tmp=op->inv; tmp; tmp=tmp->below) 
	{

		flags = query_flags (tmp);
		if (QUERY_FLAG(tmp, FLAG_NO_PICK))
			flags |=  F_NOPICK;

		/* we send never the inventory animation here */
		if (QUERY_FLAG(tmp,FLAG_ANIMATE) && !pl->contr->socket.anims_sent[tmp->animation_id])
			esrv_send_animation(&pl->contr->socket, tmp->animation_id);

		SockList_AddInt(sl, tmp->count);
		SockList_AddInt(sl, flags);
		SockList_AddInt(sl, QUERY_FLAG(tmp, FLAG_NO_PICK) ? -1 : WEIGHT(tmp));
		if(tmp->head)
			SockList_AddInt(sl, tmp->head->face->number);
		else
			SockList_AddInt(sl, tmp->face->number);
		SockList_AddChar(sl, tmp->facing);

		if(tmp->head)
			head = tmp->head;
		else 				
			head = tmp;

		len=strlen((tmp_sp=query_base_name(head)))+1; /* +1 = 0 marker for string end */
		if(len >128)
		{
			len = 128; /* 127 chars + 0 marker */
			SockList_AddChar(sl, (char ) len);
			strncpy(sl->buf+sl->len,tmp_sp,127);
			sl->len += len;
			*(sl->buf+sl->len)=0;
		}
		else
		{
			SockList_AddChar(sl, (char ) len);
    		strcpy(sl->buf+sl->len,tmp_sp);
			sl->len += len;
		}

		/* handle animations... this will change 100% when we add client
		 * sided animations.
		 */
		 SockList_AddShort(sl,tmp->animation_id);
		anim_speed=0;
		if (QUERY_FLAG(tmp,FLAG_ANIMATE))
		{
			if (tmp->anim_speed) 
				anim_speed=tmp->anim_speed;
			else 
			{
				if (FABS(tmp->speed)<0.001) 
					anim_speed=255;
				else if (FABS(tmp->speed)>=1.0)
					anim_speed=1;
				else 
					anim_speed = (int) (1.0/FABS(tmp->speed));
			}
			if (anim_speed>255)
				anim_speed=255;
		}
		SockList_AddChar(sl, (char) anim_speed);

		SockList_AddInt(sl, tmp->nrof);
		SET_FLAG(tmp, FLAG_CLIENT_SENT);
		got_one++;

		if (sl->len > (MAXSOCKBUF-MAXITEMLEN))
		{
			Send_With_Handling(&pl->contr->socket, sl);
			strcpy((char*)sl->buf,"itemx ");
			sl->len=strlen((char*)sl->buf);
			SockList_AddInt(sl, -2); /* do no delinv */
			SockList_AddInt(sl, 0);
			got_one=0;
		}

		/* oh well... */
		if(tmp->inv)
			got_one = esrv_draw_DM_inv(pl,sl, tmp);
	} /* for loop */

	SockList_AddInt(sl, 0);
	SockList_AddInt(sl, 0);
	SockList_AddInt(sl, -1);
	SockList_AddInt(sl, blank_face->number);
	len = strlen("end of inventory")+1;
	SockList_AddChar(sl, (char)len);
	add_stringlen_to_sockbuf("end of inventory", sl);
	SockList_AddShort(sl,0);
	SockList_AddChar(sl, 0);
	SockList_AddInt(sl, 0);
	return got_one;
}

/* send_inventory send the inventory for the player BUT also the inventory for
 * items. When the player obens a chest on the ground, this function is called to
 * send the inventory for the chest - and the items are shown in below. The client
 * should take care, that the items in the below windows shown can be changed here
 * too without calling the function for the look window.
 */
/* we have here no invisible flag - we can "see" invisible when we have it in the inventory.
 * that simulate the effect that we can "feel" the object in our hands.
 * but when we drop it, it rolls out of sight and vanish...
 */
void esrv_send_inventory(object *pl, object *op)
{
    object *tmp;
    int flags, got_one=0, anim_speed, len;
    SockList sl;
    char item_n[MAX_BUF];
    
    sl.buf=malloc(MAXSOCKBUF);

     strcpy((char*)sl.buf,"itemx ");
    sl.len=strlen((char*)sl.buf);

	/* yep this is strange - i merged delinv & itemx - not perfect but it works */
    SockList_AddInt(&sl, op->count); /* do delinv */
    SockList_AddInt(&sl, op->count);
    
    for (tmp=op->inv; tmp; tmp=tmp->below) {

		if(!QUERY_FLAG(pl,FLAG_SEE_INVISIBLE) && QUERY_FLAG(tmp,FLAG_IS_INVISIBLE) )
	{
		/* skip this for DMs */
		if(!QUERY_FLAG(pl,FLAG_WIZ))
			continue;
	}

	if (LOOK_OBJ(tmp)|| QUERY_FLAG(pl,FLAG_WIZ)) {
	    flags = query_flags (tmp);
	    if (QUERY_FLAG(tmp, FLAG_NO_PICK))
		flags |=  F_NOPICK;

		if(tmp->inv_animation_id)
		{
			if (!pl->contr->socket.anims_sent[tmp->inv_animation_id])
				esrv_send_animation(&pl->contr->socket, tmp->animation_id);
		}
		else
		{
			if (QUERY_FLAG(tmp,FLAG_ANIMATE) && 
				!pl->contr->socket.anims_sent[tmp->animation_id])
			esrv_send_animation(&pl->contr->socket, tmp->animation_id);
		}

		SockList_AddInt(&sl, tmp->count);
	    SockList_AddInt(&sl, flags);
	    SockList_AddInt(&sl, QUERY_FLAG(tmp, FLAG_NO_PICK) ? -1 : WEIGHT(tmp));

		if(tmp->inv_face)
		    SockList_AddInt(&sl, tmp->inv_face->number);
		else
		    SockList_AddInt(&sl, tmp->face->number);
			
        SockList_AddChar(&sl, tmp->facing);
            SockList_AddChar(&sl, tmp->type);
            SockList_AddChar(&sl, tmp->sub_type1);
            if(QUERY_FLAG(tmp, FLAG_IDENTIFIED)) 
            {
                SockList_AddChar(&sl, tmp->item_quality);
                SockList_AddChar(&sl, tmp->item_condition);
                SockList_AddChar(&sl, tmp->item_level);
                SockList_AddChar(&sl, tmp->item_skill);
            }
            else
            {
                SockList_AddChar(&sl, (char) 255);
                SockList_AddChar(&sl, (char) 255);
                SockList_AddChar(&sl, (char)255);
                SockList_AddChar(&sl, (char)255);
            }
        strncpy(item_n,query_base_name(tmp),127);
		item_n[127]=0;
		len=strlen(item_n)+1;
		SockList_AddChar(&sl, (char) len);
		memcpy(sl.buf+sl.len, item_n, len);
		sl.len += len;
		if(tmp->inv_animation_id)
		    SockList_AddShort(&sl,tmp->inv_animation_id);
		else
		    SockList_AddShort(&sl,tmp->animation_id);
		/* i use for both the same anim_speed - when we need a different,
		 * i adding inv_anim_speed.
		 */
	    anim_speed=0;
	    if (QUERY_FLAG(tmp,FLAG_ANIMATE)) {
		if (tmp->anim_speed) anim_speed=tmp->anim_speed;
		else {
		    if (FABS(tmp->speed)<0.001) anim_speed=255;
		    else if (FABS(tmp->speed)>=1.0) anim_speed=1;
		    else anim_speed = (int) (1.0/FABS(tmp->speed));
		}
		if (anim_speed>255) anim_speed=255;
	    }
	    SockList_AddChar(&sl, (char)anim_speed);
	    SockList_AddInt(&sl, tmp->nrof);
	    SET_FLAG(tmp, FLAG_CLIENT_SENT);
	    got_one++;

	    /* IT is possible for players to accumulate a huge amount of
	     * items (especially with some of the bags out there) to
	     * overflow the buffer.  IF so, send multiple item1 commands.
	     */
	    if (sl.len > (MAXSOCKBUF-MAXITEMLEN)) {
		Send_With_Handling(&pl->contr->socket, &sl);
		strcpy((char*)sl.buf,"itemx ");
		sl.len=strlen((char*)sl.buf);
		SockList_AddInt(&sl,-3); /* no delinv */
		SockList_AddInt(&sl, op->count);
		got_one=0;
	    }
	}
    }
    if (got_one)
	Send_With_Handling(&pl->contr->socket, &sl);
    free(sl.buf);
}

/* Updates object *op for player *pl.  flags is a list of values to update
 * to the client (as defined in newclient.h - might as well use the
 * same value both places.
 */

void esrv_update_item(int flags, object *pl, object *op)
{
    SockList sl;

    /* If we have a request to send the player item, skip a few checks. */
    if (op!=pl) 
	{
		if (!LOOK_OBJ(op) && !QUERY_FLAG(pl,FLAG_WIZ)) 
			return;
		/* we remove the check for op->env, because in theory, the object
		 * is hopefully in the same place, so the client should preserve
		 * order.
		 */
    }

	/* FLAG_CLIENT_SENT is debug only.  We are using it to see where
	 * this is happening - we can set a breakpoint here in the debugger
	 * and track back the call.
	    if (!QUERY_FLAG(op, FLAG_CLIENT_SENT))
			LOG(llevDebug,"We have not sent item %s (%d)\n", op->name, op->count);
	 */

    sl.buf=malloc(MAXSOCKBUF);

    strcpy((char*)sl.buf,"upditem ");
    sl.len=strlen((char*)sl.buf);

    SockList_AddShort(&sl, (uint16) flags);
    SockList_AddInt(&sl, op->count);

    if (flags & UPD_LOCATION)
	SockList_AddInt(&sl, op->env? op->env->count:0);

    if (flags & UPD_FLAGS)
	SockList_AddInt(&sl, query_flags(op));

    if (flags & UPD_WEIGHT)
	SockList_AddInt(&sl, WEIGHT(op));

    if (flags & UPD_FACE) {
		if(op->inv_face)
		{
			/*
			if (!pl->contr->socket.faces_sent[op->inv_face->number])
				esrv_send_face(&pl->contr->socket, op->inv_face->number,0);
			*/
			SockList_AddInt(&sl, op->inv_face->number);
		}
		else
		{
			/*
			if (!pl->contr->socket.faces_sent[op->face->number])
				esrv_send_face(&pl->contr->socket, op->face->number,0);
			*/
			SockList_AddInt(&sl, op->face->number);
		}
    }
    if (flags & UPD_DIRECTION)
		SockList_AddChar(&sl, (char)op->facing);
    if (flags & UPD_NAME) {
	if (pl->contr->socket.sc_version>=1024) {
	    int len;
	    char *item_p, item_n[MAX_BUF];

	    strncpy(item_n,query_base_name(op),127);
	    item_n[127]=0;
	    len=strlen(item_n);
	    item_p=query_base_name(op);
	    strncpy(item_n+len+1, item_p, 127);
	    item_n[254]=0;
	    len += strlen(item_n+1+len) + 1;
	    SockList_AddChar(&sl, (char)len);
	    memcpy(sl.buf+sl.len, item_n, len);
	    sl.len += len;
	} else
	    add_stringlen_to_sockbuf(query_base_name(op), &sl);
    }
    if (flags & UPD_ANIM) 
	{
		if(op->inv_animation_id)
		    SockList_AddShort(&sl,op->inv_animation_id);
		else
		    SockList_AddShort(&sl,op->animation_id);
	}
    if (flags & UPD_ANIMSPEED) {
	int anim_speed=0;
	if (QUERY_FLAG(op,FLAG_ANIMATE)) {
	    if (op->anim_speed) anim_speed=op->anim_speed;
	    else {
		if (FABS(op->speed)<0.001) anim_speed=255;
		else if (FABS(op->speed)>=1.0) anim_speed=1;
		else anim_speed = (int) (1.0/FABS(op->speed));
	    }
	    if (anim_speed>255) anim_speed=255;
	}
	SockList_AddChar(&sl, (char)anim_speed);
    }
    if (flags & UPD_NROF)
	    SockList_AddInt(&sl, op->nrof);

    Send_With_Handling(&pl->contr->socket, &sl);
    free(sl.buf);
}

void esrv_send_item(object *pl, object*op)
{
    int anim_speed;
    SockList sl;
    char item_n[MAX_BUF];
    
    /* If this is not the player object, do some more checks */
    if (op!=pl) {
	/* We only send 'visibile' objects to the client */
	if (! LOOK_OBJ(op)) 
	    return;
	/* if the item is on the ground, mark that the look needs to
	 * be updated.
	 */
	if (!op->env) {
	    pl->contr->socket.update_look=1;
	    return;
	}
    }

    sl.buf=malloc(MAXSOCKBUF);

    strcpy((char*)sl.buf,"itemx ");
    sl.len=strlen((char*)sl.buf);

    SockList_AddInt(&sl, -4); /* no delinv */
    SockList_AddInt(&sl, (op->env? op->env->count:0));

	/*
	if(op->env && op->inv_face)
	{
	    if (!pl->contr->socket.faces_sent[op->inv_face->number])
			esrv_send_face(&pl->contr->socket, op->inv_face->number,0);
	}
	else
	{
	    if (!pl->contr->socket.faces_sent[op->face->number])
			esrv_send_face(&pl->contr->socket, op->face->number,0);
	}*/

	if(op->env && op->inv_animation_id)
	{
	    if (!pl->contr->socket.anims_sent[op->inv_animation_id])
			esrv_send_animation(&pl->contr->socket, op->inv_animation_id);
	}
	else
	{
	    if (op->env && QUERY_FLAG(op,FLAG_ANIMATE) &&
		   !pl->contr->socket.anims_sent[op->animation_id])
		esrv_send_animation(&pl->contr->socket, op->animation_id);
	}

    SockList_AddInt(&sl, op->count);
    SockList_AddInt(&sl, query_flags(op));
    SockList_AddInt(&sl, WEIGHT(op));
	if(op->env && op->inv_face)
	    SockList_AddInt(&sl, op->inv_face->number);
	else
		SockList_AddInt(&sl, op->face->number);
    SockList_AddChar(&sl, op->facing);
    if(op->env) /* if not below */
    {
        SockList_AddChar(&sl, op->type);
        SockList_AddChar(&sl, op->sub_type1);
        if(QUERY_FLAG(op, FLAG_IDENTIFIED)) 
        {
            SockList_AddChar(&sl, op->item_quality);
            SockList_AddChar(&sl, op->item_condition);
            SockList_AddChar(&sl, op->item_level);
            SockList_AddChar(&sl, op->item_skill);
        }
        else
        {
            SockList_AddChar(&sl, (char)255);
            SockList_AddChar(&sl, (char)255);
            SockList_AddChar(&sl, (char)255);
            SockList_AddChar(&sl, (char)255);
        }
    }
    
    if (pl->contr->socket.sc_version>=1024) {
	int len;
    strncpy(item_n,query_base_name(op),127);
	item_n[127]=0;
	len=strlen(item_n)+1;
	SockList_AddChar(&sl, (char)len);
	memcpy(sl.buf+sl.len, item_n, len);
	sl.len += len;
    } else
	add_stringlen_to_sockbuf(query_base_name(op), &sl);

	if(op->env && op->inv_animation_id)
	    SockList_AddShort(&sl,op->inv_animation_id);
	else
		SockList_AddShort(&sl,op->animation_id);
    anim_speed=0;
    if (QUERY_FLAG(op,FLAG_ANIMATE)) {
	if (op->anim_speed) anim_speed=op->anim_speed;
        else {
	    if (FABS(op->speed)<0.001) anim_speed=255;
            else if (FABS(op->speed)>=1.0) anim_speed=1;
            else anim_speed = (int) (1.0/FABS(op->speed));
	}
        if (anim_speed>255) anim_speed=255;
    }
    SockList_AddChar(&sl, (char)anim_speed);
    SockList_AddInt(&sl, op->nrof);
    Send_With_Handling(&pl->contr->socket, &sl);
    SET_FLAG(op, FLAG_CLIENT_SENT);
    free(sl.buf);
}

/* Tells the client to delete an item.  Uses the item
 * command with a -1 location.
 */

void esrv_del_item(player *pl, int tag)
{
    SockList sl;

    sl.buf=malloc(MAXSOCKBUF);

    strcpy((char*)sl.buf,"delitem ");
    sl.len=strlen((char*)sl.buf);
    SockList_AddInt(&sl, tag);

    Send_With_Handling(&pl->socket, &sl);
    free(sl.buf);
}


/*******************************************************************************
 *
 * Client has requested us to do something with an object.
 *
 ******************************************************************************/

/* Takes a player and object count (tag) and returns the actual object
 * pointer, or null if it can't be found.
 */

object *esrv_get_ob_from_count(object *pl, tag_t count)
{
    object *op, *tmp;

    if (pl->count == count)
	return pl;

	/* this is special case... We can examine deep inside every inventory
	 * even from non containers.
	 */
	if(QUERY_FLAG(pl,FLAG_WIZ) )
	{
	    for(op = pl->inv; op; op = op->below)
		{
			if (op->count == count)
				return op;
			else if (op->inv)
			{
				if((tmp = esrv_get_ob_from_count_DM(op->inv, count)))
					return tmp;

			}
		}
		for(op = get_map_ob (pl->map, pl->x, pl->y); op; op = op->above)
		{
			if (op->count == count)
				return op;
			else if (op->inv)
			{
				if((tmp = esrv_get_ob_from_count_DM(op->inv, count)))
					return tmp;
			}
		}
		return NULL;
	}

    if (pl->count == count)
	return pl;

    for(op = pl->inv; op; op = op->below)
	if (op->count == count)
	    return op;
	else if (op->type == CONTAINER && pl->container == op)
	    for(tmp = op->inv; tmp; tmp = tmp->below)
		if (tmp->count == count)
		    return tmp;

    for(op = get_map_ob (pl->map, pl->x, pl->y); op; op = op->above)
	if (op->count == count)
	    return op;
	else if (op->type == CONTAINER && pl->container == op)
	    for(tmp = op->inv; tmp; tmp = tmp->below)
		if (tmp->count == count)
		    return tmp;
    return NULL;
}

/* rekursive function for DM access to non container inventories */
static object *esrv_get_ob_from_count_DM(object *pl, tag_t count)
{
	object *tmp, *op;

	for(op = pl; op; op = op->below)
	{
		if (op->count == count)
			return op;
		else if (op->inv)
		{
			if((tmp = esrv_get_ob_from_count_DM(op->inv, count)))
				return tmp;

		}
	}
	return NULL;
}


/* Client wants to examine some object.  So lets do so. */
void ExamineCmd(char *buf, int len,player *pl)
{
    long tag = atoi(buf);
    object *op = esrv_get_ob_from_count(pl->ob, tag);

    if (!op) {
	/*LOG(llevDebug, "Player '%s' tried examine the unknown object (%d)\n",pl->ob->name, tag);*/
	return;
    }
    examine (pl->ob, op);
}

/* Client wants to apply some object.  Lets do so. */
void ApplyCmd(char *buf, int len,player *pl)
{
    uint32 tag = atoi(buf);
    object *op = esrv_get_ob_from_count(pl->ob, tag);

    /* sort of a hack, but if the player saves and the player then manually
     * applies a savebed (or otherwise tries to do stuff), we run into trouble.
     */
    if (QUERY_FLAG(pl->ob, FLAG_REMOVED)) return;

    /* If the high bit is set, player applied a pseudo object. */
    if (tag & 0x80000000) {
	pl->socket.look_position = tag & 0x7fffffff;
	pl->socket.update_look = 1;
	return;
    }

    if (!op) {
	/*LOG(llevDebug, "Player '%s' tried apply the unknown object (%d)\n",pl->ob->name, tag);*/
	return;
    }
    player_apply (pl->ob, op, 0, 0);
}

/* Client wants to apply some object.  Lets do so. */
void LockItem(uint8 *data, int len,player *pl)
{
    int flag, tag;
    object *op;

    flag = data[0];
    tag = GetInt_String(data+1);
    op = esrv_get_ob_from_count(pl->ob, tag);

    if (!op) {
	/*new_draw_info(NDI_UNIQUE, 0, pl->ob,"Could not find object to lock/unlock");*/
	return;
    }
    if (!flag)
	CLEAR_FLAG(op,FLAG_INV_LOCKED);
    else
	SET_FLAG(op,FLAG_INV_LOCKED);
    esrv_update_item(UPD_FLAGS, pl->ob, op);
}

/* Client wants to apply some object.  Lets do so. */
void MarkItem(uint8 *data, int len,player *pl)
{
    int tag;
    object *op;

    tag = GetInt_String(data);
    op = esrv_get_ob_from_count(pl->ob, tag);
    if (!op) {
	/*new_draw_info(NDI_UNIQUE, 0, pl->ob,"Could not find object to mark");*/
	return;
    }
    pl->mark = op;
    pl->mark_count = op->count;
    new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "Marked item %s", query_name(op));
}


/*
 * look_at prints items on the specifc square.
 * [ removed EARTHWALL check and added check for containers inventory.
 *   Tero.Haatanen@lut.fi ]
 */
void look_at(object *op,int dx,int dy) {
    object *tmp;
    int flag=0,x,y;
    mapstruct *m;

    x = op->x + dx;
    y = op->y + dy;

    if(!(m = out_of_map(op->map, &x, &y)))
		return;

    for(tmp=get_map_ob(m, x ,y);tmp!=NULL&&tmp->above!=NULL;
	    tmp=tmp->above);

    for ( ; tmp != NULL; tmp=tmp->below ) {
	 if (IS_SYS_INVISIBLE(tmp) && !QUERY_FLAG(op, FLAG_WIZ)) continue;

	 if(!flag) {
	    if(dx||dy)
		new_draw_info(NDI_UNIQUE, 0,op,"There you see:");
	    else {
		new_draw_info(NDI_UNIQUE, 0,op,"You see:");
	    }
	    flag=1;
	 }

	 if (QUERY_FLAG(op, FLAG_WIZ))
	    new_draw_info_format(NDI_UNIQUE,0, op, "- %s (%d).",query_name(tmp),tmp->count);
	 else
	    new_draw_info_format(NDI_UNIQUE,0, op, "- %s.",query_name(tmp));

	 if((tmp->inv!=NULL || (tmp->head && tmp->head->inv)) && 
	    ( (!dx&&!dy) || tmp->type != CONTAINER || QUERY_FLAG(op, FLAG_WIZ)
	     || !(tmp->type) || tmp->type!=FLESH ))
	    inventory(op,tmp->head==NULL?tmp:tmp->head);

	 if(QUERY_FLAG(tmp, FLAG_IS_FLOOR)&&!QUERY_FLAG(op, FLAG_WIZ))	/* don't continue under the floor */
	    break;
    }

    if(!flag) {
	if(dx||dy)
	    new_draw_info(NDI_UNIQUE, 0,op,"You see nothing there.");
	else
	    new_draw_info(NDI_UNIQUE, 0,op,"You see nothing.");
    }
}



/* Client wants to apply some object.  Lets do so. */
void LookAt(char *buf, int len,player *pl)
{
    int dx, dy;
    char *cp;

    dx=atoi(buf);
    if (!(cp=strchr(buf,' '))) {
	return;
    }
    dy=atoi(cp);

    if (FABS(dx)>MAP_CLIENT_X/2 || FABS(dy)>MAP_CLIENT_Y/2)
          return;

    if(pl->blocked_los[dx+(pl->socket.mapx/2)][dy+(pl->socket.mapy/2)])
          return;
    look_at(pl->ob, dx, dy);
}

/* Move an object to a new lcoation */

void esrv_move_object (object *pl, tag_t to, tag_t tag, long nrof)
{
    object *op, *env;

    op = esrv_get_ob_from_count(pl, tag);
    if (!op) {
	/* this happens very often and can be forced when moving and trying to access 
	 * a item in the below window - because latency effects, the client has still
	 * the inventory id from a tile even the server has moved the player and updated
	 * the inventory. MT-11-2002
	 */
	/*LOG(llevDebug, "Player '%s' tried to move the unknown object (%ld)\n",pl->name, tag);*/
	return;
    }

    if (!to) {	/* drop it to the ground */

	if (op->map && !op->env) {
/*	    LOG(llevDebug,"Dropping object to ground that is already on ground\n");*/
	    return;
	}
	/* If it is an active container, then we should drop all objects
	 * in the container and not the container itself.
	 */
	if (op->inv && QUERY_FLAG(op, FLAG_APPLIED)) {
	    object *current, *next;
		/*LOG(-1,"applied container...\n");*/
	    for (current=op->inv; current!=NULL; current=next) {
		next=current->below;
		drop_object(pl, current, 0);
	    }
	    esrv_update_item(UPD_WEIGHT, pl, op);
	}
	else {
	/*	LOG(-1,"unapplied container...\n");*/
	    drop_object (pl, op, nrof);
	}
	return;
    } else if (to == pl->count) {     /* pick it up to the inventory */
	/* return if player has already picked it up */
	if (op->env == pl) return;

	pl->contr->count = nrof;
	/*LOG(-1,"pick up...\n");*/
	pick_up(pl, op);
	return ;
    }
    /* If not dropped or picked up, we are putting it into a sack */
    env = esrv_get_ob_from_count(pl, to);
    if (!env) {
      /*LOG(llevDebug, "Player '%s' tried to move object to the unknown location (%d)\n",pl->name, to);*/
      return;
    }
#if 0
    printf ("Sacks name was '%s'.\n", env->name);
#endif
    /* put_object_in_sack presumes that necessary sanity checking
     * has already been done (eg, it can be picked up and fits in
     * in a sack, so check for those things.  We should also check
     * an make sure env is in fact a container for that matter.
     */
    if (env->type == CONTAINER && can_pick(pl, op) && sack_can_hold(pl, env, op, nrof)) {
	/*LOG(-1,"put in sack...\n");*/
	put_object_in_sack (pl, env, op, nrof);
    }
}


