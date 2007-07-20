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

/* there are some more exp calc funtions in skill_util.c - but thats part
 * of /server/server and not of crosslib.a - so we can't move then easily
 * on this place - another reason to kill the crosslib.a asap.
 */

#include <global.h>
#include <stdio.h>


float           lev_exp[MAXLEVEL + 1]       =
{
    0.0f, 1.0f, 1.11f, 1.75f, 3.2f, 5.5f, 10.0f, 20.0f, 35.25f, 66.1f, 137.0f, 231.58f, 240.00f, 247.62f, 254.55f,
    260.87f, 266.67f, 272.00f, 276.92f, 281.48f, 285.71f, 289.66f, 293.33f, 296.77f, 300.00f, 303.03f, 305.88f, 308.57f,
    311.11f, 313.51f, 315.79f, 317.95f, 320.00f, 321.95f, 323.81f, 325.58f, 327.27f, 328.89f, 330.43f, 331.91f, 333.33f,
    334.69f, 336.00f, 337.25f, 338.46f, 339.62f, 340.74f, 341.82f, 342.86f, 343.86f, 344.83f, 345.76f, 346.67f, 347.54f,
    348.39f, 349.21f, 350.00f, 350.77f, 351.52f, 352.24f, 352.94f, 353.62f, 354.29f, 354.93f, 355.56f, 356.16f, 356.76f,
    357.33f, 357.89f, 358.44f, 358.97f, 359.49f, 360.00f, 360.49f, 360.98f, 361.45f, 361.90f, 362.35f, 362.79f, 363.22f,
    363.64f, 364.04f, 364.44f, 364.84f, 365.22f, 365.59f, 365.96f, 366.32f, 366.67f, 367.01f, 367.35f, 367.68f, 368.00f,
    368.32f, 368.63f, 368.93f, 369.23f, 369.52f, 369.81f, 370.09f, 370.37f, 370.64f, 370.91f, 371.17f, 371.43f, 371.68f,
    371.93f, 372.17f, 372.41f, 372.65f, 372.88f
};

/* around level 11 you need 38+(2*(your_level-11)) yellow
 * mobs with a base exp of 125 to level up.
 * Every level >11 needs 100.000 exp more as the one before but
 * also one mob more to kill.
 * This avoid things like: "you geht 342.731.123 exp from this mob,
 * you have now 1.345.535.545.667 exp."
 * even here we have around 500.000.000 max exp - thats a pretty big
 * number.
 */
sint32          new_levels[MAXLEVEL + 2]    =
{
    0, 0, 1500, 4000, 8000, 16000, 32000, 64000, 125000, 250000,        /* 9 */
    500000, 1100000, 2300000, 3600000, 5000000, 6500000, 8100000, 9800000, 11600000, 13500000, /* 19 */
    15500000, 17600000, 19800000, 22100000, 24500000, 27000000, 29600000, 32300000, 35100000, 38000000, /* 29 */
    41000000, 44100000, 47300000, 50600000, 54000000, 57500000, 61100000, 64800000, 68600000, 72500000, /* 39 */
    76500000, 80600000, 84800000, 89100000, 93500000, 98000000, 102600000, 107300000, 112100000, 117000000, /* 49 */
    122000000, 127100000, 132300000, 137600000, 143000000, 148500000, 154100000, 159800000, 165600000, 171500000,
    /*59 */
    177500000, 183600000, 189800000, 196100000, 202500000, 209000000, 215600000, 222300000, 229100000, 236000000,
    /* 69 */
    243000000, 250100000, 257300000, 264600000, 272000000, 279500000, 287100000, 294800000, 302600000, 310500000,
    /* 79 */
    318500000, 326600000, 334800000, 343100000, 351500000, 360000000, 368600000, 377300000, 386100000, 395000000,
    /* 89 */
    404000000, 413100000, 422300000, 431600000, 441000000, 450500000, 460100000, 469800000, 479600000, 489500000,
    /* 99 */
    499500000, 509600000, 519800000, 530100000, 540500000, 551000000, 561600000, 572300000, 583100000, 594000000,
    /* 109 */
    605000000, 700000000 /* 111 is only a dummy */
};

_level_color    level_color[201]        =
{
    { - 2, -1, 0, 1, 2, 3}, /* lvl 0 */
    { - 1, 0, 1, 2, 3, 4}, /* lvl 1 */
    { 0, 1, 2, 3, 4, 5}, /* lvl 2 */
    { 1, 2, 3, 4, 5, 6}, /* lvl 3 */
    { 2, 3, 4, 5, 6, 7}, /* lvl 4 */
    { 3, 4, 5, 6, 7, 8}, /* lvl 5 */
    { 4, 5, 6, 7, 8, 9}, /* lvl 6 */
    { 5, 6, 7, 8, 9, 10}, /* lvl 7 */
    { 6, 7, 8, 9, 10, 11}, /* lvl 8 */
    { 7, 8, 9, 10, 11, 12}, /* lvl 9 */
    { 7, 9, 10, 11, 12, 14}, /* lvl 10 */
    { 8, 9, 11, 12, 13, 15}, /* lvl 11 */
    { 9, 10, 12, 13, 14, 16}, /* lvl 12 */
    { 9, 11, 13, 14, 15, 17}, /* lvl 13 */
    { 10, 11, 14, 15, 16, 18}, /* lvl 14 */
    { 11, 12, 15, 16, 17, 19}, /* lvl 15 */
    { 11, 13, 16, 17, 18, 20}, /* lvl 16 */
    { 12, 14, 17, 18, 19, 21}, /* lvl 17 */
    { 13, 15, 18, 19, 20, 22}, /* lvl 18 */
    { 14, 16, 19, 20, 21, 23}, /* lvl 19 */
    { 14, 17, 20, 21, 22, 24}, /* lvl 20 */
    { 15, 17, 21, 22, 24, 26}, /* lvl 21 */
    { 16, 18, 22, 23, 25, 27}, /* lvl 22 */
    { 16, 19, 23, 24, 26, 28}, /* lvl 23 */
    { 17, 19, 24, 25, 27, 30}, /* lvl 24 */
    { 18, 20, 25, 26, 28, 31}, /* lvl 25 */
    { 19, 21, 26, 27, 29, 32}, /* lvl 26 */
    { 19, 22, 27, 28, 30, 33}, /* lvl 27 */
    { 20, 23, 28, 29, 31, 35}, /* lvl 28 */
    { 21, 24, 29, 30, 32, 36}, /* lvl 29 */
    { 22, 25, 30, 31, 33, 37}, /* lvl 30 */
    { 22, 25, 31, 32, 34, 38}, /* lvl 31 */
    { 23, 26, 32, 33, 35, 39}, /* lvl 32 */
    { 24, 27, 32, 35, 37, 41}, /* lvl 33 */
    { 25, 28, 33, 36, 38, 42}, /* lvl 34 */
    { 25, 28, 34, 37, 39, 43}, /* lvl 35 */
    { 26, 29, 35, 38, 40, 44}, /* lvl 36 */
    { 27, 30, 36, 39, 41, 45}, /* lvl 37 */
    { 28, 31, 37, 40, 42, 46}, /* lvl 38 */
    { 28, 32, 38, 41, 44, 48}, /* lvl 39 */
    { 29, 33, 39, 42, 45, 49}, /* lvl 40 */
    { 30, 34, 40, 43, 46, 50}, /* lvl 41 */
    { 30, 34, 41, 44, 47, 52}, /* lvl 42 */
    { 31, 35, 42, 45, 48, 53}, /* lvl 43 */
    { 32, 36, 43, 46, 49, 54}, /* lvl 44 */
    { 33, 37, 44, 47, 50, 55}, /* lvl 45 */
    { 33, 37, 45, 48, 51, 57}, /* lvl 46 */
    { 34, 38, 46, 49, 52, 58}, /* lvl 47 */
    { 35, 39, 47, 50, 53, 59}, /* lvl 48 */
    { 36, 40, 48, 51, 54, 60}, /* lvl 49 */
    { 36, 41, 49, 52, 55, 61}, /* lvl 50 */
    { 37, 42, 50, 53, 56, 62}, /* lvl 51 */
    { 38, 43, 51, 54, 57, 63}, /* lvl 52 */
    { 38, 43, 52, 55, 58, 65}, /* lvl 53 */
    { 39, 44, 53, 56, 59, 66}, /* lvl 54 */
    { 40, 45, 54, 57, 60, 67}, /* lvl 55 */
    { 41, 46, 55, 58, 61, 68}, /* lvl 56 */
    { 41, 47, 56, 59, 63, 70}, /* lvl 57 */
    { 42, 48, 57, 60, 64, 71}, /* lvl 58 */
    { 43, 49, 58, 61, 65, 72}, /* lvl 59 */
    { 44, 50, 59, 62, 66, 73}, /* lvl 60 */
    { 44, 50, 60, 63, 67, 75}, /* lvl 61 */
    { 45, 51, 61, 64, 68, 76}, /* lvl 62 */
    { 46, 52, 62, 65, 69, 77}, /* lvl 63 */
    { 47, 53, 63, 66, 70, 78}, /* lvl 64 */
    { 47, 53, 64, 67, 71, 79}, /* lvl 65 */
    { 48, 54, 64, 69, 73, 81}, /* lvl 66 */
    { 49, 55, 65, 70, 74, 82}, /* lvl 67 */
    { 50, 56, 66, 71, 75, 83}, /* lvl 68 */
    { 50, 56, 67, 72, 76, 84}, /* lvl 69 */
    { 51, 57, 68, 73, 77, 85}, /* lvl 70 */
    { 52, 58, 69, 74, 78, 86}, /* lvl 71 */
    { 53, 59, 70, 75, 79, 87}, /* lvl 72 */
    { 53, 60, 71, 76, 80, 89}, /* lvl 73 */
    { 54, 61, 72, 77, 81, 90}, /* lvl 74 */
    { 55, 62, 73, 78, 82, 91}, /* lvl 75 */
    { 56, 63, 74, 79, 83, 92}, /* lvl 76 */
    { 56, 63, 75, 80, 85, 94}, /* lvl 77 */
    { 57, 64, 76, 81, 86, 95}, /* lvl 78 */
    { 58, 65, 77, 82, 87, 96}, /* lvl 79 */
    { 59, 66, 78, 83, 88, 97}, /* lvl 80 */
    { 59, 67, 79, 84, 89, 99}, /* lvl 81 */
    { 60, 68, 80, 85, 90, 100}, /* lvl 82 */
    { 61, 69, 81, 86, 91, 101}, /* lvl 83 */
    { 62, 70, 82, 87, 92, 102}, /* lvl 84 */
    { 62, 70, 83, 88, 93, 103}, /* lvl 85 */
    { 63, 71, 84, 89, 94, 104}, /* lvl 86 */
    { 64, 72, 85, 90, 95, 105}, /* lvl 87 */
    { 65, 73, 86, 91, 96, 106}, /* lvl 88 */
    { 65, 73, 87, 92, 97, 108}, /* lvl 89 */
    { 66, 74, 88, 93, 98, 109}, /* lvl 90 */
    { 67, 75, 89, 94, 99, 110}, /* lvl 91 */
    { 68, 76, 90, 95, 100, 111}, /* lvl 92 */
    { 69, 77, 91, 96, 101, 112}, /* lvl 93 */
    { 69, 78, 92, 97, 103, 114}, /* lvl 94 */
    { 70, 79, 93, 98, 104, 115}, /* lvl 95 */
    { 71, 80, 94, 99, 105, 116}, /* lvl 96 */
    { 72, 81, 95, 100, 106, 117}, /* lvl 97 */
    { 72, 81, 96, 101, 107, 119}, /* lvl 98 */
    { 73, 82, 96, 103, 109, 120}, /* lvl 99 */
    { 74, 83, 97, 104, 110, 121}, /* lvl 100 */
    { 75, 84, 98, 105, 111, 122}, /* lvl 101 */
    { 75, 84, 99, 106, 112, 124}, /* lvl 102 */
    { 76, 85, 100, 107, 113, 125}, /* lvl 103 */
    { 77, 86, 101, 108, 114, 126}, /* lvl 104 */
    { 78, 87, 102, 109, 115, 127}, /* lvl 105 */
    { 79, 88, 103, 110, 116, 128}, /* lvl 106 */
    { 79, 89, 104, 111, 117, 129}, /* lvl 107 */
    { 80, 90, 105, 112, 118, 130}, /* lvl 108 */
    { 81, 91, 106, 113, 119, 131}, /* lvl 109 */
    { 82, 92, 107, 114, 120, 132}, /* lvl 110 */
    { 82, 92, 108, 115, 121, 134}, /* lvl 111 */
    { 83, 93, 109, 116, 122, 135}, /* lvl 112 */
    { 84, 94, 110, 117, 123, 136}, /* lvl 113 */
    { 85, 95, 111, 118, 124, 137}, /* lvl 114 */
    { 86, 96, 112, 119, 125, 138}, /* lvl 115 */
    { 86, 96, 113, 120, 126, 140}, /* lvl 116 */
    { 87, 97, 114, 121, 127, 141}, /* lvl 117 */
    { 88, 98, 115, 122, 128, 142}, /* lvl 118 */
    { 89, 99, 116, 123, 129, 143}, /* lvl 119 */
    { 90, 100, 117, 124, 130, 144}, /* lvl 120 */
    { 90, 101, 118, 125, 132, 146}, /* lvl 121 */
    { 91, 102, 119, 126, 133, 147}, /* lvl 122 */
    { 92, 103, 120, 127, 134, 148}, /* lvl 123 */
    { 93, 104, 121, 128, 135, 149}, /* lvl 124 */
    { 94, 105, 122, 129, 136, 150}, /* lvl 125 */
    { 94, 105, 123, 130, 137, 151}, /* lvl 126 */
    { 95, 106, 124, 131, 138, 152}, /* lvl 127 */
    { 96, 107, 125, 132, 139, 153}, /* lvl 128 */
    { 97, 108, 126, 133, 140, 154}, /* lvl 129 */
    { 97, 109, 127, 134, 141, 156}, /* lvl 130 */
    { 98, 110, 128, 135, 142, 157}, /* lvl 131 */
    { 99, 110, 128, 137, 144, 158}, /* lvl 132 */
    { 100, 111, 129, 138, 145, 159}, /* lvl 133 */
    { 101, 112, 130, 139, 146, 160}, /* lvl 134 */
    { 101, 113, 131, 140, 147, 162}, /* lvl 135 */
    { 102, 114, 132, 141, 148, 163}, /* lvl 136 */
    { 103, 115, 133, 142, 149, 164}, /* lvl 137 */
    { 104, 116, 134, 143, 150, 165}, /* lvl 138 */
    { 105, 117, 135, 144, 151, 166}, /* lvl 139 */
    { 106, 118, 136, 145, 152, 167}, /* lvl 140 */
    { 106, 118, 137, 146, 153, 169}, /* lvl 141 */
    { 107, 119, 138, 147, 154, 170}, /* lvl 142 */
    { 108, 120, 139, 148, 155, 171}, /* lvl 143 */
    { 109, 121, 140, 149, 156, 172}, /* lvl 144 */
    { 110, 122, 141, 150, 157, 173}, /* lvl 145 */
    { 110, 122, 142, 151, 159, 175}, /* lvl 146 */
    { 111, 123, 143, 152, 160, 176}, /* lvl 147 */
    { 112, 124, 144, 153, 161, 177}, /* lvl 148 */
    { 113, 125, 145, 154, 162, 178}, /* lvl 149 */
    { 114, 126, 146, 155, 163, 179}, /* lvl 150 */
    { 114, 127, 147, 156, 164, 180}, /* lvl 151 */
    { 115, 128, 148, 157, 165, 181}, /* lvl 152 */
    { 116, 129, 149, 158, 166, 182}, /* lvl 153 */
    { 117, 130, 150, 159, 167, 183}, /* lvl 154 */
    { 118, 131, 151, 160, 168, 184}, /* lvl 155 */
    { 119, 132, 152, 161, 169, 185}, /* lvl 156 */
    { 119, 132, 153, 162, 170, 187}, /* lvl 157 */
    { 120, 133, 154, 163, 171, 188}, /* lvl 158 */
    { 121, 134, 155, 164, 172, 189}, /* lvl 159 */
    { 122, 135, 156, 165, 173, 190}, /* lvl 160 */
    { 123, 136, 157, 166, 174, 191}, /* lvl 161 */
    { 123, 137, 158, 167, 175, 193}, /* lvl 162 */
    { 124, 138, 159, 168, 176, 194}, /* lvl 163 */
    { 125, 139, 160, 169, 177, 195}, /* lvl 164 */
    { 126, 139, 160, 171, 179, 196}, /* lvl 165 */
    { 127, 140, 161, 172, 180, 197}, /* lvl 166 */
    { 128, 141, 162, 173, 181, 198}, /* lvl 167 */
    { 128, 142, 163, 174, 182, 200}, /* lvl 168 */
    { 129, 143, 164, 175, 183, 201}, /* lvl 169 */
    { 130, 144, 165, 176, 184, 202}, /* lvl 170 */
    { 131, 145, 166, 177, 185, 203}, /* lvl 171 */
    { 132, 146, 167, 178, 186, 204}, /* lvl 172 */
    { 133, 147, 168, 179, 187, 205}, /* lvl 173 */
    { 133, 147, 169, 180, 189, 207}, /* lvl 174 */
    { 134, 148, 170, 181, 190, 208}, /* lvl 175 */
    { 135, 149, 171, 182, 191, 209}, /* lvl 176 */
    { 136, 150, 172, 183, 192, 210}, /* lvl 177 */
    { 137, 151, 173, 184, 193, 211}, /* lvl 178 */
    { 138, 152, 174, 185, 194, 212}, /* lvl 179 */
    { 139, 153, 175, 186, 195, 213}, /* lvl 180 */
    { 139, 153, 176, 187, 196, 214}, /* lvl 181 */
    { 140, 154, 177, 188, 197, 215}, /* lvl 182 */
    { 141, 155, 178, 189, 198, 216}, /* lvl 183 */
    { 142, 156, 179, 190, 199, 217}, /* lvl 184 */
    { 143, 157, 180, 191, 200, 218}, /* lvl 185 */
    { 144, 158, 181, 192, 201, 219}, /* lvl 186 */
    { 144, 159, 182, 193, 202, 221}, /* lvl 187 */
    { 145, 160, 183, 194, 203, 222}, /* lvl 188 */
    { 146, 161, 184, 195, 204, 223}, /* lvl 189 */
    { 147, 162, 185, 196, 205, 224}, /* lvl 190 */
    { 148, 163, 186, 197, 206, 225}, /* lvl 191 */
    { 149, 164, 187, 198, 207, 226}, /* lvl 192 */
    { 150, 165, 188, 199, 208, 227}, /* lvl 193 */
    { 150, 165, 189, 200, 209, 229}, /* lvl 194 */
    { 151, 166, 190, 201, 210, 230}, /* lvl 195 */
    { 152, 167, 191, 202, 211, 231}, /* lvl 196 */
    { 153, 168, 192, 203, 212, 232}, /* lvl 197 */
    { 154, 169, 192, 205, 214, 233}, /* lvl 198 */
    { 155, 170, 193, 206, 215, 234}, /* lvl 199 */
    { 156, 171, 194, 207, 216, 235} /* lvl 200 */
};

/*
   Since this is nowhere defined ...
   Both come in handy at least in function add_exp()
*/
#define MAX_EXPERIENCE  new_levels[MAXLEVEL]


/* init_new_exp_system() - called in init(). This function will reconfigure
 * the properties of skills array and experience categories, and links
 * each skill to the appropriate experience category
 * -b.t. thomas@astro.psu.edu
 */

void init_new_exp_system()
{
    static int  init_new_exp_done   = 0;

    if (init_new_exp_done)
        return;
    init_new_exp_done = 1;

    read_skill_params(); /* overide skills[] w/ values from skill_params */
}


/* add_exp() new algorithm. Revamped experience gain/loss routine.
 * Based on the old add_exp() function - but tailored to add experience
 * to experience objects. The way this works-- the code checks the
 * current skill readied by the player (chosen_skill) and uses that to
 * identify the appropriate experience object. Then the experience in
 * the object, and the player's overall score are updated. In the case
 * of exp loss, all exp categories which have experience are equally
 * reduced. The total experience score of the player == sum of all
 * exp object experience.  - b.t. thomas@astro.psu.edu
 */
/* The old way to determinate the right skill which is used for exp gain
 * was broken. Best way to show this is, to cast some fire balls in a mob
 * and then changing the hand weapon some times. You will get some "no
 * ready skill warnings".
 * I reworked the whole system and the both main exp gain and add functions
 * add_exp() and adjust_exp(). Its now much faster, easier and more accurate. MT
 * exp lose by dead is handled from apply_death_exp_penalty().
 */
sint32 add_exp(object *op, int exp, int skill_nr)
{
    object *exp_ob      = NULL;    /* the exp. object into which experience will go */
    object *exp_skill   = NULL; /* the real skill object */
    /*    int del_exp=0; */
    int     limit       = 0;

    /*LOG(llevBug,"ADD: add_exp() called for $d!\n", exp); */
    /* safety */
    if (!op)
    {
        LOG(llevBug, "BUG: add_exp() called for null object!\n");
        return 0;
    }

    if (op->type != PLAYER)
        return 0; /* no exp gain for mobs */


    /* ok, we have global exp gain or drain - we must grap a skill for it! */
    if (skill_nr == CHOSEN_SKILL_NO)
    {
        /* TODO: select skill */
        LOG(llevDebug, "TODO: add_exp(): called for %s with exp %d. CHOSEN_SKILL_NO set. TODO: select skill.\n",
            query_name(op), exp);
        return 0;
    }

    /* now we grap the skill exp. object from the player shortcut ptr array */
    exp_skill = CONTR(op)->skill_ptr[skill_nr];

    if (!exp_skill) /* safety check */
    {
        LOG(llevDebug, "DEBUG: add_exp(): called for %s with skill nr %d / %d exp - object has not this skill.\n",
            query_name(op), skill_nr, exp);
        return 0;
    }

    /* if we are full in this skill, then nothing is to do */
    if (exp_skill->level >= MAXLEVEL)
        return 0;

    CONTR(op)->update_skills = 1; /* we will sure change skill exp, mark for update */
    exp_ob = exp_skill->exp_obj;

    if (!exp_ob)
    {
        LOG(llevBug, "BUG: add_exp() skill:%s - no exp_op found!!\n", query_name(exp_skill));
        return 0;
    }

    /* General adjustments for playbalance */
    /* I set limit to 1/4 of a level - thats enormous much */
    limit = (new_levels[exp_skill->level + 1] - new_levels[exp_skill->level]) / 4;
    if (exp > limit)
        exp = limit;

    exp = adjust_exp(op, exp_skill, exp);   /* first we see what we can add to our skill */

    /* adjust_exp has adjust the skill and all exp_obj and player exp */
    /* now lets check for level up in all categories */
    player_lvl_adj(op, exp_skill, TRUE);
    player_lvl_adj(op, exp_ob, TRUE);
    player_lvl_adj(op, NULL, TRUE);

    /* reset the player exp_obj to NULL */
    /* I let this in but because we use single skill exp and skill nr now,
     * this broken exp_obj concept can be removed
     */
    if (op->exp_obj)
        op->exp_obj = NULL;

	fix_player(op, "add_exp" );
    return (sint32) exp; /* thats the real exp we have added to our skill */
}


/* player_lvl_adj() - for the new exp system. we are concerned with
 * whether the player gets more hp, sp and new levels.
 * -b.t.
 */
void player_lvl_adj(object *who, object *op, int flag_msg)
{
    char    buf[MAX_BUF];

    SET_FLAG(who, FLAG_NO_FIX_PLAYER);
    if (!op)        /* when rolling stats */
        op = who;

    if (op->type == SKILL && !op->last_eat) /* no exp gain for indirect skills */
    {
        LOG(llevBug, "BUG: player_lvl_adj() called for indirect skill %s (who: %s)\n", query_name(op), query_name(who));
        CLEAR_FLAG(who, FLAG_NO_FIX_PLAYER);
        return;
    }

    if (op->level < MAXLEVEL && op->stats.exp >= GET_LEVEL_EXP(op->level + 1))
    {
        op->level++;

        /* show the player some effects... */
        if (op->type == SKILL && who && who->type == PLAYER && who->map)
        {
            object *effect_ob;

            play_sound_player_only(CONTR(who), SOUND_LEVEL_UP, SOUND_NORMAL, 0, 0);

            if (level_up_arch)
            {
                /* prepare effect */
                effect_ob = arch_to_object(level_up_arch);
                effect_ob->map = who->map;
                effect_ob->x = who->x;
                effect_ob->y = who->y;

                if (!insert_ob_in_map(effect_ob, effect_ob->map, NULL, INS_NO_MERGE | INS_NO_WALK_ON))
                {
                    /* something is wrong - kill object */
                    if (!QUERY_FLAG(effect_ob, FLAG_REMOVED))
                        remove_ob(effect_ob); /* check off not needed */
                }
            }
        }

        if (op == who && op->stats.exp > 1 && is_dragon_pl(who))
            dragon_level_gain(who);


        if (who && who->type == PLAYER && op->type != EXPERIENCE && op->type != SKILL && who->level > 1)
        {
            if (who->level > 4)
                CONTR(who)->levhp[who->level] = (char) ((RANDOM() % who->arch->clone.stats.maxhp) + 1);
            else if (who->level > 2)
                CONTR(who)->levhp[who->level] = (char) ((RANDOM() % (who->arch->clone.stats.maxhp / 2)) + 1)
                                              + (who->arch->clone.stats.maxhp / 2);
            else
                CONTR(who)->levhp[who->level] = (char) who->arch->clone.stats.maxhp;
        }
        if (op->level > 1 && op->type == EXPERIENCE)
        {
            if (who && who->type == PLAYER)
            {
                if (op->stats.Pow) /* mana */
                {
                    if (op->level > 4)
                        CONTR(who)->levsp[op->level] = (char) ((RANDOM() % who->arch->clone.stats.maxsp) + 1);
                    else
                        CONTR(who)->levsp[op->level] = (char) who->arch->clone.stats.maxsp;
                }
                else if (op->stats.Wis) /* grace */
                {
                    if (op->level > 4)
                        CONTR(who)->levgrace[op->level] = (char) ((RANDOM() % who->arch->clone.stats.maxgrace) + 1);
                    else
                        CONTR(who)->levgrace[op->level] = (char) who->arch->clone.stats.maxgrace;
                }
            }
            if(flag_msg)
            {
                sprintf(buf, "You are now level %d in %s based skills.", op->level, op->name);
                new_draw_info(NDI_UNIQUE | NDI_RED, 0, who, buf);
            }
        }
        else if (flag_msg && op->level > 1 && op->type == SKILL)
        {
            sprintf(buf, "You are now level %d in the skill %s.", op->level, op->name);
            new_draw_info(NDI_UNIQUE | NDI_RED, 0, who, buf);
        }
        else if(flag_msg)
        {
            sprintf(buf, "You are now level %d.", op->level);
            new_draw_info(NDI_UNIQUE | NDI_RED, 0, who, buf);
        }

        player_lvl_adj(who, op, flag_msg); /* To increase more levels */
    }
    else if (op->level > 1 && op->stats.exp < (sint32) GET_LEVEL_EXP(op->level))
    {
        op->level--;

        if(flag_msg)
        {
            if (op->type == EXPERIENCE)
            {
                sprintf(buf, "-You are now level %d in %s based skills.", op->level, op->name);
                new_draw_info(NDI_UNIQUE | NDI_RED, 0, who, buf);
            }
            else if (op->type == SKILL)
            {
                sprintf(buf, "-You are now level %d in the skill %s.", op->level, op->name);
                new_draw_info(NDI_UNIQUE | NDI_RED, 0, who, buf);
            }
            else
            {
                sprintf(buf, "-You are now level %d.", op->level);
                new_draw_info(NDI_UNIQUE | NDI_RED, 0, who, buf);
            }
        }
        player_lvl_adj(who, op, flag_msg); /* To decrease more levels */
    }
    CLEAR_FLAG(who, FLAG_NO_FIX_PLAYER);
}


/* adjust_exp() - make sure that we don't exceed max or min set on
 * experience
 * I use this function now as global experience add and sub routine.
 * it should only called for the skills object from players.
 * This function adds or subs the exp and updates all skill objects and
 * the player global exp.
 * You need to call player_lvl_adj() after it.
 * This routine use brute force and goes through the whole inventory. We should
 * use a kind of skill container for speed this up. MT
 */
int adjust_exp(object *pl, object *op, int exp)
{
    object *tmp;
    int     i, sk_nr;
    sint32  sk_exp, pl_exp;


    /* be sure this is a skill object from a active player */
    if (op->type != SKILL || !pl || pl->type != PLAYER)
    {
        LOG(llevBug, "BUG: adjust_exp() - called for non player or non skill: skill: %s -> player: %s\n",
            query_name(op), query_name(pl));
        return 0;
    }

    /* add or sub the exp and cap it. it must be >=0 and <= MAX_EXPERIENCE */
    op->stats.exp += exp;
    if (op->stats.exp < 0)
    {
        exp -= op->stats.exp;
        op->stats.exp = 0;
    }

    if (op->stats.exp > (sint32) MAX_EXPERIENCE)
    {
        exp = exp - (op->stats.exp - MAX_EXPERIENCE);
        op->stats.exp = MAX_EXPERIENCE;
    }

    /* now we collect the exp of all skills which are in the same exp. object category */
    sk_nr = skills[op->stats.sp].category;
    sk_exp = 0;
    /* this is the old collection system  - all skills of a exp group add
     * we changed that to "best skill count"
     */
    /*
       for(tmp=pl->inv;tmp;tmp=tmp->below)
       {
           if(tmp->type==SKILL && skills[tmp->stats.sp].category == sk_nr)
           {
               if((sk_exp += tmp->stats.exp) > (sint32)MAX_EXPERIENCE)
                   sk_exp = MAX_EXPERIENCE;
           }
       }
    */
    for (tmp = pl->inv; tmp; tmp = tmp->below)
    {
        if (tmp->type == SKILL && skills[tmp->stats.sp].category == sk_nr)
        {
            if (tmp->stats.exp > sk_exp)
                sk_exp = tmp->stats.exp;
        }
    }
    /* set the exp of the exp. object to our best skill of this group */
    op->exp_obj->stats.exp = sk_exp;

    /* now we collect all exp. objects exp */
    pl_exp = 0;

    for (i = 0; i < NROFSKILLGROUPS_ACTIVE; i++)
    {
        if (CONTR(pl)->exp_obj_ptr[i]->stats.exp > pl_exp)
            pl_exp = CONTR(pl)->exp_obj_ptr[i]->stats.exp;
    }

    /* last action: set our player exp to highest group */
    pl->stats.exp = pl_exp;

    return exp; /* return the actual amount changed stats.exp we REALLY have added to our skill */
}

/* we are now VERY friendly - but not because we want. With the
 * new sytem, we never lose level, just % of the exp we gained for
 * the next level. Why? Because dropping the level on purpose by
 * dying again & again will allow under some special circumstances
 * rich players to use exploits.
 * This here is newbie friendly and it allows to make the higher
 * level simply harder. By losing increased levels at high levels
 * you need at last to make recover easy. Now you will not lose much
 * but it will be hard in any case to get exp in high levels.
 * This is a just a design adjustment.
 */
void apply_death_exp_penalty(object *op)
{
    object *tmp;
    float   loss_p;
    long    level_exp, loss_exp;

    CONTR(op)->update_skills = 1; /* we will sure change skill exp, mark for update */
    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        /* only adjust skills with level and a positive exp value - negative exp has special meaning */
        if (tmp->type == SKILL && tmp->level && tmp->last_eat == 1)
        {
            /* first, lets check there are exp we can drain. */
            level_exp = tmp->stats.exp - new_levels[tmp->level];
            if (level_exp < 0) /* just a sanity check */
                LOG(llevBug, " DEATH_EXP: Skill %s (%d %d) for player %s -> less exp as level need!\n", query_name(tmp),
                    tmp->level, tmp->stats.exp, query_name(op));
            if (!level_exp)
                continue;

            if (tmp->level < 2)
            {
                loss_exp = level_exp - (int) ((float) level_exp * 0.9);
            }
            else if (tmp->level < 3)
            {
                loss_exp = level_exp - (int) ((float) level_exp * 0.85);
            }
            else
            {
                loss_p = 0.927f - (((float) tmp->level / 5.0f) * 0.00337f);
                loss_exp = (new_levels[tmp->level + 1] - new_levels[tmp->level])
                         - (int) ((float) (new_levels[tmp->level + 1] - new_levels[tmp->level]) * loss_p);
            }

            if (loss_exp < 0)
                loss_exp = 0;
            if (loss_exp > level_exp)
                loss_exp = level_exp;

            /* again some sanity checks */
            if (loss_exp > 0)
            {
                adjust_exp(op, tmp, -loss_exp);
                player_lvl_adj(op, tmp, FALSE);
            }
        }
    }

    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        if (tmp->type == EXPERIENCE && tmp->stats.exp)
            player_lvl_adj(op, tmp, FALSE); /* adjust exp objects levels */
    }
    player_lvl_adj(op, NULL, FALSE);        /* and at last adjust the player level */
    FIX_PLAYER(op, "apply death penalty - end");
}

/* i reworked this...
 * We will get a mali or boni value here except the level match exactly.
 * Yellow means not always exactly same level but "in equal range".
 * If the target is in yellow range, the exp mul is between 0.8 and 1.1 (80%-110%)
 * If the target is in blue the exp is 40-60%. (0.4-0.6)
 * if the mob is green the range is between 25%-30% (0.25 to 0.3)
 * For orange mobs, the exp is between 1.2 and 1.4 .
 * For all over orange  its 1.4 + 0.1% per level.
 */
float calc_level_difference(int who_lvl, int op_lvl)
{
    int     r;
    float   v, tmp = 1.0f;

    /* some sanity checks */
    if (who_lvl<0 || who_lvl>200 || op_lvl<0 || op_lvl>200)
    {
        LOG(llevBug, "Calc_level:: Level out of range! (%d - %d)\n", who_lvl, op_lvl);
        return 0.0f;
    }
    if (op_lvl < level_color[who_lvl].green) /* grey */
        return 0.0f;


    if (who_lvl > op_lvl) /* op is perhaps yellow, blue or green */
    {
        if (op_lvl >= level_color[who_lvl].yellow)
        {
            r = who_lvl - level_color[who_lvl].yellow;
            if (r < 1)
                r = 1;
            v = 0.2f / (float) r;
            tmp = 1.0f - (v * (float) (who_lvl - op_lvl));
        }
        else if (op_lvl >= level_color[who_lvl].blue)
        {
            r = level_color[who_lvl].yellow - level_color[who_lvl].blue;
            if (r < 1)
                r = 1;
            v = 0.3f / (float) r;
            tmp = 0.4f + (v * (float) (op_lvl - level_color[who_lvl].blue + 1));
        }
        else /* green */
        {
            r = level_color[who_lvl].blue - level_color[who_lvl].green;
            if (r < 1)
                r = 1;
            v = 0.05f / (float) r;
            tmp = 0.25f + (v * (float) (op_lvl - level_color[who_lvl].green + 1));
        }
    }
    else if (who_lvl < op_lvl) /* check for orange - if red/purple use 1,6 + 0.1% per level */
    {
        if (op_lvl < level_color[who_lvl].orange) /* still yellow */
        {
            r = level_color[who_lvl].orange - who_lvl - 1;
            if (r < 1)
                r = 1;
            v = 0.1f / (float) r;
            tmp = 1.0f + (v * (float) (op_lvl - who_lvl));
        }
        else if (op_lvl < level_color[who_lvl].red) /* op is orange */
        {
            r = level_color[who_lvl].red - who_lvl - 1;
            if (r < 1)
                r = 1;
            v = 0.2f / (float) r;
            tmp = 1.2f + (v * (float) (op_lvl - who_lvl));
        }
        else /* red or purple! */
        {
            r = (op_lvl + 1) - level_color[who_lvl].red;
            v = 0.1f * (float) r;
            tmp = 1.4f + v;
        }
    }

    return tmp;
}

/* calc_skill_exp() - calculates amount of experience can be gained for
 * successfull use of a skill.  Returns value of experience gain.
 * If level is == -1, we get the used skill from (player) who.
 */
int calc_skill_exp(object *who, object *op, float mod, int level, int *real)
{
    int     who_lvl = level;
    int     op_exp = 0, op_lvl = 0;
    float   exp_mul, max_mul, tmp;

    if (!who || who->type != PLAYER) /* no exp for non players... its senseless to do */
    {
        LOG(llevDebug, "DEBUG: calc_skill_exp() called with who != PLAYER or NULL (%s (%s)- %s)\n", query_name(who),
            !who ? "NULL" : "", query_name(op));
        return 0;
    }

    if(level == -1)
        who_lvl = SK_level(who); /* thats the releated skill level */

    if (!op) /* hm.... */
    {
        LOG(llevBug, "BUG: calc_skill_exp() called with op == NULL (%s - %s)\n", query_name(who), query_name(op));
        op_lvl = who->map->difficulty < 1 ? 1 : who->map->difficulty;
        op_exp = 0;
    }
    else if (op->type == RUNE)
    {
        op_exp = 0;
        op_lvl = op->level;
    }
    else /* all other items/living creatures */
    {
        op_exp = op->stats.exp; /* get base exp */
        op_lvl = op->level;
    }

    if (op_lvl < 1 || op_exp < 1)
        return 0; /* no exp for no level and no exp ;) */

    if (who_lvl < 2)
        max_mul = 0.85f;
    else if (who_lvl < 3)
        max_mul = 0.7f;
    else if (who_lvl < 4)
        max_mul = 0.6f;
    else if (who_lvl < 5)
        max_mul = 0.45f;
    else if (who_lvl < 7)
        max_mul = 0.35f;
    else if (who_lvl < 8)
        max_mul = 0.3f;
    else
        max_mul = 0.25f;

    /* we get first a global level difference mulitplicator */
    exp_mul = calc_level_difference(who_lvl, op_lvl);
    op_exp = (int) (((float) op_exp * lev_exp[op_lvl] * mod)* exp_mul);
    if(real != NULL)
    {
        if(*real > 0)
            op_exp = (int)((float)(*real)*mod);
        else if(*real == 0)
            *real = op_exp;
    }
    /*LOG(-1,"real exp = %d\n",op_exp);*/
    tmp = ((float) (new_levels[who_lvl + 1] - new_levels[who_lvl]) * 0.1f) * max_mul;
    if ((float) op_exp > tmp)
    {
        /*LOG(-1,"exp to high(%d)! adjusted to: %d",op_exp, (int)tmp);*/
        op_exp = (int) tmp;
    }

    return op_exp;
}


