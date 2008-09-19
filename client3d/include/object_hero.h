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

#ifndef ObjectHero_H
#define ObjectHero_H

#include "item.h"

#define INVITEMBELOWXLEN 8
#define INVITEMBELOWYLEN 1
#define INVITEMXLEN 7
#define INVITEMYLEN 3

/**
 ** This singleton class is the final object class.
 ** It handles additional functions like weapon and equipment changes.
 *****************************************************************************/
class ObjectHero
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {MAX_SKILL = 6};
    typedef enum _attacks
    {
        /* We start with the double used attacks - for resist & protection too */
        /* damage type: physical */
        ATNR_PHYSICAL, /* = impact */
        ATNR_SLASH,
        ATNR_CLEAVE,
        ATNR_PIERCE,

        /* damage type: elemental */
        ATNR_FIRE,
        ATNR_COLD,
        ATNR_ELECTRICITY,
        ATNR_POISON,
        ATNR_ACID,
        ATNR_SONIC,

        /* damage type: magical */
        ATNR_FORCE,
        ATNR_PSIONIC,
        ATNR_LIGHT,
        ATNR_SHADOW,
        ATNR_LIFESTEAL,

        /* damage type: sphere */
        ATNR_AETHER,
        ATNR_NETHER,
        ATNR_CHAOS,
        ATNR_DEATH,

        /* damage: type only effect by invulnerable */
        ATNR_WEAPONMAGIC,
        ATNR_GODPOWER,

        /* at this point attack effects starts - only resist maps to it */
        ATNR_DRAIN,
        ATNR_DEPLETION,
        ATNR_CORRUPTION,
        ATNR_COUNTERMAGIC,
        ATNR_CANCELLATION,
        ATNR_CONFUSION,
        ATNR_FEAR,
        ATNR_SLOW,
        ATNR_PARALYZE,
        ATNR_SNARE,

        /* and the real special one here */
        ATNR_INTERNAL,
        NROFATTACKS /* index (= 32 ATM) */
    }_attacks;

    typedef enum rangetype
    {
        range_bottom        = -1,
        range_none          = 0,
        range_bow           = 1,
        range_magic         = 2,
        range_wand          = 3,
        range_rod           = 4,
        range_scroll        = 5,
        range_horn          = 6,
        range_steal         = 7,
        range_size          = 8
    } rangetype;

    typedef struct
    {
        int  Str, Dex, Con, Wis, Cha, Int, Pow;
        int  wc, ac;     /* Weapon Class and Armour Class */
        int  level;
        int  hp;         /* Hit Points. */
        int  maxhp;
        int  sp;         /* Spell points.  Used to cast spells. */
        int  maxsp;      /* Max spell points. */
        int  grace;      /* Spell points.  Used to cast spells. */
        int  maxgrace;       /* Max spell points. */
        int  exp_level;
        int  exp;            /* Experience */
        int  food;       /* How much food in stomach.  0 = starved. */
        int  dam;            /* How much damage this object does when hitting */
        int  speed;      /* Gets converted to a float for display*/
        float weapon_sp;      /* Gets converted to a float for display */
        unsigned int flags;      /* contains fire on/run on flags */
        bool   protection_change; /* Resistant value has changed */
        short  protection[NROFATTACKS];     /* Resistant values */
        short  skill_level[MAX_SKILL];  /* Level and experience totals for */
        int    skill_exp[MAX_SKILL];    /* skills */
    }
    Stats;

    typedef enum _inventory_win
    {
        IWIN_BELOW,
        IWIN_INV
    } _inventory_win;

    int weight_limit;
    int count; // Repeat count on command
    int target_mode;
    int target_code;
    int target_color;
    int inventory_win; // inventory windows
    int menustatus;
    int mark_count;
    int loc;
    int tag;
    int nrof;
    int skill_g; // skill group and entry of ready skill
    int skill_e;
    int warn_hp;
    int win_inv_slot;
    int win_inv_tag;
    int win_quick_tag;
    int win_pdoll_tag;
    int win_inv_start;
    int win_inv_count;
    int win_inv_ctag;
    int win_below_slot;
    int win_below_tag;
    int win_below_start;
    int win_below_count;
    int win_below_ctag;

    int input_mode; // mode: no, console(textstring), numinput
    int nummode;

    float gen_hp; // hp, mana and grace reg.
    float gen_sp;
    float gen_grace;

    bool no_echo; // If TRUE, don't echo keystrokes
    bool fire_on; // True if fire key is pressed = action key (ALT;CTRL)
    bool run_on; // True if run key is on = action key (ALT;CTRL)
    bool resize_twin;
    bool resize_twin_marker;
    bool firekey_on; // True if fire key is pressed = permanent mode
    bool runkey_on; // sic!
    bool echo_bindings; // If true, echo the command that the key
    bool warn_statdown;
    bool warn_statup;
    bool warn_drain;

    int window_weight;
    int real_weight;

    unsigned short count_left; // count for commands
    unsigned short mmapx, mmapy; // size of magic map
    unsigned short pmapx, pmapy; // Where the player is on the magic map
    unsigned short mapxres, mapyres;// resolution to draw on the magic map

    Stats stats; // Player stats
//    Input_State input_state; // What the input state is
    rangetype shoottype; // What type of range attack player has

    unsigned char *magicmap; // Magic map data
    unsigned char showmagic; // If 0, show normal map, otherwise, show magic map.

    unsigned char command_window; // How many outstanding commands to allow
    unsigned char ready_spell; // Index to spell that is readied
// player knows
    unsigned char map_x, map_y; // These are offset values. See object.c

    char target_hp; // hp of our target in %
    std::string last_command; // Last command entered
    std::string input_text; // keys typed (for long commands)
    std::string spells[255]; // List of all the spells the
    std::string target_name; // Rank & Name of char
    std::string num_text;
    std::string skill_name;
    std::string rankandname;
    std::string pname; // Name of char
    std::string title; // Race & Profession of character
    std::string rank; // rank
    std::string race; // alignment
    std::string godname; // alignment
    std::string alignment; // alignment
    std::string gender; // Gender
    std::string range; // Range attack chosen
    std::string player_reply;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static ObjectHero &getSingleton()
    {
        static ObjectHero Singleton; return Singleton;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectHero(const ObjectHero&); // disable copy-constructor.
    ObjectHero();
    ~ObjectHero()
    {}
};

#endif
