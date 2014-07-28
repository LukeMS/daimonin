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

    The author can be reached via e-mail to info@daimonin.org
*/
/* TODO: Reference only. Will be removed. */
#if 0
///*  the contents of this file were create solely by peterm@soda.berkeley.edu
//    all of the above disclaimers apply.  */
//
//#include <global.h>
//#ifdef NO_ERRNO_H
//extern int      errno;
//#else
//#   include <errno.h>
//#endif
//
//#ifdef sequent
///* stoopid sequent includes don't do this like they should */
//extern char    *sys_errlist[];
//extern int      sys_nerr;
//#endif
//
//void dead_player(object *op)
//{
//    char    filename[MEDIUM_BUF];
//    char    newname[MEDIUM_BUF];
//    char    path[MEDIUM_BUF];
//
//    /*  set up our paths/strings...  */
//    sprintf(path, "%s/%s/%s/%s/%s", settings.localdir, settings.playerdir, get_subdir(op->name), op->name, op->name);
//
//    strcpy(filename, path);
//    strcat(filename, ".pl");
//    strcpy(newname, filename);
//    strcat(newname, ".dead");
//
//    if (!rename(filename, newname))
//        LOG(llevBug, "BUG: dead_player(): rename error (%s) (%s)\n", filename, newname);
//}
//
//
//
//
///*  raise_dead by peterm and mehlhaff@soda.berkeley.edu  */
///*  *op  --  who is doing the resurrecting
//    dir  --  direction the spell is cast
//    spell_type  --  which spell was cast
//*/
//int cast_raise_dead_spell(object *op, int dir, int spell_type, object *corpse)
//{
//    uint8 failed = 0;
//
//    if (!op)
//    {
//        return 0;
//    }
//
//    if (!corpse)
//    {
//        mapstruct  *m;
//        int         xt = op->x + freearr_x[dir],
//                    yt = op->y + freearr_y[dir];
//
//        if ((m = out_of_map(op->map, &xt, &yt)))
//        {
//            object *this;
//
//            for (this = GET_MAP_OB(m, xt, yt); this; this = this->above)
//            {
//                /* Remove checks for immunity - bit of a hack. Anyways, only
//                 * the CORPSE type is being used corpses for players, so this
//                 * check should be sufficient. If we really want to be sure, we
//                 * could probably check the archetype or something. */
//                if (this->type == CORPSE)
//                {
//                    break;
//                }
//            }
//
//            if (!this)
//            {
//                new_draw_info(NDI_UNIQUE, 0, op, "You need a body for this spell.");
//
//                return 0;
//            }
//            else
//            {
//                corpse = this;
//            }
//        }
//    }
//
//    /* No matter what, we fry the corpse. */
//    if (corpse->map) // FIXME: implying it might be env? if so it won't be removed
//    {
//        object *effect = arch_to_object(find_archetype("burnout"));
//
//        if (effect)
//        {
//            effect->x = corpse->x;
//            effect->y = corpse->y;
//            insert_ob_in_map(effect, corpse->map, op, 0);
//        }
//
//        remove_ob(corpse);
//        check_walk_off(corpse, NULL, MOVE_APPLY_VANISHED);
//    }
//
//    switch (spell_type)
//    {
//        case SP_RAISE_DEAD:
//          /*  see if this spell fails, if so then summon some
//              undead. levels[1] is important, see below*/
//          if (resurrection_fails(op->level, corpse->level))
//          {
//              summon_hostile_monsters(op, 5, "demon");
//              summon_hostile_monsters(op, 3, "skull");
//              failed = 1;
//          }
//
//        case SP_RESURRECTION:
//          if (resurrection_fails(op->level, corpse->level))
//          {
//              summon_hostile_monsters(op, 5, "skull");
//              summon_hostile_monsters(op, 3, "lich");
//              failed = 1;
//          }
//
//        case SP_REINCARNATION:
//          if (resurrection_fails(op->level, 0))
//          {
//              summon_hostile_monsters(op, 5, "lich");
//              summon_hostile_monsters(op, 3, "demilich");
//              summon_hostile_monsters(op, 1, "spectre");
//              failed = 1;
//          }
//
//          default:
//              return 0;
//    }
//
//    if (!failed)
//    {
//        return resurrect_player(op, (char *)corpse->name, spell_type);
//    }
//    else
//    {
//        return 1;
//    }
//}
//
//
//int resurrection_fails(int levelcaster, int leveldead)
//{
//    int chance  = 9;
//    /*  scheme:  equal in level, 50% success.
//        +5 % for each level below, -5% for each level above.
//        minimum 20%
//        */
//    chance += levelcaster - leveldead;
//    if (chance < 4)
//        chance = 4;
//    if (chance > random_roll(0, 19))
//        return 0;  /* resurrection succeeds */
//    return 1;
//}
//
//
///*  name of the person to resurrect and which spell was used
//to resurrect  */
//int resurrect_player(object *op, char *playername, int rspell)
//{
//    FILE           *deadplayer, *liveplayer;
//
//    char            oldname[MEDIUM_BUF];
//    char            newname[MEDIUM_BUF];
//    char            path[MEDIUM_BUF];
//    char            buf[MEDIUM_BUF];
//    char            buf2[MEDIUM_BUF];
//
//    static char    *races[] =
//    {
//        "barbarian", "cleric", "elf", "human", "mage", "ninja", "priest", "swashbuckler", "thief", "viking", "warrior",
//        "wizard"
//    };
//
//    long int        exp;
//    int             Con;
//
//    /*  set up our paths/strings...  */
//    sprintf(path, "%s/%s/%s/%s/%s", settings.localdir, settings.playerdir, get_subdir(op->name), playername, playername);
//
//    strcpy(newname, path);
//    strcat(newname, ".pl");
//
//    strcpy(oldname, newname);
//    strcat(oldname, ".dead");
//
//    if (!(deadplayer = fopen(oldname, "r")))
//    {
//        new_draw_info(NDI_UNIQUE, 0, op, "The soul of %s cannot be reached.", playername);
//        return 0;
//    }
//
//    if (!access(newname, 0))
//    {
//        new_draw_info(NDI_UNIQUE, 0, op, "The soul of %s has already been reborn!", playername);
//        fclose(deadplayer);
//        return 0;
//    }
//
//    if (!(liveplayer = fopen(newname, "w")))
//    {
//        new_draw_info(NDI_UNIQUE, 0, op, "The soul of %s cannot be re-embodied at the moment.", playername);
//        LOG(llevBug, "BUG: Cannot write player in ressurect_player!\n");
//        fclose(deadplayer);
//        return 0;
//    }
//
//    while (!feof(deadplayer))
//    {
//        char *dummy; // purely to suppress GCC's warn_unused_result warning
//
//        dummy = fgets(buf, 255, deadplayer);
//        sscanf(buf, "%s", buf2);
//        if (!(strcmp(buf2, "exp")))
//        {
//            sscanf(buf, "%s %ld", buf2, &exp);
//            switch (rspell)
//            {
//                case SP_RAISE_DEAD:
//                  exp -= exp / 5;
//                  break;
//                case SP_RESURRECTION:
//                  exp -= exp / 10;
//                  break;
//                case SP_REINCARNATION:
//                  exp -= exp / 20;
//                  break;
//            }
//            sprintf(buf, "exp %ld\n", exp);
//        }
//        if (!(strcmp(buf2, "Con")))
//        {
//            sscanf(buf, "%s %d", buf2, &Con);
//            switch (rspell)
//            {
//                case SP_RAISE_DEAD:
//                  Con -= 2;
//                  break;
//                case SP_RESURRECTION:
//                  Con -= 1;
//                  break;
//            }
//            sprintf(buf, "Con %d\n", Con);
//        }
//        if (rspell == SP_REINCARNATION)
//        {
//            if (!(strcmp(buf2, "race")))
//            {
//                sprintf(buf, "race %s\n", races[random_roll(1, 12)]);
//            }
//        }
//        fputs(buf, liveplayer);
//    }
//    fclose(liveplayer);
//    fclose(deadplayer);
//    unlink(oldname);
//    new_draw_info(NDI_UNIQUE, 0, op, "%s lives again!", playername);
//
//    return 1;
//}
//
//
//void dead_character(char *name)
//{
//    char    buf[MEDIUM_BUF];
//    char    buf2[MEDIUM_BUF];
//
//    sprintf(buf, "%s/%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, get_subdir(name), name, name);
//    /*  peterm:  create a .dead filename....  ***.pl.dead  */
//    strcpy(buf2, buf);
//    strcat(buf, ".dead");
//    if (rename(buf2, buf) == -1)
//        LOG(llevDebug, "Crossfire character rename to dead: %s --", buf);
//}
//
//
//int dead_player_exists(char *name)
//{
//    char    buf[MEDIUM_BUF];
//
//    sprintf(buf, "%s/%s/%s/%s/%s", settings.localdir, settings.playerdir, get_subdir(name), name, name);
//    strcat(buf, ".pl.dead");
//    return !(access(buf, 0));
//}
#endif
