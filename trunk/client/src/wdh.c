/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.

    Copyright (C) 2012 Michael Toennies

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

    The author can be reached via e-mail at info@daimonin.org
*/

/* This temporary module provides the handlers for each widget's data.
 * -- Smacky 20120409 */

#include "include.h"
#include <math.h>

static struct PdollPos_t
{
    sint16 type;
    sint16 x;
    sint16 y;
}
PdollPos[] =
{
    { TYPE_ARMOUR,       93,  44 },
    { TYPE_HELMET,       93,   5 },
    { TYPE_LEGS,         93, 122 },
    { TYPE_BOOTS,        93, 161 },
    { TYPE_WEAPON,       50,  92 },
    { TYPE_SHIELD,      135,  92 },
    { TYPE_RING,         50, 131 }, // r
    { TYPE_RING,        135, 131 }, // l
    { TYPE_BRACERS,      54,  48 },
    { TYPE_CLOAK,        44,   7 },
    { TYPE_AMULET,        5,   7 },
    { TYPE_BOW,         141,   7 },
    { TYPE_WAND,        141,   7 },
    { TYPE_ROD,         141,   7 },
    { TYPE_HORN,        141,   7 },
    { TYPE_ARROW,       180,   7 },
    { TYPE_GIRDLE,       93,  83 },
    { TYPE_GLOVES,      180, 113 },
    { TYPE_SHOULDER,    132,  48 },
    { TYPE_LIGHT_APPLY,   5, 113 },

    { -1,                -1,  -1 }
};

static struct GroupPos_t
{
    sint16 x;
    sint16 y;
}
GroupPos[GROUP_MAX_MEMBER] =
{
    { 5,  29 },
    { 5,  53 },
    { 5,  77 },
    { 5, 101 },
    { 5, 125 },
    { 5, 149 }
};

static void ShowIcons(sint16 ox, sint16 oy, sint16 x, sint16 y, uint8 invxlen, uint8 invylen,
                      inventory_win_t iwin, _BLTFX *bltfx);
static void PrintInfo(sint16 x, sint16 y, item *ip, inventory_win_t iwin);

/* Process handlers. */

void wdh_process_pinfo(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;
    char   buf[SMALL_BUF];

    SDL_FillRect(widget_data[id].surface, NULL, NDI_COLR_HOTPINK);

    /* [Rank] Name. */
    if (cpl.rank[0])
    {
        sprintf(buf, "%s %s", cpl.rank, cpl.pname);
    }
    else
    {
        sprintf(buf, "%s", cpl.pname);
    }

    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 6, y + 2,
               skin_prefs.widget_title, NULL);

    /* Gender Race Title? */
    sprintf(buf, "%s %s %s", cpl.gender, cpl.race, cpl.title);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 6, y + 14,
               skin_prefs.widget_info, NULL);

    /* Alignment [God]. */
    if (strcmp(cpl.godname, "none"))
    {
        sprintf(buf, "%s follower of %s", cpl.alignment, cpl.godname);
    }
    else
    {
        sprintf(buf, "%s", cpl.alignment);
    }

    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 6, y + 26,
               skin_prefs.widget_info, NULL);

    /* Rest button. */
    sprite_blt(skin_sprites[SKIN_SPRITE_PRAY], x + 184, y + 5, NULL, NULL);
}

void wdh_process_stats(widget_id_t id)
{
    sint16   x = widget_data[id].x1,
             y = widget_data[id].y1;
    char     buf[SMALL_BUF];
    SDL_Rect box;

    if (WIDGET_REDRAW(id))
    {
        SDL_Surface *surface = widget_data[id].surface;
        _BLTFX       bltfx;
        uint16       x2;

        WIDGET_REDRAW(id) = 0;
        bltfx.surface = surface;
        bltfx.flags = 0;
        bltfx.dark_level = 0;
        bltfx.alpha = 0;
        SDL_FillRect(surface, NULL, NDI_COLR_HOTPINK);

        /* Primary stats */
        strout_blt(surface, &font_tiny, STROUT_LEFT, "Stats", 8, 1,
                   skin_prefs.widget_title, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Str", 8, 17,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%02d", cpl.stats.Str);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 30, 17,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Dex", 8, 28,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%02d", cpl.stats.Dex);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 30, 28,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Con", 8, 39,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%02d", cpl.stats.Con);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 30, 39,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Int", 8, 50,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%02d", cpl.stats.Int);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 30, 50,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Wis", 8, 61,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%02d", cpl.stats.Wis);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 30, 61,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Pow", 8, 72,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%02d", cpl.stats.Pow);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 30, 72,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Cha", 8, 83,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%02d", cpl.stats.Cha);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 30, 83,
                   skin_prefs.widget_valueEq, NULL);

        /* Health indicators */
        strout_blt(surface, &font_small, STROUT_LEFT, "HP", 58, 10,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%c%06x%d%c / %c%06x%d",
                ECC_NEWCOLR,
                percentage_colr((float)cpl.stats.hp / (float)cpl.stats.maxhp * 100),
                cpl.stats.hp, ECC_DEFCOLR,
                ECC_NEWCOLR, skin_prefs.widget_valueHi, cpl.stats.maxhp);
        strout_blt(surface, &font_small, STROUT_LEFT, buf,
                   160 - strout_width(&font_small, buf), 10,
                   skin_prefs.widget_valueEq, NULL);
        sprite_blt(skin_sprites[SKIN_SPRITE_HP_BACK], 57, 23, NULL, &bltfx);
        strout_blt(surface, &font_small, STROUT_LEFT, "Mana", 58, 35,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%c%06x%d%c / %c%06x%d",
                ECC_NEWCOLR,
                percentage_colr((float)cpl.stats.sp / (float)cpl.stats.maxsp * 100),
                cpl.stats.sp, ECC_DEFCOLR,
                ECC_NEWCOLR, skin_prefs.widget_valueHi, cpl.stats.maxsp);
        strout_blt(surface, &font_small, STROUT_LEFT, buf,
                   160 - strout_width(&font_small, buf), 35,
                   skin_prefs.widget_valueEq, NULL);
        sprite_blt(skin_sprites[SKIN_SPRITE_SP_BACK], 57, 47, NULL, &bltfx);
        strout_blt(surface, &font_small, STROUT_LEFT, "Grace", 58, 59,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%c%06x%d%c / %c%06x%d",
                ECC_NEWCOLR,
                percentage_colr((float)cpl.stats.grace / (float)cpl.stats.maxgrace * 100),
                cpl.stats.grace, ECC_DEFCOLR,
                ECC_NEWCOLR, skin_prefs.widget_valueHi, cpl.stats.maxgrace);
        strout_blt(surface, &font_small, STROUT_LEFT, buf,
                   160 - strout_width(&font_small, buf), 59,
                   skin_prefs.widget_valueEq, NULL);
        sprite_blt(skin_sprites[SKIN_SPRITE_GRACE_BACK], 57, 71, NULL, &bltfx);
        sprite_blt(skin_sprites[SKIN_SPRITE_FOOD_BACK], 87, 88, NULL, &bltfx);

        if (cpl.stats.food)
        {
            sint16 food;
            uint16 bar;
            float  fraction;

            if ((food = cpl.stats.food) < 1)
            {
                food *= -1;
                bar = SKIN_SPRITE_FOOD2;
                strout_blt(surface, &font_small, STROUT_LEFT, "Food", 58, 84,
                           skin_prefs.widget_key, NULL);
            }
            else if (food == 999)
            {
                bar = SKIN_SPRITE_FOOD2;
                strout_blt(surface, &font_small, STROUT_LEFT, "Rest", 58, 84,
                           skin_prefs.widget_key, NULL);
            }
            else
            {
                bar = SKIN_SPRITE_FOOD;
                strout_blt(surface, &font_small, STROUT_LEFT, "Wait", 58, 84,
                           skin_prefs.widget_key, NULL);
            }

            /* Adjust in order to draw the bar correctly. */
            if (++food < 0)
            {
                food = 0;
            }

            surface = skin_sprites[bar]->bitmap;
            fraction = (float)food / 1000.0;
            box.x = 0;
            box.y = 0;
            box.w = MIN(surface->w, (uint16)(surface->w * fraction));
            box.h = surface->h;

            if (food &&
                !box.w)
            {
                box.w = 1;
            }

            sprite_blt(skin_sprites[bar], 87, 88, &box, &bltfx);
        }
    }

    box.x = x;
    box.y = y;
    SDL_BlitSurface(widget_data[id].surface, NULL, ScreenSurface,
                    &box);

    if (cpl.stats.maxhp)
    {
        float        hind,
                     fraction;
        SDL_Surface *surface;

        if ((hind = cpl.stats.hp) < 0)
        {
            hind = 0;
        }

        if ((LastTick - cpl.stats.hptick) <= 1000)
        {
            if (cpl.stats.temphp > 0)
            {
                hind -= (float)((cpl.stats.temphp) *
                        (1.0 - (float)(LastTick - cpl.stats.hptick) / 1000.0));
            }
            else
            {
                hind += (float)(ABS(cpl.stats.temphp) *
                        (1.0 - (float)(LastTick - cpl.stats.hptick) / 1000.0));
            }
        }

        surface = skin_sprites[SKIN_SPRITE_HP]->bitmap;
        fraction = hind / (float)cpl.stats.maxhp;
        box.x = 0;
        box.y = 0;
        box.w = MIN(surface->w, (uint16)(surface->w * fraction));
        box.h = surface->h;

        if (hind &&
            !box.w)
        {
            box.w = 1;
        }

        sprite_blt(skin_sprites[SKIN_SPRITE_HP], x + 57, y + 23, &box, NULL);
    }

    if (cpl.stats.maxsp)
    {
        float        hind,
                     fraction;
        SDL_Surface *surface;

        if ((hind = cpl.stats.sp) < 0)
        {
            hind = 0;
        }

        if ((LastTick - cpl.stats.sptick) <= 1000)
        {
            if (cpl.stats.tempsp > 0)
            {
                hind -= (float)((cpl.stats.tempsp) *
                        (1.0 - (float)(LastTick - cpl.stats.sptick) / 1000.0));
            }
            else
            {
                hind += (float)(ABS(cpl.stats.tempsp) *
                        (1.0 - (float)(LastTick - cpl.stats.sptick) / 1000.0));
            }
        }

        surface = skin_sprites[SKIN_SPRITE_SP]->bitmap;
        fraction = hind / (float)cpl.stats.maxsp;
        box.x = 0;
        box.y = 0;
        box.w = MIN(surface->w, (uint16)(surface->w * fraction));
        box.h = surface->h;

        if (hind &&
            !box.w)
        {
            box.w = 1;
        }

        sprite_blt(skin_sprites[SKIN_SPRITE_SP], x + 57, y + 47, &box, NULL);
    }

    if (cpl.stats.maxgrace)
    {
        float        hind,
                     fraction;
        SDL_Surface *surface;

        if ((hind = cpl.stats.grace) < 0)
        {
            hind = 0;
        }

        if ((LastTick - cpl.stats.gracetick) <= 1000)
        {
            if (cpl.stats.tempgrace > 0)
            {
                hind -= (float)((cpl.stats.tempgrace) *
                        (1.0 - (float)(LastTick - cpl.stats.gracetick) / 1000.0));
            }
            else
            {
                hind += (float)(ABS(cpl.stats.tempgrace) *
                        (1.0 - (float)(LastTick - cpl.stats.gracetick) / 1000.0));
            }
        }

        surface = skin_sprites[SKIN_SPRITE_GRACE]->bitmap;
        fraction = hind / (float)cpl.stats.maxgrace;
        box.x = 0;
        box.y = 0;
        box.w = MIN(surface->w, (uint16)(surface->w * fraction));
        box.h = surface->h;

        if (hind &&
            !box.w)
        {
            box.w = 1;
        }

        sprite_blt(skin_sprites[SKIN_SPRITE_GRACE], x + 57, y + 71, &box, NULL);
    }
}

void wdh_process_menu_b(widget_id_t id)
{
    sint16   x = widget_data[id].x1,
             y = widget_data[id].y1;

    sprite_blt(skin_sprites[SKIN_SPRITE_MENU_BUTTONS], x, y, NULL, NULL);
}

void wdh_process_skill_lvl(widget_id_t id)
{
    sint16   x = widget_data[id].x1,
             y = widget_data[id].y1;
    char        buf[TINY_BUF];
    _BLTFX bltfx;
    SDL_Rect box;

    if (WIDGET_REDRAW(id))
    {
        SDL_Surface *surface = widget_data[id].surface;

        WIDGET_REDRAW(id) = 0;
        bltfx.surface = surface;
        bltfx.flags = 0;
        bltfx.alpha = 0;
        SDL_FillRect(surface, NULL, NDI_COLR_HOTPINK);
        strout_blt(surface, &font_tiny, STROUT_LEFT, "Skill groups", 3, 1,
                   skin_prefs.widget_title, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Ag:", 6, 26,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[0]);
        strout_blt(surface, &font_small, STROUT_LEFT, buf,
                   44 - strout_width(&font_small, buf), 26,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Me:", 6, 38,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[2]);
        strout_blt(surface, &font_small, STROUT_LEFT, buf,
                   44 - strout_width(&font_small, buf), 38,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Ma:", 6, 49,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[4]);
        strout_blt(surface, &font_small, STROUT_LEFT, buf,
                   44 - strout_width(&font_small, buf), 49,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Pe:", 6, 62,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[1]);
        strout_blt(surface, &font_small, STROUT_LEFT, buf,
                   44 - strout_width(&font_small, buf), 62,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Ph:", 6, 74,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[3]);
        strout_blt(surface, &font_small, STROUT_LEFT, buf,
                   44 - strout_width(&font_small, buf), 74,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Wi:", 6, 86,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[5]);
        strout_blt(surface, &font_small, STROUT_LEFT, buf,
                   44 - strout_width(&font_small, buf), 86,
                   skin_prefs.widget_valueEq, NULL);
    }

    box.x = x;
    box.y = y;
    SDL_BlitSurface(widget_data[id].surface, NULL,
                    ScreenSurface, &box);
}

void wdh_process_skill_exp(widget_id_t id)
{
    sint16   x = widget_data[id].x1,
             y = widget_data[id].y1;
    _BLTFX      bltfx;
    char        buf[256];
    float       multi = 0.0;
    double      line = 0.0; /* A float is 4 bytes, a double is 8! */
    SDL_Rect    box;
    long int liLExp = 0;
    long int liLExpTNL = 0;
    long int liTExp = 0;
    long int liTExpTNL = 0;
    float fLExpPercent = 0;

    if (WIDGET_REDRAW(id))
    {
        SDL_Surface *surface = widget_data[id].surface;

        WIDGET_REDRAW(id) = 0;
        bltfx.surface = surface;
        bltfx.flags = 0;
        bltfx.alpha = 0;
        SDL_FillRect(surface, NULL, NDI_COLR_HOTPINK);
        strout_blt(surface, &font_tiny, STROUT_LEFT, "Used", 4, -1,
                   skin_prefs.widget_title, NULL);
        strout_blt(surface, &font_tiny, STROUT_LEFT, "Skill", 4, 7,
                   skin_prefs.widget_title, NULL);

        if (cpl.skill_name[0])
        {
            uint32 colr;

            if (!options.iExpDisplay)
            {
                sprintf(buf, "~%s~", cpl.skill_name);
            }
            else
            {
                if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0 ||
                    skill_list[cpl.skill_g].entry[cpl.skill_e].exp == -2)
                {
                    sprintf(buf, "~%s~ - level: %d",
                            cpl.skill_name,
                            skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level);
                }
                else
                {
                    sprintf(buf, "~%s~ - level: **", cpl.skill_name);
                }
            }

            strout_blt(surface, &font_small, STROUT_LEFT, buf, 28, -1,
                       skin_prefs.widget_valueEq, NULL);

            if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level == server_level.level)
            {
                strout_blt(surface, &font_small, STROUT_LEFT, "MAXIMUM LEVEL REACHED!",
                           28, 9, skin_prefs.widget_valueHi, NULL);
            }
            else
            {
                if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
                {
                    multi = modf(((double)(skill_list[cpl.skill_g].entry[cpl.skill_e].exp -
                                           server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level]) /
                                  (double)(server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level + 1] -
                                           server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level]) * 10.0),
                                    &line); /* Crash happened here, guess why */

                    liTExp = skill_list[cpl.skill_g].entry[cpl.skill_e].exp;
                    liTExpTNL = server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level + 1];
                    liLExp = liTExp - server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level];
                    liLExpTNL = liTExpTNL - server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level];
                    fLExpPercent = (float)liLExp / (float)liLExpTNL * 100;
                }

                switch (options.iExpDisplay)
                {
                    /* LExp% */
                    case 1:
                        if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
                        {
                            sprintf(buf, "%#05.2f%%", fLExpPercent);
                        }
                        else
                        {
                            sprintf(buf, "**.**%%");
                        }

                        break;

                    /* LExp/LExp tnl */
                    case 2:
                        if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
                        {
                            sprintf(buf, "%ld / %ld", liLExp, liLExpTNL);
                        }
                        else
                        {
                            sprintf(buf, "** / **");
                        }

                        break;

                    /* TExp/TExp tnl */
                    case 3:
                        if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
                        {
                            sprintf(buf, "%ld / %ld", liTExp, liTExpTNL);
                        }
                        else
                        {
                            sprintf(buf, "** / **");
                        }

                        break;

                    /* (LExp%) LExp/LExp tnl */
                    case 4:
                        if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
                        {
                            sprintf(buf, "%#05.2f%% - %ld", fLExpPercent, liLExpTNL - liLExp);
                        }
                        else
                        {
                            sprintf(buf, "(**.**%%) **");
                        }

                        break;

                    default:
                        if(skill_list[cpl.skill_g].entry[cpl.skill_e].exp >=0)
                        {
                            sprintf(buf, "%d / %-9d", skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level,skill_list[cpl.skill_g].entry[cpl.skill_e].exp );
                        }
                        else if(skill_list[cpl.skill_g].entry[cpl.skill_e].exp == -2)
                        {
                            sprintf(buf, "%d / **", skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level );
                        }
                        else
                        {
                            sprintf(buf, "** / **");
                        }
                }

                strout_blt(surface, &font_small, STROUT_LEFT, buf, 28, 9,
                           skin_prefs.widget_valueEq, NULL);
            }

            sprintf(buf, "%1.2f sec", cpl.action_timer);
            colr = (cpl.action_timer == 0.0)
                   ? NDI_COLR_LIME
                   : percentage_colr(100 - (cpl.action_timer * 100.0 /
                                            cpl.action_time_max));
            strout_blt(surface, &font_small, STROUT_LEFT, buf, 160, -1, colr,
                       NULL);
        }

        sprite_blt(skin_sprites[SKIN_SPRITE_EXP_SKILL_BORDER], 143, 11, NULL,
                   &bltfx);

        if (multi)
        {
            surface = skin_sprites[SKIN_SPRITE_EXP_SKILL_LINE]->bitmap;
            box.x = 0;
            box.y = 0;
            box.w = MAX(1, MIN((uint16)(surface->w * multi), surface->w));
            box.h = surface->h;
            sprite_blt(skin_sprites[SKIN_SPRITE_EXP_SKILL_LINE], 146, 18, &box,
                       &bltfx);
        }

        if (line > 0)
        {
            uint8 i;

           for (i = 0; i < (uint8)line; i++)
            {
                sprite_blt(skin_sprites[SKIN_SPRITE_EXP_SKILL_BUBBLE],
                           146 + i * 5, 13, NULL, &bltfx);
            }
        }
    }

    box.x = x;
    box.y = y;
    SDL_BlitSurface(widget_data[id].surface, NULL,
                    ScreenSurface, &box);
}

void wdh_process_main_lvl(widget_id_t id)
{
    sint16   x = widget_data[id].x1,
             y = widget_data[id].y1;
    char     buf[MEDIUM_BUF];
    double   multi,
             line;
    SDL_Rect box;
    _BLTFX   bltfx;

    if (WIDGET_REDRAW(id))
    {
        SDL_Surface *surface = widget_data[id].surface;
        uint32       colr;
        uint8        i;

        WIDGET_REDRAW(id) = 0;
        bltfx.surface = surface;
        bltfx.flags = 0;
        bltfx.alpha = 0;
        SDL_FillRect(surface, NULL, NDI_COLR_HOTPINK);
        strout_blt(surface, &font_tiny, STROUT_LEFT, "Level / Exp", 4, 1,
                   skin_prefs.widget_key, NULL);

        /* Level. */
        sprintf(buf, "%d", cpl.stats.level);

        if (cpl.warn_drained)
        {
            colr = percentage_colr((float)cpl.stats.level /
                                   (float)cpl.stats.exp_level * 40);
        }
        else
        {
            colr = percentage_colr((float)cpl.stats.level /
                                   (float)server_level.level * 50 + 50);
        }

        strout_blt(surface, &font_large, STROUT_LEFT, buf,
                   91 - strout_width(&font_large, buf), 4, colr, NULL);

        /* Exp. */
        sprintf(buf, "%d", cpl.stats.exp);
        multi = (float)(cpl.stats.exp - server_level.exp[cpl.stats.exp_level]) /
                (float)(server_level.exp[cpl.stats.exp_level + 1] -
                        server_level.exp[cpl.stats.exp_level]);

        if (multi < 0.90)
        {
            colr = skin_prefs.widget_valueEq;
        }
        else
        {
            colr = percentage_colr(multi * 100);
        }

        strout_blt(surface, &font_small, STROUT_LEFT, buf, 5, 20, colr, NULL);

        /* Exp bubbles. */
        sprite_blt(skin_sprites[SKIN_SPRITE_EXP_BORDER], 9, 49, NULL, &bltfx);

        if ((multi = modf((multi * 10.0), &line)))
        {
            surface = skin_sprites[SKIN_SPRITE_EXP_SLIDER]->bitmap;
            box.x = 0;
            box.y = 0;
            box.w = MAX(1, MIN((uint16)(surface->w * multi), surface->w));
            box.h = surface->h;
            sprite_blt(skin_sprites[SKIN_SPRITE_EXP_SLIDER], 9, 49, &box,
                       &bltfx);
        }

        for (i = 0; i < 10; i++)
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_EXP_BUBBLE2], 10 + i * 8, 40,
                       NULL, &bltfx);
        }

        for (i = 0; i < (uint8)line; i++)
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_EXP_BUBBLE1], 10 + i * 8, 40,
                       NULL, &bltfx);
        }
    }

    box.x = x;
    box.y = y;
    SDL_BlitSurface(widget_data[id].surface, NULL,
                    ScreenSurface, &box);
}

void wdh_process_regen(widget_id_t id)
{
    sint16   x = widget_data[id].x1,
             y = widget_data[id].y1;
    char        buf[256];
    SDL_Rect    box;
    _BLTFX      bltfx;

    if (WIDGET_REDRAW(id))
    {
        SDL_Surface *surface = widget_data[id].surface;

        WIDGET_REDRAW(id) = 0;
        bltfx.surface = surface;
        bltfx.flags = 0;
        bltfx.alpha = 0;
        SDL_FillRect(surface, NULL, NDI_COLR_HOTPINK);
        strout_blt(surface, &font_tiny, STROUT_LEFT, "Regeneration", 4, 1,
                   skin_prefs.widget_title, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "HP", 61, 13,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%2.1f", cpl.gen_hp);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 75, 13,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Mana", 5, 13,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%2.1f", cpl.gen_sp);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 35, 13,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(surface, &font_small, STROUT_LEFT, "Grace", 5, 24,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "%2.1f", cpl.gen_grace);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, 35, 24,
                   skin_prefs.widget_valueEq, NULL);
    }

    box.x = x;
    box.y = y;
    SDL_BlitSurface(widget_data[id].surface, NULL,
                    ScreenSurface, &box);
}

void wdh_process_pdoll(widget_id_t id)
{
    sint16  x = widget_data[id].x1,
            y = widget_data[id].y1;
    char    buf[512];
    item   *ip;
    uint8   ring_flag = 0;

    SDL_FillRect(widget_data[id].surface, NULL, NDI_COLR_HOTPINK);

    if (!cpl.ob)
    {
        return;
    }

    strout_blt(ScreenSurface, &font_tiny, STROUT_LEFT, "Melee", x + 5, y + 40,
               skin_prefs.widget_key, NULL);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "WC", x + 5, y + 53,
               skin_prefs.widget_key, NULL);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "DPS", x + 5, y + 63,
               skin_prefs.widget_key, NULL);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "WS", x + 5, y + 73,
               skin_prefs.widget_key, NULL);
    sprintf(buf, "%02d", cpl.stats.wc);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 25, y + 53,
               skin_prefs.widget_valueEq, NULL);
    sprintf(buf, "%.1f", cpl.stats.dps);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 25, y + 63,
               skin_prefs.widget_valueEq, NULL);
    sprintf(buf, "%1.2f", cpl.stats.weapon_sp);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 25, y + 73,
               skin_prefs.widget_valueEq, NULL);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "AC", x + 180, y + 95,
               skin_prefs.widget_key, NULL);
    sprintf(buf, "%02d", cpl.stats.ac);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 195, y + 95,
               skin_prefs.widget_valueEq, NULL);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "SF", x + 5, y + 95,
               skin_prefs.widget_key, NULL);
    sprintf(buf, "%.1f", cpl.stats.spell_fumble);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 20, y + 95,
               skin_prefs.widget_valueEq, NULL);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "Speed", x + 60, y + 167,
               skin_prefs.widget_key, NULL);
    sprintf(buf, "%.1f%%", cpl.stats.speed);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 130, y + 167,
               percentage_colr(cpl.stats.speed), NULL);
    strout_blt(ScreenSurface, &font_tiny, STROUT_LEFT, "Distance", x + 170, y + 40,
               skin_prefs.widget_key, NULL);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "WC", x + 170, y + 53,
               skin_prefs.widget_key, NULL);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "DPS", x + 170, y + 63,
               skin_prefs.widget_key, NULL);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "WS", x + 170, y + 73,
               skin_prefs.widget_key, NULL);

    if (cpl.stats.dist_dps == -0.1)
    {
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "--", x + 190, y + 53,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "--", x + 190, y + 63,
                   skin_prefs.widget_valueEq, NULL);
    }
    else if (cpl.stats.dist_dps == -0.2) /* marks rods/wands/horns */
    {
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "**", x + 190, y + 53,
                   skin_prefs.widget_valueEq, NULL);
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "**", x + 190, y + 63,
                   skin_prefs.widget_valueEq, NULL);
        sprintf(buf, "%1.2f", cpl.stats.dist_time);
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 190, y + 73,
                   skin_prefs.widget_valueEq, NULL);
    }
    else
    {
        sprintf(buf, "%02d", cpl.stats.dist_wc);
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 190, y + 53,
                   skin_prefs.widget_valueEq, NULL);
        sprintf(buf, "%.1f", cpl.stats.dist_dps);
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 190, y + 63,
                   skin_prefs.widget_valueEq, NULL);
        sprintf(buf, "%1.2f", cpl.stats.dist_time);
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 190, y + 73,
                   skin_prefs.widget_valueEq, NULL);
    }

    for (ip = cpl.ob->inv; ip; ip = ip->next)
    {
        if (ip->applied)
        {
            uint16 type;
            uint8  i;
            sint16 index = -1;

            if (ip->type == TYPE_ARROW &&
                ip->stype >= 128)
            {
                type = TYPE_BOW; // throwing weapon
            }
            else
            {
                type = ip->itype;
            }

            for (i = 0; PdollPos[i].type >= 0; i++)
            {
                if (PdollPos[i].type == type)
                {
                    index = (sint16)i;

                    break;
                }
            }

            if (ip->itype == TYPE_RING)
            {
                index += ++ring_flag & 1;
            }

            if (index >= 0)
            {
                uint8 quacon = (ip->item_qua == 255) ? 255
                               : (float)ip->item_con /
                                 (float)ip->item_qua * 100;

                face_get(ip->face);
                sprite_blt_as_icon(face_list[ip->face].sprite,
                                   PdollPos[index].x + x, PdollPos[index].y + y,
                                   SPRITE_ICON_TYPE_ACTIVE, 0,
                                   (ip->flagsval & ~(F_LOCKED | F_APPLIED)),
                                   (quacon == 100) ? 0 : quacon,
                                   (ip->nrof == 1) ? 0 : ip->nrof, NULL);

                if (STROUT_TOOLTIP_HOVER_TEST(PdollPos[index].x + x,
                                              PdollPos[index].y + y))
                {
                    strout_tooltip_prepare(strout_tooltip_detail_item(ip));

                    /* Start a drag. */
                    if (MouseEvent == LB_DN &&
                        !draggingInvItem(DRAG_GET_STATUS))
                    {
                        cpl.win_pdoll_tag = ip->tag;
                        draggingInvItem(DRAG_PDOLL);
                    }
                }
            }
        }
    }
}

void wdh_process_target(widget_id_t id)
{
    sint16    x = widget_data[id].x1,
              y = widget_data[id].y1,
              hp_tmp = cpl.target_hp;
    char     *cp = NULL;

    SDL_FillRect(widget_data[id].surface, NULL, NDI_COLR_HOTPINK);

    if (cpl.target_mode)
    {
        sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_ATTACK], x + 5, y + 4, NULL,
                   NULL);
    }
    else
    {
        sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_NORMAL], x + 5, y + 4, NULL,
                   NULL);
    }

    sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_HP_B], x + 4, y + 24, NULL,
               NULL);

    switch (cpl.target_code)
    {
        case 0:
            hp_tmp = (sint16)((float)cpl.stats.hp / (float)cpl.stats.maxhp * 100);

            if (cpl.target_mode)
            {
                cp = "target self (hold attack)";
            }
            else
            {
                cp = "target self";
            }

            break;

        case 1:
            sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_TALK], x + 223, y + 7,
                       NULL, NULL);

            if (cpl.target_mode)
            {
                cp = "target and attack enemy";
            }
            else
            {
                cp = "target enemy";
            }

            break;

        case 2:
            sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_TALK], x + 223, y + 7,
                       NULL, NULL);

            if (cpl.target_mode)
            {
                cp = "target friend (hold attack)";
            }
            else
            {
                cp = "target friend";
            }
    }

    if (options.show_target_self ||
        cpl.target_code)
    {
        if (hp_tmp)
        {
            SDL_Rect     box;
            SDL_Surface *surface = skin_sprites[SKIN_SPRITE_TARGET_HP]->bitmap;
            float        fraction = (float)hp_tmp * 0.01;

            box.x = 0;
            box.y = 0;
            box.w = MAX(1, MIN((uint16)(surface->w * fraction), surface->w));
            box.h = surface->h;
            sprite_blt(skin_sprites[SKIN_SPRITE_TARGET_HP], x + 5, y + 25,
                       &box, NULL);
        }

        if (cp)
        {
            /* Draw the name of the target */
            strout_blt(ScreenSurface, &font_small, STROUT_LEFT, cpl.target_name,
                       x + 35, y + 3, cpl.target_colr, NULL);

            /* Either draw HP remaining percent and description... */
            if (hp_tmp)
            {
                char buf[9];

                sprintf(buf, "HP: %d%%", hp_tmp);
                strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf,
                           x + 35, y + 14, percentage_colr(cpl.target_hp), NULL);
                strout_blt(ScreenSurface, &font_small, STROUT_LEFT, cp,
                           x + 85, y + 14, cpl.target_colr, NULL);
            }
            /* ...or draw just the description */
            else
            {
                strout_blt(ScreenSurface, &font_small, STROUT_LEFT, cp,
                           x + 35, y + 14, cpl.target_colr, NULL);
            }
        }
    }
}

void wdh_process_quickslots(widget_id_t id)
{
//    sint16    x = widget_data[id].x1,
//              y = widget_data[id].y1,
//    int     i;
//    char    buf[3];
//    int     qsx, qsy, xoff;
//
//    if (widget_data[id].ht > 34)
//    {
//        qsx = 1;
//        qsy = 0;
//        xoff = 0;
//        sprite_blt(skin_sprites[SKIN_SPRITE_QUICKSLOTSV], x, y, NULL, NULL);
//    }
//    else
//    {
//        qsx = 0;
//        qsy = 1;
//        xoff = -17;
//        sprite_blt(skin_sprites[SKIN_SPRITE_QUICKSLOTS], x, y, NULL, NULL);
//    }
//
//    update_quickslots(-1);
//
//    for (i = MAX_QUICK_SLOTS - 1; i >= 0; i--)
//    {
//        if (quick_slots[i].shared.tag != -1)
//        {
//            /* spell in quickslot */
//            if (quick_slots[i].shared.is_spell == 1)
//            {
//                sprite_blt(spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].icon,
//                           x + quickslots_pos[i][qsx]+xoff, y + quickslots_pos[i][qsy], NULL, NULL);
//
//                if (STROUT_TOOLTIP_HOVER_TEST(x + quickslots_pos[i][qsx] + xoff,
//                                              y + quickslots_pos[i][qsy]))
//                {
//                    uint8  class = quick_slots[i].spell.classNr,
//                           group = quick_slots[i].spell.groupNr;
//                    char  *name = spell_list[group].entry[class][quick_slots[i].shared.tag].name;
//
//                    strout_tooltip_prepare(strout_tooltip_detail_spell(name, class, group));
//                }
//            }
//            /* item in quickslot */
//            else
//            {
//                item *ip;
//
//                if ((ip = locate_item_from_item(cpl.ob, quick_slots[i].shared.tag)))
//                {
//                    uint8 quacon = (ip->item_qua == 255) ? 255
//                                   : (float)ip->item_con /
//                                     (float)ip->item_qua * 100;
//
//                    sprite_blt_as_icon(face_list[ip->face].sprite,
//                                       x + quickslots_pos[i][qsx] + xoff,
//                                       y + quickslots_pos[i][qsy],
//                                       SPRITE_ICON_TYPE_ACTIVE, 0, ip->flagsval,
//                                       (quacon == 100) ? 0 : quacon,
//                                       (ip->nrof == 1) ? 0 : ip->nrof, NULL);
//
//                    if (STROUT_TOOLTIP_HOVER_TEST(x + quickslots_pos[i][qsx] + xoff,
//                                                  y + quickslots_pos[i][qsy]))
//                    {
//                        strout_tooltip_prepare(strout_tooltip_detail_item(ip));
//                    }
//                }
//            }
//        }
//        sprintf(buf, "F%d", i + 1);
//        strout_blt(ScreenSurface, &font_tiny, STROUT_LEFT, buf, x + quickslots_pos[i][qsx]+xoff + 12, y + quickslots_pos[i][qsy] - 6,
//                  skin_prefs.widget_title, NULL);
//    }
}

void wdh_process_mapname(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;

    strout_blt(ScreenSurface, &font_large, STROUT_LEFT, MapData.name, x, y,
               skin_prefs.widget_title, NULL);
}

void wdh_process_range(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;
    char        buf[MEDIUM_BUF];
    SDL_Rect    rec_range;
    SDL_Rect    rec_item;
    item       *tmp, *tmp2;

    rec_range.w = 160;
    rec_range.h = font_small.line_height;
    rec_item.w = 185;
    rec_item.h = font_small.line_height;
    examine_range_inv();
    sprite_blt(skin_sprites[SKIN_SPRITE_RANGE], x, y, NULL, NULL);

    switch (fire_mode.mode)
    {
        case FIRE_MODE_ARCHERY_ID:
            if (fire_mode.weapon == FIRE_ITEM_NO)
            {
                sprintf(buf, "No range weapon applied");
                strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 5, y + 36,
                           skin_prefs.widget_info, &rec_range);
            }
            else
            {
                if (!(tmp2 = locate_item(fire_mode.weapon)))
                {
                    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, "Using nothing",
                               x + 5, y + 36, skin_prefs.widget_info, &rec_range);
                }
                else
                {
                    item  *ip;
                    uint8  quacon;

                    sprintf(buf, "Using %s", tmp2->s_name);

                    if ((ip = locate_item(fire_mode.weapon)))
                    {
                        quacon = (ip->item_qua == 255)
                                 ? 255 : (float)ip->item_con / (float)ip->item_qua * 100;
                        sprite_blt_as_icon(face_list[ip->face].sprite, x + 5, y + 2,
                                           SPRITE_ICON_TYPE_ACTIVE, 0,
                                           (ip->flagsval & ~(F_LOCKED | F_APPLIED)),
                                           (quacon == 100) ? 0 : quacon,
                                           (ip->nrof == 1) ? 0 : ip->nrof, NULL);
                    }

                    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 5, y + 36,
                               skin_prefs.widget_valueEq, &rec_range);
                }

                if (fire_mode.ammo != FIRE_ITEM_NO)
                {
                    if (!(tmp = locate_item_from_item(cpl.ob, fire_mode.ammo)))
                    {
                        sprintf(buf, "Ammo not selected");
                    }
                    else
                    {
                        item  *ip;
                        uint8  quacon;

                        if ((ip = locate_item(fire_mode.ammo)))
                        {
                            quacon = (ip->item_qua == 255)
                                     ? 255 : (float)ip->item_con / (float)ip->item_qua * 100;
                            sprite_blt_as_icon(face_list[ip->face].sprite, x + 45, y + 2,
                                               SPRITE_ICON_TYPE_ACTIVE, 0,
                                               (ip->flagsval & ~(F_LOCKED | F_APPLIED)),
                                               (quacon == 100) ? 0 : quacon,
                                               (ip->nrof == 1) ? 0 : ip->nrof, NULL);
                        }

                        if (tmp->itype == TYPE_ARROW)
                        {
                            sprintf(buf, "Ammo %s (%d)",
                                         tmp->s_name, tmp->nrof);
                        }
                        else
                        {
                            sprintf(buf, "Ammo %s", tmp->s_name);
                        }
                    }
                }

                else if (tmp2->itype == TYPE_BOW)
                {
                    sprintf(buf, "Ammo not selected");
                }
                else if (tmp2->itype == TYPE_ARROW)
                {
                    sprintf(buf, "Amount: %d", tmp2->nrof);
                }
                else
                {
//                    sprintf(buf, "Type: %d",tmp2->itype);
                    buf[0] = '\0';
                }

                strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 5, y + 47,
                           skin_prefs.widget_valueEq, &rec_item);
            }

            sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_MARKER], x + 5, y + 2,
                       NULL, NULL);

            break;

        case FIRE_MODE_SPELL_ID:
            if (!fire_mode.spell)
            {
                sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_WIZARD_NO], x + 5, y + 2,
                           NULL, NULL);
                sprintf(buf, "No spell selected");
                strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 5, y + 47,
                           skin_prefs.widget_info, &rec_item);
            }
            else
            {
                /* we use wiz spells as default */
                sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_WIZARD], x + 5, y + 2,
                           NULL, NULL);

                if (fire_mode.spell->flag == -1)
                {
                    fire_mode.spell = NULL;
                }
                else
                {
                    sprite_blt(fire_mode.spell->icon, x + 45, y + 2, NULL, NULL);
                    strout_blt(ScreenSurface, &font_small, STROUT_LEFT,
                               fire_mode.spell->name, x + 5, y + 47,
                               skin_prefs.widget_valueEq, &rec_item);
                }
            }

            sprintf(buf, "Cast spell");
            strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 5, y + 36,
                       skin_prefs.widget_info, &rec_range);

            break;

        case FIRE_MODE_SKILL_ID:
            if (!fire_mode.skill)
            {
                sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_SKILL_NO], x + 5, y + 2,
                           NULL, NULL);
                sprintf(buf, "No skill selected");
                strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 5, y + 47,
                           skin_prefs.widget_info, &rec_item);
            }
            else
            {
                sprite_blt(skin_sprites[SKIN_SPRITE_RANGE_SKILL], x + 5, y + 2,
                           NULL, NULL);

                if (fire_mode.skill->flag == -1)
                {
                    fire_mode.skill = NULL;
                }
                else
                {
                    sprite_blt(fire_mode.skill->icon, x + 45, y + 2, NULL, NULL);
                    strout_blt(ScreenSurface, &font_small, STROUT_LEFT,
                              fire_mode.skill->name, x + 5, y + 47,
                              skin_prefs.widget_valueEq, &rec_item);
                }
            }

            sprintf(buf, "Use skill");
            strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 5, y + 36,
                       skin_prefs.widget_info, &rec_range);

            break;

        default:
            LOG(LOG_ERROR, "Unknown fire mode %u\n", fire_mode.mode);
    }
}

void wdh_process_resist(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;
    _BLTFX bltfx;
    SDL_Rect box;

    if (WIDGET_REDRAW(id))
    {
        SDL_Surface *surface = widget_data[id].surface;
        uint8        num;
        char         buf[5];
        sint16       xt[] = { 4, 36, 68, 100, 132, 164, -1 },
                     yt[] = { 3, 15, 27,  39,  51,  63, -1 };
        uint8        xp = 2,
                     yp = 0;

        WIDGET_REDRAW(id) = 0;
        bltfx.surface = surface;
        bltfx.flags = 0;
        bltfx.alpha = 0;
        SDL_FillRect(surface, NULL, NDI_COLR_HOTPINK);
        strout_blt(surface, &font_tiny, STROUT_LEFT, "Resistance Table", 4,  1,
                   skin_prefs.widget_title, NULL);

        for (num = 0; num < ATNR_INTERNAL; num++)
        {
            sint8  resist = cpl.stats.protection[num];
            uint32 colr = (!resist)
                          ? NDI_COLR_GREY
                          : percentage_colr(((resist + 100) * 0.005 * 100));

            strout_blt(surface, &font_small, STROUT_LEFT, (char *)player_attackredraw[num].abbr,
                       xt[xp], yt[yp], skin_prefs.widget_key, NULL);
            sprintf(buf, "%02d", resist);
            strout_blt(surface, &font_small, STROUT_LEFT, buf, xt[xp] + 16, yt[yp],
                       colr, NULL);

            if (xt[++xp] == -1)
            {
                xp = 0;

                if (yt[++yp] == -1)
                {
                    break;
                }
            }
        }
    }

    box.x = x;
    box.y = y;
    SDL_BlitSurface(widget_data[id].surface, NULL,
                    ScreenSurface, &box);
}

void wdh_process_chatwin(widget_id_t id)
{
    textwin_show_window(TEXTWIN_CHAT_ID);
}

void wdh_process_msgwin(widget_id_t id)
{
    textwin_show_window(TEXTWIN_MSG_ID);
}

void wdh_process_console(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;
    SDL_Rect box;

    sprite_blt(skin_sprites[SKIN_SPRITE_TEXTINPUT],x, y, NULL, NULL);
    box.x = x + 8;
    box.y = y + 6;
    box.w = skin_sprites[SKIN_SPRITE_TEXTINPUT]->bitmap->w - 22;
    box.h = font_small.line_height;
    strout_input(&font_small, &box, 0);
}

void wdh_process_number(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;
    SDL_Rect box;
    char     buf[TINY_BUF];

    box.x = x + 8;
    box.y = y + 6;
    box.w = skin_sprites[SKIN_SPRITE_NUMBER]->bitmap->w - 22;
    box.h = font_small.line_height;
    sprite_blt(skin_sprites[SKIN_SPRITE_NUMBER], x, y, NULL, NULL);
    sprintf(buf, "%s how many from %d %s",
            (cpl.nummode == NUM_MODE_GET) ? "Get" : "Drop", cpl.nrof,
            cpl.num_text);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 8, y + 6,
               skin_prefs.widget_title, NULL);
    box.y = y + 25;
    strout_input(&font_small, &box, 0);
}

void wdh_process_statometer(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;
    char buf[TINY_BUF];

    if (!options.statsupdate)
    {
        WIDGET_SHOW(id) = 0;

        return;
    }

    strout_blt(ScreenSurface, &font_large, STROUT_LEFT, "Stat-O-Meter:",
               x + 2, y + 2, skin_prefs.widget_title, NULL);
    sprintf(buf, "EXP: %d", statometer.exp);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 2, y + 15,
               skin_prefs.widget_key, NULL);
    sprintf(buf, "(%.2f/hour)", statometer.exphour);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 82, y + 15,
               skin_prefs.widget_valueEq, NULL);
    sprintf(buf, "Kills: %d", statometer.kills);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 2, y + 25,
               skin_prefs.widget_key, NULL);
    sprintf(buf, "(%.2f/hour)", statometer.killhour);
    strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 82, y + 25,
               skin_prefs.widget_valueEq, NULL);
}

void wdh_process_main_inv(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;

    if (cpl.inventory_win != IWIN_INV)
    {
        char  buf[TINY_BUF];
        float weight = cpl.real_weight,
              limit = cpl.weight_limit;

        if (!options.playerdoll)
        {
            WIDGET_SHOW(WIDGET_PDOLL_ID) = 0;
        }

        widget_data[id].ht = 32;
        SDL_FillRect(widget_data[id].surface, NULL, NDI_COLR_HOTPINK);
        sprintf(buf, "Carry %c%06x%4.3f%c kg",
                ECC_NEWCOLR, percentage_colr(100 - (weight / limit * 100)),
                weight / 1000.0, ECC_DEFCOLR);
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 140, y + 4,
                   skin_prefs.widget_key, NULL);
        sprintf(buf, "Limit %c%06x%4.3f%c kg",
                ECC_NEWCOLR, skin_prefs.widget_valueEq, limit / 1000.0,
                ECC_DEFCOLR);
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x + 140, y + 15,
                   skin_prefs.widget_key, NULL);
        strout_blt(ScreenSurface, &font_tiny, STROUT_LEFT, "(SHIFT for inventory)",
                   x + 32, y + 9, skin_prefs.widget_info, NULL);

        return;
    }

    if (!options.playerdoll)
    {
       WIDGET_SHOW(WIDGET_PDOLL_ID) = 1;
    }

    widget_data[id].ht = 129;
    sprite_blt(skin_sprites[SKIN_SPRITE_INVENTORY], x, y, NULL, NULL);
    blt_window_slider(skin_sprites[SKIN_SPRITE_INV_SCROLL],
                      ((cpl.win_inv_count - 1) / INVITEMXLEN) + 1, INVITEMYLEN,
                      cpl.win_inv_start / INVITEMXLEN, -1, x + 229, y + 40);

    if (cpl.ob)
    {
        ShowIcons(x, y, 4, 30, INVITEMXLEN, INVITEMYLEN, IWIN_INV, NULL);
    }
}

void wdh_process_below_inv(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;

    SDL_FillRect(widget_data[id].surface, NULL, NDI_COLR_HOTPINK);
    blt_window_slider(skin_sprites[SKIN_SPRITE_BELOW_SCROLL],
                      ((cpl.win_below_count - 1) / INVITEMBELOWXLEN) + 1, INVITEMBELOWYLEN,
                      cpl.win_below_start / INVITEMBELOWXLEN, -1, x + 263, y + 30);

    if (cpl.below)
    {
        ShowIcons(x, y, 5, 19, INVITEMBELOWXLEN, INVITEMBELOWYLEN, IWIN_BELOW,
                  NULL);
    }
}

static void ShowIcons(sint16 ox, sint16 oy, sint16 x, sint16 y, uint8 invxlen, uint8 invylen,
                      inventory_win_t iwin, _BLTFX *bltfx)
{
    uint16  i,
            start = (iwin == IWIN_INV)
                    ? cpl.win_inv_start : cpl.win_below_start,
            slot = (iwin == IWIN_INV) ? cpl.win_inv_slot : cpl.win_below_slot;
    item   *wp = (iwin == IWIN_INV) ? cpl.ob : cpl.below,
           *ip = wp->inv,
           *cip = cpl.sack->inv,
           *tmp = NULL;

    for (i = 0; i < start; i++)
    {
        if (!ip)
        {
            return;
        }

        if (cpl.container &&
            cpl.container->tag == ip->tag)
        {
            tmp = ip;

            for (i += 1; i < start; i++)
            {
                if (!cip)
                {
                    break;
                }

                cip = cip->next;
            }

            if (cip)
            {
                i = 0;
                ip = tmp;

                goto jump_in_container;
            }
        }

        ip = ip->next;
    }

    for (i = 0; i < invxlen * invylen; i++)
    {
        sint16             xi,
                           yi;
        sprite_icon_type_t type;
        uint8              selected,
                           quacon;

        if (!ip)
        {
            return;
        }

        face_get(ip->face);
        xi = (i % invxlen) * 32 + ox + x;
        yi = (i / invxlen) * 32 + oy + y;
        type = (iwin == IWIN_INV &&
                (int)ip->tag == cpl.mark_count)
               ? SPRITE_ICON_TYPE_ACTIVE : SPRITE_ICON_TYPE_NONE;
        selected = (cpl.inventory_win == iwin &&
                    i + start == slot) ? 1 : 0;
        quacon = (ip->item_qua == 255) ? 255
                 : (float)ip->item_con / (float)ip->item_qua * 100;
        sprite_blt_as_icon(face_list[ip->face].sprite, xi, yi, type, selected,
                           ip->flagsval, (quacon == 100) ? 0 : quacon,
                           (ip->nrof == 1) ? 0 : ip->nrof, bltfx);

        if (selected)
        {
            PrintInfo(ox, oy, ip, iwin);
        }

        if (STROUT_TOOLTIP_HOVER_TEST(xi, yi))
        {
            strout_tooltip_prepare(strout_tooltip_detail_item(ip));
        }

        if (cpl.container &&
            cpl.container->tag == ip->tag &&
            cip)
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_CMARK_START], xi, yi, NULL, bltfx);
            i += 1;

jump_in_container:
            for (; i < invxlen * invylen; i++)
            {
                face_get(cip->face);
                xi = (i % invxlen) * 32 + ox + x;
                yi = (i / invxlen) * 32 + oy + y;
                type = (iwin == IWIN_INV &&
                        (int)cip->tag == cpl.mark_count)
                       ? SPRITE_ICON_TYPE_ACTIVE : SPRITE_ICON_TYPE_NONE;
                selected = (cpl.inventory_win == iwin &&
                            i + start == slot) ? 1 : 0;
                quacon = (cip->item_qua == 255) ? 255
                         : (float)cip->item_con / (float)cip->item_qua * 100;

                sprite_blt_as_icon(face_list[cip->face].sprite, xi, yi, type,
                                   selected, cip->flagsval,
                                   (quacon == 100) ? 0 : quacon,
                                   (cip->nrof == 1) ? 0 : cip->nrof, bltfx);

                if (selected)
                {
                    PrintInfo(ox, oy, cip, iwin);
                }

                if (STROUT_TOOLTIP_HOVER_TEST(xi, yi))
                {
                    strout_tooltip_prepare(strout_tooltip_detail_item(cip));
                }

                if ((cip = cip->next))
                {
                    sprite_blt(skin_sprites[SKIN_SPRITE_CMARK_MIDDLE], xi, yi, NULL, bltfx);
                }
                else
                {
                    sprite_blt(skin_sprites[SKIN_SPRITE_CMARK_END], xi, yi, NULL, bltfx);

                    break;
                }
            }
        }

        ip = ip->next;
    }
}

static void PrintInfo(sint16 x, sint16 y, item *ip, inventory_win_t iwin)
{
    char         buf[MEDIUM_BUF];
    SDL_Surface *surface = ScreenSurface;

    x += ((iwin == IWIN_INV) ? 36 : 4);

    /* Print 'nrof name'. */
    if (ip->nrof == 1)
    {
        sprintf(buf, "%s", ip->s_name);
    }
    else
    {
        sprintf(buf, "%d %s", ip->nrof, ip->s_name);
    }

    strout_blt(surface, &font_small, STROUT_LEFT, buf, x, y + 4, skin_prefs.widget_title, NULL);

    /* In the below inv this is all the info we get. This is simply a real
     * estate issue. There just isn't space to squeeze in more info. This could
     * be addressed by using a tooltip or embiggening the window. Arguably
     * though it makes little sense that the player inherently know the weight,
     * condition, etc of items on the floor, but note that this info IS
     * available to the client, so the current restriction is insecure. */
    /* The above is not entirely true. 0.10.6 HAS tooltips. Quacon info is not
     * sent by the server for objects not in a player inv and it seems that
     * weight is not sent for objects that have never been in some kind of inv
     * (ie, map objects directly on the floor).
     * -- Smacky 20120405 */
    if (iwin == IWIN_BELOW)
    {
       return;
    }

    sprintf(buf, "weight: ");
    strout_blt(surface, &font_small, STROUT_LEFT, buf, x, y + 16, skin_prefs.widget_key, NULL);
    x += strout_width(&font_small, buf);
    sprintf(buf, "%4.3f ", (float)ip->weight / 1000.0);
    strout_blt(surface, &font_small, STROUT_LEFT, buf, x, y + 16, skin_prefs.widget_valueEq, NULL);
    x += strout_width(&font_small, buf);

    if (ip->item_qua == 255) /* this comes from server when not identified */
    {
        strout_blt(surface, &font_small, STROUT_LEFT, "(not identified)", x, y + 16,
                   skin_prefs.widget_info, NULL);
    }
    else
    {
        sprintf(buf, "con: ");
        strout_blt(surface, &font_small, STROUT_LEFT, buf, x, y + 16, skin_prefs.widget_key,
                   NULL);
        x += strout_width(&font_small, buf);
        sprintf(buf, "%d ", ip->item_con);
        strout_blt(ScreenSurface, &font_small, STROUT_LEFT, buf, x, y + 16,
                   percentage_colr((float)ip->item_con /
                                   (float)ip->item_qua * 100), NULL);
        x += strout_width(&font_small, buf);
        sprintf(buf, "/ %d", ip->item_qua);
        strout_blt(surface, &font_small, STROUT_LEFT, buf, x, y + 16, skin_prefs.widget_valueHi,
                   NULL);
        x += strout_width(&font_small, buf);
        sprintf(buf, "allowed: ");
        strout_blt(surface, &font_small, STROUT_LEFT, buf, x, y + 16, skin_prefs.widget_key,
                   NULL);
        x += strout_width(&font_small, buf);

        if (ip->item_level)
        {
            sprintf(buf, "lvl %d", ip->item_level);

            if (ip->item_skill)
            {
                sprintf(strchr(buf, '\0'), " %s",
                        player_skill_group[ip->item_skill - 1].abbr);
            }

            if ((!ip->item_skill &&
                 ip->item_level <= cpl.stats.level) ||
                (ip->item_skill &&
                 ip->item_level <= cpl.stats.skill_level[ip->item_skill - 1]))
            {
                strout_blt(surface, &font_small, STROUT_LEFT, buf, x, y + 16,
                           skin_prefs.widget_valueEq, NULL);
            }
            else
            {
                strout_blt(surface, &font_small, STROUT_LEFT, buf, x, y + 16,
                           skin_prefs.widget_valueLo, NULL);
            }
        }
        else
        {
            strout_blt(surface, &font_small, STROUT_LEFT, "all", x, y + 16,
                       skin_prefs.widget_valueHi, NULL);
        }
    }
}

void wdh_process_group(widget_id_t id)
{
    sint16 x = widget_data[id].x1,
           y = widget_data[id].y1;
    SDL_Rect    box;
    char        buf[SMALL_BUF];

    box.x = 0;
    box.y = 0;
    box.w = 120;

    if (global_group_status < GROUP_INVITE)
    {
        box.h = 27;
    }
    else if (global_group_status == GROUP_INVITE ||
             global_group_status == GROUP_WAIT)
    {
        box.h = 135;
    }
    else
    {
        box.h = group_count * 24 + 31;
    }

    widget_data[id].ht = box.h + 4;
    SDL_FillRect(widget_data[id].surface, NULL, NDI_COLR_HOTPINK);
    sprite_blt(skin_sprites[SKIN_SPRITE_GROUP_BG_BOTTOM], x, y + box.h,
               NULL, NULL);

    strout_blt(ScreenSurface, &font_tiny, STROUT_LEFT, "Group:", x + 50, y + 2,
               skin_prefs.widget_title, NULL);

    if (global_group_status < GROUP_INVITE)
    {
        strout_blt(ScreenSurface, &font_tiny, STROUT_LEFT, "type '/help group' for info",
                   x + 13, y + 13, skin_prefs.widget_info, NULL);

        return;
    }

    if (global_group_status == GROUP_INVITE ||
        global_group_status == GROUP_WAIT)
    {
        sprite_blt(skin_sprites[SKIN_SPRITE_GROUP_INVITE], x + 10, y + 32,
                   NULL, NULL);
        sprintf(buf, "%c%06xGROUP INVITE\n\n\n%c%06x%s%c\n\nhas invited you\nto join a group.",
                ECC_NEWCOLR, skin_prefs.widget_key,
                ECC_NEWCOLR, skin_prefs.pname_other, group_invite, ECC_DEFCOLR);
        box.x = x + 28;
        box.y = y + 13;
        box.w = strout_width(&font_small, buf);
        box.h = font_small.line_height * 7;
        strout_blt(ScreenSurface, &font_small, STROUT_CENTER, buf, box.x, box.y,
                   skin_prefs.widget_info, &box);

        if (global_group_status == GROUP_INVITE)
        {
            if (add_button(x + 4, y + 110, 101, SKIN_SPRITE_BUTTON_BLACK_UP,
                           "join", "join"))
            {
                global_group_status = GROUP_WAIT;
                client_cmd_generic("/join");
            }
            if (add_button(x + 61, y + 110, 102, SKIN_SPRITE_BUTTON_BLACK_UP,
                           "deny", "deny"))
            {
                global_group_status = GROUP_NO;
                client_cmd_generic("/deny");
            }
        }
    }
    else /* status: GROUP_MEMBER */
    {
        uint8 i;

        if (add_button(x + 4, y + 7, 103, SKIN_SPRITE_SMALL_UP, "leave", "leave"))
        {
            if (global_group_status != GROUP_LEAVE)
            {
                global_group_status = GROUP_LEAVE;
                client_cmd_generic("/leave");
            }
        }

        for (i = 0; i < GROUP_MAX_MEMBER; i++)
        {
            if (group[i].name[0])
            {
                uint32 colr = (i == 0)
                              ? skin_prefs.pname_leader
                              : skin_prefs.pname_member;

                sprite_blt(skin_sprites[SKIN_SPRITE_GROUP],
                           x + GroupPos[i].x + 2, y + GroupPos[i].y + 1,
                           NULL, NULL);
                strout_blt(ScreenSurface, &font_small, STROUT_LEFT, group[i].name,
                           x + GroupPos[i].x + 33, y + GroupPos[i].y + 1,
                          colr, NULL);
                sprintf(buf, "%3d", group[i].level);
                strout_blt(ScreenSurface, &font_tiny, STROUT_LEFT, buf,
                           x + GroupPos[i].x + 8, y + GroupPos[i].y,
                           skin_prefs.widget_valueEq, NULL);

                if (group[i].maxhp)
                {
                    float        hind = MAX(0, group[i].hp),
                                 fraction = hind / (float)group[i].maxhp;
                    SDL_Surface *surface = skin_sprites[SKIN_SPRITE_GROUP_HP]->bitmap;

                    box.x = 0;
                    box.y = 0;
                    box.h = surface->h;
                    box.w = MAX(1, MIN((int) (surface->w * fraction), surface->w));
                    sprite_blt(skin_sprites[SKIN_SPRITE_GROUP_HP],
                                x + GroupPos[i].x + 2, y + GroupPos[i].y + 17,
                                &box, NULL);
                }

                if (group[i].maxsp)
                {

                    float        hind = MAX(0, group[i].sp),
                                 fraction = hind / (float)group[i].maxsp;
                    SDL_Surface *surface = skin_sprites[SKIN_SPRITE_GROUP_MANA]->bitmap;

                    box.x = 0;
                    box.y = 0;
                    box.h = surface->h;
                    box.w = MAX(1, MIN((int) (surface->w * fraction), surface->w));
                    sprite_blt(skin_sprites[SKIN_SPRITE_GROUP_MANA],
                                x + GroupPos[i].x + 2, y + GroupPos[i].y + 19,
                                &box, NULL);
                }

                if (group[i].maxgrace)
                {
                    float        hind = MAX(0, group[i].grace),
                                 fraction = hind / (float)group[i].maxgrace;
                    SDL_Surface *surface = skin_sprites[SKIN_SPRITE_GROUP_GRACE]->bitmap;

                    box.x = 0;
                    box.y = 0;
                    box.h = surface->h;
                    box.w = MAX(1, MIN((int) (surface->w * fraction), surface->w));
                    sprite_blt(skin_sprites[SKIN_SPRITE_GROUP_GRACE],
                                x + GroupPos[i].x + 2, y + GroupPos[i].y + 21,
                                &box, NULL);
                }
            }
        }
    }
}

/* Event handlers. */

void wdh_event_below_inv(widget_id_t id, SDL_Event *e)
{
    sint16 mx = e->motion.x - widget_data[id].x1,
           my = e->motion.y - widget_data[id].y1;

    switch (e->type)
    {
        case SDL_MOUSEBUTTONUP:
            if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
            {
                /* KEYFUNC_APPLY and KEYFUNC_DROP works only if
                 * cpl.inventory_win = IWIN_INV. The tag must be placed in
                 * cpl.win_inv_tag. So we do this and after DnD we restore the
                 * old values. */
                inventory_win_t old_win = cpl.inventory_win;
                sint32          old_tag = cpl.win_inv_tag;

                cpl.inventory_win = IWIN_INV;

                /* Drop items to floor. */
                if (draggingInvItem(DRAG_GET_STATUS) != DRAG_QUICKSLOT_SPELL)
                {
                    process_macro_keys(KEYFUNC_DROP, 0);
                }

                cpl.inventory_win = old_win;
                cpl.win_inv_tag = old_tag;
                draggingInvItem(DRAG_NONE);
                itemExamined = 0;
            }

            break;

        case SDL_MOUSEBUTTONDOWN:
            if (mx >= 5 &&
                mx <= widget_data[id].wd - 11 &&
                my >= 19 &&
                my <= widget_data[id].ht - 4)
            {
                cpl.win_below_slot = (mx - 5) / 32;
                cpl.win_below_tag = get_inventory_data(cpl.below,
                                                       &cpl.win_below_ctag,
                                                       &cpl.win_below_slot,
                                                       &cpl.win_below_start,
                                                       &cpl.win_below_count,
                                                       INVITEMBELOWXLEN,
                                                       INVITEMBELOWYLEN);

                if (cpl.win_below_tag >= 0)
                {
                    if (e->button.button == SDL_BUTTON_LEFT)
                    {
                        draggingInvItem(DRAG_IWIN_BELOW);
                    }
                    else
                    {
                        process_macro_keys(KEYFUNC_APPLY, 0);
                    }
                }
            }
            else if (mx >= 263 &&
                     mx <= 268)
            {
                if (my >= 20 &&
                    my <= 29)
                {
                    cpl.win_below_slot = cpl.win_below_slot - INVITEMBELOWXLEN;

                    if (cpl.win_below_slot < 0)
                    {
                        cpl.win_below_slot = 0;
                    }
                }
                else if (my >= 42 &&
                         my <= 51)
                {
                    cpl.win_below_slot = cpl.win_below_slot + INVITEMBELOWXLEN;

                    if (cpl.win_below_slot > cpl.win_below_count - 1)
                    {
                        cpl.win_below_slot = cpl.win_below_count - 1;
                    }
                }

                cpl.win_below_tag = get_inventory_data(cpl.below,
                                                       &cpl.win_below_ctag,
                                                       &cpl.win_below_slot,
                                                       &cpl.win_below_start,
                                                       &cpl.win_below_count,
                                                       INVITEMBELOWXLEN,
                                                       INVITEMBELOWYLEN);
            }

            break;
    }
}

void wdh_event_pinfo(widget_id_t id, SDL_Event *e)
{
    sint16 mx = e->motion.x - widget_data[id].x1,
           my = e->motion.y - widget_data[id].y1;

    if (e->type == SDL_MOUSEBUTTONDOWN &&
        e->button.button == SDL_BUTTON_LEFT)
    {
        if (mx >= 184 &&
            mx <= 210 &&
            my >= 5 &&
            my <= 35)
        {
            client_cmd_generic("/rest");
        }
    }
}

void wdh_event_menu_b(widget_id_t id, SDL_Event *e)
{
    sint16 mx = e->motion.x - widget_data[id].x1,
           my = e->motion.y - widget_data[id].y1;

    if (e->type == SDL_MOUSEBUTTONDOWN &&
        e->button.button == SDL_BUTTON_LEFT)
    {
        if (mx >= 3 &&
            mx <= 44)
        {
            if (my >= 1 &&
                my <= 24) /* spell list */
            {
                check_menu_macros("?M_SPELL_LIST");
            }
            else if (my >= 26 &&
                     my <= 49) /* skill list */
            {
                check_menu_macros("?M_SKILL_LIST");
            }
            else if (my >= 51 &&
                     my <= 74) /* quest list */
            {
                client_cmd_generic("/qlist");
            }
            else if (my >= 76 &&
                     my <= 99) /* online help */
            {
                process_macro_keys(KEYFUNC_HELP, 0);
            }
        }
    }
}

void wdh_event_main_inv(widget_id_t id, SDL_Event *e)
{
    sint16 mx = e->motion.x - widget_data[id].x1,
           my = e->motion.y - widget_data[id].y1;

    switch (e->type)
    {
        case SDL_MOUSEBUTTONUP:
            if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
            {
                /* KEYFUNC_APPLY and KEYFUNC_DROP works only if
                 * cpl.inventory_win = IWIN_INV. The tag must be placed in
                 * cpl.win_inv_tag. So we do this and after DnD we restore the
                 * old values. */
                inventory_win_t old_win = cpl.inventory_win;
                sint32          old_tag = cpl.win_inv_tag;

                cpl.inventory_win = IWIN_INV;

                if (draggingInvItem(DRAG_GET_STATUS) == DRAG_PDOLL)
                {
                    cpl.win_inv_tag = cpl.win_pdoll_tag;
                    process_macro_keys(KEYFUNC_APPLY, 0);
                }

                cpl.inventory_win = old_win;
                cpl.win_inv_tag = old_tag;
            }
            else if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_BELOW)
            {
                cpl.inventory_win = IWIN_BELOW;
                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100);
                process_macro_keys(KEYFUNC_GET, 0);
                cpl.inventory_win = IWIN_INV; // keep inv open.
            }

            draggingInvItem(DRAG_NONE);
            itemExamined = 0;

            break;

        case SDL_MOUSEBUTTONDOWN:
            /* inventory (open / close) */
            if (mx >= 4 &&
                mx <= 22 &&
                my >= 4 &&
                my <= 26)
            {
                if (cpl.inventory_win == IWIN_INV)
                {
                    cpl.inventory_win = IWIN_BELOW;
                }
                else
                {
                    cpl.inventory_win = IWIN_INV;
                }

                break;
            }

            if (mx > 226 &&
                mx < 236)/* scrollbar */
            {
                if (my <= 39 &&
                    my >= 30 &&
                    cpl.win_inv_slot >= INVITEMXLEN)
                {
                    cpl.win_inv_slot -= INVITEMXLEN;
                }
                else if (my >= 116 &&
                         my <= 125)
                {
                    cpl.win_inv_slot += INVITEMXLEN;

                    if (cpl.win_inv_slot > cpl.win_inv_count)
                    {
                        cpl.win_inv_slot = cpl.win_inv_count;
                    }
                }
            }
            else if (mx > 3)
            {
                /* stuff */
                if (e->button.button == SDL_BUTTON_WHEELUP &&
                    cpl.win_inv_slot >= INVITEMXLEN)
                {
                    cpl.win_inv_slot -= INVITEMXLEN;
                }
                else if (e->button.button == SDL_BUTTON_WHEELDOWN)
                {
                    cpl.win_inv_slot += INVITEMXLEN;

                    if (cpl.win_inv_slot > cpl.win_inv_count)
                    {
                        cpl.win_inv_slot = cpl.win_inv_count;
                    }
                }
                else if((e->button.button == SDL_BUTTON_LEFT ||
                         e->button.button == SDL_BUTTON_RIGHT ||
                         e->button.button == SDL_BUTTON_MIDDLE) &&
                        my >= 30 &&
                        my <= 124)
                {
                    cpl.win_inv_slot = (my - 30) / 32 * INVITEMXLEN +
                                       (mx - 3) / 32 + cpl.win_inv_start;
                    cpl.win_inv_tag = get_inventory_data(cpl.ob,
                                                         &cpl.win_inv_ctag,
                                                         &cpl.win_inv_slot,
                                                         &cpl.win_inv_start,
                                                         &cpl.win_inv_count,
                                                         INVITEMXLEN,
                                                         INVITEMYLEN);

                    if (e->button.button == SDL_BUTTON_RIGHT ||
                        e->button.button == SDL_BUTTON_MIDDLE)
                    {
                        process_macro_keys(KEYFUNC_MARK, 0);
                    }
                    else if (cpl.win_inv_tag >= 0)
                    {
                        if (cpl.inventory_win == IWIN_INV)
                        {
                            draggingInvItem(DRAG_IWIN_INV);
                        }
                    }
                }
            }

            break;

        case SDL_MOUSEMOTION:
            /* scrollbar-sliders */
            if (e->button.button == SDL_BUTTON_LEFT &&
                !draggingInvItem(DRAG_GET_STATUS))
            {
                /* IWIN_INV Slider */
                if (cpl.inventory_win == IWIN_INV &&
                    my + 38 &&
                    my + 116 &&
                    mx + 227 &&
                    mx + 236)
                {
                    if (old_mouse_y - e->motion.y > 0)
                    {
                        cpl.win_inv_slot -= INVITEMXLEN;
                    }
                    else if (old_mouse_y - e->motion.y < 0)
                    {
                        cpl.win_inv_slot += INVITEMXLEN;
                    }

                    if (cpl.win_inv_slot > cpl.win_inv_count)
                    {
                        cpl.win_inv_slot = cpl.win_inv_count;
                    }

                    break;
                }
            }
    }
}

void wdh_event_pdoll(widget_id_t id, SDL_Event *e)
{
    inventory_win_t old_win = cpl.inventory_win;
    sint32          old_tag = cpl.win_inv_tag;

    cpl.inventory_win = IWIN_INV;

    if (draggingInvItem(DRAG_GET_STATUS) == DRAG_QUICKSLOT)
    {
        cpl.win_inv_tag = cpl.win_quick_tag;

        if (!(locate_item(cpl.win_inv_tag))->applied)
        {
            process_macro_keys(KEYFUNC_APPLY, 0); /* drop to player-doll */
        }
    }
    else if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_INV)
    {
        if ((locate_item(cpl.win_inv_tag))->applied)
        {
            textwin_show_string(0, skin_prefs.widget_info,
                                "This is applied already!");
        }
        else
        {
            process_macro_keys(KEYFUNC_APPLY, 0);
        }
    }
    else if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_BELOW)
    {
        item *ip;

        cpl.inventory_win = IWIN_BELOW;
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100);
        process_macro_keys(KEYFUNC_GET, 0); /* get to inv */

        /* In case object disappears or auto-applies when picked up. */
        if ((ip = locate_item(cpl.win_inv_tag)) &&
            !ip->applied)
        {
            cpl.inventory_win = IWIN_INV;
            cpl.win_inv_tag = cpl.win_below_tag;
            process_macro_keys(KEYFUNC_APPLY, 0);
        }
    }

    cpl.inventory_win = old_win;
    cpl.win_inv_tag = old_tag;
    draggingInvItem(DRAG_NONE);
    itemExamined = 0;

    return;
}

void wdh_event_skill_exp(widget_id_t id, SDL_Event *e)
{
    /* TODO: Tidy later. */
   /* make a button for robed's exp-patch */
    int i, ii, j, jj, bFound = 0;

    /* lets find the skill... and setup the shortcuts to the exp values */
    for (ii = 0; ii <= SKILL_LIST_MAX && (!bFound); ii++)
    {
        jj = cpl.skill_g + ii;
        if (jj >= SKILL_LIST_MAX)
            jj -= SKILL_LIST_MAX;

        for (i = 0; i < DIALOG_LIST_ENTRY && (!bFound); i++)
        {
            // First page, we have to be offset (and break before looping)
            if (ii == 0)
            {
                j = cpl.skill_e + i + 1;
                if (j >= DIALOG_LIST_ENTRY)
                    break;
            }
            // Other pages we look through MUST NOT BE OFFSET
            else
                j = i;

            if (j >= DIALOG_LIST_ENTRY)
                j -= DIALOG_LIST_ENTRY;

            /* we have a list entry */
            if (skill_list[jj].entry[j].flag == LIST_ENTRY_KNOWN)
            {
                /* First one we find is the one we want */
                sprintf(cpl.skill_name, "%s", skill_list[jj].entry[j].name);
                cpl.skill_g = jj;
                cpl.skill_e = j;
                bFound = 1;
                break;
            }
        }
    }

    WIDGET_REDRAW(id) = 1;
}

void wdh_event_quickslots(widget_id_t id, SDL_Event *e)
{
//    if (e == SDL_MOUSEBUTTONUP)
//    {
//        if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
//        {
//            int ind = get_quickslot(x, y);
//            if (ind != -1) /* valid slot */
//            {
//                if (draggingInvItem(DRAG_GET_STATUS) == DRAG_QUICKSLOT_SPELL)
//                {
//                    quick_slots[ind].shared.is_spell = 1;
//                    quick_slots[ind].spell.groupNr = quick_slots[cpl.win_quick_tag].spell.groupNr;
//                    quick_slots[ind].spell.classNr = quick_slots[cpl.win_quick_tag].spell.classNr;
//                    quick_slots[ind].shared.tag = quick_slots[cpl.win_quick_tag].spell.spellNr;
//                    quick_slots[cpl.win_quick_tag].shared.tag = -1;
//                    cpl.win_quick_tag = -1;
//                }
//                else
//                {
//                    if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_INV)
//                        cpl.win_quick_tag = cpl.win_inv_tag;
//                    else if (draggingInvItem(DRAG_GET_STATUS) == DRAG_PDOLL)
//                        cpl.win_quick_tag = cpl.win_pdoll_tag;
//                    quick_slots[ind].shared.tag = cpl.win_quick_tag;
//                    quick_slots[ind].item.invSlot = ind;
//                    quick_slots[ind].shared.is_spell = 0;
//                    /* now we do some tests... first, ensure this item can fit */
//                    update_quickslots(-1);
//                    /* now: if this is null, item is *not* in the main inventory
//                                       * of the player - then we can't put it in quickbar!
//                                       * Server will not allow apply of items in containers!
//                                       */
//                    if (!locate_item_from_inv(cpl.ob->inv, cpl.win_quick_tag))
//                    {
//                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, 100);
//                        textwin_show_string(0, skin_prefs.widget_info, "Only items from main inventory allowed in quickbar!");
//                    }
//                    else
//                    {
//                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100); /* no bug - we 'get' it in quickslots */
//                        textwin_show_string(0, skin_prefs.widget_info, "set F%d to %s",
//                                           ind + 1,
//                                           locate_item(cpl.win_quick_tag)->s_name);
//                    }
//                }
//            }
//            draggingInvItem(DRAG_NONE);
//            itemExamined = 0; /* ready for next item */
//        }
//    }
//    else /*Mousedown Event */
//    {
//        /* drag from quickslots */
//        int   ind = get_quickslot(x, y);
//        if (ind != -1 && quick_slots[ind].shared.tag != -1) /* valid slot */
//        {
//            cpl.win_quick_tag = quick_slots[ind].shared.tag;
//            if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
//            {
//                if (quick_slots[ind].shared.is_spell == 1)
//                {
//                    draggingInvItem(DRAG_QUICKSLOT_SPELL);
//                    quick_slots[ind].spell.spellNr = quick_slots[ind].shared.tag;
//                    cpl.win_quick_tag = ind;
//                }
//                else
//                {
//                    draggingInvItem(DRAG_QUICKSLOT);
//                    quick_slots[ind].shared.tag = -1;
//                }
//            }
//            else
//            {
//                inventory_win_t stemp = cpl.inventory_win;
//                int             itemp = cpl.win_inv_tag;
//
//                cpl.inventory_win = IWIN_INV;
//                cpl.win_inv_tag = quick_slots[ind].shared.tag;
//                process_macro_keys(KEYFUNC_APPLY, 0);
//                cpl.inventory_win = stemp;
//                cpl.win_inv_tag = itemp;
//            }
//        }
//        else if (x >= widget_data[id].x1+266
//                 && x <= widget_data[id].x1 + 282
//                 && y >= widget_data[id].y1
//                 && y <= widget_data[id].y1 + 34
//                 && (widget_data[id].ht <= 34))
//        {
//            widget_data[id].wd = 34;
//            widget_data[id].ht = 282;
//            widget_data[id].x1 +=266;
//        }
//        else if (x >= widget_data[id].x1
//                 && x <= widget_data[id].x1 + 34
//                 && y >= widget_data[id].y1
//                 && y <= widget_data[id].y1 + 15
//                 && (widget_data[id].ht > 34))
//        {
//            widget_data[id].wd = 282;
//            widget_data[id].ht = 34;
//            widget_data[id].x1 -=266;
//        }
//    }
//
//    return;
}

void wdh_event_range(widget_id_t id, SDL_Event *e)
{
    if (e->type == SDL_MOUSEBUTTONDOWN)
    {
        switch (e->button.button)
        {
            case SDL_BUTTON_LEFT:
                process_macro_keys(KEYFUNC_RANGE, 0);

                break;

            case SDL_BUTTON_WHEELUP:
                process_macro_keys(KEYFUNC_RANGE, 0);

                break;

            case SDL_BUTTON_WHEELDOWN:
                process_macro_keys(KEYFUNC_RANGE_BACK, 0);

                break;

            default:
                process_macro_keys(KEYFUNC_RANGE_BACK, 0);
        }
    }
    else if (e->type == SDL_MOUSEBUTTONUP)
    {
        sint8  drag = draggingInvItem(DRAG_GET_STATUS);
        sint32 tag = (drag == DRAG_IWIN_INV)
                     ? cpl.win_inv_tag : cpl.win_quick_tag;
        item  *ip;

        switch (drag)
        {
            case DRAG_IWIN_INV:
            case DRAG_QUICKSLOT:
                if ((ip = (tag == -1) ? NULL : locate_item(tag)) &&
                    (ip->itype == TYPE_ARROW ||
                     ip->itype == TYPE_BOW ||
                     ip->itype == TYPE_WAND ||
                     ip->itype == TYPE_ROD ||
                     ip->itype == TYPE_HORN) &&
                    !ip->applied)
                {
                    client_cmd_apply(tag);
                }

                break;

            case DRAG_QUICKSLOT_SPELL:
                fire_mode.spell = &spell_list[quick_slots[tag].spell.groupNr].entry[quick_slots[tag].spell.classNr][quick_slots[tag].spell.spellNr];
                fire_mode.mode = FIRE_MODE_SPELL_ID;

                break;
        }
    }
}

void wdh_event_target(widget_id_t id, SDL_Event *e)
{
    sint16 mx = e->motion.x - widget_data[id].x1,
           my = e->motion.y - widget_data[id].y1;

    if (e->type == SDL_MOUSEBUTTONDOWN &&
        e->button.button == SDL_BUTTON_LEFT)
    {
        /* combat modus */
        if (mx >= 4 &&
            mx <= 37 &&
            my >= 4 &&
            my <= 37)
        {
            check_keys(SDLK_c);
        }
        /* talk button */
        else if (mx >= 224 &&
                 mx <= 258 &&
                 my >= 8 &&
                 my <= 24)
        {
            if (cpl.target_code)
            {
                char buf[6] = "hello";

                client_cmd_guitalk(GUI_NPC_MODE_NPC, buf);
            }
        }
    }
}

void wdh_event_number(widget_id_t id, SDL_Event *e)
{
    sint16 mx = e->motion.x - widget_data[id].x1,
           my = e->motion.y - widget_data[id].y1;

    if (e->type == SDL_MOUSEBUTTONDOWN &&
        e->button.button == SDL_BUTTON_LEFT)
    {
        /* close number input */
        if (InputStringFlag &&
            cpl.input_mode == INPUT_MODE_NUMBER)
        {
            if (mx >= 240 &&
                mx <= 248 &&
                my >= 6 &&
                my <= 16)
            {
                SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
                InputStringFlag = 0;
                InputStringEndFlag = 1;
            }
        }
    }
}

void wdh_event_chatwin(widget_id_t id, SDL_Event *e)
{
    textwin_event(e->type, e, TEXTWIN_CHAT_ID);
}

void wdh_event_msgwin(widget_id_t id, SDL_Event *e)
{
    textwin_event(e->type, e, TEXTWIN_MSG_ID);
}
