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

#include <stdio.h>
#include <global.h>

/* real exp of a mob: base_exp * lev_exp[mob->level] */
float lev_exp[MAXLEVEL+1]= {
0.0f,
1.0f, 1.11f, 1.75f,3.2f,							/* normal needed mobs zo level (exp 100): 20,20.22.8 */
5.3f,	9.6f, 17.0f, 31.25f,58.1f,			/* 25, 30, 33 , 36, 40 */
88.8f, 104.1f, 120.0f,120.1f,140.0f,		/* 43 45 - 50 */
160.0f, 160.1f, 160.2f, 160.3f, 180.0f,
180.0f,180.0f,180.0f, 200.0f, 200.0f,
200.0f, 200.0f, 220.0f, 220.0f, 220.0f,
220.0f, 220.0f,220.0f, 240.0f, 240.0f,
240.0f,240.0f,240.0f,240.0f,240.0f,
240.0f, 260.0f,260.0f,260.0f,260.0f,
260.0f,260.0f,260.0f,260.0f,260.0f,
280.0f,280.0f,280.0f,280.0f,280.0f,
280.0f,280.0f,280.0f,280.0f,280.0f,
280.0f,280.0f,280.0f,280.0f,280.0f,
300.0f,300.0f,300.0f,300.0f,300.0f,
300.0f,300.0f,300.0f,300.0f,300.0f,
300.0f,300.0f,300.0f,300.0f,300.0f,
300.0f,300.0f, 320.0f, 320.0f, 320.f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f, 320.0f, 320.f,320.0f, 320.0f,
320.0f
};


uint32 new_levels[MAXLEVEL+2]={
0,
0,2000,4000, 8000,
16000,32000,64000,125000,250000,		/* 9 */
500000,900000,1400000,2000000,2600000,
3300000,4100000,4900000,5700000,6600000,	/* 19 */
7500000,8400000,9300000,10300000,11300000,
12300000,13300000,14400000,15500000,16600000,	/* 29 */
17700000,18800000,19900000,21100000,22300000,	
23500000,24700000,25900000,27100000,28300000,	/* 39 */
29500000,30800000,32100000,33400000,34700000,
36000000,37300000,38600000,39900000,41200000,	/* 49 */
42600000,44000000,45400000,46800000,48200000,
49600000,51000000,52400000,53800000,55200000,	/* 59 */
56600000,58000000,59400000,60800000,62200000,
63700000,65200000,66700000,68200000,69700000,	/* 69 */
71200000,72700000,74200000,75700000,77200000,
78700000,80200000,81700000,83200000,84700000,	/* 79 */
86200000,87700000,89300000,90900000,92500000,
94100000,95700000,97300000,98900000,100500000,	/* 89 */
102100000,103700000,105300000,106900000,108500000,
110100000,111700000,113300000,114900000,116500000,	/* 99 */
118100000,119700000,121300000,122900000,124500000, 	
126100000,127700000,129300000,130900000,785400000,
1570800000,1570800000	/* 110 */
};

float exp_att_mult[NROFATTACKS+2] = {
0.0f,				/* AT_PHYSICAL	*/
0.0f,				/* AT_MAGIC	*/
0.0f,				/* AT_FIRE	*/
0.0f,				/* AT_ELECTRICITY */
0.0f,				/* AT_COLD	*/
0.0f,				/* AT_WATER	*/ /*AT_CONFUSION!*/
0.4f,				/* AT_ACID	*/
1.5f,				/* AT_DRAIN	*/
0.0f,				/* AT_WEAPONMAGIC */
0.1f,				/* AT_GHOSTHIT	*/
0.3f,				/* AT_POISON	*/
0.2f,				/* AT_DISEASE	*/
0.3f,				/* AT_PARALYZE	*/
0.0f,				/* AT_TIME */
0.0f,				/* AT_FEAR	*/
0.0f,				/* AT_CANCELLATION */
0.0f,				/* AT_DEPLETE */
0.0f,				/* AT_DEATH */
0.0f,				/* AT_CHAOS */
0.0f				/* AT_COUNTERSPELL */
};

float exp_prot_mult[NROFATTACKS+2] = {
0.4f,				/* AT_PHYSICAL	*/
0.5f,				/* AT_MAGIC	*/
0.1f,				/* AT_FIRE	*/
0.1f,				/* AT_ELECTRICITY */
0.1f,				/* AT_COLD	*/
0.1f,				/* AT_WATER	*/
0.1f,				/* AT_ACID	*/
0.1f,				/* AT_DRAIN	*/
0.1f,				/* AT_WEAPONMAGIC */
0.1f,				/* AT_GHOSTHIT	*/
0.1f,				/* AT_POISON	*/
0.1f,				/* AT_DISEASE	*/
0.1f,				/* AT_PARALYZE	*/
0.1f,				/* AT_TIME */
0.1f,				/* AT_FEAR	*/
0.0f,				/* AT_CANCELLATION */
0.0f,				/* AT_DEPLETE */
0.0f,				/* AT_DEATH */
0.0f,				/* AT_CHAOS */
0.0f				/* AT_COUNTERSPELL */

};

/*
 * Returns true if the monster specified has any innate abilities.
 */

int has_ability(object *ob) {
  object *tmp;

  for(tmp=ob->inv;tmp!=NULL;tmp=tmp->below)
    if(tmp->type==ABILITY||tmp->type==SPELLBOOK) 
      return TRUE;

  return FALSE;
}
