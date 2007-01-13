/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include "object_hero.h"
//================================================================================================
// Init all static Elemnts.
//================================================================================================
ObjectHero::ObjectHero()
{
    //new_player(0, "", 0, 0);
    fire_on = 0;
    firekey_on = 0;
    resize_twin = 0;
    resize_twin_marker = 0;
    run_on = runkey_on = 0;
    inventory_win = IWIN_BELOW;
    count_left = 0;
    container_tag = -996;
    container = 0;
    memset(&stats, 0, sizeof(Stats));
    stats.maxsp = 1;
    stats.maxhp = 1;
    gen_hp = 0.0f;
    gen_sp = 0.0f;
    gen_grace = 0.0f;
    target_hp = 0;
    stats.maxgrace = 1;
    stats.speed = 1;

    count_left = 0;
    stats.maxsp = 1;    /* avoid div by 0 errors */
    stats.maxhp = 1;    /* ditto */
    stats.maxgrace = 1; /* ditto */
    // ditto - displayed weapon speed is weapon speed/speed.
    stats.speed = 0;
    stats.weapon_sp = 0;

    last_command = "";
    input_text = "";
    alignment = "";
    gender = "";
    range = "";
    // this is set from title in stat cmd.
    pname = "";
    title = "";
    map_x = 0;
    map_y = 0;
    container_tag = -997;
    container = 0;
    magicmap = 0;

//    RangeFireMode = 0;
}
