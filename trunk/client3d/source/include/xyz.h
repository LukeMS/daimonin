/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef DEFINES_H
#define DEFINES_H

#include "logfile.h"
#include "network.h"

const char FILE_CLIENT_SPELLS[]   = "../../srv_files/client_spells";
const char FILE_CLIENT_SKILLS[]   = "../../srv_files/client_skills";
const char FILE_CLIENT_SETTINGS[] = "../../srv_files/client_settings";
const char FILE_CLIENT_BMAPS[]    = "../../srv_files/client_bmap";
const char FILE_CLIENT_ANIMS[]    = "../../srv_files/client_anims";
const char FILE_BMAPS_TMP[]       = "../../srv_files/bmaps.tmp";
const char FILE_ANIMS_TMP[]       = "..././srv_files/anims.tmp";
const char FILE_DAIMONIN_P0[]     = "./daimonin.p0";
const char FILE_BMAPS_P0[]        = "./bmaps.p0";


enum { M_MOVED, M_PRESSED, M_CLICKED, M_DRAGGED, M_ENTERED, M_EXITED, M_RELEASED };


#define MAXANIM 2000 

typedef struct Animations
{
    int     loaded;         /* 0= all fields are invalid, 1= anim is loaded */
    int     frame;          /* length of one a animation frame (num_anim/facings) */
    unsigned short *faces;
    unsigned char   facings;        /* number of frames */
    unsigned char   num_animations; /* number of animations.  Value of 2 means
                            * only faces[0],[1] have meaningfull values.
                            */
    unsigned char   flags;
} Animations;

typedef struct _anim_table
{
    int                 len;            /* len of anim_cmd data */
    char               *anim_cmd;       /* faked animation command */
}_anim_table;

extern _anim_table  anim_table[MAXANIM]; /* the stored "anim commands" we created out of anims.tmp */
extern Animations   animations[MAXANIM];
 


#define MAXANIM 2000 
extern _anim_table  anim_table[MAXANIM]; /* the stored "anim commands" we created out of anims.tmp */
extern Animations   animations[MAXANIM];
 


#define MAX_BMAPTYPE_TABLE 5000
#define BMAPTABLE 5003 /* prime nubmer for hash table */ 

#define MAXHASHSTRING 20 /* for hash table (bmap, ...) */ 
 
typedef struct  _bmaptype_table
{
    char           *name;
    int             pos;
    int             len;
    unsigned int    crc;
}_bmaptype_table;


extern int              bmaptype_table_size; 

typedef struct _bmaptype
{
    char               *name;
    int                 num;
    int                 len;
    int                 pos;
    unsigned int        crc;
}_bmaptype;

extern _bmaptype   *bmap_table[BMAPTABLE]; 

 
typedef struct _dialog_list_set
{
    int group_nr;
    int entry_nr;
    int class_nr;   /* for spell-list => spell, prayer, ... */
    int key_change;
}_dialog_list_set;

#define LIST_ENTRY_UNUSED -1 /* this entry is unused */
#define LIST_ENTRY_USED    1 /* entry is used but player don't have it */
#define LIST_ENTRY_KNOWN   2 /* player know this used entry */
#define LIST_NAME_MAX 64
#define DIALOG_LIST_ENTRY 26
#define OPTWIN_MAX_TABLEN 14

// spell list defines
#define SPELL_LIST_MAX 20        // groups of spells //
#define SPELL_LIST_CLASS 2

typedef struct _spell_list_entry
{
    int             flag;           /* -1: entry is unused */
    char            name[LIST_NAME_MAX];      /* name of entry */
    char            icon_name[32];
    struct _Sprite *icon;
    char            desc[4][96];               /* description (in 4 rows) */
}_spell_list_entry;

typedef struct _spell_list
{
    _spell_list_entry   entry[SPELL_LIST_CLASS][DIALOG_LIST_ENTRY];
}_spell_list;
 

#define LIST_ENTRY_UNUSED -1 /* this entry is unused */
#define LIST_ENTRY_USED    1 /* entry is used but player don't have it */
#define LIST_ENTRY_KNOWN   2 /* player know this used entry */
#define LIST_NAME_MAX 64
#define DIALOG_LIST_ENTRY 26
#define OPTWIN_MAX_TABLEN 14

/* skill list defines */
#define SKILL_LIST_MAX 7        /* groups of skills */

typedef struct _skill_list_entry
{
    int             flag;                   /* -1: entry is unused */
    char            name[LIST_NAME_MAX];   /* name of entry */
    char            icon_name[32];
    struct _Sprite *icon;
    char            desc[4][96];               /* description (in 4 rows) */
    int             exp_level;              /* -1: skill has no level or exp */
    int             exp;                    /* exp of this skill */
}_skill_list_entry;

typedef struct _skill_list
{
    _skill_list_entry   entry[DIALOG_LIST_ENTRY];
}_skill_list;
 


extern _dialog_list_set skill_list_set;

extern _dialog_list_set spell_list_set;
 




const int MAX_SKILL = 6;
 
typedef struct Stat_struct
{
    char           Str, Dex, Con, Wis, Cha, Int, Pow;
    char           wc, ac;     /* Weapon Class and Armour Class */
    char           level;
    short          hp;         /* Hit Points. */
    short          maxhp;
    short          sp;         /* Spell points.  Used to cast spells. */
    short          maxsp;      /* Max spell points. */
    short          grace;      /* Spell points.  Used to cast spells. */
    short          maxgrace;       /* Max spell points. */
    int            exp;            /* Experience.  Killers gain 1/10. */
    short          food;       /* How much food in stomach.  0 = starved. */
    short          dam;            /* How much damage this object does when hitting */
    int            speed;      /* Gets converted to a float for display*/
    int            weapon_sp;      /* Gets converted to a float for display */
    unsigned short flags;      /* contains fire on/run on flags */
    short          protection[20];     /* Resistant values */
    unsigned int   protection_change   : 1; /* Resistant value has changed */
    short          skill_level[MAX_SKILL];  /* Level and experience totals for */
    int            skill_exp  [MAX_SKILL];    /* skills */
} Stats;
 

void read_settings(void);
void read_spells(void);
void read_skills(void);
void read_anims(void);
bool read_bmaps(void); 
int read_anim_tmp(void);

#endif