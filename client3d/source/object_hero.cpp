/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#include "object_hero.h"
//================================================================================================
// Init all static Elemnts.
//================================================================================================
ObjectHero::ObjectHero()
{
    fire_on = firekey_on = 0;
    run_on = runkey_on = 0;
    resize_twin = resize_twin_marker = 0;
    inventory_win = IWIN_BELOW;
    count_left = 0;
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
    stats.maxsp = 1;    // avoid div by 0 errors.
    stats.maxhp = 1;    // avoid div by 0 errors.
    stats.maxgrace = 1; // avoid div by 0 errors.
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
    magicmap = 0;
//    RangeFireMode = 0;
}
