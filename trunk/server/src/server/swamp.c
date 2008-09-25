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

void walk_on_deep_swamp(object *op, object *victim)
{
    if (victim->type == PLAYER && !IS_AIRBORNE(victim) && victim->stats.hp >= 0)
    {
        new_draw_info(NDI_UNIQUE, 0, victim, "You are down to your knees " "in the swamp.");
        op->stats.food = 1;
        victim->speed_left -= (float) (SLOW_PENALTY(op));
    }
}

void move_deep_swamp(object *op)
{
    object *above   = op->above;
    object *nabove;

    while (above)
    {
        nabove = above->above;
        if (above->type == PLAYER && !IS_AIRBORNE(above) && above->stats.hp >= 0)
        {
            if (op->stats.food < 1)
            {
                LOG(llevDebug, "move_deep_swamp(): player is here, but state is %d\n", op->stats.food);
                op->stats.food = 1;
            }
            switch (op->stats.food)
            {
                case 1:
                  if (random_roll(0, 2) == 0)
                  {
                      new_draw_info(NDI_UNIQUE, 0, above, "You are down to your waist in the wet swamp.");
                      op->stats.food = 2;
                      above->speed_left -= (float) (SLOW_PENALTY(op));
                  }
                  break;
                case 2:
                  if (random_roll(0, 2) == 0)
                  {
                      new_draw_info(NDI_UNIQUE, 0, above, "You are down to your NECK in the dangerous swamp.");
                      op->stats.food = 3;
                      FREE_AND_ADD_REF_HASH(CONTR(above)->killer, shstr_cons.drowning);
                      above->stats.hp--;
                      above->speed_left -= (float) (SLOW_PENALTY(op));
                  }
                  break;
                case 3:
                  if (random_roll(0, 4) == 0)
                  {
                      /* player is ready to drown - only woodsman skill can save him */
                      if (random_roll(0, 4) == 0 || !change_skill(above, SK_WOODSMAN))
                      {
                          op->stats.food = 0;
                          new_draw_info_format(NDI_UNIQUE | NDI_ALL, 1, NULL, "%s disappeared into a swamp.",
                                               above->name);
                          FREE_AND_ADD_REF_HASH(CONTR(above)->killer, shstr_cons.drowning);

                          above->stats.hp = -1;
                          kill_player(above); /* player dies in the swamp */
                      }
                      else
                      {
                          op->stats.food = 2;
                          new_draw_info(NDI_UNIQUE, 0, above, "You almost drowned in the swamp! You");
                          new_draw_info(NDI_UNIQUE, 0, above, "survived due to your woodsman skill.");
                      }
                  }
                  break;
            }
        }
        else if (!IS_LIVE(above))
        {
            if (random_roll(0, 2) == 0)
                decrease_ob(above);
        }
        above = nabove;
    }
}
