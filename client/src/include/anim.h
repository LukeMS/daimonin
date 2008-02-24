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

    Client-side animation stuff, 2008 Alderan

*/
#if !defined(__ANIM_H)
#define __ANIM_H

/* This defines our maximum animation sequences/stances we support
 * ATM we only have idle, walking and attacking
 * This must be increased as we add new sequences like bow, death...
 */
#define MAX_SEQUENCES   3

/* This is the maximum possible sequences number the code support, 32 should be really enough */
#define MAX_CODE_SEQUENCES   32

#if MAX_SEQUENCES > MAX_CODE_SEQUENCES
#error "The code only supports a maximum of 32 anim sequences"
#endif

#define ASEQ_NUM_MASK   0x1F

/* we will have 3 flags, which currently are unused, but maybe later we need them
 * like a flag to play an anim-sequence only one time, and stay on the last frame...
 */
#define ASEQ_FLAG_1     0x80
#define ASEQ_FLAG_2     0x40
#define ASEQ_FLAG_3     0x20

#define DEFAULT_ANIM_DELAY   4

#define ASEQ_DIR_RESET  0x01
#define ASEQ_DIR_LOADED 0x01
#define ASEQ_MAPPED     0x80

extern uint8 defaultmappings[MAX_SEQUENCES];

typedef enum animobjtype
{
    ATYPE_NO = 0,
    ATYPE_TILE,
    ATYPE_ITEM,

    ATYPE_END
} animobjtype;

typedef enum sequencetype
{
    ASEQ_NO         = -1,
    ASEQ_IDLE       = 0,
    ASEQ_WALK       = 1,
    ASEQ_ATTACK     = 2,

    ASEQ_END
} sequencetype;

typedef struct AnimSeqDir
{
    uint8   frames;
    uint16  *faces;
    uint8   *delays;
    uint8   flags;         /* some flags, like loaded...*/
}
AnimSeqDir;

typedef struct AnimSeq
{
    AnimSeqDir  dirs[9];    /* our 9 directions */
    uint8   flags;          /* flags, most important will be the ASEQ_DIR_RESET flag, which tells the client
                             * that he cannot change directions WITHOUT the need of restarting the anim */
}
AnimSeq;


typedef struct Animations
{
    AnimSeq *aSeq[MAX_SEQUENCES]; /* pointer to our sequence structs */
    uint8   flags;                  /* flags...*/
    Boolean loaded;               /* is basic animation loaded? */
}
Animations;

extern Animations    animation[MAXANIM];


typedef struct AnimCmds
{
    int                 len;            /* len of anim_cmd data */
    char               *anim_cmd;       /* faked animation command */
} AnimCmds;

extern AnimCmds      animcmd[MAXANIM];


typedef struct anim_list
{
    struct anim_list   *next;
    uint8               current_frame;      /* our currently shown frame */
    uint16              last_frame_tick;    /* time of the last frame shown */
    uint8               speed;              /* speed _delta_ normally 100(%) means normal speed */
    uint8               dir;                /* our current direction we use */
    sint8               sequence;           /* currently active sequence */
    uint16              animnum;            /* the animation number */
    void               *obj;                /* pointer to the object we animate */
    uint8               layer;              /* layer on map we animate */
    uint8               objtype;            /* what type of object we habe to animate */

}
anim_list;

extern anim_list    *AnimListStart;


extern void         new_anim_remove(anim_list *al); /* deletes the anim from our animation list */
extern anim_list *  new_anim_add(uint16 anim, uint8 sequence, uint8 dir, uint8 speed); /* adds a new admin */
extern anim_list *  new_anim_add_tile(uint16 anim, uint8 sequence, uint8 dir, uint8 speed, int x, int y, uint8 layer);
extern void         new_anim_change(anim_list *al, uint16 anim, uint8 sequence, uint8 dir, uint8 speed, Boolean restart); /* modify all parameters */
extern void         new_anim_reset(anim_list *al);
extern void         new_anim_remove_tile_all(struct MapCell *mc);
extern void         new_anim_remove_tile(anim_list *al);
extern void         new_anim_remove_item(item *it);
extern anim_list *  new_anim_add_item(uint16 anim, uint8 sequence, uint8 dir, uint8 speed, item *it);
extern Boolean      new_anim_load_and_check(uint16 anim, uint8 sequence, uint8 dir);

extern void         new_anim_animate(uint32 curTick);

extern void         create_anim_tmp();
extern int          load_anim_tmp(void);





#endif
