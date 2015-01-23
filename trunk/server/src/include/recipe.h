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

#ifndef __RECIPE_H
#define __RECIPE_H

/* 'recipe' and 'recipelist' are used by the alchemy code */
typedef struct recipestruct
{
    const char             *title;      /* distinguishing name of product */
    const char             *arch_name;   /* the archetype of the final product made */
    int                     chance;       /* chance that recipe for this item will appear
                                           * in an alchemical grimore */
    int                     index;    /* an index value derived from formula ingredients */
    int                     transmute;    /* if defined, one of the formula ingredients is
                                          * used as the basis for the product object_t */
    int                     yield;        /*  The maximum number of items produced by the recipe */
    shstr_linked_t            *ingred;    /* comma delimited list of ingredients */
    struct recipestruct    *next;
    const char             *keycode;   /* keycode needed to use the recipe */
} recipe;

typedef struct recipeliststruct
{
    int                         total_chance;
    int                         number;         /* number of recipes in this list */
    struct recipestruct        *items;  /* pointer to first recipe in this list */
    struct recipeliststruct    *next;   /* pointer to next recipe list */
} recipelist;

#endif /* ifndef __RECIPE_H */
