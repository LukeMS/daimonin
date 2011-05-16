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

    Client-side animation stuff, 2008 Alderan

*/

#include <include.h>

anim_list *AnimListStart = NULL;
Animations  animation[MAXANIM];
AnimCmds     animcmd[MAXANIM];


/* this are our default mappings, means if a certain sequence/stance is missing it will be mapped as
 * default to the sequences here defined. This default mapping can be overwritten in the arc with
 * 'sequencemap <numtomapto>'.
 */
uint8       defaultmappings[MAX_SEQUENCES] =
{
0,
0, /* map walk to idle */
1, /* map attack to walk */
};

/**
 * deletes an animation from the linked-list
 * the last shown face will stay, if the anim is still on the map. (for tileanims)
 * NEVER call this alone! You must make sure the refering pointers in the different objects like MapCell,
 * item, ... are NULLed.
 */
void new_anim_remove(anim_list *al)
{
    anim_list *ptr;

    if (!AnimListStart)
    {
        LOG(LOG_DEBUG, "BUG: na_remove: tried to remove anim from empty list!\n");
        /* we won't free this pointer, not that we crash the system */
        return;
    }

    if (AnimListStart==al)
        AnimListStart=al->next;
    else
    {
        ptr=AnimListStart;

        while ((ptr->next!=al) && (ptr->next!=NULL))
            ptr=ptr->next;

        if (ptr->next==NULL)
        {
            LOG(LOG_DEBUG, "BUG: na_remove: anim not in list!\n");
            /* we won't free this pointer, not that we crash the system */
            return;
        }

        ptr->next=al->next;
    }
    FREE(al);
    return;
}

/**
 * adds a new animation with base parameters to the list
 * this is the low-level function, it won't provide any linking with the object to animate,
 * as well no type is set.
 */
anim_list *new_anim_add(uint16 anim, uint8 sequence, uint8 dir, uint8 speed)
{
    anim_list   *al = NULL;

    /* check for ranges */
    if ((anim>=MAXANIM) || (sequence>=MAX_SEQUENCES) || (dir>=9) || !speed)
    {
        LOG(LOG_DEBUG,"na_add: anim-params out of bound\n");
        return NULL;
    }

    MALLOC(al, sizeof(anim_list));

    if (!al)
    {
        LOG(LOG_DEBUG, "na_add: could not allocate memory for new anim!\n");
        return NULL;
    }

    al->next = AnimListStart;
    AnimListStart = al;
    al->speed = speed;
    al->dir = dir;
    al->sequence = sequence;
    al->animnum = anim;

    return al;
}

/**
 * adds a new animation to tile x,y. does all the linking and typesetting stuff
 */
anim_list *new_anim_add_tile(uint16 anim, uint8 sequence, uint8 dir, uint8 speed, int x, int y, uint8 layer)
{
    anim_list *al=NULL;
    struct MapCell *mc;

    if ((x<0) || (y<0) || (x>=MapStatusX) || (y>=MapStatusY))
    {
        LOG(LOG_DEBUG,"na_add_with_tile: x,y out of bounds: %d,%d\n",x,y);
        return NULL;
    }


    if (the_map.cells[x][y].anim[layer])
    {
        al = the_map.cells[x][y].anim[layer];
        new_anim_change(al, anim, sequence, dir, speed, 0);
        return al;
    }

    /* dynamic loading */
    if (!new_anim_load_and_check(anim, sequence, dir))
    {
        LOG(LOG_DEBUG,"na_add_tile: anim does not exist: a:%d, s:%d, d:%d\n",anim, sequence, dir);
        return NULL;
    }

    if (animation[anim].aSeq[sequence]->dirs[dir].frames==1)
    {
        the_map.cells[x][y].faces[layer] = animation[anim].aSeq[sequence]->dirs[dir].faces[0];
        map_redraw_flag = 1;

        return NULL;
    }

    al = new_anim_add(anim, sequence, dir, speed);

    if (!al)
        return NULL;

    mc = &(the_map.cells[x][y]);
    al->obj=mc;
    al->layer = layer;
    al->objtype = ATYPE_TILE;
    mc->anim[layer]=al;

    new_anim_reset(al);

    return al;

}


/**
 * modifies an existing anim
 * to not change the values, put in the old one
 * changing sequence or anim will always result in a reset
 *
 * NOTE: animation speed is determined by different values:
 * The base delay is always 50ms for a frame, maybe we provide an option for the players to tweak this
 * value, to fasten or slow the animations, second each frame has an own delay which is a multiple of the base delay
 * which is assumed to be 50ms (means a max framerate of 20 an anim can have).
 * the speed you set here is a factor, 100 = 1.0, eg:
 * delay for the actual frame is 5, which means in terms of time 5*50ms = 250ms,
 * if you set speed to 120(%), you get 250/1.2 = 208ms for that frame...
 * keep in mind that this will affect also the currently shown frame-delay
 */
void new_anim_change(anim_list *al, uint16 anim, uint8 sequence, uint8 dir, uint8 speed, uint8 restart)
{
    uint8 need_reset = restart;
    if (!al)
        return;

    /* if the new requested anim not exists, return, keep the old one */
    /* Do we want his bahavior? */
    if (!new_anim_load_and_check(anim, sequence, dir))
    {
        LOG(LOG_DEBUG,"na_change: anim doesn't exists: a:%d, s:%d, d:%d\n",anim, sequence, dir);
        return;
    }
    if (animation[anim].aSeq[sequence]->dirs[dir].frames==1)
    {
        switch (al->objtype)
        {
            case ATYPE_TILE:
                ((struct MapCell *)al->obj)->faces[al->layer] =
                    animation[anim].aSeq[sequence]->dirs[dir].faces[0];
                map_redraw_flag = 1;
                new_anim_remove_tile(al);
            break;
            case ATYPE_ITEM:
                ((item *)al->obj)->face = animation[anim].aSeq[sequence]->dirs[dir].faces[0];
                new_anim_remove_item(((item *)al->obj));
                //inv_redraw_flag = 1;
            break;
        }
        return;
    }

    if (speed!=al->speed)
    {
        if (speed==0)
            LOG(LOG_DEBUG, "na_change called with zero speed!\n");
        else
            al->speed = speed;
    }

    if (dir!=al->dir)
    {
        if (dir>=9)
            LOG(LOG_DEBUG, "na_change: dir out of bounds: %d\n",dir);
        else
        {
            al->dir = dir;
            if (animation[al->animnum].aSeq[al->sequence]->flags & ASEQ_DIR_RESET)
                need_reset = 1;
        }
    }

    if (sequence!=al->sequence)
    {
        if (sequence>=MAX_SEQUENCES)
            LOG(LOG_DEBUG,"na_change: sequence out of limit!\n");
        else
        {
            al->sequence = sequence;
            need_reset = 1;
        }
    }

    if (anim!=al->animnum)
    {
        if (anim>=MAXANIM)
            LOG(LOG_DEBUG,"na_change_anim: animnum out of limit!\n");
        else
        {
            al->animnum = anim;
            need_reset = 1;
        }
    }

    if (need_reset)
        new_anim_reset(al);

    return;
}

uint8 new_anim_load_and_check(uint16 anim, uint8 sequence, uint8 dir)
{
    /* test if we have already loaded the basic anim (doesn't mean the needed faces are already loaded, only the structure) */
    if (!animation[anim].loaded)
    {
        NewAnimCmd((unsigned char *)animcmd[anim].anim_cmd, animcmd[anim].len);
    }

    /* be sure the loading was successful... */
    if (!animation[anim].loaded)
    {
        LOG(LOG_DEBUG,"Error loading animation...%d\n",anim);
        return 0;
    }

    /* lets look if the sequence exists, not all anims have all sequences or mappings... */
    /* since we provide default mappings, this sanity check should never trigger */
    if (!animation[anim].aSeq[sequence])
    {
        LOG(LOG_DEBUG,"Error loading animation, sequence doesn't exist: a:%d, s:%d\n",anim, sequence);
        return 0;
    }

    /* now lets look if the wanted direction has an animation (eg. frames >0) */
    if (animation[anim].aSeq[sequence]->dirs[dir].frames<=0)
    {
        LOG(LOG_DEBUG,"Error loading animation, direction doesn't exist: a:%d, s:%d, d:%d\n",anim, sequence,dir);
        return 0;
    }

    /* ok, if we are here the animation exists, lets check if the faces for this animation are loaded, if not do it */
    if (!(animation[anim].aSeq[sequence]->dirs[dir].flags & ASEQ_DIR_LOADED))
    {
        uint8 i;

        animation[anim].aSeq[sequence]->dirs[dir].flags |= ASEQ_DIR_LOADED;

        for (i = 0; i < animation[anim].aSeq[sequence]->dirs[dir].frames; i++)
        {
            face_get(animation[anim].aSeq[sequence]->dirs[dir].faces[i]);
        }
    }

    return 1;
}


/**
 * resets the current animation to start again from the first frame
 */
void new_anim_reset(anim_list *al)
{
    if (!al)
        return;

    al->current_frame = 0;
    al->last_frame_tick = SDL_GetTicks();

    switch (al->objtype)
    {
        case ATYPE_TILE:
            ((struct MapCell *)al->obj)->faces[al->layer] = animation[al->animnum].aSeq[al->sequence]->dirs[al->dir].faces[0];
            map_redraw_flag = 1;
        break;
        case ATYPE_ITEM:
            ((item *)al->obj)->face = animation[al->animnum].aSeq[al->sequence]->dirs[al->dir].faces[0];
            //inv_redraw_flag = 1;
        break;
    }

    return;

}

/**
 * completely removes all animations from a MapCell, and the anim_list
 */
void new_anim_remove_tile_all(struct MapCell *mc)
{
    int i;

    for (i=0;i<MAXFACES;i++)
    {
        if (mc->anim[i])
            new_anim_remove(mc->anim[i]);

        mc->anim[i]=NULL;
    }

    return;

}

/**
 * removes the given animation from the mapcell
 */
void new_anim_remove_tile(anim_list *al)
{
    if (!al->obj)
    {
        LOG(LOG_DEBUG, "na_rem_single_anim_from_tile: error called with no tile-reference\n");
        return;
    }

    ((struct MapCell *)al->obj)->anim[al->layer]=NULL;
    new_anim_remove(al);

    return;
}


/**
 * high-level funtions to add an anim to an item, will set all types, linkings...
 * this also checks if the item has already an animation set, and if yes, only changes
 * the parameters. So its save to call these function more than one time on an item, and can
 * also be used to change an items animation. The advantage over using change... is you
 * don't have to take care that the animation already exits, if not its created.
 */
anim_list *new_anim_add_item(uint16 anim, uint8 sequence, uint8 dir, uint8 speed, item *it)
{

    anim_list *al=NULL;

    if (!it)
    {
        LOG(LOG_DEBUG,"na_add_item: no item given!\n");
        return NULL;
    }

    /* if there si already a anim set, we simple call new_anim_change... */
    if (it->anim)
    {
        al = it->anim;
        /* set new anim params, try to not restart the anim if possible */
        new_anim_change(al, anim, sequence, dir, speed, 0);

        return al;
    }
    /* dynamic loading */
    if (!new_anim_load_and_check(anim, sequence, dir))
    {
        LOG(LOG_DEBUG,"na_add_item: anim does not exist: a:%d, s:%d, d:%d\n",anim, sequence, dir);
        return NULL;
    }

    if (animation[anim].aSeq[sequence]->dirs[dir].frames==1)
    {
        it->face = animation[anim].aSeq[sequence]->dirs[dir].faces[0];
        return NULL;
    }

    al = new_anim_add(anim, sequence, dir, speed);

    if (!al)
        return NULL;

    al->obj=it;
    al->layer = 255;
    al->objtype = ATYPE_ITEM;
    it->anim=al;

    new_anim_reset(al);

    return al;
}

/**
 * high-level removing of animation from item, clears linkage...
 */
void new_anim_remove_item(item *it)
{
    if (it->anim)
        new_anim_remove(it->anim);
    it->anim=NULL;

    return;
}

/**
 * our main animation function
 * this will do the animations, set the faces in the objects, and set updateflags when needed
 */
void new_anim_animate(uint32 curTick)
{
#ifdef PROFILING
    Uint32 ts = SDL_GetTicks();
#endif
    uint8 got_map  = 0;
    uint8 got_item = 0;
#ifdef ANIM_FRAMESKIP
    uint32             lasttime;
    uint8 new_face = 0;
#endif
    anim_list *node = NULL;


#ifdef ANIM_FRAMESKIP
    for (node=AnimListStart;node;node=node->next)
    {
        if ((curTick-(node->last_frame_tick))<(uint32)options.anim_frame_time)
            continue;

        lasttime = node->last_frame_tick;

        while ((lasttime+=(uint32)((float)options.anim_frame_time *
                            (float)(animation[node->animnum].aSeq[node->sequence]->dirs[node->dir].delays[node->current_frame])
                            / ((float)node->speed/100.0f))) <= curTick)
        {
            new_face = 1;

            if (++(node->current_frame) >= animation[node->animnum].aSeq[node->sequence]->dirs[node->dir].frames)
                node->current_frame = 0;
        }

        if (new_face)
        {
            /* for exact animations we shift the last_time a bit */
            node->last_frame_tick = curTick - (lasttime-curTick);

            switch (node->objtype)
            {
                case ATYPE_TILE:
                    ((struct MapCell *)node->obj)->faces[node->layer] =
                        animation[node->animnum].aSeq[node->sequence]->dirs[node->dir].faces[node->current_frame];
                    got_map = 1;
                break;
                case ATYPE_ITEM:
                    ((item *)node->obj)->face =
                        animation[node->animnum].aSeq[node->sequence]->dirs[node->dir].faces[node->current_frame];
                    got_item = 1;
                break;
            }
        }
    }
#else
    for (node=AnimListStart;node;node=node->next)
    {
        /* eat that if! :) */
        if ((curTick-(node->last_frame_tick)) >
                ((options.anim_frame_time * animation[node->animnum].aSeq[node->sequence]->dirs[node->dir].delays[node->current_frame])
                / (node->speed/100.0f)))
        {

            /* TODO: for really slow clients: options to skip frame(s) when the last frame tick is to long ago */
            /* TODO: code flag which allows us to play an anim only once... */

            if (++(node->current_frame) >= animation[node->animnum].aSeq[node->sequence]->dirs[node->dir].frames)
                node->current_frame = 0;
            node->last_frame_tick = curTick;

            switch (node->objtype)
            {
                case ATYPE_TILE:
                    ((struct MapCell *)node->obj)->faces[node->layer] =
                        animation[node->animnum].aSeq[node->sequence]->dirs[node->dir].faces[node->current_frame];
                    got_map = 1;
                break;
                case ATYPE_ITEM:
                    ((item *)node->obj)->face =
                        animation[node->animnum].aSeq[node->sequence]->dirs[node->dir].faces[node->current_frame];
                    got_item = 1;
                break;

            }
        }
    }
#endif
    if (got_map)
        map_redraw_flag = 1;

#ifdef PROFILING
    LOG(LOG_MSG, "[Prof] new_anim_animate: %d\n",SDL_GetTicks() - ts);
#endif
    return;
}

void anim_init(void)
{
    uint16 i,
           num = 0;
    uint8  anim_cmd[MEDIUM_BUF];

    for (i = 0; i < MAXANIM; i++)
    {
        uint16 j;

        for (j = 0; j < MAX_SEQUENCES; j++)
        {
            if (animation[i].aSeq[j] && !(animation[i].aSeq[j]->flags & 0x80))
            {
                uint8 k;

                for (k = 0; k < 9; k++)
                {
                    if (!(animation[i].aSeq[j]->dirs[k].flags & ASEQ_MAPPED))
                    {
                        if (animation[i].aSeq[j]->dirs[k].faces)
                        {
                            FREE(animation[i].aSeq[j]->dirs[k].faces);
                        }

                        if (animation[i].aSeq[j]->dirs[k].delays)
                        {
                            FREE(animation[i].aSeq[j]->dirs[k].delays);
                        }
                    }
                }

                FREE(animation[i].aSeq[j]);
            }
        }

        FREE(animcmd[i].anim_cmd);
    }

    memset(animation, 0, sizeof(animation));
    memset(animcmd, 0, sizeof(animcmd));

    /* animation #0 is like face id #0 a bug catch - if ever
     * appear in game flow its a sign of a uninit of simply
     * buggy operation. */
    anim_cmd[num++] = (uint8)((0 >> 8) & 0xff);
    anim_cmd[num++] = (uint8)(0 & 0xff);
    anim_cmd[num++] = 0;  /* flags ... */
    anim_cmd[num++] = 0;  /* first sequencenum */
    anim_cmd[num++] = 0;  /* sequence flags */

    for (i = 0; i < 9; i++)
    {
        anim_cmd[num++] = i;    // dir x
        anim_cmd[num++] = 1;    // one frame
        anim_cmd[num++] = 0;    // face id 0 (2bytes)
        anim_cmd[num++] = 0;    // face id 0
        anim_cmd[num++] = 10;   // delay 10
        anim_cmd[num++] = 0xFF; // endmarker
    }

    /* this should be a real bugcatcher, so we set all sequences possible, of course we only reference them */
    for (i = 1; i < MAX_SEQUENCES; i++)
    {
        anim_cmd[num++] = i;    // sequence 1-max
        anim_cmd[num++] = 0x80; // setting map-flag
        anim_cmd[num++] = 0;    // seq num we map to
    }

    anim_cmd[num++] = 0xFF;
    MALLOC(animcmd[0].anim_cmd, num);
    memcpy(animcmd[0].anim_cmd, anim_cmd, num);
    animcmd[0].len = num;
}

/* lets do it analog like the old way
* This Command will most likely never called from the server, because we have our animations
* alredy got in the client_anims. This command is called from the client itself to load up the animations.
* This gives us the possibility to later let the server generate dynamic animations.
* Because this is mostly used client sided and for better understanding i didn't squeze out every bit and didn't optimize it.
* TODO: checks for reading beyond len!! <-- The SEGFAULT crash? :)
*/
void NewAnimCmd(unsigned char *data, int len)
{
    short animnum;
    uint8 sequence;
    uint8 dir;
    int pos=0, i;
    AnimSeq *as=NULL;
    int   seqmap[MAX_SEQUENCES];
    int   dirmap[9];

    for (i=0;i<MAX_SEQUENCES;i++)
        seqmap[i]=-1;

    memset(dirmap, -1, sizeof(dirmap));

    /* 2 Bytes animation number */
    animnum = (*(data+pos++) << 8);
    animnum |= *(data+pos++);

    if (animnum<0 || animnum >=MAXANIM)
    {
        LOG(LOG_DEBUG, "NewAnimCmd: animnum invalid: %d\n",animnum);
        return;
    }

    /* one byte global flags */
    animation[animnum].flags = *(data+pos++);

    /* one byte sequence number, 0xFF is the end marker */
    while ((sequence=*(data + pos++))!= 0xFF)
    {
        if (sequence >= MAX_SEQUENCES)
        {
            LOG(LOG_DEBUG, "NewAnimCmd: sequence invalid: %d\n", sequence);
            return;
        }
        MALLOC(as, sizeof(AnimSeq));
        if (!as)
        {
            LOG(LOG_DEBUG, "NewAnimCmd: out of memory allocating AnimSeq: %d (%d)\n",sequence, animnum);
            return;
        }
        animation[animnum].aSeq[sequence] = as;
        /* one byte flags */
        as->flags = *(data + pos++);

        /* if the highest bit in flags is set, this sequence is mapped to another sequence, the next byte tells us which */
        if (as->flags & 0x80)
        {
            uint8 mapseq;

            /* 1 Byte sequencenum which this sequence is mapped to */
            mapseq = *(data + pos++);
            if(mapseq >= MAX_SEQUENCES)
            {
                LOG(LOG_DEBUG, "NewAnimCmd: sequence invalid: %d\n", mapseq);
                return;
            }

            /* to map forward we need the pointer, which we get later, so lets save for now in a temp array */
            seqmap[sequence]=mapseq;
            /* thats all for this sequence */
            continue;
        }

        memset(dirmap, -1, sizeof(dirmap));

        /* now we load our directions */
        /* one byte dir number, 0xFF is the end marker */
        while ((dir=*(data + pos++))!= 0xFF)
        {
            if (dir & ASEQ_MAPPED) /*its mapped, next byte tells us to which */
            {
                dir &= 0x7F;
                if(dir >= 9)
                {
                    LOG(LOG_DEBUG,"NewAnimCmd: invalid direction %d\n", dir);
                    return;
                }
                dirmap[dir] = *(data + pos++);
                if (dirmap[dir]>8)
                {
                    LOG(LOG_DEBUG,"NewAnimCmd: dirmap to dir > 8, ignored!\n");
                    dirmap[dir]=-1;
                }
                continue;
            }
            dir &= 0x7F; /* preparation for dir mappings */
            if(dir >= 9)
            {
                LOG(LOG_DEBUG,"NewAnimCmd: invalid direction %d\n", dir);
                return;
            }

            as->dirs[dir].flags = 0;
            /* one byte frame count (255 frames for one sequence should be enough... */
            as->dirs[dir].frames = *(data + pos++);
            MALLOC(as->dirs[dir].faces, sizeof(uint16) * as->dirs[dir].frames);
            MALLOC(as->dirs[dir].delays, sizeof(uint8) * as->dirs[dir].frames);

            if (!as->dirs[dir].faces || !as->dirs[dir].delays)
            {
                LOG(LOG_DEBUG, "NewAnimCmd: out of memory allocating face/delay buf: d:%d s:%d a:%d\n",dir,sequence, animnum);
                return;
            }
            for (i = 0; i < as->dirs[dir].frames; i++)
            {
                as->dirs[dir].faces[i] = (*(data + pos++) << 8 );
                as->dirs[dir].faces[i] |= *(data + pos++) ;
                as->dirs[dir].delays[i] = *(data+pos++);
            }
        }
        /* now we do the dirmaps, when having a dirmap we copy the date of one dir to another,
        * the face and delay list is only a pointer, so we don't need more mem */
        for (i=0;i<9;i++)
        {
            if (dirmap[i]!=-1)
            {
                memcpy(&(as->dirs[i]), &(as->dirs[dirmap[i]]), sizeof(AnimSeqDir));
                as->dirs[i].flags |= ASEQ_MAPPED;
            }
        }
    }

/* I'm not entirely sure what the intended behaviour of this code is. However,
 * right now it can assign the same address to multiple pointers which all ends
 * up being freed later, producing undefined behaviour as a result of a double
 * free.
 * As a temporary measure, this code has been disabled for now in order to
 * avoid this issue. It doesn't seem to break anything, but probably needs to
 * be looked at when client-side animations is implemented.
 * -- Lippy 20100822 */
#if 0
    /* lets do the mappings */

    /* first the default mappings */
    for (i=1;i<MAX_SEQUENCES;i++)
    {
        if (!animation[animnum].aSeq[i])
        {
            animation[animnum].aSeq[i] = animation[animnum].aSeq[defaultmappings[i]];
        }
    }

    /* now overwrite the mappings whith the mappings from the arc */
    for (i=0;i<MAX_SEQUENCES;i++)
    {
        if (seqmap[i]!=-1)
        {
            if (!animation[animnum].aSeq[seqmap[i]])
            {
                LOG(LOG_DEBUG,"NewAnimCmd: SeqMap to non existing Seq: a:%d, curSeq: %d, wantedSeq: %d\n",animnum, i, seqmap[i]);
                continue;
            }
            animation[animnum].aSeq[i] = animation[animnum].aSeq[seqmap[i]];
        }
    }
#endif

    /* mark it as successful loaded */
    animation[animnum].loaded = 1;
    return;
}
